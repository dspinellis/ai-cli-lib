/*-
 *
 *  ai-readline - readline wrapper to obtain a generative AI suggestion
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

#include "CuTest.h"
#include "safe.h"

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
}

CuSuite*
cu_safe_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_strtocard);
	SUITE_ADD_TEST(suite, test_asprintf);

	return suite;
}
