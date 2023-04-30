Parth Patel - NetID: pp847
Ernie Oscar Cangas - NetID: egc48

Files included in myshell.tar
	1. mysh.c
		- main myshell file which contains the source code for the shell
	2. mysh.h
		- myshell header file
	3. Makefile
		- a make file used to make it easy to compile run and test the shell
	4. testing scripts located in tests subdirectory
		- builtins.sh
		- pathnames.sh
		- barenames.sh
		- errors.sh
	5. hello.c
		- a simple c program that is used for testing. It just prints hello world when run
	6. helloarg.c
		- a simple c program that is used for testing. It prints the arguments passed in when run

myshell Design:
	- executeLine
		- handles line execution taking care of error cases and cases where there is and is not a pipe, splitting the commands into subcommands if there is a pipe
		- it also calls the helper functions for tokenize and argumentalize for each subcommand and executes them using the executeCommand function
	- tokenize
		- tokenizer that converts input into tokens for processing
	- argumentalize
		- function that processes the tokens from the tokenize function
		- this function also identifies any input and output redirections as well as wildcard characters
	- executeCommand
		- this function handles command execution
		- this function also determines the type of command, whether it be a built-in function or an executable filename or path
	- cd
		- this function handles directory changes
	- getWildFiles
		- this function handles wildcard processing as well as directory wildcards using glob which is a function from the posix library so we are allowed to use it
	- executeProgram
		- this function handles program execution for executable barenames and pathnames using child processes
	- more detailed comments can be found in mysh.c

myshell extensions:
- HOME extension (implemented using the GLOB_TILDE flag in the glob function)
- directory wildcard extension (implemented using the glob function)

Testing Plan:
	- Running the test scripts below will demonstrate most of the scenarios that needed to be check. Each of the test scripts uses echo to write out the check it will execute before it does so that it is easier to see which checks are being run in a command-by-command manner
	- The scripts test the scenarios described in both the project description and the additional notes thoroughly as well as some additional test cases
		1. builtins.sh
			- this will test the functionality of builtin functions and check many different cases (Errors, redirects, pipes)
		2. pathnames.sh
			- this will test the functionality of pathname executable functions
		3. barenames.sh
			- this will test the functionality of barename executable functions
		4. error.sh
			- this will give shell inputs that intentionally give an error

Execution in terminal:
	1. Ensure that you are in the correct directory where the files reside
	2. Compile all the files using this command: make
	3. Run all the testscripts on mysh using this command: make run
	(optional). Run mysh in interactive mode using this command: ./mysh
	4. Clean the environment using this command: make clean
	
