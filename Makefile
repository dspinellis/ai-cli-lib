PROGS=rl_driver ai_readline.so
RL_SRC=ai_readline.c config.c ini.c memory.c
TEST_SRC=$(wildcard *_test.c)
LIB=-lreadline


all: $(PROGS)

rl_driver: rl_driver.c
	cc rl_driver.c $(LIB) -o $@

ai_readline.so: $(RL_SRC)
	gcc -shared -fPIC $(RL_SRC) -o $@ -ldl

e2e-test: $(PROGS)
	LD_PRELOAD=`pwd`/ai_readline.so ./rl_driver

all-tests: $(TEST_SRC) $(RL_SRC)
	gcc all_tests.c  $(TEST_SRC) $(RL_SRC) CuTest.c $(LIB) -o $@

unit-test: all-tests
	./all-tests
