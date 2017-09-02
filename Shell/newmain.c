#include "ls_implement.h"
#include <curses.h>

#define TOKEN_BUFSIZE 64
#define DELIMITERS " \t\r\n\a"
#define BACK_DELIMITERS "&"
#define SEMICOLON_DELIMITERS ";"

void shell_loop(void);
int launchProcess(char **, int);
char **splitCommand(char *, int*);
char **splitBackCommand(char *, int*);
char **semicolonSeperator(char *, int* );
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
int run_watch(char **);
int run_pinfo(char **);
void nwInterrupt(int );
void printWatch(char **, int*);
void nwDirty(int);
void printPinfo(char **, int* );
void printProcess(pid_t);

char hostname[256];
char username[256];
char cwd[1024];
char *wd;
char *builtin[] = {"echo", "cd", "ls", "pwd", "nightswatch",  "exit","pinfo"}; // HELP
int (*builtin_func[]) (char **) = {&run_echo, &run_cd, &run_ls, &run_pwd, &run_watch, &run_exit, &run_pinfo};
int background[1000] = {0};
char **background_process;
int bgpointer = 0;

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
		getwd(cwd);
		wd = strstr(cwd, username) + strlen(username);
		printf("<%s@%s:~%s > ", username, hostname, wd);
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
					printf("[-] Done %d %s\n" , ch_pid, background_process[i]);
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
void printProcess(pid_t pid)
{
	char  buffer[500];
	char  *tempbuffer;
	char exe_path[1024];
	unsigned long buf_size = 0;
	int i;
	sprintf(buffer,"/proc/%d/status",pid);
	FILE * fd = fopen(buffer,"r");
	printf("pid -- %d\n\n",pid);
	getline(&tempbuffer,&buf_size,fd);
	getline(&tempbuffer,&buf_size,fd);
	getline(&tempbuffer,&buf_size,fd);
	printf("%s",tempbuffer);
	for(i=0;i<15;i++)
		getline(&tempbuffer,&buf_size,fd);
	printf("%s",tempbuffer);
	fclose(fd);
	sprintf(buffer,"/proc/%d/exe",pid);
	int ret  = readlink(buffer,exe_path,1000);	
	if(ret == -1)
		printf("Executable path -- Not defined in proc\n\n");
	else
	{
		exe_path[ret] = '\0';
		printf("Executable path -- %s\n\n",exe_path);
	}
}


void printPinfo(char **args, int* args_len)
{
		int pid;
		if(*args_len == 1)
		{
			//printf("Parent info\n");
			printProcess(getpid());

		}
		else
		{
		 	pid= atoi(args[1]);
		 	printProcess(pid);
		 	//printf("%d ProcessInfo\n",pid);
		}

}

