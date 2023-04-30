#include "mysh.h" // header file
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // C standard library
#include <stdbool.h> // C standard library
#include <string.h> // C standard library
#include <fcntl.h> // BOTH
#include <unistd.h> // BOTH
#include <sys/wait.h> // POSIX standard library
#include <sys/stat.h> // POSIX standard library
#include <glob.h> // POSIX standard library, imported to use the glob function "You may use any standard C or POSIX libraries installed on the iLab." - professor (https://rutgers.instructure.com/courses/210688/discussion_topics/2830919)
#include <limits.h> // C standard library, used to get limits for sizes

#define INITIAL_BUFFER_SIZE 1024

// Executes the line passed in as a paramater, and returns an EXIT STATUS for the commands executed in the line. If there is an exit command, the function exits with the exit status of the entire program (which is why it requires a exit status parameter)
int executeLine(char* line, int SHELL_EXIT_STATUS) {
    // if the command is empty, just return the SHELL_EXIT_STATUS
    if (strcmp(line, "") == 0) {
        return SHELL_EXIT_STATUS;
    }
    bool isEmpty = true;
    for (int i = 0; line[i] != '\0'; i++) {
        if (!isspace(line[i])) {
            isEmpty = false;
            break;
        }
    }
    if (isEmpty) {
        return SHELL_EXIT_STATUS;
    }

    // get the max size of the arguments allowed in each command
    long ARG_MAX = sysconf(_SC_ARG_MAX);
    if (ARG_MAX == -1) {
        perror("sysconf");
        return 1;
    }
    // this variable ensures that the exit status of this executed line is preserved if any of the commands fail so that subsequent successful commands don't replace a 1.
    int LINE_EXIT_STATUS = 0;
    // check if the line has a pipe, if so then there will be two commands in the line
    if (strchr(line, '|')) {
        char* C1 = strtok_r(line, "|", &line);
        char* C2 = strtok_r(line, "|", &line);
        // check if line has a second pipe since "mysh allows for a single pipe connecting two processes."
        if (strtok_r(line, "|", &line)) {
            printf("Error: the line contains more than one pip!\n");
            return 1;
        }
        char** tokensC1 = (char **) malloc(sizeof(char *) * ARG_MAX);
        if (tokensC1 == NULL) {
            perror("Error mallocing tokens");
            return 1;
        }
        // get all the tokens in the command
        int numTokensC1 = tokenize(C1, tokensC1);
        char* STDINC1 = NULL;
        char* STDOUTC1 = NULL;
        char** argsC1 = (char **) malloc(sizeof(char *) * ARG_MAX);
        int numArgsC1 = 0;
        if (argumentalize(tokensC1, numTokensC1, argsC1, &numArgsC1, &STDINC1, &STDOUTC1) == 1) {
            return 1;
        }

        char** tokensC2 = (char **) malloc(sizeof(char *) * ARG_MAX);
        if (tokensC2 == NULL) {
            perror("Error mallocing tokens");
            return 1;
        }
        // get all the tokens in the command
        int numTokensC2 = tokenize(C2, tokensC2);
        char* STDINC2 = NULL;
        char* STDOUTC2 = NULL;
        char** argsC2 = (char **) malloc(sizeof(char *) * ARG_MAX);
        int numArgsC2 = 0;
        if (argumentalize(tokensC2, numTokensC2, argsC2, &numArgsC2, &STDINC2, &STDOUTC2) == 1) {
            return 1;
        }

        // if the first command has a output redirect or the second command has an input redirect, piping is overriden so return an error
        if (STDOUTC1 != NULL || STDINC2 != NULL) {
            printf("Error: piping is overriden by file redirect\n");
            return 1;
        }
        // if the first command is exit we dont have to pipe, we can just execute the second command
        else if (strcmp(argsC1[0], "exit") == 0) {
            // set fd for standard input and standard output
            int fd_in = 0;
            int fd_out = 1;
            // open the output file for second command if any in the appropriate modes and change the fd for output (we know there is no input file because of the first if condition)
            if (STDOUTC2) {
                fd_out = open(STDOUTC2, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                if (fd_out == -1) {
                    perror("Error opening file");
                    return 1;
                }
            }
            if (executeCommand(argsC2, numArgsC2, &fd_in, &fd_out, SHELL_EXIT_STATUS, LINE_EXIT_STATUS) == 1) {
                exit(1);
            } else {
                exit(SHELL_EXIT_STATUS);
            }
        }
        // if the second command is exit we dont have to pipe, we can just execute the first command
        else if (strcmp(argsC2[0], "exit") == 0) {
            // set fd for standard input and standard output
            int fd_in = 0;
            int fd_out = 1;
            // open the input file for first command if any in the appropriate modes and change the fd for input (we know there is no output file because of the first if condition)
            if (STDINC1) {
                fd_in = open(STDINC1, O_RDONLY);
                if (fd_in == -1) {
                    perror("Error opening file");
                    return 1;
                }
            }
            if (executeCommand(argsC1, numArgsC1, &fd_in, &fd_out, SHELL_EXIT_STATUS, LINE_EXIT_STATUS) == 1) {
                exit(1);
            } else {
                exit(SHELL_EXIT_STATUS);
            }
        } 
        else {
            int pipefd[2];
            if (pipe(pipefd) < 0) {
                perror("Error creating pipe");
                return 1;
            }
            // set fd for standard input (for command 1) and standard output (for command 2)
            int fd_inC1 = 0;
            int fd_outC2 = 1;
            // open the input(for command 1) and output (for command 2) files if any in the appropriate modes and change the fd for input and output
            if (STDINC1) {
                fd_inC1 = open(STDINC1, O_RDONLY);
                if (fd_inC1 == -1) {
                    perror("Error opening file");
                    return 1;
                }
            }
            if (STDOUTC2) {
                fd_outC2 = open(STDOUTC2, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                if (fd_outC2 == -1) {
                    perror("Error opening file");
                    return 1;
                }
            }
            
            LINE_EXIT_STATUS = executeCommand(argsC1, numArgsC1, &fd_inC1, &pipefd[1], SHELL_EXIT_STATUS, LINE_EXIT_STATUS);
            if (executeCommand(argsC2, numArgsC2, &pipefd[0], &fd_outC2, SHELL_EXIT_STATUS, LINE_EXIT_STATUS) == 1) {
                return 1;
            } else {
                return LINE_EXIT_STATUS;
            }
        }
    }
    // line does not have a pipe so it is going to perform a single command which is the entire line
    else {
        char* command = line;
        char** tokens = (char **) malloc(sizeof(char *) * ARG_MAX);
        if (tokens == NULL) {
            perror("Error mallocing tokens");
            return 1;
        }
        // get all the tokens in the command
        int numTokens = tokenize(command, tokens);
        char* STDIN = NULL;
        char* STDOUT = NULL;
        char** args = (char **) malloc(sizeof(char *) * ARG_MAX);
        int numArgs = 0;
        if (argumentalize(tokens, numTokens, args, &numArgs, &STDIN, &STDOUT) == 1) {
            return 1;
        }
        // set fd for standard input and standard output
        int fd_in = 0;
        int fd_out = 1;
        // open the input output files if any in the appropriate modes and change the fd for input and output
        if (STDIN) {
            fd_in = open(STDIN, O_RDONLY);
            if (fd_in == -1) {
                perror("Error opening file");
                return 1;
            }
        }
        if (STDOUT) {
            fd_out = open(STDOUT, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
            if (fd_out == -1) {
                perror("Error opening file");
                return 1;
            }
        }
        return executeCommand(args, numArgs, &fd_in, &fd_out, SHELL_EXIT_STATUS, LINE_EXIT_STATUS);
    }
    return LINE_EXIT_STATUS;
}

int tokenize(char* command, char **tokens) {
    // Remove any leading whitespace from the string
    while (*command == ' ' || *command == '\t') {
        command++;
    }
    // Remove any trailing whitespace from the string
    size_t len = strlen(command);
    while (len > 0 && (command[len-1] == ' ' || command[len-1] == '\t' || command[len-1] == '\n')) {
        command[len-1] = '\0';
        len--;
    }
    // begin tokenizing by getting the first token
    char *token = strtok(command, " \t\n");
    // continue until all tokens have been obtained
    int i = 0;
    while (token != NULL) {
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
            tokens[i++] = token; // add the token to the array
            token = strtok(NULL, " \t\n"); // get the next token
            continue;
        }
        // check if token contains '<' or '>'
        char *input_pos = strchr(token, '<');
        char *output_pos = strchr(token, '>');
        if (input_pos != NULL || output_pos != NULL) {
            // split token into two if it contains '<' or '>'
            if (input_pos != NULL) {
                *input_pos = '\0'; // null-terminate the first token
                tokens[i++] = token; // add the first token to the array
                tokens[i++] = "<"; // add the '<' token to the array
                token = input_pos + 1; // advance token to the next part of the string
            } else {
                *output_pos = '\0'; // null-terminate the first token
                tokens[i++] = token; // add the first token to the array
                tokens[i++] = ">"; // add the '>' token to the array
                token = output_pos + 1; // advance token to the next part of the string
            }
        } else {
            tokens[i++] = token; // add the token to the array
            token = strtok(NULL, " \t\n"); // get the next token
        }
    }
    int j, k;
    // shift non-empty elements to the front of the array
    for (j = 0, k = 0; j < i; j++) {
        if (tokens[j] != NULL && strlen(tokens[j]) > 0) {
            tokens[k++] = tokens[j];
        }
    }
    // resize the array to the new length
    tokens[k] = NULL;

    return k;
}

int argumentalize(char** tokens, int numTokens, char** args, int* numArgs, char** STDIN, char** STDOUT) {
    int numInputs = 0;
    int numOutputs = 0;
    for (int i = 0; i < numTokens; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            numInputs++;
            if (tokens[i + 1] == NULL) {
                printf("Error: no token after redirection '<'!\n");
                return 1;
            }
        } else if (strcmp(tokens[i], ">") == 0) {
            numOutputs++;
            if (tokens[i + 1] == NULL) {
                printf("Error: no token after redirection '>'!\n");
                return 1;
            }
        }
    }
    if (numInputs > 1) {
        printf("Error: the command contains more than one input redirection '<'!\n");
        return 1;
    } 
    else if (numOutputs > 1) {
        printf("Error: the command contains more than one output redirection '>'!\n");
        return 1;
    }
    if (args == NULL) {
        perror("Error mallocing args");
        return 1;
    }
    for (int i = 0; i < numTokens; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            char* nextToken = tokens[++i];
            // if the next token has a wildcard, we must check if there are multiple files that fit, which would cause an error
            if (strchr(nextToken, '*') || strchr(nextToken, '~')) {
                int wildCount;
                char** wildArgs = getWildFiles(nextToken, &wildCount);
                // wild args has an error so just set STDIN as the nextToken
                if (wildArgs == NULL) {
                    *STDIN = nextToken;
                } 
                else if (wildCount == 1){
                    *STDIN = wildArgs[0];
                }
                else {
                    printf("Error: ambiguous redirect\n");
                    return 1;
                }
            } 
            // no wildcard in token so just add token normally
            else {
                *STDIN = nextToken;
            }
        } 
        else if (strcmp(tokens[i], ">") == 0) {
            char* nextToken = tokens[++i];
            // if the next token has a wildcard, we must check if there are multiple files that fit, which would cause an error
            if (strchr(nextToken, '*') || strchr(nextToken, '~')) {
                int wildCount;
                char** wildArgs = getWildFiles(nextToken, &wildCount);
                // wild args has an error so just set STDIN as the nextToken
                if (wildArgs == NULL) {
                    *STDOUT = nextToken;
                }
                else if (wildCount == 1){
                    *STDIN = wildArgs[0];
                }
                else {
                    printf("Error: ambiguous redirect\n");
                    return 1;
                }
            } 
            // no wildcard in token so just add token normally
            else {
                *STDOUT = nextToken;
            }
        }
        else {
            // check if current token has a wildcard as long as its not the first argument. Mysh checks the first argument later to make sure that if the first arg is a pathname executable or barename executable, we detect and ambiguous case rather than adding the other ambiguous files as arguments.
            if (i != 0 && (strchr(tokens[i], '*') || strchr(tokens[i], '~'))) {
                int wildCount;
                char** wildArgs = getWildFiles(tokens[i], &wildCount);
                // wild args has an error so just add the token
                if (wildArgs == NULL) {
                    args[(*numArgs)++] = tokens[i];
                } 
                // token has a wild card or is a file, add the wildArgs to the args list
                else {
                    for (int j = 0; j < wildCount; j++) {
                        args[(*numArgs)++] = wildArgs[j];
                    }
                }
            } 
            // no wildcard in token so just add token normally
            else {
                args[(*numArgs)++] = tokens[i];
            }
        }
    }
    return 0;
}

