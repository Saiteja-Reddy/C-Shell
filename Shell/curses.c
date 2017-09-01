#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <curses.h>
#include <time.h>

int main(int argc, char const *argv[])
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

			FILE* fd = fopen("/proc/interrupts", "r");
			fseek(fd, 0, SEEK_SET);			
			getline(&cpuinfo, &bufsize, fd);
			fclose(fd);
	while(1)
	{
		current = time(NULL);
		if(wgetch(win) == 'q')
			break;
		mvwaddstr(win,1,10,"NIGHTSWATCH -- keyboard interrupts");
		mvwaddstr(win, 3, 10, cpuinfo);					
		if((current- start)%2 == 0 && current!=prevcurrent)
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
//		printf("%s\n", buffer);	
//		sleep(2);
		noecho();

	endwin();
 	return 0;
}