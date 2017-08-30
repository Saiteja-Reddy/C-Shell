#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <uuid/uuid.h>

#define TOKEN_BUFSIZE 64
#define DELIMITERS " \t\r\n\a"

void shell_loop(void);
char* readCommand(void);
int launchProcess(char **);
char **splitCommand(char *);
int executeCommand(char **);
int run_echo(char **);
int run_ls(char **);
int run_cd(char **);
int run_pwd(char **);
int run_exit(char **);
int printFileDir(char *, int []);
char f_type(mode_t);

char hostname[256];
char username[256];
char cwd[1024];
char *wd;
char *builtin[] = {"echo", "cd", "ls", "pwd", "exit"}; // HELP
int (*builtin_func[]) (char **) = {&run_echo, &run_cd, &run_ls, &run_pwd, &run_exit};

int main(int argc, char const *argv[])
{
	int i, j, flag;
	int userid = getuid();
	struct passwd *uinfo = getpwuid(userid);
	gethostname(hostname, 200);
	strcpy(username, uinfo->pw_name) ;
	shell_loop();
	return 0;
}

void shell_loop(void)
{
	char *in_line;
	char **args;
	int out = 1;
	while (out)
	{
		getwd(cwd);
		wd = strstr(cwd, username) + strlen(username);
		printf("<%s@%s:~%s >", username, hostname, wd);
		in_line = readCommand();
		args = splitCommand(in_line);
		out = executeCommand(args);
		// printf("%d\n", out);
	}
}

char* readCommand(void)
{
	unsigned long bufsize = 0;
	char *buffer;
	getline(&buffer, &bufsize, stdin);
	return buffer;
}

char **splitCommand(char *line)
{
	unsigned long bufsize = TOKEN_BUFSIZE;
	int position = 0;
	char **args = malloc(sizeof(char*) * bufsize);
	char *arg;
	if (!args)
	{
		fprintf(stderr, "shell: Allocation Error\n");
		exit(1);
	}

	arg = strtok(line, DELIMITERS);
	while (arg != NULL)
	{
		args[position++] = arg;
		arg = strtok(NULL, DELIMITERS);
	}
	args[position] = NULL;
	return args;
}

int launchProcess(char **args)
{
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		if (execvp(args[0], args) == -1)
		{
			perror("Shell"); // Print Approporiate Error
		}
		exit(1);
	}
	else if (pid > 0)
	{
		wait(NULL);
	}
	else
	{
		fprintf(stderr, "shell: Error Forking Child\n");
		exit(1);
	}
	return 1;
}

int executeCommand(char **args)
{
	if (args[0] == NULL)
		return 1;

	int i;
	int count = sizeof(builtin) / sizeof(char *);
	for (i = 0; i < count; ++i)
	{
		if (strcmp(builtin[i], args[0]) == 0)
			return (*builtin_func[i])(args);
	}

	// printf("%s\n", args[0]);
	return launchProcess(args);
}

int run_echo(char **args)
{
	printf("IN ECHO : %s\n", args[0]);
	return 1;
}

int run_cd(char **args)
{
	printf("IN CD : %s\n", args[0]);
	if (args[1] == NULL)
	{
		fprintf(stderr, "Please Enter an argument for cd\n");
		return 1;
	}
	if (chdir(args[1]) != 0)
	{
		perror("Shell");
	}
	return 1;
}

int run_ls(char **args)
{
	printf("IN LS : %s\n", args[0]);

	char *temp;
	int i = 1;
	int j;
	int flags[3] = {0, 0, 0};
	int fileRead = 0;
	do
	{
		temp = args[i];
		if (temp == NULL)
			break;
		if (temp[0] == '-' && !fileRead)
		{
			for (j = 1; j < strlen(temp); ++j)
			{
				switch (temp[j])
				{
				case 'a':
					flags[0] = 1;
					break;
				case 'l':
					flags[1] = 1;
					break;				
				default: // Other than -a / -l
					flags[2] = 1;
					break;
				}
			}
			if(strlen(temp) == 1)
			{
				fileRead = 1;
				printf(" - : No such file or directory\n");
			}
		}
		else
		{
			fileRead = 1;
			int return_val = printFileDir(temp, flags);
			if(return_val == 0)
			{
				i++;
				continue;
			}
		}
		i++;
		temp = args[i];
	} while (temp);

	if(fileRead == 0)
	{
		printFileDir(".", flags);
	}

	return 1;
}

int printFileDir(char *temp, int flags[])
{
	struct stat sb;
	if (stat(temp, &sb) == -1)
	{
		printf("%s: No such file or directory\n", temp);
		return 0;
	}
	if (S_ISDIR(sb.st_mode))
	{
		if(!(temp[0] == '.' || strlen(temp) == 1))
			printf("%s:\n", temp );
		struct dirent *direc_entry;
		DIR* direc_stream = opendir(temp);
		direc_entry = readdir(direc_stream);
		while (direc_entry)
		{
			if(flags[1] == 1)
			{
				struct stat fileStat;
				stat(direc_entry->d_name, &fileStat);
				printf("%c", f_type(fileStat.st_mode));
				printf((fileStat.st_mode & S_IRUSR)?"r":"-");
				printf((fileStat.st_mode & S_IWUSR)?"w":"-");
				printf((fileStat.st_mode & S_IXUSR)?"x":"-");
				printf((fileStat.st_mode & S_IRGRP)?"r":"-");
				printf((fileStat.st_mode & S_IWGRP)?"w":"-");
				printf((fileStat.st_mode & S_IXGRP)?"x":"-");
				printf((fileStat.st_mode & S_IROTH)?"r":"-");
				printf((fileStat.st_mode & S_IWOTH)?"w":"-");
				printf((fileStat.st_mode & S_IXOTH)?"x":"-");
				printf(" ");
				printf("%d ",fileStat.st_nlink);
				struct passwd *owner = getpwuid(fileStat.st_uid);
				struct group *group_own = getgrgid(fileStat.st_uid);
				printf("%s ", 	owner->pw_name);
				printf("%s ", 	group_own->gr_name);
				printf("%s\n", direc_entry->d_name);

			}
			else
			{
				printf("%s\n", direc_entry->d_name);
			}
			direc_entry = readdir(direc_stream);
		}
		closedir(direc_stream);
		printf("\n");
		return 1;
	}
	else
	{
		printf("%s\n", temp);
		return 1;
	}	
}

char f_type(mode_t mode)
{
    char c;
    switch (mode & S_IFMT)
    {
    case S_IFBLK:
        c = 'b';
        break;
    case S_IFCHR:
        c = 'c';
        break;
    case S_IFDIR:
        c = 'd';
        break;
    case S_IFIFO:
        c = 'p';
        break;
    case S_IFLNK:
        c = 'l';
        break;
    case S_IFREG:
        c = '-';
        break;
    case S_IFSOCK:
        c = 's';
        break;
    default:
        c = '?';
        break;
    }
    return (c);
}
int run_pwd(char **args)
{
	printf("IN PWD : %s\n", args[0]);
	printf("%s\n", cwd);
	return 1;
}

int run_exit(char **args)
{
	printf("IN EXIT : %s\n", args[0]);
	// exit(1);
	return 0;
}
