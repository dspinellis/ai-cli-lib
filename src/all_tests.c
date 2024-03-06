/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Unit test driver
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

#include <stdio.h>

#include "CuTest.h"

CuSuite* cu_config_suite();
CuSuite* cu_fetch_anthropic_suite();
CuSuite* cu_fetch_openai_suite();
CuSuite* cu_fetch_llamacpp_suite();
CuSuite* cu_support_suite();

void
run_all_tests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, cu_config_suite());
	CuSuiteAddSuite(suite, cu_fetch_anthropic_suite());
	CuSuiteAddSuite(suite, cu_fetch_openai_suite());
	CuSuiteAddSuite(suite, cu_fetch_llamacpp_suite());
	CuSuiteAddSuite(suite, cu_support_suite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

int main(void)
{
	run_all_tests();
}
