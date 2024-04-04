/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Safe memory allocation and other functions.
 *  The allocation functions exit the program with an error messge
 *  if allocation fails.
 *
 *  Copyright 2023-2024 Diomidis Spinellis
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

#include <curl/curl.h>
#include <stdio.h>

#include "config.h"

extern CURL *acl_curl;

int acl_safe_asprintf(char **strp, const char *fmt, ...);
char *acl_safe_strdup(const char *s);
char *acl_range_strdup(const char *begin, const char *end);
const char *acl_short_program_name(void);

int acl_strtocard(const char *string);

// Extendable string
typedef struct string {
    char *ptr;
    size_t len;
} string_t;


char *acl_json_escape(const char *s);
int acl_readline_printf(const char *fmt, ...);
void acl_string_init(string_t *s, const char *value);
size_t acl_string_write(void *data, size_t size, size_t nmemb, string_t *s);
size_t acl_string_append(string_t *s, const char *data);
int acl_string_appendf(string_t *s, const char *fmt, ...);
int curl_initialize(config_t *config);
void acl_write_log(config_t *config, const char *message);
void acl_errorf(const char *format, ...);
