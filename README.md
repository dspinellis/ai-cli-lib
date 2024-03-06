![Build Status](https://img.shields.io/github/actions/workflow/status/dspinellis/ai-cli-lib/main.yml?branch=main)

# ai-cli-lib: AI help for CLI programs
The __ai-cli__
library detects programs that offer interactive command-line editing
through the __readline__ library,
and modifies their interface to allow obtaining help from a GPT
large language model server, such as Anthropic's or OpenAI's,
or one provided through a
[llama.cpp](https://github.com/ggerganov/llama.cpp) server.
Think of it as a command line copilot.


![Demonstration](https://gist.githubusercontent.com/dspinellis/b25d9b3c3e6d6a6260774c32dc7be817/raw/cc191580cd9aa94bb2a2471e1f43b75a49817b64/ai-cli.gif)

## Build
The _ai-cli_
library has been built and tested under the Debian GNU/Linux (bullseye)
distribution
(natively under version 11 and the x86_64 and armv7l architectures
and under Windows Subsystem for Linux version 2),
under macOS (Ventura 13.4) on the arm64 architecture
using Homebrew packages and executables linked against GNU Readline
(not the macOS-supplied _editline_ compatibility layer),
and under Cygwin (3.4.7).
On Linux,
in addition to _make_, a C compiler, and the GNU C library,
the following packages are required:
`libcurl4-openssl-dev`
`libjansson-dev`
`libreadline-dev`.
On macOS, in addition to an Xcode installation, the following Homebrew
packages are required:
`jansson`
`readline`.
On Cygwin in addition to _make_, a C compiler, and the GNU C library,
the following packages are required:
`libcurl-devel`,
`libjansson-devel`,
`libreadline-devel`.
Package names may be different on other systems.

```sh
cd src
make
```

## Test
### Unit testing
```sh
cd src
make unit-test
```

### End-to-end testing
```sh
cd src
make e2e-test
```
This will provide you a simple read-print loop where you can test the
_ai-cli_ library's capability to
link with the Readline API of third party programs.

## Install
```sh
cd src

# Global installation for all users
sudo make install

# Local installation for the user executing the command
make install PREFIX=~
```

## Run
* Under __Linux__ and __Cygwin__ set the `LD_PRELOAD` environment variable
  to load the library using its full path.
  For example, under _bash_ run
  `export LD_PRELOAD=/usr/local/lib/ai_cli.so` (global installation) or
  `export LD_PRELOAD=/home/myname/lib/ai_cli.so` (local installation).
* Under __macOS__ set the `DYLD_INSERT_LIBRARIES` environment variable to load the
  library using its full path.
  For example, under _bash_ run
  `export DYLD_INSERT_LIBRARIES=/Users/myname/lib/ai_cli.dylib`.
  Also set the `DYLD_LIBRARY_PATH` environment variable to include
  the Homebrew library directory, e.g.
  `export DYLD_LIBRARY_PATH=/opt/homebrew/lib:$DYLD_LIBRARY_PATH`.
* Perform one of the following.
  * Obtain your
    [Anthropic API key](https://console.anthropic.com/settings/keys)
    or
    [OpenAI API key](https://platform.openai.com/api-keys)
    and configure it in the `.aicliconfig` file in your home directory.
    This is done with a `key={key}` entry in the file's
    `[anthropic]` or `[openai]` section.
    In addition, add `api=anthropic` or `api=openai` in the file's
    `[general]` section.
    See the file [ai-cli-config](src/ai-cli-config) to understand how configuration
    files are structured.
    Anthropic currently provides free trial credits to new users.
    Note that OpenAI API access requires a different (usage-based)
    subscription from the ChatGPT one.
  * Configure a [llama.cpp](https://github.com/ggerganov/llama.cpp) server
    and list its `endpoint` (e.g. `endpoint=http://localhost:8080/completion`
    in the configuration file's `[llamacpp]` section.
    In addition, add `api=llamacpp` in the file's `[general]` section.
    In brief running a _llama.cpp_ server involves
    * compiling [llama.cpp](https://github.com/ggerganov/llama.cpp) (ideally
      with GPU support),
    * downloading, converting, and quantizing suitable model
      files (use files with more than 7 billion parameters only on GPUs
      with sufficient memory to hold them),
    * Running the server with a command such as `server -m models/llama-2-13b-chat/ggml-model-q4_0.gguf -c 2048 --n-gpu-layers 100`.
* Run the interactive command-line programs, such as
  _bash_, _mysql_, _psql_, _gdb_, _sqlite3_, _bc_, as you normally would.
* If the program you want to prompt in natural language isn't linked
with the GNU Readline library, you can still make it work with Readline,
by invoking it through [rlwrap](https://github.com/hanslub42/rlwrap).
This however looses the program-specific context provision, because
the program's name appears to The _ai-cli_ library as `rlwrap`.
* To obtain AI help, enter a natural language prompt and press `^X-a` (Ctrl-X followed by a)
  in the (default) _Emacs_ key binding mode or `V` if you have configured
  _vi_ key bindings.

### Note for macOS users
Note that macOS ships with the _editline_ line-editing library,
which is currently not compatible with the _ai-cli_ library
(it has been designed to tap onto GNU Readline).
However, Homebrew tools link with GNU Readline, so they can be used
with the _ai-cli_ library.
To find out whether a tool you're using links with GNU Readline (`libreadline`)
or with _editline_ (`libedit`),
use the _which_ command to determine the command's full
path, and then the _otool_ command to see the libraries it is linked with.
In the example below,
`/usr/bin/sqlite3` isn't linked with GNU Readline,
but `/opt/homebrew/opt/sqlite/bin/sqlite3` is linked with _editline_.

```
$ which sqlite3
/usr/bin/sqlite3

$ otool -L /usr/bin/sqlite3
/usr/bin/sqlite3:
        /usr/lib/libncurses.5.4.dylib (compatibility version 5.4.0, current version 5.4.0)
        /usr/lib/libedit.3.dylib (compatibility version 2.0.0, current version 3.0.0)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1319.100.3)

$ otool -L /opt/homebrew/opt/sqlite/bin/sqlite3
/opt/homebrew/opt/sqlite/bin/sqlite3:
        /opt/homebrew/opt/readline/lib/libreadline.8.dylib (compatibility version 8.2.0, current version 8.2.0)
        /usr/lib/libncurses.5.4.dylib (compatibility version 5.4.0, current version 5.4.0)
        /usr/lib/libz.1.dylib (compatibility version 1.0.0, current version 1.2.11)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1319.100.3)

```

Consequently,
if you want to use the capabilities of the _ai-cli_ library, configure your system
to use the Homebrew commands in preference to the ones supplied with macOS.

## Shell startup configuration
You can configure the _ai-cli_ library to be always available in your _bash_ shell by
adding the following lines in your `.bashrc` file.
Adjust the `AI_CLI_LIB` setting to match the _ai-cli_ library installation path;
it is currently set for a local installation in your home directory.
```bash
# >>> initialize the ai-cli library >>>

# Location of the ai-cli shared library; adjust as needed.
AI_CLI_LIB=("$HOME/lib/ai_cli."*)

# Execute only if configured, installed, and not initialized.
if [[ -r ~/.aicliconfig && -r $AI_CLI_LIB && "$LD_PRELOAD" != *$AI_CLI_LIB* ]]
then
  # Set Linux and Cygwin environment variable.
  if [ -z "$LD_PRELOAD" ] ; then
    export LD_PRELOAD="$AI_CLI_LIB"
  else
    LD_PRELOAD="$LD_PRELOAD:$AI_CLI_LIB"
  fi

  # Set macOS environment variables.
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

  # Overlay current bash with a new instance, which will include the required
  # environment variables.
  if shopt -q login_shell ; then
    exec -l bash
  else
    exec bash
  fi

fi
# <<< initialize the ai-cli library <<<
```

## Reference documentation
The _ai-cli_ reference documentation is provided as Unix manual
pages.
* [ai-cli(7) — library](https://dspinellis.github.io/manview/?src=https%3A%2F%2Fraw.githubusercontent.com%2Fdspinellis%2Fai-cli%2Fmain%2Fsrc%2Fai_cli.7&name=ai_cli(7)&link=https%3A%2F%2Fgithub.com%2Fdspinellis%2Fai-cli)
* [ai-cli(5) — configuration](https://dspinellis.github.io/manview/?src=https%3A%2F%2Fraw.githubusercontent.com%2Fdspinellis%2Fai-cli%2Fmain%2Fsrc%2Fai_cli.5&name=ai_cli(5)&link=https%3A%2F%2Fgithub.com%2Fdspinellis%2Fai-cli)

## Contribute
Contributions are welcomed through GitHub pull requests.
Before working on something substantial,
open an issue to signify your interest and coordinate with others.
Particular useful are:
* multi-shot prompts for systems not yet supported
  (see the [ai-cli-config](src/ai-cli-config) file),
* support for other large language models
  (start from the [OpenAI_fetch.c](src/openai_fetch.c) file),
* support for other libraries (mainly [editline](https://man.netbsd.org/editline.3)),
* ports to other platforms and distributions.

## See also
* Diomidis Spinellis. [Commands as AI Conversations](https://doi.org/10.1109/MS.2023.3307170). _IEEE Software_ 40(6), November/December 2023. doi: 10.1109/MS.2023.3307170
* [edX open online course on Unix tools for data, software, and production engineering](https://www.spinellis.gr/unix/?ai-cli)
* Agarwal, Mayank et al. [NeurIPS 2020 NLC2CMD Competition: Translating Natural Language to Bash Commands](https://arxiv.org/pdf/2103.02523.pdf). ArXiv abs/2103.02523 (2021): n. pag.
* [celery-ai](https://github.com/ortegaalfredo/celery-ai): similar idea, but without program-specific prompts; works by monitoring keyboard events through [pynput](https://pynput.readthedocs.io/).
* [ChatGDB](https://github.com/pgosar/ChatGDB): a _gdb_-specific front-end.
* [ai-cli](https://github.com/abhagsain/ai-cli): a (similarly named) command line interface to AI models.
* [Warp AI](https://www.warp.dev/warp-ai): A terminal with integrated AI capabilities.

## Acknowledgements
* API requests are made using [libcurl](https://curl.se/libcurl/).
* The configuration file parsing is based on [inih](https://github.com/benhoyt/inih).
* Unit testing uses [CuTest](https://github.com/ennorehling/cutest).
* JSON is parsed using [Jansson](https://github.com/akheron/jansson/).
