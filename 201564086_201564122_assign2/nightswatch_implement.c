#include "nightswatch_implement.h"

int run_watch(char **args)
{
	// printf("IN Watch : %s\n", args[0]);
	return 1;
}

void printWatch(char **args, int* args_len)
{
	// printf("IN Print Watch : %s\n", args[0]);
	int opt;
	int index = 0;
	if(*args_len == 1)
	{
		printf(RED "nightswatch: Use nightswatch [options] <valid command>\nHere valid commands are 'interrupt' and 'dirty'\n" RESET);
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
				printf(RED "nightswatch: failed to parse argument: '%s'\n" RESET, args[index] );
				optind = 1;
				return;
			}
		}
		time_int =  atoi(args[index]);
	}
	if(index + 1 == *args_len)
	{
		printf(RED "nightswatch:\n Use nightswatch [options] <valid command>\n Here valid commands are 'interrupt' and 'dirty'\n" RESET);
		optind = 1;
		return;
	}

	if(strcmp(args[index + 1], "dirty") == 0)
	{
		nwDirty(time_int);
	}
	else if(strcmp(args[index + 1], "interrupt") == 0)
	{
		nwInterrupt(time_int);
	}
	else
	{
		printf(RED "nightswatch:\n Use nightswatch [options] <valid command>\n Here valid commands are 'interrupt' and 'dirty'\n" RESET);
		optind = 1;
		return;
	}

	optind = 1;
}
void nwInterrupt(int time_int)
{
	char c;
	WINDOW* curr = initscr();
	WINDOW * win; 
	win = newwin(800,600,1,1);
	keypad(win, TRUE);
	noecho();
	curs_set(0);
	nodelay(win,1);
	int j = 10;
	int k = 0;
	int i;
	int start = time(NULL) , current,prevcurrent = time(NULL);
	k=7;
	unsigned long bufsize = 0;
	char *buffer;
	char label[200];
	FILE * fd;
	FILE * fd1;
	sprintf(label, "\t%s\tTime specified = %d s","NIGHTSWATCH -- Interrupt",time_int);

	char *cpuinfo;
	fd = fopen("/proc/interrupts","r");
	getline(&cpuinfo,&bufsize,fd);
	fclose(fd);

	while(1)
	{
		current = time(NULL);
		if(wgetch(win) == 'q')
		{
			wclear(win);
			break;
		}
		mvwaddstr(win,1,10,label);
		mvwaddstr(win,3,10,cpuinfo);
		if((current- start)%time_int == 0 && current!=prevcurrent)
		{ 
			char *nowbuffer;
			fd1 = fopen("/proc/interrupts", "r");
			prevcurrent = current;
			k += 1;
			unsigned long nowbufsize = 0;
			fseek(fd1, 0, SEEK_SET);	
			getline(&nowbuffer, &nowbufsize, fd1);			
			getline(&nowbuffer, &nowbufsize, fd1);			
			getline(&nowbuffer, &nowbufsize, fd1);			
			mvwaddstr(win, k, 10, nowbuffer);
			fclose(fd1);

		}
		if(k>25)
		{
			wclear(win);
			k=5;

		}

		wrefresh(win);
	}
	noecho();
	curs_set(1);
	delwin(win);
	// delwin(curr);
	endwin();

}
void nwDirty(int time_int)
{
	char c;
	WINDOW* curr = initscr();
	WINDOW * win; 
	win = newwin(800,600,1,1);
	keypad(win, TRUE);
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
		{
			wclear(win);
			break;
		}
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
	curs_set(1);
	delwin(win);
	// delwin(curr);
	endwin();
}