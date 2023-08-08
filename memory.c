/*-
 *
 *  ai-readline - readline wrapper to obtain a generative AI suggestion
 *  Safe memory allocation functions.
 *  These functions exit the program with an error messge if allocation
 *  fails.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>


// Exit with a failure if result_ok is false
static void
verify(bool result_ok)
{
	if (!result_ok) {
		fprintf(stderr, "ai_readline: memory allocation failed.\n");
		exit(EXIT_FAILURE);
	}
}

void *
safe_malloc(size_t size)
{
	void *p = malloc(size);
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
