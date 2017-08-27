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
int pos;

int main(int argc, char const *argv[])
{
	int i,j,flag;
	int userid = getuid();
	struct passwd *uinfo = getpwuid(userid);
	gethostname(hostname, 200);
	getwd(cwd);
	strcpy(username,uinfo->pw_name) ;
	pos = 0;
	for (i = 0; i < strlen(cwd); ++i)
	{
		flag = 0;
		for (j = 0; j < strlen(username) ; ++j)
		{
			if(cwd[i + j] != username[j])
			{
				flag = 1;
				break;
			}
		}
		if(flag == 0)
		{
			pos = i+j;
			break;	
		}
	}
	shell_loop();
	return 0;
}

void shell_loop(void)
{
	char *in_line;

	while(1)
	{
		printf("<%s@%s:~%s>", username, hostname, cwd+pos);
		in_line = readCommand();
		printf("%s\n", in_line);
	}
}

char* readCommand(void)
{
	char *buffer = malloc(sizeof(char) * 128);
	int c = 0;
	int pos = 0;
	while(1)
	{
		c = getchar();
		if(c == EOF || c == '\n')
		{
			buffer[pos] = '\0';
			return buffer;
		}
		else
		{
			buffer[pos++] = c;
		}
	}
}