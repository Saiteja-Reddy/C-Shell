#include "ls_implement.h"
#include "echo_implement.h"
#include "nightswatch_implement.h"
#include "pinfo_implement.h"
#include "jobs_ll.h"

#include <ctype.h>
#include <signal.h>

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
int run_setenv(char **);
int run_unsetenv(char **);
int run_jobs(char **);
int run_kjob(char **);
int run_fg(char **);
int run_bg(char **);
int run_overkill(char **);
int run_quit(char **);

char* getCommand(void);
char* idCommand(char *);
int runBuiltin(char **);


char hostname[256];
char username[256];
char cwd[2017];
char *wd;
char *builtin[] = {"echo", "cd", "ls", "pwd", "nightswatch",  "exit","pinfo" , 
"setenv", "unsetenv", "jobs", "kjob", "fg", "bg", "overkill", "quit"}; // HELP
int (*builtin_func[]) (char **) = {&run_echo, &run_cd, &run_ls, &run_pwd, &run_watch,
 &run_exit, &run_pinfo , &run_setenv, &run_unsetenv, &run_jobs, &run_kjob, &run_fg, &run_bg,
 &run_overkill, &run_quit};

char currentDIR[2017];
pid_t childPID = -1;
char nowProcess[1000];

qjob* head;
int mainShellPID;

void catchCTRL_C(int sig)
{
	// signal(SIGINT, SIG_IGN);	
	// printf("\nCTRL C caught\n");
	printf("\n");
	// printf("%d - PID CURRENT\n", childPID );
	if(getpid() != mainShellPID)
		return;

	if(childPID != -1)
	{
		// printf("%d - to send killsignal\n", childPID );
		kill(childPID, SIGINT);
	}
	signal(SIGINT, catchCTRL_C);	

}

void catchCTRL_Z(int sig)
{
	// signal(SIGTSTP, SIG_IGN);

	if(getpid() != mainShellPID)
		return;
	// printf("\nCTRL Z caught\n");
	printf("\n");
	// printf("%d - PID CURRENT\n", childPID );
	// printf("%d - Here in \n", getpid() );
	if(childPID != -1)
	{	
		// printf("%d - to send killsignal\n", childPID );
		kill(childPID, SIGTTIN);	
		kill(childPID, SIGTSTP);
		// kill(childPID, SIGSTOP);
		addtoLL(head, nowProcess , childPID, 0);		
		printf(KMAG "[+] %d %s\n" RESET, childPID, nowProcess);		
	}
	// printLLsize(head);
	signal(SIGTSTP, catchCTRL_Z);	
}

