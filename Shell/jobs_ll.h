#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

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

struct node
{
	char name[1000];
	pid_t pid;
	struct node *next;
};
typedef struct node qjob;

void addtoLL(qjob*, char [], pid_t);
void printLL(qjob*);
void removeLL(qjob*, pid_t);
int killLL(qjob* , pid_t , int );
void printDoneJobs(qjob* );
void printLLsize(qjob*);
qjob* getjob(qjob* , int );