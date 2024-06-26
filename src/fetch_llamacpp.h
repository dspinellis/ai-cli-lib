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

#include "config.h"

#if defined(UNIT_TEST)
char *llamacpp_get_response_content(const char *json_response);
#endif
char *acl_fetch_llamacpp(config_t *config, const char *prompt, int history_length);
