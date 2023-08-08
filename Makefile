PROGS=rl_driver ai_readline.so
RL_SRC=ai_readline.c config.c ini.c memory.c


all: $(PROGS)

rl_driver: rl_driver.c
	cc rl_driver.c -lreadline -o $@

ai_readline.so: $(RL_SRC)
	gcc -shared -fPIC $(RL_SRC) -o $@ -ldl

e2e-test: $(PROGS)
	LD_PRELOAD=`pwd`/ai_readline.so ./rl_driver

all-tests:
	gcc all_tests.c json_parse_test.c CuTest.c -o $@

unit-test: all-tests
	./all-tests
