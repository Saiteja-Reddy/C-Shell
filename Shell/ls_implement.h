#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <uuid/uuid.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>


int run_ls(char **);
int printFileDir(char *, int []);
char f_type(mode_t);
char* getMonth(int);
void PrintFileInfo(struct stat);
long long getTotalBlocks(char*);