int main(int argc, char const *argv[])
{
	head = (qjob*)malloc(sizeof(qjob));
	head->next = NULL;
	mainShellPID = getpid();
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
		signal(SIGTSTP, catchCTRL_Z);
		signal(SIGINT, catchCTRL_C);	
	while (out)
	{	
		bg = 0;
		childPID = -1;
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
			printDoneJobs(head);	
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
		if(strcmp(charg, "nightswatch") == 0)
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
	childPID = pid;
	strcpy(nowProcess, args[0]);
	if (pid == 0)
	{
		// signal(SIGINT, SIG_IGN);
		// signal(SIGTSTP, SIG_IGN);
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

		int isin = 0;
		int isout = 0;
		int infd = -1;
		int outfd = -1;
		char **nowargs;
		int pos = 0;
		for (int i = 0; args[i] != NULL; ++i)
		{
			if(strcmp(args[i], "<") == 0)
			{
				isin = 1;
				int largs = pos;
				nowargs = malloc(sizeof(char*) * largs);
				int j = 0;
				for (j = 0; j < largs; ++j)
				{
					nowargs[j] = args[j];
				}
				nowargs[j++] = 0;
				infd = open(args[++i], O_RDONLY);
				break;
			}
			pos = pos + 1;
		}

		if(isin == 0)
		{
			printf("Here my exe\n");
			if (execvp(args[0], args) == -1)
			{
				perror("Shell"); // Print Approporiate Error
			}
		}
		else
		{
			// printf("Redirect Here Stuff Here\n");
			dup2(infd,0);
			if(execvp(nowargs[0], nowargs) == -1)
					perror("Shell");

			// int i;
			// for (i = 0; i < args_pos - 1; ++i)
			// {
			// 	int pipe_p[2];
			// 	pipe(pipe_p);

			// 	pid_t npid = fork();

			// 	if(npid == 0)
			// 	{
			// 		dup2(pipe_p[1],1);
			// 		if(execvp(args_table[i][0], args_table[i]) == -1)
			// 		{
			// 			perror("Shell");
			// 		}
			// 		abort();
			// 	} 

			// 	dup2(pipe_p[0], 0);
			// 	close(pipe_p[1]);
			// }

			// if(execvp(args_table[i][0], args_table[i]) == -1)
			// 		perror("Shell");

		}
		for (int i = 0; i < pos; ++i)
		{
			 free(nowargs[i]);
		}
		free(nowargs);
		exit(1);
	}
	else if (pid > 0)
	{
		if(bg == 0)
		{
			// wait(NULL);
			waitpid(-1,NULL,WUNTRACED);
		}
		else
		{
			kill(pid, SIGTTIN);
			// kill(pid, SIGTSTP);
			kill(pid, SIGCONT);
			addtoLL(head, args[0], pid, 1);
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

int runBuiltin(char **args)
{
	int i;
	int count = sizeof(builtin) / sizeof(char *);
	for (i = 0; i < count; ++i)
	{
		if (strcmp(builtin[i], args[0]) == 0)
		{
			return (*builtin_func[i])(args);
			break;
		}
	}
	return 0;
}

int executeCommand(char **args, int bg)
{
	// if(bg == 1)
		// printf("Background Must be Run\n");
	if (args[0] == NULL)
		return 1;

	int boss = runBuiltin(args);
	if(boss == 1)
	{
		// printf("Here\n");
		return 1;
	}
	if(strcmp(args[0],"exit") == 0)
	{
		return 0;
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
	char* args1[] = {""};
	run_overkill(args1);
	return 0;
}

/// ----- 

int run_setenv(char **args)
{
	// printf("IN setenv");
	int count = 0, i;
	for ( i = 0; args[i] != NULL; ++i)
		count = count + 1;
	// printf("%d\n", count );

	if(count >=4)
			printf(RED "setenv: Too many arguments\nUsage: setenv var [value]>\n" RESET);
	else if(count <= 2)
			printf(RED "setenv: Too few arguments\nUsage: setenv var [value]>\n" RESET);
	else
	{
		if(setenv(args[1], args[2], 1) == -1)
		{
			perror("Shell");
		}		
	}

	return 1;
}
int run_unsetenv(char **args)
{
	// printf("IN unsetenv");
	int count = 0, i;
	for ( i = 0; args[i] != NULL; ++i)
		count = count + 1;
	// printf("%d\n", count );

	if(count >=3)
			printf(RED "setenv: Too many arguments\nUsage: setenv var [value]>\n" RESET);
	else if(count <= 1)
			printf(RED "setenv: Too few arguments\nUsage: setenv var [value]>\n" RESET);
	else
	{
		if(unsetenv(args[1]) == -1)
		{
			perror("Shell");
		}		
	}	
	return 1;
}

int run_jobs(char **args)
{
	// printf("IN jobs");
	printLL(head);
	return 1;
}

int run_kjob(char **args) // make job number
{
	// printf("IN kjob");
	int count = 0, i;
	for ( i = 0; args[i] != NULL; ++i)
		count = count + 1;
	// printf("%d\n", count );

	if(count >=4)
			printf(RED "kjob: Too many arguments\nUsage: kjob <jobNumber> <signalNumber>\n" RESET);
	else if(count <= 2)
			printf(RED "kjob: Too few arguments\nUsage: kjob <jobNumber> <signalNumber>\n" RESET);
	else
	{

		int inC = atoi(args[1]);
		int sig = atoi(args[2]);
		qjob* job_node = getjob(head, inC);
		if(job_node !=NULL)
		{
			int pid = job_node -> pid;
			kill(pid,sig);
		}
		else
		{
			printf("No such pid exists\n");
		}

	}

	return 1;
}
int run_fg(char **args)
{
	// printf("IN fg");
	// printf("IN kjob");
	int count = 0, i;
	for ( i = 0; args[i] != NULL; ++i)
		count = count + 1;
	// printf("%d\n", count );

	if(count >=3)
			printf(RED "fg: Too many arguments\nUsage: fg <jobNumber>\n" RESET);
	else if(count <= 1)
			printf(RED "fg: Too few arguments\nUsage: fg <jobNumber>\n" RESET);
	else
	{

		int inC = atoi(args[1]);
		qjob* job_node = getjob(head, inC);
		if(job_node !=NULL)
		{
			int pid = job_node -> pid;
			int flag = 1;
			kill(pid, SIGCONT);
			// kill(pid, SIGTTIN);
			// kill(pid, SIGTTOU);
			childPID = pid;	
			strcpy(nowProcess, job_node->name);
			removeLL(head, pid);
			// wait(NULL);
			waitpid(-1,NULL,WUNTRACED);
		}
		else
		{
			printf("No such pid exists\n");
		}
	}

	return 1;
}
int run_bg(char **args) // make jobnumber
{
	// printf("IN bg");
	// printf("IN kjob");
	int count = 0, i;
	for ( i = 0; args[i] != NULL; ++i)
		count = count + 1;
	// printf("%d\n", count );

	if(count >=3)
			printf(RED "bg: Too many arguments\nUsage: bg <jobNumber>\n" RESET);
	else if(count <= 1)
			printf(RED "bg: Too few arguments\nUsage: bg <jobNumber>\n" RESET);
	else
	{

		int inC = atoi(args[1]);
		qjob* job_node = getjob(head, inC);
		if(job_node !=NULL)
		{
			int pid = job_node -> pid;
			int flag = 1;
			kill(pid, SIGTTIN);
			kill(pid, SIGCONT);
			changestatLL(head,pid,1);
			// signal(SIGINT,SIG_IGN);
		}
		else
		{
			printf("No such pid exists\n");
		}

	}

	return 1;
}
int run_overkill(char **args)
{
	// printf("IN overkill");
	killALL(head);
	return 1;
}
int run_quit(char **args)
{
	// printf("IN quit");
	return 0;
}