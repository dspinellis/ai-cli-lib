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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "config.h"

typedef char *(*readline_t)(const char *prompt);

/*
 * Dynamically obtained pointer to readline(3) variables..
 * This avoids an undefined symbol errors when a program isn't linked with
 * readline.
 */
static char **rl_line_buffer_ptr;
static int *rl_end_ptr;
static int *rl_point_ptr;
static Keymap vi_movement_keymap_ptr;

/*
 * The user has has asked for AI to be queried on the typed text
 * Replace the user's text with the queried on
 */
static int
query_ai(int count, int key)
{
	char buff[1024];
	add_history(*rl_line_buffer_ptr);
	snprintf(buff, sizeof(buff), "The AI answer to \"%s\" is 42", *rl_line_buffer_ptr);
	rl_begin_undo_group();
	rl_delete_text(0, *rl_end_ptr);
	*rl_point_ptr = 0;
	rl_insert_text(buff);
	rl_end_undo_group();
	return 0;
}

// Initialize hook and key bindigs
static void
initialize(void)
{
	/*
	 * See if readline library is linked and obtain required symbols
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

	static config_t config;

	read_config(&config);

	if (!config.prompt_system) {
		fprintf(stderr, "No default ai-readline configuration loaded.  Installation problem?\n");
		return;
	}

	if (!config.api_key) {
		fprintf(stderr, "No API key configured. Please obtain and configure an API key.\n");
		return;
	}

	// Add named function, making it available to the user
	rl_add_defun("query-ai", query_ai, -1);

	// Bind it to (Emacs and vi
	if (config.binding_emacs)
		rl_bind_keyseq(config.binding_emacs, query_ai);
	if (config.binding_vi)
		rl_bind_key_in_map(*config.binding_vi, query_ai, vi_movement_keymap_ptr);
}

__attribute__((constructor)) static void setup(void)
{
	initialize();
}