void printWatch(char **args, int* args_len)
{
	// printf("IN Print Watch : %s\n", args[0]);
	int opt;
	int index = 0;
	if(*args_len == 1)
	{
		printf("nightswatch: Use nightswatch [options] <valid command>\nHere valid commands are 'interrupt' and 'dirty'\n");
		optind = 1;
		return;
	}

	while( (opt =  getopt(*args_len, args, "n")) != -1 )
	{
		if(opt == 63)
		{
			optind = 1;
			return;
		}
		if(opt == 'n')
			index = optind;
	}
	int i; char c;
	int time_int = 2;
	
	if(index != 0)
	{
		for (i = 0; i < strlen(args[index]); ++i)
		{
			c = args[index][i];
			if(!(c >= 48 && c <= 57))
			{
				printf("nightswatch: failed to parse argument: '%s'\n", args[index] );
				optind = 1;
				return;
			}
		}
		time_int =  atoi(args[index]);
	}
	if(index + 1 == *args_len)
	{
		printf("nightswatch:\n Use nightswatch [options] <valid command>\n Here valid commands are 'interrupt' and 'dirty'\n");
		optind = 1;
		return;
	}

	if(strcmp(args[index + 1], "interrupt") == 0)
	{
		/// Interrupt Code here
		nwInterrupt(time_int);
	}
	else if(strcmp(args[index + 1], "dirty") == 0)
	{
		/// Dirty Code here
		// printf("Dirty Here\n");
		nwDirty(time_int);
	}
	else
	{
		printf("nightswatch:\n Use nightswatch [options] <valid command>\n Here valid commands are 'interrupt' and 'dirty'\n");
		optind = 1;
		return;
	}

	optind = 1;
}
void nwInterrupt(int time_int)
{
	char c;
	WINDOW* curr = initscr();
	keypad(stdscr, TRUE);
	WINDOW * win; 
	win = newwin(800,600,1,1);
	noecho();
	curs_set(0);
	nodelay(win,1);
	int j = 10;
	int k = 0;
	int start = time(NULL) , current,prevcurrent = time(NULL);
	k=5;
	unsigned long bufsize = 0;
	char *buffer;
	char  *cpuinfo;	
	char label[200];
			FILE* fd = fopen("/proc/interrupts", "r");
			fseek(fd, 0, SEEK_SET);			
			getline(&cpuinfo, &bufsize, fd);
			fclose(fd);
	sprintf(label, "\t%s\tTime specified = %d s","NIGHTSWATCH -- keyboard interrupts",time_int);
	while(1)
	{
		current = time(NULL);
		if(wgetch(win) == 'q')
			break;
		mvwaddstr(win,1,10,label);
		mvwaddstr(win, 3, 10, cpuinfo);					
		if((current- start)%time_int == 0 && current!=prevcurrent)
		{ 
			fd = fopen("/proc/interrupts", "r");
			prevcurrent = current;
			k += 1;
			fseek(fd, 0, SEEK_SET);			
			getline(&buffer, &bufsize, fd);
				getline(&buffer, &bufsize, fd);
			getline(&buffer, &bufsize, fd);			
			mvwaddstr(win, k, 10, buffer);
			fclose(fd);

		}
		if(k>25)
		{
			wclear(win);
			k=5;

		}

		wrefresh(win);
	}
		noecho();

	endwin();

}
void nwDirty(int time_int)
{
	char c;
	WINDOW* curr = initscr();
	keypad(stdscr, TRUE);
	WINDOW * win; 
	win = newwin(800,600,1,1);
	noecho();
	curs_set(0);
	nodelay(win,1);
	int j = 10;
	int k = 0;
	int i;
	int start = time(NULL) , current,prevcurrent = time(NULL);
	k=5;
	unsigned long bufsize = 0;
	char *buffer;
	char label[200];
	FILE * fd;
	sprintf(label, "\t%s\tTime specified = %d s","NIGHTSWATCH -- dirty",time_int);
	while(1)
	{
		current = time(NULL);
		if(wgetch(win) == 'q')
			break;
		mvwaddstr(win,1,10,label);
		if((current- start)%time_int == 0 && current!=prevcurrent)
		{ 
			fd = fopen("/proc/meminfo", "r");
			prevcurrent = current;
			k += 1;
			fseek(fd, 0, SEEK_SET);	
			for(i=0;i<17;i++)		
				getline(&buffer, &bufsize, fd);			
			mvwaddstr(win, k, 10, buffer);
			fclose(fd);

		}
		if(k>25)
		{
			wclear(win);
			k=5;

		}

		wrefresh(win);
	}
		noecho();

	endwin();

}

int run_pinfo(char **args)
{
	// printf("IN Watch : %s\n", args[0]);
	return 1;
}

int run_watch(char **args)
{
	// printf("IN Watch : %s\n", args[0]);
	return 1;
}

int run_cd(char **args)
{
	// printf("IN CD : %s\n", args[0]);
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

	char **allOut = (char**)malloc(sizeof(char*)*2);
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
			all_bufsize = position + all_bufsize;
			// printf("%d - Updated All buf\n", all_bufsize );
      		allOut = (char **)realloc(allOut, all_bufsize * sizeof(char*));
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
		free(allOut[i]);
	}
}

char* echoOutLine(int *statePtr, char *iter)
{
	char *printBuffer = (char*)malloc(sizeof(char)*1000);
	int *position = (int *)malloc(sizeof(int));
	int state = *statePtr;
	*position = 0;
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
		// printf("%d\n", strlen(printBuffer) );
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
    // If we have exceeded the buffer, reallocate.
    if (*(position) >= pbufsize) {
      pbufsize = *(position) + pbufsize;
      printBuffer = (char*)realloc(printBuffer, pbufsize*sizeof(char));
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