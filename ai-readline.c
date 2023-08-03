/*-
 *
 *  ai-readline - readline wrapper to obtain a generative AI suggestion
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
#include <string.h>
#include <dlfcn.h>
#include <readline/history.h>

typedef char *(*readline_t)(const char *prompt);
static readline_t real_readline;


// Prompt AI to generate the input
static const char ai_prompt[] = "`ai ";

char *
readline(const char *input_prompt)
{
	if (!real_readline) {
		real_readline = dlsym(RTLD_NEXT, "readline");
	}

	char *typed = real_readline(input_prompt);
	if (!typed)
		return typed;

	if (memcmp(typed, ai_prompt, sizeof(ai_prompt) - 1) != 0)
		return typed;

	const char *user_prompt = typed + sizeof(ai_prompt) - 1;
	// Invoke AI API and get its response
	char buff[1024];
	snprintf(buff, sizeof(buff), "The AI answer to \"%s\" is 42", user_prompt);
	char *ai_response = strdup(buff);
	add_history(typed);
	free(typed);
	return ai_response;
}

__attribute__((constructor)) static void setup(void)
{
	fprintf(stderr, "Use \"%s your-prompt\" to have AI generate the input for you.\n", ai_prompt);
}
