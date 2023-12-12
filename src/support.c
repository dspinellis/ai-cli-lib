/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Safe memory allocation and other support functions.
 *  The allocation functions exit the program with an error messge
 *  if allocation fails.
 *
 *  Copyright 2023 Diomidis Spinellis
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <dlfcn.h>
#include <jansson.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "support.h"

static FILE *logfile;
CURL *curl;

// Exit with the specified formatted error message
void
errorf(const char *format, ...)
{
	fprintf(stderr, "ai_cli: ");
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

// Exit with a failure if result_ok is false
static void
verify(bool result_ok)
{
	if (!result_ok)
		errorf("memory allocation failed.");
}

void *
safe_malloc(size_t size)
{
	void *p = malloc(size);
	verify(p != NULL);
	return p;
}

void *
safe_calloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);
	verify(p != NULL);
	return p;
}

void *
safe_realloc(void *ptr, size_t size)
{
	void *p = realloc(ptr, size);
	verify(p != NULL);
	return p;
}

char *
safe_strdup(const char *s)
{
	void *p = strdup(s);
	verify(p != NULL);
	return p;
}

// Return a dnamically allocated string in the range [begin, end)
char *
range_strdup(const char *begin, const char *end)
{
	void *p = strndup(begin, end - begin);
	verify(p != NULL);
	return p;
}

int
safe_asprintf(char **strp, const char *fmt, ...)
{
	int result;
	va_list args;

	va_start(args, fmt);
	result = vasprintf(strp, fmt, args);
	va_end(args);

	verify(result != -1);
	return result;
}

// Return the cardinal number in string or -1 on error
int
strtocard(const char *string)
{
	if (*string == '\0')
		return -1;

	char *endptr;
	errno = 0;
	long value = strtol(string, &endptr, 10);

	if (errno != 0 || *endptr != '\0' || value < 0)
		return -1;
	return (int)value;
}

// Initialize s as the specified string
void
string_init(string_t *s, const char *value)
{

	s->len = strlen(value);
	s->ptr = safe_malloc(s->len + 1);
	strcpy(s->ptr, value);
}

// Write result data into string s
size_t
string_write(void *data, size_t size, size_t nmemb, string_t *s)
{
	size_t bytes = size * nmemb;
	size_t new_len = s->len + bytes;
	s->ptr = safe_realloc(s->ptr, new_len + 1);
	memcpy(s->ptr + s->len, data, bytes);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return bytes;
}

// Append specified data to the string
size_t
string_append(string_t *s, const char *data)
{
	size_t bytes = strlen(data);
	size_t new_len = s->len + bytes;
	s->ptr = safe_realloc(s->ptr, new_len + 1);
	memcpy(s->ptr + s->len, data, bytes);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return bytes;
}

// Append printf(3)-style formatted output to string
int
string_appendf(string_t *s, const char *fmt, ...)
{
	int result;
	va_list args;

	va_start(args, fmt);
	char *strp;
	result = vasprintf(&strp, fmt, args);
	va_end(args);

	verify(result != -1);

	string_append(s, strp);
	free(strp);

	return result;
}

// Return the short name of the program being used
const char *
short_program_name(void)
{
#ifdef MACOS
	return getprogname();
#else
	// GNU libc-specific; defined in errno.h
	return program_invocation_short_name;
#endif
}

// Show a message during readline processing
int
readline_printf(const char *fmt, ...)
{
	int result;
	va_list args;

	va_start(args, fmt);
	rl_save_prompt();
	result = vprintf(fmt, args);
	va_end(args);
	rl_restore_prompt();
	rl_on_new_line();
	rl_redisplay();

	return result;
}

/*
 * Return the string suitably escaped for JSON.
 * Each new call frees the previously allocated values.
 */
char *
json_escape(const char *s)
{
	static json_t *string;
	static char *result;

	if (string)
		json_decref(string);
	if (result)
		free(result);

	string = json_string(s);
	result = json_dumps(string, JSON_ENCODE_ANY);
	return result;
}

// Output an ISO timestamp (with microseconds) to the specified file
void
timestamp(FILE *f)
{
	struct timeval tv;
	char buffer[30];
	struct tm *tm_info;

	gettimeofday(&tv, NULL);
	tm_info = localtime(&tv.tv_sec);

	strftime(buffer, 26, "%Y-%m-%dT%H:%M:%S", tm_info);
	fprintf(f, "{ \"timestamp\": \"%s.%06ld\" }\n", buffer, (long)tv.tv_usec);
}

/*
 * Initialize curl connections
 * Return 0 on success -1 on error
 */
int
curl_initialize(config_t *config)
{
/*
 * Under Linux link at runtime (late binding) to minimize linking cost
 * (binding will only be performed by programs that use readline)
 * and to avoid crashes associated with seccomp filtering.
 *
 * Under Cygwin link at compile time, because late binding isn't supported.
 */
#if !defined(__CYGWIN__)
	if (!dlopen("libcurl." DLL_EXTENSION, RTLD_NOW | RTLD_GLOBAL)) {
		readline_printf("\nError loading libcurl: %s\n", dlerror());
		return -1;
	}
	if (!dlopen("libjansson." DLL_EXTENSION, RTLD_NOW | RTLD_GLOBAL)) {
		readline_printf("\nError loading libjansson: %s\n", dlerror());
		return -1;
	}
#endif

	if (config->general_logfile)
		logfile = fopen(config->general_logfile, "a");

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (!curl)
		readline_printf("\nCURL initialization failed.\n");
	return curl ? 0 : -1;
}

// Write the specified string to the logfile, if enabled
void
write_log(config_t *config, const char *message)
{
	if (!logfile)
		return;
	if (config->general_timestamp)
		timestamp(logfile);
	fputs(message, logfile);
	fflush(logfile);
}
