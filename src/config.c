/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Configuration parsing and access
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

#include "config.h"
#include "support.h"
#include "ini.h"


const char hidden_config_name[] = ".aicliconfig";

/*
 * Prefixes for providing n-shot user and assistant prompts in ini files.
 * Example:
 * [prompt-gdb]
 * user-1 = Disable breakpoint number 4
 * assistant-1 = delete 4
 */
const char prompt_ini_prefix[] = "prompt-";
const char user_ini_prefix[] = "user-";
const char assistant_ini_prefix[] = "assistant-";

/*
 * Prefixes for providing n-shot user and assistant prompts in
 * environment variables.  Examples:
 * AI_CLI_prompt_gdb_user_1=Disable breakpoint number 4
 * AI_CLI_prompt_gdb_assistant_1=delete 4
 */
const char env_prompt_prefix[] = "AI_CLI_prompt_";
const char user_env_prefix[] = "user_";
const char assistant_env_prefix[] = "assistant_";

// Return true if the specified string starts with the given prefix
STATIC bool
starts_with(const char *string, const char *prefix)
{
	return memcmp(string, prefix, strlen(prefix)) == 0;
}

/*
 * Return the identifier associated with a prompt environment variable
 * (e.g. sqlite3 for AI_CLI_prompt_sqlite3_system)
 * in dynamically allocated memory.
 * Return null if the variable does not contain an identifier.
 */
STATIC char *
prompt_id(const char *entry)
{
	// prompt_name is something like {id}_system
	const char *id_begin = entry + sizeof(env_prompt_prefix) - 1;
	const char *id_end = strchr(id_begin, '_');
	if (!id_end)
		return NULL;
	return range_strdup(id_begin, id_end);
}


/*
 * Return a pointer to the prompt details for program being executes
 * program, or NULL if no such details are recorded.
 */
uaprompt_t
prompt_find(config_t *config, const char *program_name)
{
	for (uaprompt_t p = config->shots; p; p = p->next)
		if (strcmp(p->program, program_name) == 0)
			return p;
	return NULL;
}

/*
 * Return a pointer to the newly added prompt details for the specified
 * program.
 */
uaprompt_t
prompt_add(config_t *config, const char *program_name)
{
	uaprompt_t p = safe_calloc(1, sizeof(*p));
	p->program = safe_strdup(program_name);
	p->next = config->shots;
	config->shots = p;
	return p;
}

/*
 * Return the 0-based prompt ordinal number n associated with the specified
 * prefixed name.  The name string is suffixed with -n.
 * Return -1 if the string doesn't exactly match a positive integer
 * up to the number of allowed prompts.
 */
STATIC int
prompt_number(const char *name, const char *prompt_prefix)
{
	int result = strtocard(name + strlen(prompt_prefix));
	return result <= 0 || result > NPROMPTS ? -1 : result - 1;
}

// Return true if the string contains the value true
bool
strtobool(const char *string)
{
	return strcmp(string, "true") == 0;
}

/*
 * Set configuration values from environment variables or to the
 * given configuration file value.
 * If section is NULL, then set all (non program-specific) configuration
 * variables from any environment variables corresponding to them
 * (AI_CLI_section_name).  In this case the section, name, and value parameters
 * are not used.
 * If section is not NULL, set the configuration value from
 * the specified section, name, and value.
 */
