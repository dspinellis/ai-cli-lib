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
 * Prefixes for providing n-shot user and assistant prompts
 * Example:
 * [prompt-gdb]
 * user-1 = Disable breakpoint number 4
 * assistant-1 = delete 4
 */
const char prompt_prefix[] = "prompt-";
const char user_prefix[] = "user-";
const char assistant_prefix[] = "assistant-";

// Return true if the specified string starts with the given prefix
STATIC bool
starts_with(const char *string, const char *prefix)
{
	return memcmp(string, prefix, strlen(prefix)) == 0;
}


/*
 * Return a pointer to the prompt details for the specified
 * program, or NULL if no such details are recorded.
 */
uaprompt_t
prompt_find(config_t * config, const char *program_name)
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
prompt_add(config_t * config, const char *program_name)
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


// Process each line of the config
//    [section]
//    name = value
// puts result in user
//
// returns 1 if success 0 otherwise
static int
config_handler(void* user, const char* section, const char* name,
    const char* value)
{
	config_t *pconfig = (config_t *)user;

	if (pconfig->general_verbose)
		fprintf(stderr, "Config [%s]: %s=%s\n", section, name, value);

	/*
	 * Set the specified section, s, and name, n, from the corresponding
	 * environment variable or to the given configuration file value
	 * available through the variable named value.
	 * The configuration or environment string value is converted to the
	 * reqired type using the supplied function fn.
	 */
	#define MATCH(s, n, fn) do { \
		const char *env = getenv("AI_CLI_" #s "_" #n); \
		if (env) { \
			pconfig->s ## _ ## n = fn(env); \
			pconfig->s ## _ ## n ## _set = true; \
			return 1; \
		} \
		if (strcmp(section, #s) == 0 && strcmp(name, #n) == 0) { \
			pconfig->s ## _ ## n = fn(value); \
			pconfig->s ## _ ## n ## _set = true; \
			return 1; \
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

	if (!starts_with(section, prompt_prefix)) {
                fprintf(stderr, "\nUnknown section in config...\n section [%s]\n", section);
		return 0;  /* unknown section/name, error */
        }
	/* program specific section. It can override some global config values
	 * 
	 * [prompt-gdb]
         * A user or assistant n-short prompt, such as:
	 * user-1 = Disable breakpoint number 4
	 */
	const char *program_name = section + sizeof(prompt_prefix) - 1;
	uaprompt_t prompt = prompt_find(pconfig, program_name);
        
#define MATCH_PROGRAM(s, n, fn)            \
            if (strcmp(name, #n) == 0) {  \
                pconfig->s ## _ ## n = fn(value);                       \
                pconfig->s ## _ ## n ## _set = true;                    \
                if (pconfig->general_verbose) {                         \
                    fprintf(stderr, "   Overriding general config [%s] with program specific [%s] value [%s]\n",\
                            #n, program_name, value);                   \
                }\
                return 1;                                               \
            }                                                           


	if (!prompt)
		prompt = prompt_add(pconfig, program_name);
	if (starts_with(name, user_prefix)) {
		int n = prompt_number(name, user_prefix);
		if (n == -1)
			return 0; // Bad number error
		prompt->user[n] = strdup(value);
		return 1;
	} else if (starts_with(name, assistant_prefix)) {
		int n = prompt_number(name, assistant_prefix);
		if (n == -1)
			return 0; // Bad number error
		prompt->assistant[n] = strdup(value);
		return 1;
	}
        MATCH_PROGRAM(prompt, system, safe_strdup);
        MATCH_PROGRAM(prompt, context, strtocard);
#undef MATCH_PROGRAM
        fprintf(stderr, "\nUnknown config value...\n name [%s] value [%s]\n", name, value);
	return 0;  /* unknown section/name, error */
}

static void
ini_checked_parse(const char* filename, ini_handler handler, config_t *config)
{
	if (config->general_verbose)
		fprintf(stderr, "Config reading %s\n", filename);
	int val = ini_parse(filename, handler, config);
	// When unable to open file val is -1, which we ignore
	if (val > 0) {
		fprintf(stderr, "%s:%d:1: Initialization file error.\n", filename, val);
		exit(EXIT_FAILURE);
	}
}

/*
 * Read the configuration file from diverse directories into config.
 */
void
read_config(config_t *config)
{
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
}

/*
 * Read the configuration file from the specified file path into config.
 * This allows testing the config handler.
 */
void
read_file_config(config_t *config, const char *file_path)
{
	ini_checked_parse(file_path, config_handler, config);
}
