/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Safe memory allocation and other functions.
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

// Used for declarations that should be static but aren't for unit testing
#define STATIC

int safe_asprintf(char **strp, const char *fmt, ...);
void *safe_calloc(size_t nmemb, size_t size);
void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);
char *safe_strdup(const char *s);
const char *short_program_name(void);

int strtocard(const char *string);

// Extendable string
typedef struct string {
    char *ptr;
    size_t len;
} string_t;

int readline_printf(const char *fmt, ...);
void string_init(string_t *s, const char *value);
size_t string_write(void *data, size_t size, size_t nmemb, string_t *s);
size_t string_append(string_t *s, const char *data);
int string_appendf(string_t *s, const char *fmt, ...);