int executeCommand(char** args, int numArgs, int* fd_in, int* fd_out, int SHELL_EXIT_STATUS, int LINE_EXIT_STATUS) {
    bool alreadyExecuted = false;
    char* firstToken = args[0];

    // check the first token to see what function we must run (built-ins and file/bare names)
    if (strcmp(firstToken, "exit") == 0) {
        if (LINE_EXIT_STATUS == 1) {
            exit(1);
        } else {
            exit(SHELL_EXIT_STATUS);
        }
    }
    else if (strcmp(firstToken, "cd") == 0) {
        // we dont need to worry about file redirection with cd since it doesn't accept any input or produce any output (based on the ilab linux terminal and its not mentioned in the project description) so mysh will not as well
        // if the second argument/token is empty, change directory to the HOME directory
        if (args[1] == NULL || strcmp(args[1], "~/") == 0) {
            return cd(getenv("HOME"));
        }
        else {
            // if there is a third argument, there is an error (cd cannot run with more than one argument after the cd argument)
            if (args[2] != NULL) {
                printf("Error: too many arguments for cd\n");
                return 1;
            } else {
                return cd(args[1]);
            }
        }
    }
    else if (strcmp(firstToken, "pwd") == 0) {
        // check if we need to pwd to a file instead of standard output, we dont have to worry about STDIN for pwd since it doesnt do anything in a standard linux terminal like ilab's and there is no part in the project description that says we have to get input from a file for pwd.
        if (*fd_out != 1) {
            // check if pwd is followed by another argument, which would cause an error so we write nothing to the file (create if file doesnt exit)
            if (args[1] != NULL) {
                // write nothing to file
                close(*fd_out);
            }
            else {
                // write pwd to file
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    ssize_t bytesWritten = write(*fd_out, cwd, strlen(cwd));
                    if (bytesWritten == -1 || bytesWritten != strlen(cwd)) {
                        perror("Error writing to output file");
                        close(*fd_out);
                        return 1;
                    }
                    close(*fd_out);
                } else {
                    perror("Error getting cwd");
                    close(*fd_out);
                    return 1;
                }
            }
        }
        // there is not output file so we can just pwd to standard output
        else {
            // check if pwd is followed by another argument which would cause error
            if (args[1] != NULL) {
                printf("Error: too many arguments for pwd\n");
                return 1;
            }
            else {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("%s\n", cwd);
                } else {
                    perror("Error getting cwd");
                    return 1;
                }                
            }
        }
        return 0;
    }
    else if (strchr(firstToken, '/')) {
        // check if the first token has a wildcard
        if (strchr(firstToken, '*') || strchr(firstToken, '~')) {
            int wildCount;
            char** wildArgs = getWildFiles(firstToken, &wildCount);
            // wild args has an error or there is just one file so just execute using the original name
            if (wildArgs == NULL) {
                // do nothing since we will use firstToken as it is currently
            } 
            else if (wildCount == 1){
                firstToken = wildArgs[0];
            }
            else {
                printf("Error: ambiguous pathname executable\n");
                return 1;
            }
        }
        args[0] = firstToken;

        return executeProgram(args[0], args, fd_in, fd_out);
    }
    else {
        // check the cwd for an executable first (look below for the exact quote from the additional notes announcement) and then the 6 specified directories since this is a bare name case
        // "For example, if the working directory contains a file named "echo", the command "ec*o foo" should work (unless the working directory contains other files whose names begin with "ec" and end with "o"). " - Additional Notes from the professor
        char *directories[] = {"./", "/usr/local/sbin/", "/usr/local/bin/", "/usr/sbin/", "/usr/bin/", "/sbin/", "/bin/" }; // I didnt have the working directory as a check before if the token didnt have a "/" but I added it because of the additional notes which say that we have to check it as well if we are provided a bare name
        int fileFound = false;
        for (int i = 0; i < 7; i++) {
            // Allocate memory for the full path
            char testPath[strlen(directories[i]) + strlen(firstToken) + 2];
            testPath[0] = '\0';
            // concat the directory path and the name of the executable file
            strcat(testPath, directories[i]);
            strcat(testPath, firstToken);
            
            char* path = testPath;
            
            // get all files that match the path if token has wildcard
            if (strchr(firstToken, '*') || strchr(firstToken, '~')) {
                int wildCount;
                char** paths = getWildFiles(testPath, &wildCount);
                // if no paths are found in the directory we are searching in, continue running with the token unchanged (if no names match the pattern, mysh should pass the token to the command unchanged.)
                if (paths == NULL) {
                    // do nothing
                } 
                // if one path exists, set the path we are checking to the path of the file and continue running
                else if (wildCount == 1) {
                    path = paths[0];
                    fileFound = true;
                }
                // if more than one path if found, print an error and stop executing this command
                else {
                    printf("Error: ambiguous barename executable\n");
                    return 1;
                }
            }
            struct stat st;
            // check if file exists
            if (stat(path, &st) == 0) {
                fileFound = true;
                // check if file can be executed
                if (st.st_mode & S_IXUSR) {
                    // change the first argument to be the real path of the bare name executable
                    args[0] = path;
                    // execute file
                    return executeProgram(args[0], args, fd_in, fd_out);
                }
                // file cannot be executed to print error and stop iterating through the directories
                else {
                    printf("Error executing file\n");
                    return 1;
                }
            }
        }
        if (!fileFound) {
            perror("Error finding file");
            return 1;
        }
    }
    return 0;
}

