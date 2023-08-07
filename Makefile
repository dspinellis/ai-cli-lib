PROGS=rl_driver ai_readline.so


all: $(PROGS)

rl_driver: rl_driver.c
	cc rl_driver.c -lreadline -o $@

ai_readline.so: ai_readline.c
	gcc -shared -fPIC ai_readline.c ini.c -o $@ -ldl

e2e-test: $(PROGS)
	LD_PRELOAD=`pwd`/ai_readline.so ./rl_driver

all-tests:
	gcc all_tests.c json_parse_test.c CuTest.c -o $@

unit-test: all-tests
	./all-tests
