#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <readline/history.h>
#include <jansson.h>
#include <dlfcn.h>

#include "config.h"
#include "support.h"

static CURL *curl;
static char *authorization;
static char *system_role;
static const char *program_name;
static FILE *logfile;

// Return the response content from an OpenAI JSON response
STATIC char *
get_response_content(const char *json_response)
{
	json_error_t error;
	json_t *root = json_loads(json_response, 0, &error);
	if (!root) {
		readline_printf("\nOpenAI JSON error: on line %d: %s\n", error.line, error.text);
		return NULL;
	}

	char *ret;
	json_t *choices = json_object_get(root, "choices");
	if (choices) {
		json_t *first_choice = json_array_get(choices, 0);
		json_t *message = json_object_get(first_choice, "message");
		json_t *content = json_object_get(message, "content");
		ret = safe_strdup(json_string_value(content));
	} else {
		json_t *error = json_object_get(root, "error");
		if (error) {
			json_t *message = json_object_get(error, "message");
			readline_printf("\nOpenAI API invocation error: %s\n", json_string_value(message));
		} else
			readline_printf("\nOpenAI API invocation error: %s\n", json_response);
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
	if (!dlopen("libcurl.so", RTLD_NOW | RTLD_GLOBAL)) {
		readline_printf("\nError loading libcurl.so: %s\n", dlerror());
		return -1;
	}
	if (!dlopen("libjansson.so", RTLD_NOW | RTLD_GLOBAL)) {
		readline_printf("\nError loading libjansson.so: %s\n", dlerror());
		return -1;
	}

	if (config->general_logfile)
		logfile = fopen(config->general_logfile, "a");
	program_name = short_program_name();
	safe_asprintf(&authorization, "Authorization: Bearer %s", config->openai_key);
	safe_asprintf(&system_role, config->prompt_system, program_name);
	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (!curl)
		readline_printf("\nCURL initialization failed.\n");
	return curl ? 0 : -1;
}


/*
 * Fetch response from the OpenAI API given the provided prompt.
 * Provide context in the form of n-shot prompts and history prompts.
 */
char *
openai_fetch(config_t *config, const char *prompt, int history_length)
{
	CURLcode res;

	if (!curl && initialize(config) < 0)
		return NULL;

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, authorization);

	struct string json_response;
	string_init(&json_response, "");

	struct string json_request;
	string_init(&json_request, "{\n");
	string_appendf(&json_request, "  \"model\": %s,\n",
	    json_escape(config->openai_model));
	string_appendf(&json_request, "  \"temperature\": %g,\n",
	    config->openai_temperature);

	string_append(&json_request, "  \"messages\": [\n");
	string_appendf(&json_request,
	    "    {\"role\": \"system\", \"content\": %s},\n",
	    json_escape(system_role));


	// Add user and assistant n-shot prompts
	uaprompt_t uaprompts = prompt_find(config, program_name);
	for (int i = 0; uaprompts && i < NPROMPTS; i++) {
		if (uaprompts->user[i])
			string_appendf(&json_request,
			    "    {\"role\": \"user\", \"content\": %s},\n",
			    json_escape(uaprompts->user[i]));
		if (uaprompts->assistant[i])
			string_appendf(&json_request,
			    "    {\"role\": \"assistant\", \"content\": %s},\n",
			    json_escape(uaprompts->assistant[i]));
	}

	// Add history prompts as context
	for (int i = config->prompt_context - 1; i >= 0; --i) {
		HIST_ENTRY *h = history_get(history_length - 1 - i);
		if (h == NULL)
			continue;
		string_appendf(&json_request,
		    "    {\"role\": \"user\", \"content\": %s},\n",
		    json_escape(h->line));
	}

	// Finally, add the user prompt
	string_appendf(&json_request,
	    "    {\"role\": \"user\", \"content\": %s}\n", json_escape(prompt));
	string_append(&json_request, "  ]\n}\n");

	if (logfile)
		fputs(json_request.ptr, logfile);

	curl_easy_setopt(curl, CURLOPT_URL, config->openai_endpoint);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, string_write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_response);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_request.ptr);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		free(json_request.ptr);
		readline_printf("\nOpenAI API call failed: %s\n",
		    curl_easy_strerror(res));
		return NULL;
	}

	if (logfile)
		fputs(json_response.ptr, logfile);

	char *text_response = get_response_content(json_response.ptr);
	free(json_request.ptr);
	free(json_response.ptr);
	return text_response;
}
