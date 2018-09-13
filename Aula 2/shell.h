/*
	Referências:
	- Brennan.io - https://brennan.io/2015/01/16/write-a-shell-in-c/
	- chdir (OpenGroup) - http://pubs.opengroup.org/onlinepubs/009695399/functions/chdir.html
	- dirent.h (OpenGroup) - http://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html
*/

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIMITER " \t\r\n\a"  // Delimiters for arguments
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void main_loop(void);
char *read_line();
char **get_args(char *line);
int execute(char **args);
int launch(char **args);
int mve(char **args);
int dir(char **agrs);
int mke(char **args);
int rme(char **args);
int hlp(char **args);
int clr(char **args);
int out(char **args);
int num_builtins();
char *getCurrentPath(void);

// Built in commands (names, descriptions and functions - declared below)'
char *builtin_str[] = { "mve", "dir", "mke", "rme", "hlp", "clr", "out" };
char *builtin_str_desc[] = { "I'll change the directory you are in. Be careful :)",  // mve
	"I'll show you your files (no details).", //  dir
	"Let's create a new directory!",  // mke
	"Sure! I'll delete anything you want.",  // rme
	"Your assistant. You can ask how to use me ;)",  // hlp
	"What a mess! Let me clean this up.",  // clr
	"Ok, goodbye!" };  // out
int(*builtin_func[]) (char **) = { &mve, &dir, &mke, &rme, &hlp, &clr, &out };

int main(int argc, char **argv) {

	main_loop();

	return EXIT_SUCCESS;

}

void main_loop(void) {

	char *line, *history, **args;
	int status;

	printf("-------------- WELCOME TO RSH --------------\n");

	do {
		printf(getCurrentPath());
		printf(" > ");

		line = read_line();
		args = get_args(line);  // Explting line in command and arguments

	execution: status = execute(args);

		free(line);
		free(args);
	} while (status);

}

char *getCurrentPath(void) {

	long size = pathconf(".", _PC_PATH_MAX);
	char *path;

	if ((path = (char *)malloc((size_t)size)) != NULL) {
		getcwd(path, (size_t)size);
		return path;
	}

	return "";

}

char *read_line(void) {

	char *line = NULL;
	ssize_t buffer_size = 0;

	getline(&line, &buffer_size, stdin);

	return line;

}

char **get_args(char *line) {

	int buffer_size = TOKEN_BUFFER_SIZE, position = 0;
	char **tokens = malloc(buffer_size * sizeof(char *)); // Setting up tokens to store command and arguments
	char *token;

	if (!tokens) {
		fprintf(stderr, "rsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, TOKEN_DELIMITER);  // Spliting it out

	// Executing until 'find spaces'
	while (token != NULL) {
		tokens[position] = token;
		position++;  // Updating array of tokens

		// Reallocating memory (+ 64)
		if (position >= buffer_size) {
			buffer_size += TOKEN_BUFFER_SIZE;
			tokens = realloc(tokens, buffer_size * sizeof(char *));
			if (!tokens) {
				fprintf(stderr, "rsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, TOKEN_DELIMITER);
	}
	tokens[position] = NULL;  // Setting up new argument token

	return tokens;

}

int execute(char **args) {

	// No command was found
	if (args[0] == NULL) {
		return 1;
	}

	for (int i = 0; i < num_builtins(); i++) {
		// Comparing to built in functions (mve, hlp, dir, cop, cut, out)
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);  // Calling built in function 
		}
	}

	return launch(args);  // Calling custom program launcher

}

int launch(char **args) {

	pid_t pid;  // Holds the process copy
	pid_t wpid;  // Holds the state of processes 
	int status;

	pid = fork();  // Running another processes (copy)

	if (pid == 0) {  // Second processes running
		if (execvp(args[0], args) == -1) {  // v -> program name, p -> program name besides full path
			perror("rsh");  // System's error message
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {  // Error in fork()
		perror("rsh");
	}
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);  // Listening to process's state changes
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));  // Ends if processes are exited (1) or killed (2)
	}

	return 1;

}

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);  // Needed for comparing built in functions
}

int mve(char **args) {

	if (args[1] == NULL) {  // No argument was found
		fprintf(stderr, "rsh: expected argument to \"mve\" command\n");
	}
	else {
		if (chdir(args[1]) != 0) {  // chdir changes current directory (0 is good!)
			perror("rsh");
		}
	}

	return 1;

}

int dir(char **args) {

	struct dirent *de;  // Directory stream type

	DIR *dr = opendir(".");  // Opening stream

	if (dr == NULL) {
		fprintf(stderr, "rsh: could not open current directory");
	}

	printf("-------------- YOUR FILES --------------\n\n");

	while ((de = readdir(dr)) != NULL) {
		printf("   %s\n", de->d_name);  // Printing all files in current directory
	}

	printf("\n");

	closedir(dr);  // Closing stream

	return 1;

}

int mke(char **args) {

	if (args[1] == NULL) {  // No argument was found
		fprintf(stderr, "rsh: expected argument to \"mke\" command\n");
	}
	else {
		struct stat st = { 0 };

		if (stat(args[1], &st) == -1) {  // Verifying if folder already exists 
			mkdir(args[1], 0700);
		}
	}

	return 1;

}

int rme(char **args) {

	if (args[1] == NULL) {  // No argument was found
		fprintf(stderr, "rsh: expected argument to \"rme\" command\n");
	}
	else {
		if (remove(args[1]) != 0) {  // Standard C linrary (0 is good!)
			perror("rsh");
		}
	}

	return 1;

}

int hlp(char **args) {

	// Printing header
	printf("-------------- HELP --------------\n");
	printf("The following programs are built in rsh:\n\n");

	for (int i = 0; i < num_builtins(); i++) {
		printf("   %s -> %s\n", builtin_str[i], builtin_str_desc[i]);  // Printing each built in command
	}

	printf("\n");

	return 1;

}

int clr(char **args) {

	system("clear");

	return 1;

}

int out(char **agrs) {
	return 0;
}
