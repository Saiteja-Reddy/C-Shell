#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

#define TOKEN_BUFSIZE 64
#define DELIMITERS " \t\r\n\a"

void shell_loop(void);
char* readCommand(void);
int launchProcess(char **);
char **splitCommand(char *);

char hostname[256];
char username[256];
char cwd[1024];
char *wd;

int main(int argc, char const *argv[])
{
	int i,j,flag;
	int userid = getuid();
	struct passwd *uinfo = getpwuid(userid);
	gethostname(hostname, 200);
	getwd(cwd);
	strcpy(username,uinfo->pw_name) ;
	wd = strstr(cwd, username) + strlen(username);
	shell_loop();
	return 0;
}

void shell_loop(void)
{
	char *in_line;
	char **args;
	int out;
	while(1)
	{
		printf("<%s@%s:~%s>", username, hostname, wd);
		in_line = readCommand();
		args = splitCommand(in_line);
		out = launchProcess(args);
		printf("%s\n", in_line);
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
	if(!args)
	{
		fprintf(stderr, "shell: Allocation Error\n");
		exit(1);
	}	

	arg = strtok(line, DELIMITERS);
	while(arg != NULL)
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
	if(pid == 0)
	{
		execvp(args[0], args);
	}
	else if(pid > 0)
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
