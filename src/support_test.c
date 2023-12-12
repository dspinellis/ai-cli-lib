/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Safe memory allocation and other functions.
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

#include <stdlib.h>

#include "CuTest.h"
#include "support.h"

void
test_strtocard(CuTest* tc)
{
	CuAssertIntEquals(tc, 12, strtocard("12"));
	CuAssertIntEquals(tc, -1, strtocard(""));
	CuAssertIntEquals(tc, -1, strtocard("x"));
	CuAssertIntEquals(tc, -1, strtocard("3x"));
	CuAssertIntEquals(tc, -1, strtocard("-3"));
}

void
test_asprintf(CuTest* tc)
{
	char *result;
	safe_asprintf(&result, "a=%d", 42);
	CuAssertStrEquals(tc, "a=42", result);
	free(result);
}

void
test_range_strdup(CuTest* tc)
{
	char string[] = "01234";
	char *result;
	result = range_strdup(string + 1, string + 3);
	CuAssertStrEquals(tc, "12", result);
	free(result);
}

void
test_string(CuTest* tc)
{
	string_t s;
	string_init(&s, "hello");
	string_write(", ", 1, 2, &s);
	string_append(&s, "world!");
	CuAssertStrEquals(tc, "hello, world!", s.ptr);

	string_appendf(&s, " The answer is %d.",  42);
	CuAssertStrEquals(tc, "hello, world! The answer is 42.", s.ptr);
}

void
test_short_program_name(CuTest* tc)
{
	CuAssertStrEquals(tc, "all-tests", short_program_name());
}

void
test_json_escape(CuTest* tc)
{
	CuAssertStrEquals(tc, "\"a \\\" (quote) and a \\\\ (backslash)\"", json_escape("a \" (quote) and a \\ (backslash)"));
}


CuSuite*
cu_support_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_strtocard);
	SUITE_ADD_TEST(suite, test_asprintf);
	SUITE_ADD_TEST(suite, test_string);
	SUITE_ADD_TEST(suite, test_short_program_name);
	SUITE_ADD_TEST(suite, test_json_escape);
	SUITE_ADD_TEST(suite, test_range_strdup);

	return suite;
}
