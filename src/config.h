/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Configuration parsing and access
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

#pragma once

#include <stdbool.h>

#include "unit_test.h"

// Number of supported n-shot prompts
#define NPROMPTS 3

typedef struct {
	// Name of running program (not a configuration item)
	const char *program_name;

	// All settings are listed in section, key alphabetic order
	// (Unless otherwise specified)

	// Anthropic API
	// See https://docs.anthropic.com/claude/reference/messages_post
	const char *anthropic_endpoint;	// API endpoint URL
	const char *anthropic_key;	// API key
	int anthropic_max_tokens;	// Max output tokens
	const char *anthropic_model;	// Name (e.g. claude-3-opus-20240229)
	double anthropic_temperature;
	int anthropic_top_k;
	double anthropic_top_p;
	const char *anthropic_version;	// API version, e.g. 2023-06-01

	// Single character for invoking AI help in Vi mode
	const char *binding_vi;
	// Character sequence for invoking AI help in Emacs mode
	const char *binding_emacs;

	const char *general_api;	// API to use
	const char *general_logfile;	// File to log requests and responses
	const char *general_response_prefix; // Added in pasted responses
	bool general_timestamp;		// Timestamp log entries
	bool general_verbose;		// Verbose program operation

	const char *llamacpp_endpoint;		// API endpoint URL
	// Other llama.cpp parameters in the order documented in
	// https://github.com/ggerganov/llama.cpp/blob/master/examples/server/README.md
	double llamacpp_temperature;
	int llamacpp_top_k;
	double llamacpp_top_p;
	int llamacpp_n_predict;
	int llamacpp_n_keep;
	double llamacpp_tfs_z;
	double llamacpp_typical_p;
	double llamacpp_repeat_penalty;
	int llamacpp_repeat_last_n;
	bool llamacpp_penalize_nl;
	double llamacpp_presence_penalty;
	double llamacpp_frequency_penalty;
	int llamacpp_mirostat;
	double llamacpp_mirostat_tau;
	double llamacpp_mirostat_eta;
	int llamacpp_seed;

	const char *openai_endpoint;	// API endpoint URL
	const char *openai_key;		// API key
	const char *openai_model;	// Model to use (e.g. gpt-3.5)
	double openai_temperature;	// Generation temperature

	int prompt_context;		// # past prompts to provide as context
	const char *prompt_system;	// System prompt

	// Specific program's comment character to be prefixed in prompts
	const char *prompt_comment;
	// Up to three training shots for a specific program
	const char *prompt_user[NPROMPTS];
	const char *prompt_assistant[NPROMPTS];

	// All the above parameters; set to true is set by configuration
	// All listed in section, key alphabetic order
	bool anthropic_endpoint_set;
	bool anthropic_key_set;
	bool anthropic_max_tokens_set;
	bool anthropic_model_set;
	bool anthropic_temperature_set;
	bool anthropic_top_k_set;
	bool anthropic_top_p_set;
	bool anthropic_version_set;

	bool binding_emacs_set;
	bool binding_vi_set;

	bool general_api_set;
	bool general_logfile_set;
	bool general_response_prefix_set;
	bool general_timestamp_set;
	bool general_verbose_set;

	bool llamacpp_endpoint_set;
	bool llamacpp_frequency_penalty_set;
	bool llamacpp_mirostat_eta_set;
	bool llamacpp_mirostat_set;
	bool llamacpp_mirostat_tau_set;
	bool llamacpp_n_keep_set;
	bool llamacpp_n_predict_set;
	bool llamacpp_penalize_nl_set;
	bool llamacpp_presence_penalty_set;
	bool llamacpp_repeat_last_n_set;
	bool llamacpp_repeat_penalty_set;
	bool llamacpp_seed_set;
	bool llamacpp_temperature_set;
	bool llamacpp_tfs_z_set;
	bool llamacpp_top_k_set;
	bool llamacpp_top_p_set;
	bool llamacpp_typical_p_set;

	bool openai_endpoint_set;
	bool openai_key_set;
	bool openai_model_set;
	bool openai_temperature_set;

	bool prompt_comment_set;
	bool prompt_context_set;
	bool prompt_system_set;
} config_t;

void acl_read_config(config_t *config);

#if defined(UNIT_TEST)
void read_file_config(config_t *config, const char *file_path);
#endif

char *acl_system_role_get(config_t *config);