static int
fixed_matcher(config_t *pconfig, const char* section,
    const char* name, const char* value)
{
	/*
	 * Set the specified section, s, and name, n, from the corresponding
	 * environment variable or to the given configuration file value
	 * available through the variable named value.
	 * The configuration or environment string value is converted to the
	 * reqired type using the supplied function fn.
	 */
	#define MATCH(s, n, fn) do { \
		if (section) { \
			if (strcmp(section, #s) == 0 && strcmp(name, #n) == 0) { \
				pconfig->s ## _ ## n = fn(value); \
				pconfig->s ## _ ## n ## _set = true; \
				return 1; \
			} \
		} else { \
			const char *env = getenv("AI_CLI_" #s "_" #n); \
			if (env) { \
				pconfig->s ## _ ## n = fn(env); \
				pconfig->s ## _ ## n ## _set = true; \
			} \
		} \
	} while(0);

	MATCH(general, verbose, strtobool);
	MATCH(general, logfile, safe_strdup);
	MATCH(general, timestamp, strtobool);
	MATCH(general, api, safe_strdup);

	MATCH(openai, endpoint, safe_strdup);
	MATCH(openai, key, safe_strdup);
	MATCH(openai, model, safe_strdup);
	MATCH(openai, temperature, atof);

	MATCH(binding, vi, safe_strdup);
	MATCH(binding, emacs, safe_strdup);

	MATCH(prompt, context, strtocard);
	MATCH(prompt, system, safe_strdup);

	MATCH(llamacpp, endpoint, safe_strdup);
	MATCH(llamacpp, temperature, atof);
	MATCH(llamacpp, top_k, atoi);
	MATCH(llamacpp, top_p, atof);
	MATCH(llamacpp, n_predict, atoi);
	MATCH(llamacpp, n_keep, atoi);
	MATCH(llamacpp, tfs_z, atof);
	MATCH(llamacpp, typical_p, atof);
	MATCH(llamacpp, repeat_penalty, atof);
	MATCH(llamacpp, repeat_last_n, atoi);
	MATCH(llamacpp, penalize_nl, strtobool);
	MATCH(llamacpp, presence_penalty, atof);
	MATCH(llamacpp, frequency_penalty, atof);
	MATCH(llamacpp, mirostat, atoi);
	MATCH(llamacpp, mirostat_tau, atof);
	MATCH(llamacpp, mirostat_eta, atof);
	MATCH(llamacpp, seed, atoi);

	return 0;
}

/*
 * Callback for the configuration .ini file reader
 * Called with the section, name, and value read
 */
static int
config_handler(void* user, const char* section, const char* name,
    const char* value)
{
	config_t *pconfig = (config_t *)user;

	if (pconfig->general_verbose)
		fprintf(stderr, "Config [%s]: %s=%s\n", section, name, value);

	if (fixed_matcher(pconfig, section, name, value))
		return 1;

	if (!starts_with(section, prompt_ini_prefix))
		return 0;  /* unknown section/name, error */

	/*
	 * A user or assistant n-short prompt, such as:
	 * [prompt-gdb]
	 * user-1 = Disable breakpoint number 4
	 * or a program-specific system prompt, such as:
	 * [prompt-bc]
	 * system = The bc command is already invoked
	 */
	const char *program_name = section + sizeof(prompt_ini_prefix) - 1;
	uaprompt_t prompt = prompt_find(pconfig, program_name);
	if (!prompt)
		prompt = prompt_add(pconfig, program_name);

	if (strcmp(name, "system") == 0) {
		prompt->system = strdup(value);
		return 1;
	}

	if (starts_with(name, user_ini_prefix)) {
		int n = prompt_number(name, user_ini_prefix);
		if (n == -1)
			return 0; // Bad number error
		prompt->user[n] = strdup(value);
		return 1;
	} else if (starts_with(name, assistant_ini_prefix)) {
		int n = prompt_number(name, assistant_ini_prefix);
		if (n == -1)
			return 0; // Bad number error
		prompt->assistant[n] = strdup(value);
		return 1;
	}
	return 0;  /* unknown section/name, error */
}

/*
 * Override or set configuration values based on set environment variables
 * of the form AI_CLI_section_name.
 * Prompt settings are specified as AI_CLI_prompt_id, where id can be
 * a program name.
 * Alternatively the program name can be specified through the
 * AI_CLI_prompt_{id}_name variable.
 * User and assistant prompts are specified as AI_CLI_prompt_{id}_user_{n}
 * and AI_CLI_prompt_{id}_assistant_{n}.
 * The system prompt is specified as AI_CLI_prompt_{id}_system.
 */
static void
env_override(config_t *config)
{
	// Match all fixed configuration values
	fixed_matcher(config, NULL, NULL, NULL);

	/*
	 * Now look for prompt configurations in environment variables.
	 * A user or assistant n-short prompt, such as:
	 * AI_CLI_prompt_gdb_user_1=Disable breakpoint number 4
	 * or a program-specific system prompt, such as:
	 * AI_CLI_prompt_bc_system=The bc command is already invoked
	 */
	char **env;
	extern char **environ;
	for (env = environ; *env; env++) {
		char *entry = *env;
		if (!starts_with(entry, env_prompt_prefix))
			continue;
		// E.g. sqlite3 or gitconfig (which will be named git-config)
		char *program_name = prompt_id(entry);
		if (!program_name)
			errorf("Missing program identifier in prompt environment variable %s", entry);
		char *prompt_name_begin = entry + sizeof(env_prompt_prefix) +
			strlen(program_name);
		const char *prompt_name_end = strchr(prompt_name_begin, '=');
		if (!prompt_name_end)
			errorf("Missing value in prompt environment variable %s", entry);
		char *prompt_name = range_strdup(prompt_name_begin, prompt_name_end);
		const char *prompt_value = prompt_name_end + 1;

		uaprompt_t prompt = prompt_find(config, program_name);
		if (!prompt)
			prompt = prompt_add(config, program_name);

		if (strcmp(prompt_name, "system") == 0) {
			prompt->system = strdup(prompt_value);
		} else if (starts_with(prompt_name, user_env_prefix)) {
			int n = prompt_number(prompt_name, user_env_prefix);
			if (n == -1)
				errorf("Invalid prompt value in environment variable %s", entry);
			else
				prompt->user[n] = strdup(prompt_value);
		} else if (starts_with(prompt_name, assistant_env_prefix)) {
			int n = prompt_number(prompt_name, assistant_env_prefix);
			if (n == -1)
				errorf("Invalid prompt value in environment variable %s", entry);
			else
				prompt->assistant[n] = strdup(prompt_value);
		}
		free(program_name);
		free(prompt_name);
	}
}

static void
ini_checked_parse(const char* filename, ini_handler handler, config_t *config)
{
	if (config->general_verbose)
		fprintf(stderr, "Config reading %s\n", filename);
	int val = ini_parse(filename, handler, config);
	// When unable to open file val is -1, which we ignore
	if (val > 0)
		errorf("%s:%d:1: Initialization file error", filename, val);
}

/*
 * Read the configuration file from diverse directories into config.
 */
void
read_config(config_t *config)
{
	config->program_name = short_program_name();

	ini_checked_parse("/usr/share/ai-cli/config", config_handler, config);
	ini_checked_parse("/usr/local/share/ai-cli/config", config_handler, config);
	ini_checked_parse("ai-cli-config", config_handler, config);

	// $HOME/.aicliconfig
	char *home_dir;
	if ((home_dir = getenv("HOME")) != NULL) {
		char *home_config;

		safe_asprintf(&home_config, "%s/%s", home_dir, "share/ai-cli/config");
		ini_checked_parse(home_config, config_handler, config);
		free(home_config);

		safe_asprintf(&home_config, "%s/%s", home_dir, hidden_config_name);
		ini_checked_parse(home_config, config_handler, config);
		free(home_config);
	}

	// .aicliconfig
	ini_checked_parse(hidden_config_name, config_handler, config);
	env_override(config);
}

/*
 * Read the configuration file from the specified file path into config.
 * This allows testing the config handler.
 */
void
read_file_config(config_t *config, const char *file_path)
{
	// Allow unit tests to override this value
	if (!config->program_name)
		config->program_name = short_program_name();

	ini_checked_parse(file_path, config_handler, config);
	env_override(config);
}

/*
 * Return the system role prompt string in dynamically allocated memory.
 */
char *
system_role_get(config_t *config)
{
	char *system_role;
	safe_asprintf(&system_role, config->prompt_system, config->program_name);
	return system_role;
}

/*
 * Set the name of the program executing the library.
 * Provided for testing.
 */
void
set_program_name(config_t *config, const char *name)
{
	config->program_name = name;
}
