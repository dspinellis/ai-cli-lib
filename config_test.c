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

	CuAssertPtrNotNull(tc, config.prompt_system);
}

CuSuite*
cu_config_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_read_config);

	return suite;
}
