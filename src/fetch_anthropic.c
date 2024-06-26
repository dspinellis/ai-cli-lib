/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  anthropic access function
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
#include "fetch_anthropic.h"
#include "unit_test.h"

// HTTP headers
static char *key_header;
static char *version_header;

// Return the response content from an Anthropic JSON response
STATIC char *
anthropic_get_response_content(const char *json_response)
{
	json_error_t error;
	json_t *root = json_loads(json_response, 0, &error);
	if (!root) {
		acl_readline_printf("\nanthropic JSON error: on line %d: %s\n", error.line, error.text);
		return NULL;
	}

	char *ret;
	json_t *content = json_object_get(root, "content");
	if (content) {
		json_t *first_content = json_array_get(content, 0);
		json_t *text = json_object_get(first_content, "text");
		ret = acl_safe_strdup(json_string_value(text));
	} else {
		json_t *error = json_object_get(root, "error");
		if (error) {
			json_t *message = json_object_get(error, "message");
			acl_readline_printf("\nAnthropic invocation error: %s\n", json_string_value(message));
		} else
			acl_readline_printf("\nAnthropic invocation error: %s\n", json_response);
		ret = NULL;
	}

	json_decref(root);
	return ret;
}

/*
 * Initialize curl and anthropic connection
 * Sets curl variable
 * Return 0 on success -1 on error
 */
static int
initialize(config_t *config)
{
	if (config->general_verbose)
		fprintf(stderr, "\nInitializing Anthropic, program name [%s] system prompt to use [%s]\n",
		    acl_short_program_name(), config->prompt_system);
	acl_safe_asprintf(&key_header, "x-api-key: %s", config->anthropic_key);
	acl_safe_asprintf(&version_header, "anthropic-version: %s", config->anthropic_version);
	return curl_initialize(config);
}

/*
 * Fetch response from the anthropic API given the provided prompt.
 * Provide context in the form of n-shot prompts and history prompts.
 */
char *
acl_fetch_anthropic(config_t *config, const char *prompt, int history_length)
{
	CURLcode res;

	if (!acl_curl && initialize(config) < 0)
		return NULL;

	if (config->general_verbose)
		fprintf(stderr, "\nContacting Anthropic API...\n");

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "content-type: application/json");
	headers = curl_slist_append(headers, key_header);
	headers = curl_slist_append(headers, version_header);

	struct string json_response;
	acl_string_init(&json_response, "");

	struct string json_request;
	acl_string_init(&json_request, "{\n");

	acl_string_appendf(&json_request, "  \"model\": %s,\n",
	    acl_json_escape(config->anthropic_model));
	acl_string_appendf(&json_request, "  \"max_tokens\": %d,\n",
	    config->anthropic_max_tokens);

	char *system_role = acl_system_role_get(config);
	acl_string_appendf(&json_request, "  \"system\": %s,\n",
	    acl_json_escape(system_role));
	free(system_role);

	// Add configuration settings
	if (config->anthropic_temperature_set)
		acl_string_appendf(&json_request, "  \"temperature\": %g,\n", config->anthropic_temperature);
	if (config->anthropic_top_k_set)
		acl_string_appendf(&json_request, "  \"top_k\": %d,\n", config->anthropic_top_k);
	if (config->anthropic_top_p_set)
		acl_string_appendf(&json_request, "  \"top_p\": %g,\n", config->anthropic_top_p);

	acl_string_append(&json_request, "  \"messages\": [\n");

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
	bool context_explained = false;
	for (int i = config->prompt_context - 1; i >= 0; --i) {
		HIST_ENTRY *h = history_get(history_length - 1 - i);
		if (h == NULL)
			continue;
		if (!context_explained) {
			context_explained = true;
			acl_string_appendf(&json_request,
			    "    {\"role\": \"user\", \"content\": \"Before my final prompt to which I expect a reply, I am also supplying you as context with one or more previously issued commands, to which you simply reply OK\"},\n");
			acl_string_appendf(&json_request,
			    "    {\"role\": \"assistant\", \"content\": \"OK\"},\n");
		}
		acl_string_appendf(&json_request,
		    "    {\"role\": \"user\", \"content\": %s},\n",
		    acl_json_escape(h->line));
		acl_string_appendf(&json_request,
		    "    {\"role\": \"assistant\", \"content\": \"OK\"},\n");
	}

	// Finally, add the user prompt
	acl_string_appendf(&json_request,
	    "    {\"role\": \"user\", \"content\": %s}\n", acl_json_escape(prompt));
	acl_string_append(&json_request, "  ]\n}\n");

	acl_write_log(config, json_request.ptr);

	curl_easy_setopt(acl_curl, CURLOPT_URL, config->anthropic_endpoint);
	curl_easy_setopt(acl_curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(acl_curl, CURLOPT_WRITEFUNCTION, acl_string_write);
	curl_easy_setopt(acl_curl, CURLOPT_WRITEDATA, &json_response);
	curl_easy_setopt(acl_curl, CURLOPT_POSTFIELDS, json_request.ptr);

	res = curl_easy_perform(acl_curl);

	if (res != CURLE_OK) {
		free(json_request.ptr);
		acl_readline_printf("\nAnthropic API call failed: %s\n",
		    curl_easy_strerror(res));
		return NULL;
	}

	acl_write_log(config, json_response.ptr);

	char *text_response = anthropic_get_response_content(json_response.ptr);
	free(json_request.ptr);
	free(json_response.ptr);
	return text_response;
}
