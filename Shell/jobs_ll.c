#include "jobs_ll.h"
#include "pinfo_implement.h"
#include <sys/types.h>
#include <sys/wait.h>

void addtoLL(qjob* head, char name[], pid_t pid )
{
	qjob* curr = head;
	qjob* now;
	while(curr)
	{
		now = curr->next;
		if(now == NULL)
		{
			now  = (qjob*)malloc(sizeof(qjob));
			strcpy(now->name, name);
			now->pid = pid;
			now->next = NULL;
			curr->next = now;
			break;
		}
		curr = now;
	}
}

qjob* getjob(qjob* head, int Cpid)
{
	qjob* curr = head;
	qjob* now;
	int count = 0;
	while(curr)
	{
		now = curr->next;
		count += 1;
		if(now != NULL)
		{
			if(count == Cpid)
			{
				return now;
			}
		}
		curr = now;
	}
}

void printLLsize(qjob* head)
{
	qjob* curr = head->next;
	int count  = 0;
	while(curr)
	{
		count += 1;
		curr = curr->next;
	}
	printf("LL SIze - - %d\n", count );
}

void printLL(qjob* head)
{
	qjob* curr = head->next;
	int count  = 0;
	while(curr)
	{
		count += 1;
		printf("[%d] \t %s [%d]\n",count, curr->name, curr->pid );
		getState(curr->pid);
		curr = curr->next;
	}
}

void removeLL(qjob* head, pid_t pid)
{
	qjob* curr = head;
	qjob* now;
	while(curr)
	{
		now = curr->next;
		if(now != NULL)
		{
			if(now->pid == pid)
			{
				curr-> next = now->next;
				free(now);
				break;
			}
		}
		curr = now;
	}
}

int killLL(qjob* head, pid_t pid, int sig)
{
	qjob* curr = head;
	qjob* now;
	while(curr)
	{
		now = curr->next;
		if(now != NULL)
		{
			if(now->pid == pid)
			{
				curr-> next = now->next;
				kill(pid, sig);
				printf("Killed %d - %s\n" , pid, now->name);
				free(now);
				return 1;
				break;
			}
		}
		curr = now;
	}
	return 0;
}

void printDoneJobs(qjob* head)
{
	qjob* curr = head;
	qjob* now;
	while(curr)
	{
		now = curr->next;
		if(now != NULL)
		{
			pid_t return_pid = waitpid(now->pid, NULL, WNOHANG);
			if(return_pid == now->pid)
			{
				printf(KGRN "[-] Done %d %s\n" RESET, now->pid, now->name);
				curr-> next = now->next;
				free(now);
			}
		}
		curr = curr->next;
	}
}