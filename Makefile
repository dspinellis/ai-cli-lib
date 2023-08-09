PROGS=rl_driver ai_readline.so
RL_SRC=ai_readline.c config.c ini.c openai_fetch.c support.c
TEST_SRC=$(wildcard *_test.c)
LIB=-lreadline -lcurl -ljansson
CFLAGS=-DINI_MAX_LINE=1000 -DINI_USE_STACK=0 -DINI_ALLOW_REALLOC=1 -DINI_STOP_ON_FIRST_ERROR=1 -Wall -Werror
CFLAGS+=-O0 -g

all: $(PROGS)

rl_driver: rl_driver.c
	cc rl_driver.c $(LIB) -o $@

ai_readline.so: $(RL_SRC)
	gcc -shared -fPIC $(CFLAGS) $(RL_SRC) -o $@ -ldl -lcurl -ljansson

e2e-test: $(PROGS)
	LD_PRELOAD=`pwd`/ai_readline.so ./rl_driver

all-tests: $(TEST_SRC) $(RL_SRC)
	gcc $(CFLAGS) all_tests.c  $(TEST_SRC) $(RL_SRC) CuTest.c $(LIB) -ldl -o $@

unit-test: all-tests
	./all-tests
