/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Test Anthropic response parsing.
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

#include "CuTest.h"
#include "anthropic_fetch.h"

static const char json_response[] = "{"
    "  \"content\": ["
    "    {"
    "      \"text\": \"shutdown -h now\","
    "      \"type\": \"text\""
    "    }"
    "  ],"
    "  \"id\": \"msg_013Zva2CMHLNnXjNJJKqJ2EF\","
    "  \"model\": \"claude-3-opus-20240229\","
    "  \"role\": \"assistant\","
    "  \"stop_reason\": \"end_turn\","
    "  \"stop_sequence\": null,"
    "  \"type\": \"message\","
    "  \"usage\": {"
    "    \"input_tokens\": 10,"
    "    \"output_tokens\": 25"
    "  }"
    "}";

static void
test_response_parse(CuTest* tc)
{
	const char *response = anthropic_get_response_content(json_response);
	CuAssertStrEquals(tc, "shutdown -h now", response);
}

CuSuite*
cu_fetch_anthropic_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_response_parse);

	return suite;
}
