# Help: Set PREFIX=~ for a local install; PREFIX=/usr for a system install.
# Help: By default PREFIX is set to install in /usr/local.
PREFIX ?= /usr/local
LIBPREFIX ?= "$(PREFIX)/lib"
MANPREFIX ?= "$(PREFIX)/share/man/"
SHAREPREFIX ?= "$(PREFIX)/share/ai-readline"

SHARED_LIB=ai_readline.so
PROGS=rl_driver $(SHARED_LIB)
RL_SRC=ai_readline.c config.c ini.c openai_fetch.c support.c
TEST_SRC=$(wildcard *_test.c)
LIB=-lcurl -ljansson

# Maximum configuration file size length; allocate on the stack; check errors
# Report all warnings, treat warnings as errors
CFLAGS=-DINI_MAX_LINE=2048 -DINI_USE_STACK=0 -DINI_ALLOW_REALLOC=1 -DINI_STOP_ON_FIRST_ERROR=1 -Wall -Werror

# Help: Set DEBUG=1 to compile with debug information.
ifdef DEBUG
CFLAGS+=-O0 -g
else
CFLAGS+=-O2
endif


all: $(PROGS)

rl_driver: rl_driver.c
	cc rl_driver.c $(LIB) -lreadline -o $@

$(SHARED_LIB): $(RL_SRC)
	gcc -shared -fPIC $(CFLAGS) $(RL_SRC) -o $@ -ldl $(LIB)

e2e-test: $(PROGS) # Help: Invoke the library with a readline read/print loop
	LD_PRELOAD=`pwd`/$(SHARED_LIB) ./rl_driver

all-tests: $(TEST_SRC) $(RL_SRC)
	gcc $(CFLAGS) all_tests.c $(TEST_SRC) $(RL_SRC) CuTest.c $(LIB) -ldl -lreadline -o $@

unit-test: all-tests # Help: Run unit tests
	./all-tests

clean: # Help: Remove generated files
	rm -f $(PROGS) all-tests

install: ai_readline.so # Help: Install library and manual pages
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man5
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man7
	@mkdir -p $(DESTDIR)$(LIBPREFIX)
	@mkdir -p $(DESTDIR)$(SHAREPREFIX)
	install $(SHARED_LIB) $(DESTDIR)$(LIBPREFIX)/
	install -m 644 ai_readlib.5 $(DESTDIR)$(MANPREFIX)/man5
	install -m 644 ai_readlib.7 $(DESTDIR)$(MANPREFIX)/man7
	install -m 644 ai-readline-config $(DESTDIR)$(SHAREPREFIX)/config

help: # Help: Show this help message
	@echo 'The following make targets are available.'
	@sed -n 's/^\([^:]*:\).*# [H]elp: \(.*\)/printf "%-20s %s\\n" "\1" "\2"/p' Makefile | sh | sort
	@echo
	@echo 'You can modify the operation of make with the following assignments.'
	@sed -n 's/^# [H]elp: \(.*\)/\1/p' Makefile
