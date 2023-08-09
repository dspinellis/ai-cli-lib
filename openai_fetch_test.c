/*-
 *
 *  ai-readline - readline wrapper to obtain a generative AI suggestion
 *  Test OpenAI response parsing.
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
#include "openai_fetch.h"

static const char json_response[] = "{\n"
	"  \"id\": \"chatcmpl-7lg1IuegIknbhVaP00yWmdOeCeWi1\",\n"
	"  \"object\": \"chat.completion\",\n"
	"  \"created\": 1691597296,\n"
	"  \"model\": \"gpt-3.5-turbo-0613\",\n"
	"  \"choices\": [\n"
	"    {\n"
	"      \"index\": 0,\n"
	"      \"message\": {\n"
	"        \"role\": \"assistant\",\n"
	"        \"content\": \"help\"\n"
	"      },\n"
	"      \"finish_reason\": \"stop\"\n"
	"    }\n"
	"  ],\n"
	"  \"usage\": {\n"
	"    \"prompt_tokens\": 116,\n"
	"    \"completion_tokens\": 1,\n"
	"    \"total_tokens\": 117\n"
	"  }\n"
	"}\n";

void
test_response_parse(CuTest* tc)
{
	const char *response = get_response_content(json_response);
	CuAssertStrEquals(tc, "help", response);
}

CuSuite*
cu_fetch_openai_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_response_parse);

	return suite;
}
