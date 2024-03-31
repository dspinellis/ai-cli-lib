/*-
 *
 *  ai-cli - readline wrapper to obtain a generative AI suggestion
 *  Dummy HAL 9000 access function, that can be used for testing.
 *
 *  Copyright 2024 Diomidis Spinellis
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
#include "fetch_hal.h"
#include "support.h"

/*
 * Fetch response from the dummy HAL 9000 API.
 * See https://en.wikipedia.org/wiki/HAL_9000 and
 * https://en.wikiquote.org/wiki/2001:_A_Space_Odyssey_(film).
 * This endpoint can be used for testing the ai-cli-lib functionality
 * without the need to use a networked API.
 */
char *
acl_fetch_hal(config_t *config, const char *prompt, int history_length)
{
	return acl_safe_strdup("# I'm sorry, Dave. I'm afraid I can't do that.");
}
