/*-
 *
 *  ai_cli - readline wrapper to obtain a generative AI suggestion
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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "config.h"
#include "support.h"

#include "fetch_anthropic.h"
#include "fetch_hal.h"
#include "fetch_llamacpp.h"
#include "fetch_openai.h"

/*
 * Dynamically obtained pointer to readline(3) variables..
 * This avoids a undefined symbol errors when a program isn't linked with
 * readline.
 */
static char **rl_line_buffer_ptr;
static int *rl_end_ptr;
static int *rl_point_ptr;
static Keymap vi_movement_keymap_ptr;
static int *history_length_ptr;

// Loaded configuration
static config_t config;

// API fetch function, e.g. fetch_openai or fetch_llamacpp

char * (*fetch)(config_t *config, const char *prompt, int history_length);

/*
 * Add the specified prompt to the RL history, as a comment if the
 * comment prefix is defined.
 */
static void
add_prompt_to_history(const char *prompt)
{
	if (prompt == NULL)
		return;

	if (!config.prompt_comment_set) {
		add_history(prompt);
		return;
	}

	char *commented_prompt;
	safe_asprintf(&commented_prompt, "%s %s", config.prompt_comment,
	    prompt);
	add_history(commented_prompt);
	free(commented_prompt);
}

/*
 * The user has has asked for AI to be queried on the typed text
 * Replace the user's text with the queried on
 */
static int
query_ai(int count, int key)
{
	static char *prev_response;

	if (prev_response) {
		free(prev_response);
		prev_response = NULL;
	}

	add_prompt_to_history(*rl_line_buffer_ptr);
	char *response = fetch(&config, *rl_line_buffer_ptr,
	    *history_length_ptr);
	if (!response)
		return -1;
	rl_crlf();
	rl_on_new_line();
	rl_delete_text(0, *rl_end_ptr);
	*rl_point_ptr = 0;
	if (config.general_response_prefix_set) {
		rl_insert_text(config.general_response_prefix);
		rl_insert_text(" ");
	}
	rl_insert_text(response);
	prev_response = response;
	/*
	 * The readline_internal_teardown() function will restore
	 * the original history line iff the line being edited
	 * was originally in the history, AND the line has changed.
	 * Avoid this by clearing the undo list.
	 * This results in the commented prompt rather than the
	 * ucommented prompt being stored in the history.
	 */
	rl_free_undo_list();
	return 0;
}


/*
 * This is called when the dynamic library is loaded.
 * If the program is linked with readline(3),
 * read configuration and set keybindings for AI completion.
 */
#ifndef UNIT_TEST
__attribute__((constructor)) static
#endif
void
setup(void)
{
	/*
	 * See if readline(3) is linked and obtain required symbols
	 * This avoids undefined symbol errors for programs not
	 * using readline and also the initialization overhead.
	 */
	dlerror();
	rl_line_buffer_ptr = dlsym(RTLD_DEFAULT, "rl_line_buffer");
	if (dlerror())
		return; // Program not linked with readline(3)


	// Obtain remaining variable symbols
	rl_end_ptr = dlsym(RTLD_DEFAULT, "rl_end");
	rl_point_ptr = dlsym(RTLD_DEFAULT, "rl_point");
	vi_movement_keymap_ptr = dlsym(RTLD_DEFAULT, "vi_movement_keymap");
	history_length_ptr = dlsym(RTLD_DEFAULT, "history_length");

	read_config(&config);

	if (!config.prompt_system) {
		fprintf(stderr, "No default ai-cli configuration loaded.  Installation problem?\n");
		return;
	}

// Require a given configuration value
#define REQUIRE(section, value) do { \
	if (!config.section ## _ ## value ## _set) { \
		fprintf(stderr, "Missing %s value in [%s] configuration section.\n", #value, #section); \
		return; \
	} \
} while (0);

	REQUIRE(general, api);

	if (strcmp(config.general_api, "openai") == 0) {
		fetch = fetch_openai;
		REQUIRE(openai, key);
		REQUIRE(openai, endpoint);
	} else if (strcmp(config.general_api, "anthropic") == 0) {
		fetch = fetch_anthropic;
		REQUIRE(anthropic, key);
		REQUIRE(anthropic, endpoint);
		REQUIRE(anthropic, version);
	} else if (strcmp(config.general_api, "hal") == 0) {
		fetch = fetch_hal;
	} else if (strcmp(config.general_api, "llamacpp") == 0) {
		fetch = fetch_llamacpp;
		REQUIRE(llamacpp, endpoint);
	} else {
		fprintf(stderr, "Unsupported API: [%s].\n", config.general_api);
		return;
	}
	if (config.general_verbose)
		fprintf(stderr, "API set to %s\n", config.general_api);

	// Add named function, making it available to the user
	rl_add_defun("query-ai", query_ai, -1);

	/*
	 * Bind the function to Emacs and vi.
	 * Programs linked with libedit (editline) lack rl_bind_keyseq and
	 * will fail if it's executed. To avoid this, first test if the
	 * function i* s available.
	 */
	if (config.binding_emacs && dlsym(RTLD_DEFAULT, "rl_bind_keyseq"))
		rl_bind_keyseq(config.binding_emacs, query_ai);
	if (config.binding_vi)
		rl_bind_key_in_map(*config.binding_vi, query_ai, vi_movement_keymap_ptr);
}
