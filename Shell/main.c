#include "ls_implement.h"
#include "echo_implement.h"
#include "nightswatch_implement.h"
#include "pinfo_implement.h"

#include <ctype.h>

#define TOKEN_BUFSIZE 64
#define DELIMITERS " \t\r\n\a"
#define BACK_DELIMITERS "&"
#define SEMICOLON_DELIMITERS ";"
#define RESET "\x1B[0m"
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"

void shell_loop(void);
int launchProcess(char **, int);
char **splitCommand(char *, int*);
char **splitBackCommand(char *, int*);
char **semicolonSeperator(char *, int* );
int executeCommand(char **, int);
int run_cd(char **);
int run_pwd(char **);
int run_exit(char **);
char* getCommand(void);
char* idCommand(char *);

char hostname[256];
char username[256];
char cwd[2017];
char *wd;
char *builtin[] = {"echo", "cd", "ls", "pwd", "nightswatch",  "exit","pinfo"}; // HELP
int (*builtin_func[]) (char **) = {&run_echo, &run_cd, &run_ls, &run_pwd, &run_watch, &run_exit, &run_pinfo};
int background[1000] = {0};
char **background_process;
int bgpointer = 0;
char currentDIR[2017];

int main(int argc, char const *argv[])
{
	int i, j, flag;
	int userid = getuid();
	struct passwd *uinfo = getpwuid(userid);
	gethostname(hostname, 200);
	strcpy(username, uinfo->pw_name) ;
	// getwd(currentDIR);
	getcwd(currentDIR, 2000);
	shell_loop();
	return 0;
}

void shell_loop(void)
{
	int i,j;
	char *in_line;
	char **args;
	int out = 1;
	int *args_len = (int*)malloc(sizeof(int));
	char **backArgs;
	char **commands;
	int *back_args_len = (int*)malloc(sizeof(int));
	int *semi_args_len = (int*)malloc(sizeof(int));
	int *back_argCommand_len = (int*)malloc(sizeof(int));
	int bg = 0;
	background_process = malloc(sizeof(char*)*1000);
	for (i = 0; i < 1000; ++i)
	{
		background_process[i] = malloc(sizeof(char)*100);
	}

	while (out)
	{
		bg = 0;
		// getwd(cwd);
		getcwd(cwd, 2000);
		if(strstr(cwd,currentDIR) != NULL)
			wd = strstr(cwd, currentDIR) + strlen(currentDIR);
		else
			wd = cwd;
		printf(KCYN "<%s@%s:~%s > " RESET, username, hostname, wd);
		in_line = getCommand();
		commands = semicolonSeperator(in_line, semi_args_len);
		
		for (j = 0; j < *(semi_args_len); ++j)
		{

				backArgs = splitBackCommand(commands[j], back_args_len);
				// backArgs = splitBackCommand(in_line, back_args_len);
				if(*back_args_len == 1)
				{
					in_line = idCommand(commands[j]);
					// in_line = idCommand(in_line);
					args = splitCommand(in_line, args_len);		
					out = executeCommand(args, bg);
				}
				else
				{
					for (i = 0; i < *(back_args_len) - 1; ++i)
					{
						bg = 1;
						in_line = idCommand(backArgs[i]);
						args = splitCommand(backArgs[i], back_argCommand_len);		
						out = executeCommand(args, bg);
					}	
					bg = 0;
					in_line = idCommand(backArgs[i]);
					args = splitCommand(backArgs[i], back_argCommand_len);		
					out = executeCommand(args, bg);
				}
		
		}	

		for (i = 0; i < bgpointer; ++i)
		{
			// printf("Here at BG\n");
			pid_t ch_pid = background[i];
			pid_t return_pid = waitpid(ch_pid, NULL, WNOHANG);
			if(return_pid == ch_pid)
				{
					printf(KGRN "[-] Done %d %s\n" RESET, ch_pid, background_process[i]);
					free(background_process[i]);
				}
		}	
	}
	free(commands);
	free(args);
	free(backArgs);
	free(in_line);	
}

