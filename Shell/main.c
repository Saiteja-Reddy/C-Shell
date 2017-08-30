#include "ls_implement.h"

#define TOKEN_BUFSIZE 64
#define DELIMITERS " \t\r\n\a"

void shell_loop(void);
char* readCommand(void);
int launchProcess(char **);
char **splitCommand(char *);
int executeCommand(char **);
int run_echo(char **);
int run_cd(char **);
int run_pwd(char **);
int run_exit(char **);

char hostname[256];
char username[256];
char cwd[1024];
char *wd;
char *builtin[] = {"echo", "cd", "ls", "pwd", "exit"}; // HELP
int (*builtin_func[]) (char **) = {&run_echo, &run_cd, &run_ls, &run_pwd, &run_exit};

int main(int argc, char const *argv[])
{
	int i, j, flag;
	int userid = getuid();
	struct passwd *uinfo = getpwuid(userid);
	gethostname(hostname, 200);
	strcpy(username, uinfo->pw_name) ;
	shell_loop();
	return 0;
}

void shell_loop(void)
{
	char *in_line;
	char **args;
	int out = 1;
	while (out)
	{
		getwd(cwd);
		wd = strstr(cwd, username) + strlen(username);
		printf("<%s@%s:~%s >", username, hostname, wd);
		in_line = readCommand();
		args = splitCommand(in_line);
		out = executeCommand(args);
		// printf("%d\n", out);
	}
}

char* readCommand(void)
{
	unsigned long bufsize = 0;
	char *buffer;
	getline(&buffer, &bufsize, stdin);
	return buffer;
}

char **splitCommand(char *line)
{
	unsigned long bufsize = TOKEN_BUFSIZE;
	int position = 0;
	char **args = malloc(sizeof(char*) * bufsize);
	char *arg;
	if (!args)
	{
		fprintf(stderr, "shell: Allocation Error\n");
		exit(1);
	}

	arg = strtok(line, DELIMITERS);
	while (arg != NULL)
	{
		args[position++] = arg;
		arg = strtok(NULL, DELIMITERS);
	}
	args[position] = NULL;
	return args;
}

int launchProcess(char **args)
{
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		if (execvp(args[0], args) == -1)
		{
			perror("Shell"); // Print Approporiate Error
		}
		exit(1);
	}
	else if (pid > 0)
	{
		wait(NULL);
	}
	else
	{
		fprintf(stderr, "shell: Error Forking Child\n");
		exit(1);
	}
	return 1;
}

int executeCommand(char **args)
{
	if (args[0] == NULL)
		return 1;

	int i;
	int count = sizeof(builtin) / sizeof(char *);
	for (i = 0; i < count; ++i)
	{
		if (strcmp(builtin[i], args[0]) == 0)
			return (*builtin_func[i])(args);
	}

	// printf("%s\n", args[0]);
	return launchProcess(args);
}

int run_echo(char **args)
{
	printf("IN ECHO : %s\n", args[0]);
	return 1;
}

int run_cd(char **args)
{
	printf("IN CD : %s\n", args[0]);
	if (args[1] == NULL)
	{
		fprintf(stderr, "Please Enter an argument for cd\n");
		return 1;
	}
	if (chdir(args[1]) != 0)
	{
		perror("Shell");
	}
	return 1;
}

int run_pwd(char **args)
{
	printf("IN PWD : %s\n", args[0]);
	printf("%s\n", cwd);
	return 1;
}

int run_exit(char **args)
{
	printf("IN EXIT : %s\n", args[0]);
	// exit(1);
	return 0;
}
