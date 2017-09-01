#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <curses.h>

int main(int argc, char const *argv[])
{
	pid_t pid = fork();
	if(pid == 0)
	{
		FILE* fd = fopen("/proc/interrupts", "r");
		unsigned long bufsize = 0;
		char *buffer;	
		getline(&buffer, &bufsize, fd);
		printf("%s\n", buffer);		
		while(1)
		{
			fseek(fd, 0, SEEK_SET);
			getline(&buffer, &bufsize, fd);
			getline(&buffer, &bufsize, fd);
			getline(&buffer, &bufsize, fd);
			printf("%s\n", buffer);	
			sleep(2);
			fclose(fd);
			fd = fopen("/proc/interrupts", "r");		
		}
		fclose(fd);
	}	
	else 
	{
		char c;
		while(1)
		{
			c = getch();
			printf("salkdmalksmdklamsdklmalkd - %c\n", c );
			if(c == 'q')
			{
				kill(pid, SIGKILL);
				exit(1);
			}
		}
	}
	return 0;
}