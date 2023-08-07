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

#include "ini.h"

const char config_name[] = ".airlconfig";

typedef struct {
	const char *api_endpoint;
	const char *api_key;
	// Number of past prompts to provide as context
	int prompt_context;
} config_t;

typedef char *(*readline_t)(const char *prompt);

/*
 * The user has has asked for AI to be queried on the typed text
 * Replace the user's text with the queried on
 */
static int
query_ai(int count, int key)
{
	char buff[1024];
	// add_history(rl_line_buffer);
	// snprintf(buff, sizeof(buff), "The AI answer to \"%s\" is 42", rl_line_buffer);
	snprintf(buff, sizeof(buff), "The AI answer to \"%s\" is 42", "foo");
	rl_begin_undo_group();
	rl_delete_text(0, rl_end);
	rl_point = 0;
	rl_insert_text(buff);
	rl_end_undo_group();
}

static bool
is_emacs_mode(void)
{
	char *editing_mode = rl_get_keymap_name(rl_get_keymap());
	return strcmp(editing_mode, "emacs") == 0;
}

static int
config_handler(void* user, const char* section, const char* name,
    const char* value)
{
	config_t *pconfig = (config_t *)user;

	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("prompt", "context")) {
		pconfig->prompt_context = atoi(value);
	} else if (MATCH("API", "endpoint")) {
		pconfig->api_endpoint = strdup(value);
	} else if (MATCH("API", "key")) {
		pconfig->api_key = strdup(value);
	} else {
		return 0;  /* unknown section/name, error */
	}
	return 1;
}

/*
 * Read the configration file from the current directory or $HOME into config.
 * Return true if the file could be read, otherwise return false.
 */
static bool
read_config(config_t *config)
{

	// Read from current directory
	if (ini_parse(config_name, config_handler, config) == 0)
		return true;

	// Read fom the home directory
	char *home_dir;
	if ((home_dir = getenv("HOME")) == NULL)
		return false;

	char *home_config;
	asprintf(&home_config, "%s/%s", home_dir, config_name);
	bool ret = ini_parse(home_config, config_handler, &config) == 0;
	free(home_config);
	return ret;
}

// Initialize hook and key bindigs
static void
initialize(void)
{
	static config_t config;

	if (!read_config(&config)) {
		fprintf(stderr, "Can't load '.airlconfig'.\n");
		return;
	}

	if (!config.api_key) {
		fprintf(stderr, "No API key set in .airlconfig.\n");
		return;
	}

	// Read any configuration from .inputrc
	rl_initialize();

	// Add named function, making it available to the user
	rl_add_defun("query-ai", query_ai, -1);

	// Bind it to ^Xa (Emacs) or V (vi)
	int ret;
	char *binding;
	if (is_emacs_mode()) {
		binding = "\\C-xa";
		ret = rl_bind_keyseq(binding, query_ai);
	} else {
		binding = "V";
		ret = rl_bind_key_in_map(*binding, query_ai, vi_movement_keymap);
	}
	if (ret == 0)
		fprintf(stderr, "AI completion bound to [%s]\n", binding);
	else
		fprintf(stderr, "Unable to bind readline key for AI completion.\n");
}

__attribute__((constructor)) static void setup(void)
{
	initialize();
}
