#
# ai_cli - readline wrapper to obtain a generative AI suggestion
#
# GNU Makefile for building, installing, and testing ai-cli-lib
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

# Help: Set PREFIX=~ for a local install; PREFIX=/usr for a system install.
# Help: By default PREFIX is set to install in /usr/local.
PREFIX ?= /usr/local
LIBPREFIX ?= "$(PREFIX)/lib"
MANPREFIX ?= "$(PREFIX)/share/man/"
SHAREPREFIX ?= "$(PREFIX)/share/ai-cli"

PROGS=rl_driver $(SHARED_LIB)
RL_SRC=ai_cli.c config.c ini.c fetch_anthropic.c fetch_hal.c fetch_openai.c \
       fetch_llamacpp.c support.c
TEST_SRC=$(wildcard *_test.c)
LIB=-lcurl -ljansson

# Maximum configuration file size length; allocate on the stack; check errors
# Report all warnings, treat warnings as errors
CFLAGS=-DINI_MAX_LINE=2048 -DINI_USE_STACK=0 -DINI_ALLOW_REALLOC=1 -DINI_STOP_ON_FIRST_ERROR=1 -Wall -Werror -L/usr/local/lib

# Help: Set DEBUG=1 to compile with debug information.
ifdef DEBUG
    CFLAGS+=-O0 -g
else
    CFLAGS+=-O2
endif

OS := $(shell uname)
ifeq ($(OS), Darwin)
    HOMEBREW_PREFIX := $(shell brew --prefix)
    READLINE_INCLUDE=$(shell brew info readline | sed -n 's/export CPPFLAGS=//p')
    CFLAGS += -I$(HOMEBREW_PREFIX)/include $(READLINE_INCLUDE) -DMACOS

    READLINE_LIB=$(shell brew info readline | sed -n 's/export LDFLAGS=//p')
    LDFLAGS += -L$(HOMEBREW_PREFIX)/lib $(READLINE_LIB)

    DLL_EXTENSION=dylib
    SHARED_FLAGS=-dynamiclib -undefined dynamic_lookup
    PRELOAD_VAR=DYLD_INSERT_LIBRARIES
    SET_ADD_LIB=DYLD_LIBRARY_PATH=/opt/homebrew/lib:$$DYLD_LIBRARY_PATH
else
    ifeq ($(findstring CYGWIN,$(OS)),CYGWIN)
        DLL_EXTENSION=dll
        SHARED_FLAGS=-shared -fPIC
        PRELOAD_VAR=LD_PRELOAD
        SHARED_LIB_LIB=$(LIB) -lreadline
    else
        DLL_EXTENSION=so
        SHARED_FLAGS=-shared -fPIC
        PRELOAD_VAR=LD_PRELOAD
    endif
endif

CFLAGS += '-DDLL_EXTENSION="$(DLL_EXTENSION)"'

SHARED_LIB=ai_cli.$(DLL_EXTENSION)

all: $(PROGS)

rl_driver: rl_driver.c
	$(CC) $(CFLAGS) $(LDFLAGS) rl_driver.c $(LIB) -lreadline -o $@

$(SHARED_LIB): $(RL_SRC)
	$(CC) $(SHARED_FLAGS) $(CFLAGS) $(LDFLAGS) $(RL_SRC) -o $@ -ldl $(SHARED_LIB_LIB)

verify-global-defs: # Help: Verify prefix of globally visible definitions
	$(CC) $(CFLAGS) $(RL_SRC) -c
	# Check that global not undefined (U) definitions are prefixed
	# The output (grep success) signifies unprefixed global defs
	! nm *.o | sed 's/^ /x/' | awk '$$2 ~ /[A-TVZ]/ {print $$3}' | grep -Ev '^(acl|ini|curl)'

e2e-run: $(PROGS) # Help: Invoke the library with a readline read/print loop
	$(PRELOAD_VAR)=`pwd`/$(SHARED_LIB) $(SET_ADD_LIB) ./rl_driver

e2e-test: $(PROGS) # Help: Test the readline hook
	printf 'Open the pod bay doors HAL\eV\030A' | \
	  $(PRELOAD_VAR)=`pwd`/$(SHARED_LIB) $(SET_ADD_LIB) AI_CLI_general_api=hal ./rl_driver | \
	  grep Dave

all-tests: $(TEST_SRC) $(RL_SRC)
	$(CC) -DUNIT_TEST $(CFLAGS) $(LDFLAGS) all_tests.c -DUNIT_TEST $(TEST_SRC) $(RL_SRC) CuTest.c $(LIB) -ldl -lreadline -o $@

unit-test: all-tests # Help: Run unit tests
	./all-tests

clean: # Help: Remove generated files
	rm -f $(PROGS) all-tests

install: ai_cli.$(DLL_EXTENSION) # Help: Install library and manual pages
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man5
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man7
	@mkdir -p $(DESTDIR)$(LIBPREFIX)
	@mkdir -p $(DESTDIR)$(SHAREPREFIX)
	install $(SHARED_LIB) $(DESTDIR)$(LIBPREFIX)/
	install -m 644 ai_cli.5 $(DESTDIR)$(MANPREFIX)/man5
	install -m 644 ai_cli.7 $(DESTDIR)$(MANPREFIX)/man7
	install -m 644 ai-cli-config $(DESTDIR)$(SHAREPREFIX)/config

help: # Help: Show this help message
	@echo 'The following make targets are available.'
	@sed -n 's/^\([^:]*:\).*# [H]elp: \(.*\)/printf "%-20s %s\\n" "\1" "\2"/p' Makefile | sh | sort
	@echo
	@echo 'You can modify the operation of make with the following assignments.'
	@sed -n 's/^# [H]elp: \(.*\)/\1/p' Makefile
