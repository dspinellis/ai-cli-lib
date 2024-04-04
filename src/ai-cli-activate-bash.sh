#
# ai_cli - readline wrapper to obtain a generative AI suggestion
#
# Bash commands to activate ai-cli
#
# Copyright 2023-2024 Diomidis Spinellis
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

if [ "$0" != bash -a "$0" != -bash ] ; then
  echo 'The ai-cli-lib activation script must be sourced, not executed.' 1>&2
  exit 1
fi

AI_CLI_LIB=("__LIBPREFIX__/ai_cli."*)
if [[ $- == *i*                                 # Shell is interactive
  && -r ~/.aicliconfig                          # User has configured script
  && -r $AI_CLI_LIB                             # Library installed
  && "$LD_PRELOAD" != *$AI_CLI_LIB* ]]          # Variable not set
then
  # Linux and Cygwin (is also used for checking)
  if [ -z "$LD_PRELOAD" ] ; then
    export LD_PRELOAD="$AI_CLI_LIB"
  else
    LD_PRELOAD="$LD_PRELOAD:$AI_CLI_LIB"
  fi

  # macOS
  if [ -z "$DYLD_LIBRARY_PATH" ] ; then
    export DYLD_LIBRARY_PATH=/opt/homebrew/lib
  else
    DYLD_LIBRARY_PATH="/opt/homebrew/lib:$DYLD_LIBRARY_PATH"
  fi
  if [ -z "$DYLD_INSERT_LIBRARIES" ] ; then
    export DYLD_INSERT_LIBRARIES="$AI_CLI_LIB"
  else
    DYLD_INSERT_LIBRARIES="$LD_PRELOAD:$AI_CLI_LIB"
  fi

  if shopt -q login_shell ; then
    exec -l bash
  else
    exec bash
  fi
fi
