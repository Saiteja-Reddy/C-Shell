#include "pinfo_implement.h"

void printProcess(pid_t pid)
{
	char  buffer[500];
	char  *tempbuffer;
	char exe_path[1024];
	unsigned long buf_size = 0;
	int i;
	sprintf(buffer,"/proc/%d/status",pid);
	FILE * fd = fopen(buffer,"r");
	if(fd == NULL)
	{
		printf(RED "There is no process with pid - %d\n" RESET, pid );
		return;
	}
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
		else if(*args_len >= 3)
		{
			printf(RED "pinfo: Too many arguments\nUsage: pinfo <pid>\n" RESET);
			return;
		}
		else
		{
			int i;
			for (i = 0; i < strlen(args[1]); ++i)
			{
				if(!(args[1][i] >= '0' && args[1][i] <= '9'))
				{
					printf(RED "pinfo: Please Enter a number for PID\n" RESET);
					return;
				}
			}
		 	pid= atoi(args[1]);
		 	printProcess(pid);
		 	//printf("%d ProcessInfo\n",pid);
		}

}



int run_pinfo(char **args)
{
	int i;
	int *count = (int*)malloc(sizeof(int));
	*count = 0;
	// printf("IN Watch : %s\n", args[0]);
	for ( i = 0; args[i] != NULL; ++i)
	{
		*count = *count + 1;
	}
	// printf("%d are no of args\n", count );
	printPinfo(args, count);	
	return 1;
}