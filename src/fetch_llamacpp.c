/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  llama.cpp access function
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
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <readline/history.h>
#include <jansson.h>

#include "config.h"
#include "support.h"
#include "fetch_llamacpp.h"
#include "unit_test.h"

// Return the response content from a llama.cpp JSON response
STATIC char *
llamacpp_get_response_content(const char *json_response)
{
	json_error_t error;
	json_t *root = json_loads(json_response, 0, &error);
	if (!root) {
		acl_readline_printf("\nllama.cpp JSON error: on line %d: %s\n", error.line, error.text);
		return NULL;
	}

	char *ret;
	json_t *content = json_object_get(root, "content");
	if (content) {
		const char assistant[] = "Assistant: ";
		const char *response = json_string_value(content);

		// Remove everything after the first newline
		char *eol = strchr(response, '\n');
		if (eol)
			*eol = '\0';

		if (memcmp(response, assistant, sizeof(assistant) - 1) == 0)
			ret = acl_safe_strdup(response + sizeof(assistant) - 1);
		else {
			acl_readline_printf("\nllama.cpp did not provide a suitable response.\n");
			ret = NULL;
		}
	} else {
		acl_readline_printf("\nllama.cpp invocation error: %s\n", json_response);
		ret = NULL;
	}

	json_decref(root);
	return ret;
}

/*
 * Initialize llama.cpp connection
 * Return 0 on success -1 on error
 */
static int
initialize(config_t *config)
{
	if (config->general_verbose)
		fprintf(stderr, "\nInitializing Llamacpp API, program name [%s] system prompt to use [%s]\n",
		    acl_short_program_name(), config->prompt_system);
	return curl_initialize(config);
}

// Append the specified role's prompt to the string s and then the terminator
static void
prompt_append(struct string *s, const char *role, const char *prompt)
{
	if (!prompt || !*prompt)
		return;
	char *escaped = acl_json_escape(prompt) + 1;
	// Remove trailing quote
	escaped[strlen(escaped) - 1] = '\0';
	acl_string_appendf(s, "%s: %s\\n", role, escaped);
}

/*
 * Fetch response from the llama.cpp API given the provided prompt.
 * Provide context in the form of n-shot prompts and history prompts.
 */
char *
acl_fetch_llamacpp(config_t *config, const char *prompt, int history_length)
{
	CURLcode res;

	if (!curl && initialize(config) < 0)
		return NULL;

	if (config->general_verbose)
		fprintf(stderr, "\nContacting Llamacpp API...\n");

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	struct string json_response;
	acl_string_init(&json_response, "");

	struct string json_request;
	acl_string_init(&json_request, "{\n");

	char *system_role = acl_system_role_get(config);
	char *escaped = acl_json_escape(system_role);
	free(system_role);

	// Remove trailing quote
	escaped[strlen(escaped) - 1] = '\0';
	acl_string_appendf(&json_request, "  \"prompt\": %s\\n", escaped);


	// Add user and assistant n-shot prompts
	for (int i = 0; i < NPROMPTS; i++) {
		prompt_append(&json_request, "User", config->prompt_user[i]);
		prompt_append(&json_request, "Assistant", config->prompt_assistant[i]);
	}

	// Add history prompts as context
	for (int i = config->prompt_context - 1; i >= 0; --i) {
		HIST_ENTRY *h = history_get(history_length - 1 - i);
		if (h == NULL)
			continue;
		prompt_append(&json_request, "Command", h->line);
	}

	// Finally, add the user prompt
	prompt_append(&json_request, "User", prompt);
	acl_string_append(&json_request, "\",\n");

	// Add configuration settings
	if (config->llamacpp_temperature_set)
		acl_string_appendf(&json_request, "  \"temperature\": %g,\n", config->llamacpp_temperature);
	if (config->llamacpp_top_k_set)
		acl_string_appendf(&json_request, "  \"top_k\": %d,\n", config->llamacpp_top_k);
	if (config->llamacpp_top_p_set)
		acl_string_appendf(&json_request, "  \"top_p\": %g,\n", config->llamacpp_top_p);
	if (config->llamacpp_n_predict_set)
		acl_string_appendf(&json_request, "  \"n_predict\": %d,\n", config->llamacpp_n_predict);
	if (config->llamacpp_n_keep_set)
		acl_string_appendf(&json_request, "  \"n_keep\": %d,\n", config->llamacpp_n_keep);
	if (config->llamacpp_tfs_z_set)
		acl_string_appendf(&json_request, "  \"tfs_z\": %g,\n", config->llamacpp_tfs_z);
	if (config->llamacpp_typical_p_set)
		acl_string_appendf(&json_request, "  \"typical_p\": %g,\n", config->llamacpp_typical_p);
	if (config->llamacpp_repeat_penalty_set)
		acl_string_appendf(&json_request, "  \"repeat_penalty\": %g,\n", config->llamacpp_repeat_penalty);
	if (config->llamacpp_repeat_last_n_set)
		acl_string_appendf(&json_request, "  \"repeat_last_n\": %d,\n", config->llamacpp_repeat_last_n);
	if (config->llamacpp_penalize_nl_set)
		acl_string_appendf(&json_request, "  \"penalize_nl\": %s,\n", config->llamacpp_penalize_nl ? "true" : "false");
	if (config->llamacpp_presence_penalty_set)
		acl_string_appendf(&json_request, "  \"presence_penalty\": %g,\n", config->llamacpp_presence_penalty);
	if (config->llamacpp_frequency_penalty_set)
		acl_string_appendf(&json_request, "  \"frequency_penalty\": %g,\n", config->llamacpp_frequency_penalty);
	if (config->llamacpp_mirostat_set)
		acl_string_appendf(&json_request, "  \"mirostat\": %d,\n", config->llamacpp_mirostat);
	if (config->llamacpp_mirostat_tau_set)
		acl_string_appendf(&json_request, "  \"mirostat_tau\": %g,\n", config->llamacpp_mirostat_tau);
	if (config->llamacpp_mirostat_eta_set)
		acl_string_appendf(&json_request, "  \"mirostat_eta\": %g,\n", config->llamacpp_mirostat_eta);
	// End with a non-comma
	acl_string_appendf(&json_request, "  \"stop\": []\n}\n");

	acl_write_log(config, json_request.ptr);

	curl_easy_setopt(curl, CURLOPT_URL, config->llamacpp_endpoint);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, acl_string_write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_response);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_request.ptr);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		free(json_request.ptr);
		acl_readline_printf("\nllama.cpp API call failed: %s\n",
		    curl_easy_strerror(res));
		return NULL;
	}

	acl_write_log(config, json_response.ptr);

	char *text_response = llamacpp_get_response_content(json_response.ptr);
	free(json_request.ptr);
	free(json_response.ptr);
	return text_response;
}
