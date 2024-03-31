/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Configuration parsing and access
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

#include <stdlib.h>
#include <stdio.h>

#include "CuTest.h"
#include "config.h"

static const char *LOCAL_CONFIG = "ai-cli-config";

extern bool starts_with(const char *string, const char *prefix);
extern char *prompt_id(const char *entry);


void
test_read_config(CuTest* tc)
{
	static config_t config = {"gdb"};

	read_file_config(&config, LOCAL_CONFIG);

	CuAssertIntEquals(tc, 3, config.prompt_context);
	CuAssertPtrNotNull(tc, config.openai_endpoint);
	CuAssertPtrNotNull(tc, config.prompt_system);

	CuAssertStrEquals(tc, "Disable breakpoint number 4", config.prompt_user[0]);
	CuAssertStrEquals(tc, "delete 4", config.prompt_assistant[0]);
	CuAssertTrue(tc, config.prompt_user[2] == NULL);
}

void
test_prompt_id(CuTest* tc)
{
	CuAssertStrEquals(tc, "sqlite3", prompt_id("AI_CLI_prompt_sqlite3_system"));
	CuAssertTrue(tc, prompt_id("AI_CLI_prompt_sqlite3") == NULL);
}

// Read configuration file overloaded by an environment variable
void
test_read_overloaded_config(CuTest* tc)
{
	static config_t config = {"gdb"};

	CuAssertTrue(tc, setenv("AI_CLI_binding_vi", "A", 1) == 0);
	CuAssertTrue(tc, setenv("AI_CLI_prompt_bash_system", "You are using bash", 1) == 0);
	CuAssertTrue(tc, setenv("AI_CLI_prompt_bc_system", "You are using bc", 1) == 0);
	CuAssertTrue(tc, setenv("AI_CLI_prompt_gdb_system", "You are using gdb", 1) == 0);
	CuAssertTrue(tc, setenv("AI_CLI_prompt_gdb_user_1", "Disable breakpoint 3", 1) == 0);
	CuAssertTrue(tc, setenv("AI_CLI_prompt_gdb_assistant_1", "delete 3", 1) == 0);
	read_file_config(&config, LOCAL_CONFIG);

	// Values overloaded or set from environment
	CuAssertStrEquals(tc, "A", config.binding_vi);
	CuAssertStrEquals(tc, "You are using gdb", config.prompt_system);
	CuAssertStrEquals(tc, "Disable breakpoint 3", config.prompt_user[0]);
	CuAssertStrEquals(tc, "delete 3", config.prompt_assistant[0]);

	// Value from file
	CuAssertIntEquals(tc, 3, config.prompt_context);

	CuAssertTrue(tc, config.prompt_user[2] == NULL);

	CuAssertTrue(tc, unsetenv("AI_CLI_binding_vi") == 0);
	CuAssertTrue(tc, unsetenv("AI_CLI_prompt_bash_system") == 0);
	CuAssertTrue(tc, unsetenv("AI_CLI_prompt_gdb_system") == 0);
	CuAssertTrue(tc, unsetenv("AI_CLI_prompt_bc_system") == 0);
	CuAssertTrue(tc, unsetenv("AI_CLI_prompt_gdb_user_1") == 0);
	CuAssertTrue(tc, unsetenv("AI_CLI_prompt_gdb_assistant_1") == 0);
}

// Read configuration file and an added environment variable
void
test_read_env_added_config(CuTest* tc)
{
	static config_t config;

	CuAssertTrue(tc, setenv("AI_CLI_general_logfile", "foo.log", 1) == 0);
	read_file_config(&config, LOCAL_CONFIG);
	// Value added from environment
	CuAssertStrEquals(tc, "foo.log", config.general_logfile);
	// Value from file
	CuAssertIntEquals(tc, 3, config.prompt_context);
	CuAssertTrue(tc, unsetenv("AI_CLI_general_logfile") == 0);
}

void
test_system_role_get(CuTest* tc)
{
	static config_t config;

	read_file_config(&config, LOCAL_CONFIG);
	char *system_role = acl_system_role_get(&config);
	CuAssertTrue(tc, starts_with(system_role, "You are an assistant"));
	free(system_role);
}

void
test_starts_with(CuTest* tc)
{
	CuAssertTrue(tc, starts_with("prompt-gdb", "prompt-"));
	CuAssertTrue(tc, !starts_with("prompt", "prompt-"));
}

void
test_prompt_number(CuTest* tc)
{
	extern int prompt_number(const char *name, const char *prompt_prefix);

	CuAssertIntEquals(tc, 0, prompt_number("user-1", "user-"));
	CuAssertIntEquals(tc, 2, prompt_number("user-3", "user-"));
	CuAssertIntEquals(tc, 2, prompt_number("user_3", "user-"));
	CuAssertIntEquals(tc, -1, prompt_number("user-4", "user-"));
	CuAssertIntEquals(tc, -1, prompt_number("user-n4", "user-"));
	CuAssertIntEquals(tc, -1, prompt_number("user-4n", "user-"));
}

CuSuite*
cu_config_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_starts_with);
	SUITE_ADD_TEST(suite, test_prompt_number);
	SUITE_ADD_TEST(suite, test_prompt_id);
	SUITE_ADD_TEST(suite, test_read_config);
	SUITE_ADD_TEST(suite, test_read_overloaded_config);
	SUITE_ADD_TEST(suite, test_read_env_added_config);
	SUITE_ADD_TEST(suite, test_system_role_get);

	return suite;
}
