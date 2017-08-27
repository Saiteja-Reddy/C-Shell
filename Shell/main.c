#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	char hostname[256];
	int i,j,flag;
	int userid = getuid();
	struct passwd *uinfo = getpwuid(userid);
	gethostname(hostname, 200);
	char cwd[1024];
	getwd(cwd);
	char *username = uinfo->pw_name ;
	int pos = 0;
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
	// printf("%d , %s\n",pos, username);
	printf("<%s@%s:~%s>", username, hostname, cwd+pos);
	return 0;
}