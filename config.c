/*-
 *
 *  ai-readline - readline wrapper to obtain a generative AI suggestion
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
#include "memory.h"
#include "ini.h"

bool verbose;

const char config_name[] = ".airlconfig";


static int
config_handler(void* user, const char* section, const char* name,
    const char* value)
{
	config_t *pconfig = (config_t *)user;

	if (verbose)
		printf("Config [%s]: %s=%s\n", section, name, value);

	// Set the specified section and name to the given value
	#define MATCH(s, n, v) do { \
		if (strcmp(section, #s) == 0 && strcmp(name, #n) == 0) { \
			pconfig->s ## _ ## n = v; \
			return 1; \
		} \
	} while(0);

	MATCH(prompt, context, atoi(value));
	MATCH(prompt, system, safe_strdup(value));
	MATCH(api, endpoint, safe_strdup(value));
	MATCH(api, key, safe_strdup(value));

	return 0;  /* unknown section/name, error */
}

/*
 * Read the configuration file from diverse directories into config.
 */
void
read_config(config_t *config)
{
	ini_parse("/usr/share/ai-readline/config", config_handler, config);
	ini_parse("/usr/local/share/ai-readline/config", config_handler, config);
	ini_parse("ai-readline-config", config_handler, config);

	// $HOME/.airlconfig
	char *home_dir;
	if ((home_dir = getenv("HOME")) != NULL) {
		char *home_config;
		safe_asprintf(&home_config, "%s/%s", home_dir, config_name);
		ini_parse(home_config, config_handler, &config) == 0;
		free(home_config);
	}

	// .airlconfig
	ini_parse(config_name, config_handler, config);
}
