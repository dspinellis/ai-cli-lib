![Build Status](https://img.shields.io/github/actions/workflow/status/dspinellis/ai-cli/main.yml?branch=main)

# AI-CLI: AI help for CLI programs
The __ai_cli__
library detects programs that offer interactive command-line editing
through the __readline__ library,
and modifies their interface to allow obtaining help from OpenAI's
GPT large language model.
Think of it as a command line copilot.


![Demonstration](https://gist.githubusercontent.com/dspinellis/b25d9b3c3e6d6a6260774c32dc7be817/raw/cc191580cd9aa94bb2a2471e1f43b75a49817b64/ai-cli.gif)

## Build
The __ai_cli__
library has been built and tested under the Debian GNU/Linux (bullseye)
distribution
(natively under version 11 and the x86_64 and armv7l architectures
and under Windows Subsystem for Linux version 2),
and under macOS (Ventura 13.4) on the arm64 architecture
using Homebrew packages and executables linked against GNU Readline
(not the macOS-supplied _editline_ compatibility layer).
On Linux,
in addition to _make_ and the GNU C library,
the following packages are required:
`libcurl4`
`libcurl4-openssl-dev`
`libjansson4`
`libjansson-dev`
`libreadline-dev`.
On macOS, in addition to an Xcode installation, the following Homebrew
packages are required:
`jansson`
`readline`.
Package names may be different on other systems.

```
cd src
make
```

## Test
### Unit testing
```
cd src
make unit-test
```

### End-to-end testing
```
cd src
make e2e-test
```
This will provide you a simple read-print loop where you can test the _ai_cli_ capability to
link with the Readline API of third party programs.

## Install
```
cd src

# Global installation for all users
sudo make install

# Local installation for the currently logged-in user
make install PREFIX=~
```

## Run
* Under __Linux__ set the `LD_PRELOAD` environment variable to load the
  library using its full path.
  For example, under _bash_ run
  `export LD_PRELOAD=/home/myname/lib/ai_cli.so`.
* Under __macOS__ set the `DYLD_INSERT_LIBRARIES` environment variable to load the
  library using its full path.
  For example, under _bash_ run
  `export DYLD_INSERT_LIBRARIES=/Users/myname/lib/ai_cli.dylib`.
  Also set the `DYLD_LIBRARY_PATH` environment variable to include
  the Homebrew library directory, e.g.
  `export DYLD_LIBRARY_PATH=/opt/homebrew/lib:$DYLD_LIBRARY_PATH`.
* [Obtain your OpenAI API key](https://platform.openai.com/signup),
  and configure it in the `.aicliconfig` file in your home directory.
  This is done with a `key=value` entry in the file's `[openai]` section.
  See the file [ai-cli-config](src/ai-cli-config) to understand how configuration
  files are structured.
  Note that OpenAI API access requires a different (usage-based)
  subscription from the ChatGPT one.
  OpenAI currently provides free trial credits to new users.
* Run the interactive command-line programs, such as
  _bash_, _mysql_, _psql_, _gdb_, as you normally would.
* To obtain AI help, enter a natural language prompt and press `^X-a` (Ctrl-X followed by a)
  in the (default) _Emacs_ key binding mode or `V` if you have configured
  _vi_ key bindings.

### Note for macOS users
Note that macOS ships with the _editline_ line-editing library,
which is currently not compatible with _ai-cli_
(it has been designed to tap onto GNU Readline).
However, Homebrew tools link with GNU Readline, so they can be used
with _ai-cli_.
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
if you want to use the capabilities of _ai-cli_, configure your system
to use the Homebrew commands in preference to the ones supplied with macOS.

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
* [edX open online course on Unix tools for data, software, and production engineering](https://www.spinellis.gr/unix/?ai-cli)
* Diomidis Spinellis. AI Everywhere. _IEEE Software_ 40(6), November/December 2023.

## Acknowledgements
* API requests are made using [libcurl](https://curl.se/libcurl/)
* The configuration file parsing is based on [inih](https://github.com/benhoyt/inih)
* Unit testing uses [CuTest](https://github.com/ennorehling/cutest)
* JSON is parsed using [Jansson](https://github.com/akheron/jansson/)