int cd(char* path) {
    if (chdir(path) != 0) {
        perror("Error changing directory");
        return 1;
    }
    return 0;
}

char** getWildFiles(char* path, int* filecount) {
    // use glob to get all the files that match the path or are in cwd with the name that contains a wildcard
    // professor said glob is not prefered but didnt say we can't use it
    glob_t glob_result;
    // EXTENSION 3.2: if the path contains a tilde (~) the glob function uses the complete path from the home directory because it has a GLOB_TILDE flag
    // EXTENSION 3.3: using the glob function, the cases where multiple wildcards (*) are used is handled automatically
    int result = glob(path, GLOB_TILDE, NULL, &glob_result);
    if (result != 0) {
        return NULL;
    }
    char** paths = malloc(glob_result.gl_pathc * sizeof(char *));
    if (paths == NULL) {
        printf("Error: mallocing\n");
        globfree(&glob_result);
        return NULL;
    }
    for (size_t i = 0; i < glob_result.gl_pathc; i++) {
        paths[i] = strdup(glob_result.gl_pathv[i]);
    }
    *filecount = glob_result.gl_pathc;
    globfree(&glob_result);
    return paths;
}

int executeProgram(char* path, char** args, int* fd_in, int* fd_out) {
    int status;
    pid_t pid = fork();
    if (pid == -1) {
        perror("Error forking");
        return 1;
    } else if (pid == 0) { // child process
        // if fd for input file is not the standard input, we must dup2 to change the input and then close the file
        if (*fd_in != 0) {
            dup2(*fd_in, STDIN_FILENO);
            close(*fd_in);
        }
        // if fd for output file is not the standard output, we must dup2 to change the output and then close the file
        if (*fd_out != 1) {
            dup2(*fd_out, STDOUT_FILENO);
            close(*fd_out);
        }
        if (execv(path, args) == -1) {
            perror("Error execv failed");
            // since the child process is done, we can exit that way the parent process knows that WIFEXITED is false
            exit(1);
        }
    } else { // parent process
        if (*fd_in != 0) {
            close(*fd_in);
        }
        if (*fd_out != 1) {
            close(*fd_out);
        }
        if (wait(&status) == -1) {
            perror("Error wait failed");
            return 1;
        }
        if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int EXIT_STATUS = 0;
    // if there are two arguments for mysh, run in batch mode
    if (argc == 2) {
        // open the script file
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            EXIT_STATUS = 1;
        } 
        else {
            // create a buffer dynamically (so we can make it bigger) that will store the data read from the file with an initial size of INITIAL_BUFFER_SIZE (1024)
            char* buffer = (char*) malloc(INITIAL_BUFFER_SIZE);
            if (buffer == NULL) {
                perror("Error mallocing buffer");
                EXIT_STATUS = 1;
            }
            char* lineStart; 
            char* lineEnd;
            int lineLength;
            int bytesRead;
            int bufferSize = INITIAL_BUFFER_SIZE;
            int leftovers = 0;
            // read from the file the amount of space we have left in the buffer
            while ((bytesRead = read(fd, buffer + leftovers, bufferSize - leftovers)) > 0) {
                lineStart = buffer;
                // buffer contains atleast 1 "\n". lineEnd points to the earliest "\n".
                while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
                    lineLength = lineEnd - lineStart;
                    // malloc an pointer to a string that will be size of the line
                    char* line = (char*) malloc(lineLength + 1);
                    if (line == NULL) {
                        perror("Error mallocing line");
                        return 1;
                    }
                    // copy line of size lineLength from buffer
                    memcpy(line, lineStart, lineLength);
                    // set last byte to terminator
                    line[lineLength] = '\0';
                    // this either preserves the exit status value (incase it was 1 before) or sets the last exit status to 1 if there was an error executing the line, ensuring that a one is not overwritten by a 0
                    EXIT_STATUS = executeLine(line, EXIT_STATUS) == 0 ? EXIT_STATUS : 1;

                    free(line);
                    // move pointer to the start of the next line
                    lineStart = lineEnd + 1;
                }
                // buffer does not contain a "\n" anymore, we must store leftovers and read again
                leftovers = buffer + bytesRead - lineStart;
                if (leftovers > 0) {
                    // move leftover characters from the bytesRead to the start of the buffer
                    memmove(buffer, lineStart, leftovers);
                    // if the number of byters leftover are the greater than or equal to bytesRead, we need a larger buffer (takes care of when the command size is huge, since buffer will double until it is big enough to store the entire line)
                    if (leftovers >= bytesRead) {
                        // increase the buffer size and reallocate it
                        bufferSize = bufferSize * 2;
                        buffer = realloc(buffer, bufferSize);
                    }
                }
            }
            if (bytesRead == -1) {
                perror("Error reading");
                EXIT_STATUS = 1;                
            }
            free(buffer);
            close(fd);
        }
    }
      // if there is only 1 argument for mysh, run in interactive mode
    else if (argc == 1) {
        printf("Welcome to my shell!\n");
        // this variable store the path for the shell, so when cd is called it is updated to that directory
        int fd = fileno(stdin);

        // this ensures that shell runs until exit is called
        while (true) {
            // check if the last inputed command has a bad exit status
            if (EXIT_STATUS == 1) {
                printf("!msyh> ");
            }
            else {
                printf("mysh> ");
            }
            fflush(stdout);

            int buffer_size = INITIAL_BUFFER_SIZE;
            char *buffer = malloc(buffer_size);
            ssize_t bytes_read;
            ssize_t total_bytes_read = 0;

            while (true) {
                bytes_read = read(fd, buffer + total_bytes_read, buffer_size - total_bytes_read - 1);
                if (bytes_read < 0) {
                    perror("Error reading input");
                    EXIT_STATUS = 1;
                    break;
                }
                total_bytes_read += bytes_read;

                if (total_bytes_read == buffer_size - 1) {
                    // Input command is longer than the current buffer size, need to realloc
                    buffer_size *= 2;
                    buffer = realloc(buffer, buffer_size);
                }
                else {
                    break; // the entire input command is stored in the buffer so we can execute it
                }
            }

            if (bytes_read >= 0) { // Don't process the buffer if there was an error reading input
                buffer[total_bytes_read - 1] = '\0'; // replace newline character with a null terminator
                EXIT_STATUS = executeLine(buffer, EXIT_STATUS);
            }
            free(buffer);
        }
    }
    else {
        printf("Error: enter valid number of arguments!\n");
    }
    return EXIT_STATUS;
}