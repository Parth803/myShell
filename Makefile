all: build

build: clean create compile

create:
	mkdir out

compile:
	gcc mysh.c -o mysh
	gcc hello.c -o hello
	gcc helloarg.c -o helloarg

run:
	./mysh tests/builtins.sh ; ./mysh tests/pathnames.sh ; ./mysh tests/barenames.sh ; ./mysh tests/errors.sh

clean:
	rm -f $(filter-out README.txt, $(wildcard *.txt))
	rm -f mysh
	rm -f hello
	rm -f helloarg
	rm -rf out