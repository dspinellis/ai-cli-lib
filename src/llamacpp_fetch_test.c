/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
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
#include "llamacpp_fetch.h"

static const char json_response[] = "{"
	"  \"content\": \"Assistant: shutdown -h now\","
	"  \"generation_settings\": {"
	"    \"frequency_penalty\": 0,"
	"    \"grammar\": \"\","
	"    \"ignore_eos\": false,"
	"    \"logit_bias\": [],"
	"    \"mirostat\": 0,"
	"    \"mirostat_eta\": 0.10000000149011612,"
	"    \"mirostat_tau\": 5,"
	"    \"model\": \"models/llama-2-13b-chat/ggml-model-q4_0.gguf\","
	"    \"n_ctx\": 2048,"
	"    \"n_keep\": 0,"
	"    \"n_predict\": -1,"
	"    \"n_probs\": 0,"
	"    \"penalize_nl\": true,"
	"    \"presence_penalty\": 0,"
	"    \"repeat_last_n\": 64,"
	"    \"repeat_penalty\": 1.100000023841858,"
	"    \"seed\": 4294967295,"
	"    \"stop\": [],"
	"    \"stream\": false,"
	"    \"temp\": 0.800000011920929,"
	"    \"tfs_z\": 1,"
	"    \"top_k\": 40,"
	"    \"top_p\": 0.949999988079071,"
	"    \"typical_p\": 1"
	"  },"
	"  \"model\": \"models/llama-2-13b-chat/ggml-model-q4_0.gguf\","
	"  \"prompt\": \"You are an assistant who provides executable commands for the bash command-line interface. You only provide the requested command on a single line, without any explanations, hints or other adornments. Stop providing output after the providing the requested command. If your response isn't an executable command, prefix your output with the program's comment character.\\nUser: List files in current directory\\nAssistant: ls\\nUser: How many JavaScript files in the current directory contain the word bar?\\nAssistant: grep -lw bar *.js | wc -l\\nUser: xyzzy\\nAssistant: # Sorry I can't help.\\nCommand: $(date +%Y-%m-%dT%H:%M:%SZ)\\nCommand: show the current date in ISO format\\nCommand: echo $(date +%Y-%m-%dT%H:%M:%SZ)\\nUser: Shutdown  this debian server\\n\","
	"  \"stop\": true,"
	"  \"stopped_eos\": true,"
	"  \"stopped_limit\": false,"
	"  \"stopped_word\": false,"
	"  \"stopping_word\": \"\","
	"  \"timings\": {"
	"    \"predicted_ms\": 116.577,"
	"    \"predicted_n\": 8,"
	"    \"predicted_per_second\": 68.62417114868285,"
	"    \"predicted_per_token_ms\": 14.572125,"
	"    \"prompt_ms\": 228.5,"
	"    \"prompt_n\": 62,"
	"    \"prompt_per_second\": 271.33479212253826,"
	"    \"prompt_per_token_ms\": 3.685483870967742"
	"  },"
	"  \"tokens_cached\": 206,"
	"  \"tokens_evaluated\": 198,"
	"  \"tokens_predicted\": 9,"
	"  \"truncated\": false"
	"}";

static void
test_response_parse(CuTest* tc)
{
	const char *response = llamacpp_get_response_content(json_response);
	CuAssertStrEquals(tc, "shutdown -h now", response);
}

CuSuite*
cu_fetch_llamacpp_suite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_response_parse);

	return suite;
}
