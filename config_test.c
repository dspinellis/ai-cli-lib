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

#include "CuTest.h"
#include "config.h"

void
test_read_config(CuTest* tc)
{
	static config_t config;

	read_config(&config);

	CuAssertIntEquals(tc, 3, config.prompt_context);
	CuAssertPtrNotNull(tc, config.openai_endpoint);
	CuAssertPtrNotNull(tc, config.prompt_system);

	uaprompt_t gdb = prompt_find(&config, "gdb");
	CuAssertPtrNotNull(tc, gdb);
	CuAssertStrEquals(tc, "Disable breakpoint number 4", gdb->user[0]);
	CuAssertStrEquals(tc, "delete 4", gdb->assistant[0]);
	CuAssertTrue(tc, gdb->user[2] == NULL);
}

void
test_starts_with(CuTest* tc)
{
	extern bool starts_with(const char *string, const char *prefix);

	CuAssertTrue(tc, starts_with("prompt-gdb", "prompt-"));
	CuAssertTrue(tc, !starts_with("prompt", "prompt-"));
}

void
test_prompt_number(CuTest* tc)
{
	extern int prompt_number(const char *name, const char *prompt_prefix);

	CuAssertIntEquals(tc, 0, prompt_number("user-1", "user-"));
	CuAssertIntEquals(tc, 2, prompt_number("user-3", "user-"));
	CuAssertIntEquals(tc, -1, prompt_number("user-4", "user-"));
	CuAssertIntEquals(tc, -1, prompt_number("user-n4", "user-"));
	CuAssertIntEquals(tc, -1, prompt_number("user-4n", "user-"));
}

void
test_prompt_add_find(CuTest* tc)
{
	extern uaprompt_t prompt_add(config_t * config, const char *program_name);
	static config_t config;
	CuAssertTrue(tc, prompt_find(&config, "foo") == NULL);
	uaprompt_t p = prompt_add(&config, "foo");
	(void)prompt_add(&config, "bar");
	CuAssertPtrNotNull(tc, p);
	uaprompt_t p2 = prompt_find(&config, "foo");
	CuAssertPtrEquals(tc, p2, p);
}

CuSuite*
cu_config_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_starts_with);
	SUITE_ADD_TEST(suite, test_prompt_number);
	SUITE_ADD_TEST(suite, test_prompt_add_find);
	SUITE_ADD_TEST(suite, test_read_config);

	return suite;
}
