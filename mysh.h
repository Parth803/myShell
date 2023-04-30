#ifndef MYSH_H
#define MYSH_H

int executeLine(char* line, int SHELL_EXIT_STATUS);
int tokenize(char* command, char **tokens);
int argumentalize(char** tokens, int numtokens, char** args, int* numArgs, char** STDIN, char** STDOUT);
int executeCommand(char** args, int numArgs, int* fd_in, int* fd_out, int SHELL_EXIT_STATUS, int LINE_EXIT_STATUS);
int cd(char* path);
char** getWildFiles(char* path, int* filecount);
int executeProgram(char* path, char** args, int* fd_in, int* fd_out);

#endif