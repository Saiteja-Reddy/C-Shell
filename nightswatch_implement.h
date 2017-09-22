#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <stdlib.h>

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


void nwInterrupt(int );
void printWatch(char **, int*);
void nwDirty(int);
int run_watch(char **);