char* getCommand(void)
{
	unsigned long bufsize = 0;
	char *buffer;
	getline(&buffer, &bufsize, stdin);
	return buffer;
}

char* idCommand(char * buffer)
{
	char *cpBuffer = (char*)malloc(sizeof(char)*1000);
	strcpy(cpBuffer,buffer);

	char *charg;
	charg =  strtok(cpBuffer, DELIMITERS);
	if(charg!= NULL)
	{
		if(strcmp(charg, "echo") == 0)
		{
			checkEcho(buffer);
			return cpBuffer;
		}
		else if(strcmp(charg, "nightswatch") == 0)
		{
			// printf("Watch Command to be executed Here\n");
			int *args_len = (int*)malloc(sizeof(int));
			char **args;
			args = splitCommand(buffer, args_len);	
			printWatch(args, args_len);	
			free(args);
			free(args_len);
			return cpBuffer;
		}
		else if(strcmp(charg, "pinfo")==0)
		{
		//	printf("Pinfo must be executed\n");
			int *args_len = (int*)malloc(sizeof(int));
			char **args;
			args = splitCommand(buffer, args_len);	
			printPinfo(args, args_len);		

			free(args);
			free(args_len);
		
			return cpBuffer;
		}
	}
	free(cpBuffer);
	return buffer;
}

char **semicolonSeperator(char *line, int* semicolon_args_len)
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

	arg = strtok(line, SEMICOLON_DELIMITERS);
	while (arg != NULL)
	{
		args[position++] = arg;
		// printf("%s\n", arg);		
		arg = strtok(NULL, SEMICOLON_DELIMITERS);
	}
	args[position] = NULL;
	*semicolon_args_len = position;
	return args;
} 

char **splitBackCommand(char *line, int* back_args_len)
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

	arg = strtok(line, BACK_DELIMITERS);
	while (arg != NULL)
	{
		args[position++] = arg;
		// printf("%s\n", arg);		
		arg = strtok(NULL, BACK_DELIMITERS);
	}
	args[position] = NULL;
	*back_args_len = position;
	return args;
}

char **splitCommand(char *line, int* args_len)
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
	*args_len = position;
	return args;
}

int launchProcess(char **args, int bg)
{
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		if(bg == 1)
		{
			// printf("Background\n");
			if(setpgid(0,0) == 0)
			{
				// pid_t processid;
				// processid = getpid();
				// printf("Process in BG now  [+]%d\n" , processid);
			}
			else
			{
				fprintf(stderr, "shell: Unable to start as BG Process\n");
				exit(1);
			}
		}

		if (execvp(args[0], args) == -1)
		{
			perror("Shell"); // Print Approporiate Error
		}

		exit(1);
	}
	else if (pid > 0)
	{
		if(bg == 0)
		{
			wait(NULL);
		}
		else
		{
			strcpy(background_process[bgpointer] , args[0]);
			background[bgpointer++] = pid;
			printf(KMAG "[+] %d %s\n" RESET, pid, args[0]);
		}
	}
	else
	{
		fprintf(stderr, "shell: Error Forking Child\n");
		exit(1);
	}
	return 1;
}

int executeCommand(char **args, int bg)
{
	// if(bg == 1)
		// printf("Background Must be Run\n");
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
	return launchProcess(args, bg);
}



int run_cd(char **args)
{
	// printf("IN CD : %s\n", args[0]);
	if (args[1] == NULL)
	{
		fprintf(stderr, "Please Enter an argument for cd\n");
		return 1;
	}
	char buffer[2000];
	sprintf(buffer, "%s", args[1]);
	if(args[1][0] == '~')
	{
		sprintf(buffer, "%s/%s",currentDIR,args[1] + 1);
		// printf("%s\n" ,buffer);
	}
	// if (chdir(args[1]) != 0)
	if (chdir(buffer) != 0)
	{
		perror("Shell");
	}
	return 1;
}

int run_pwd(char **args)
{
	// printf("IN PWD : %s\n", args[0]);
	printf("%s\n", cwd);
	return 1;
}

int run_exit(char **args)
{
	// printf("IN EXIT : %s\n", args[0]);
	// exit(1);
	return 0;
}