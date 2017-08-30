#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

void shell_loop(void);
char* readCommand(void);

char hostname[256];
char username[256];
char cwd[1024];
char *wd;

int main(int argc, char const *argv[])
{
	int i,j,flag;
	int userid = getuid();
	struct passwd *uinfo = getpwuid(userid);
	gethostname(hostname, 200);
	getwd(cwd);
	strcpy(username,uinfo->pw_name) ;
	wd = strstr(cwd, username) + strlen(username);
	shell_loop();
	return 0;
}

void shell_loop(void)
{
	char *in_line;

	while(1)
	{
		printf("<%s@%s:~%s>", username, hostname, wd);
		in_line = readCommand();
		printf("%s\n", in_line);
	}
}

char* readCommand(void)
{
	unsigned long bufsize = 0;
	char *buffer;
	getline(&buffer, &bufsize, stdin);
	return buffer;
}

