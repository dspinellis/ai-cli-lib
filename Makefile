PROGS=rl-test ai-readline.so

all: $(PROGS)

rl-test: rl-test.c
	cc rl-test.c -lreadline -o rl-test

ai-readline.so: ai-readline.c
	gcc -shared -fPIC ai-readline.c -o ai-readline.so -ldl

test:$(PROGS)
	LD_PRELOAD=`pwd`/ai-readline.so rl-test
