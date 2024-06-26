/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  OpenAI access function
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
#include "unit_test.h"

static char *authorization;

// Return the response content from an OpenAI JSON response
STATIC char *
openai_get_response_content(const char *json_response)
{
	json_error_t error;
	json_t *root = json_loads(json_response, 0, &error);
	if (!root) {
		acl_readline_printf("\nOpenAI JSON error: on line %d: %s\n", error.line, error.text);
		return NULL;
	}

	char *ret;
	json_t *choices = json_object_get(root, "choices");
	if (choices) {
		json_t *first_choice = json_array_get(choices, 0);
		json_t *message = json_object_get(first_choice, "message");
		json_t *content = json_object_get(message, "content");
		ret = acl_safe_strdup(json_string_value(content));
	} else {
		json_t *error = json_object_get(root, "error");
		if (error) {
			json_t *message = json_object_get(error, "message");
			acl_readline_printf("\nOpenAI API invocation error: %s\n", json_string_value(message));
		} else
			acl_readline_printf("\nOpenAI API invocation error: %s\n", json_response);
		ret = NULL;
	}

	json_decref(root);
	return ret;
}

/*
 * Initialize OpenAI connection
 * Return 0 on success -1 on error
 */
static int
initialize(config_t *config)
{

	if (config->general_verbose)
		fprintf(stderr, "\nInitializing openAI API, program name [%s] system prompt to use [%s]\n",
		    acl_short_program_name(), config->prompt_system);
	acl_safe_asprintf(&authorization, "Authorization: Bearer %s", config->openai_key);
	return curl_initialize(config);
}

/*
 * Fetch response from the OpenAI API given the provided prompt.
 * Provide context in the form of n-shot prompts and history prompts.
 */
char *
acl_fetch_openai(config_t *config, const char *prompt, int history_length)
{
	CURLcode res;

	if (!acl_curl && initialize(config) < 0)
		return NULL;

	if (config->general_verbose)
		fprintf(stderr, "\nContacting OpenAI API...\n");

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, authorization);

	struct string json_response;
	acl_string_init(&json_response, "");

	struct string json_request;
	acl_string_init(&json_request, "{\n");
	acl_string_appendf(&json_request, "  \"model\": %s,\n",
	    acl_json_escape(config->openai_model));
	acl_string_appendf(&json_request, "  \"temperature\": %g,\n",
	    config->openai_temperature);

	acl_string_append(&json_request, "  \"messages\": [\n");

	char *system_role = acl_system_role_get(config);
	acl_string_appendf(&json_request,
	    "    {\"role\": \"system\", \"content\": %s},\n",
	    acl_json_escape(system_role));
	free(system_role);

	// Add user and assistant n-shot prompts
	for (int i = 0; i < NPROMPTS; i++) {
		if (config->prompt_user[i])
			acl_string_appendf(&json_request,
			    "    {\"role\": \"user\", \"content\": %s},\n",
			    acl_json_escape(config->prompt_user[i]));
		if (config->prompt_assistant[i])
			acl_string_appendf(&json_request,
			    "    {\"role\": \"assistant\", \"content\": %s},\n",
			    acl_json_escape(config->prompt_assistant[i]));
	}

	// Add history prompts as context
	for (int i = config->prompt_context - 1; i >= 0; --i) {
		HIST_ENTRY *h = history_get(history_length - 1 - i);
		if (h == NULL || h->line == NULL || h->line[0] == '\0')
			continue;
		acl_string_appendf(&json_request,
		    "    {\"role\": \"user\", \"content\": %s},\n",
		    acl_json_escape(h->line));
	}

	// Finally, add the user prompt
	acl_string_appendf(&json_request,
	    "    {\"role\": \"user\", \"content\": %s}\n", acl_json_escape(prompt));
	acl_string_append(&json_request, "  ]\n}\n");

	acl_write_log(config, json_request.ptr);

	curl_easy_setopt(acl_curl, CURLOPT_URL, config->openai_endpoint);
	curl_easy_setopt(acl_curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(acl_curl, CURLOPT_WRITEFUNCTION, acl_string_write);
	curl_easy_setopt(acl_curl, CURLOPT_WRITEDATA, &json_response);
	curl_easy_setopt(acl_curl, CURLOPT_POSTFIELDS, json_request.ptr);

	res = curl_easy_perform(acl_curl);

	if (res != CURLE_OK) {
		free(json_request.ptr);
		acl_readline_printf("\nOpenAI API call failed: %s\n",
		    curl_easy_strerror(res));
		return NULL;
	}

	acl_write_log(config, json_response.ptr);

	char *text_response = openai_get_response_content(json_response.ptr);
	free(json_request.ptr);
	free(json_response.ptr);
	return text_response;
}
