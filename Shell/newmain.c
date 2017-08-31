#include "ls_implement.h"

#define TOKEN_BUFSIZE 64
#define DELIMITERS " \t\r\n\a"
#define BACK_DELIMITERS "&"
#define SEMICOLON_DELIMITERS ";"

void shell_loop(void);
int launchProcess(char **, int);
char **splitCommand(char *, int*);
char **semicolonSeperator(char *, int*);
char **splitBackCommand(char *, int*);
int executeCommand(char **, int);
int run_echo(char **);
int run_cd(char **);
int run_pwd(char **);
int run_exit(char **);
void checkEcho(char *);
char* addToPrintBuf(char *, char , int *);
char* echoOutLine(int *, char *); // Pre Defs
char* getCommand(void);
char* idCommand(char *);

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
	int *args_len = (int*)malloc(sizeof(int));
	char **backArgs;
	char **commands;
	int *back_args_len = (int*)malloc(sizeof(int));
	int *semicolon_args_len = (int*)malloc(sizeof(int));
	int *back_argCommand_len = (int*)malloc(sizeof(int));
	int bg = 0 , j;

	while (out)
	{
		bg = 0;
		getwd(cwd);
		wd = strstr(cwd, username) + strlen(username);
		printf("<%s@%s:~%s > ", username, hostname, wd);
		in_line = getCommand();
		commands = semicolonSeperator(in_line,semicolon_args_len);
		for(j=0;j< *(semicolon_args_len); j++)
		{
			backArgs = splitBackCommand(commands[j], back_args_len);
			if(*back_args_len == 1)
			{
				in_line = idCommand(commands[j]);
				args = splitCommand(in_line, args_len);		
				out = executeCommand(args, bg);
			}
			else
			{
				int i;
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
	}
	free(args);
	free(commands);
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
	}
	free(cpBuffer);
	return buffer;
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
			wait(NULL);
		else
		{
			printf("[+] %d %s\n" , pid, args[0]);
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

///Echo Here 
void checkEcho(char *buffer)
{
	unsigned long bufsize = 0;
	char *iter = buffer;
	iter = iter + 5;
	int pbufsize = 10;
	int *statePtr = malloc(sizeof(int));
	*statePtr = 0;
	char *printBuffer;
	printBuffer = echoOutLine(statePtr, iter);
	// printf("%s", printBuffer);

	char **allOut = (char**)malloc(sizeof(char*)*10);
	int all_bufsize = 2;
	int position = 0;
	allOut[position++] = printBuffer;

	// printf("%d\n", state);
	while(*statePtr == 2 || *statePtr == 3)
	{
		printf(">");
		getline(&buffer, &bufsize, stdin);
		printBuffer = echoOutLine(statePtr, buffer);	

		allOut[position++] = printBuffer;		
		if(position >= all_bufsize)
		{
			all_bufsize += all_bufsize;
      		allOut = realloc(allOut, all_bufsize);
			if (!allOut) {
			    fprintf(stderr, "Shell: allOut reallocation error\n");
			    exit(EXIT_FAILURE);
			  }      		
		}
	}	
	int i;
	for (i = 0; i < position; ++i)
	{
		printf("%s", allOut[i]);
		// free(allout[])
	}
}

char* echoOutLine(int *statePtr, char *iter)
{
	char *printBuffer = (char*)malloc(sizeof(char)*100);
	int *position = malloc(sizeof(int));
	int state = *statePtr;

	char now = *(iter);
	while(now != '\0')
	{
		switch(state)
		{
			case 0:
				switch(now)
				{
					case 34:
						state = 2;
						break;
					case 39:
						state = 3;
						break;
					case 32:
						break;
					default:
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						state = 1;
						break;
				}
				break;
			case 1:
				switch(now)
				{
					case 34:
						state = 2;
						break;
					case 39:
						state = 3;
						break;
					case 32:
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						state = 0;
						break;
					default:
					printBuffer = addToPrintBuf(printBuffer, now, position);
						// printf("%c",now);
						break;
				}
				break;
			case 2:
				switch(now)
				{
					case 34:
						state = 1;
						break;
					default:
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						break;
				}
				break;

			case 3:
				switch(now)
				{
					case 39:
						state = 1;
						break;
					default:
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						break;
				}
				break;
		}
		iter = iter + 1;
		now = *(iter);
	}
	// return state;
	*(statePtr) = state;
	printBuffer[*(position)] = '\0';
	return printBuffer;
}

char* addToPrintBuf(char *printBuffer, char now, int *position)
{
	int pbufsize = 1000;
	char c = now;
    printBuffer[*(position)] = c;
	*(position) = *(position) + 1;
	// printf("ajkdnaksndkansdkjnk\n");
    // If we have exceeded the buffer, reallocate.
    if (*(position) >= pbufsize) {
      pbufsize += pbufsize;
      printBuffer = realloc(printBuffer, pbufsize);
      if (!printBuffer) {
        fprintf(stderr, "Shell: printBuffer reallocation error\n");
        exit(EXIT_FAILURE);
      }
    }	

    return printBuffer;
}

int run_echo(char **args)
{
	// printf("IN ECHO : %s\n", args[0]);
	return 1;
}
// Echo Ends Here