#include "ls_implement.h"

int run_ls(char **args)
{
	// printf("IN LS : %s\n", args[0]);
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
		char full_path[1000];
		printf("%s:\n", temp);
		if(flags[1] == 1)
		{
			printf("total: %lld\n", getTotalBlocks(temp));
		}
		direc_entry = readdir(direc_stream);
		while (direc_entry)
		{
				char* fileName = direc_entry->d_name;
				if(fileName[0] != '.' || flags[0] == 1 )
				{				
					struct stat fileStat;
					if(flags[1] == 1)
					{
						sprintf(full_path, "%s/%s",temp,fileName); // Important to append Path
						lstat(full_path, &fileStat); // Important to put lstat
						// printf("%d\n", fileStat.st_blocks );
						PrintFileInfo(fileStat);
					}
					if(S_ISLNK(fileStat.st_mode) && flags[1] == 1)
					{
						char linkpoints[1024];
						int temp = readlink(full_path, linkpoints, 1024);
						if(temp != -1)
						{
							linkpoints[temp] = '\0';
							printf("%s -> %s \n",fileName,linkpoints);
						}
						else
						{
							printf("%s\n", fileName);
						}
					}
					else
					{
						printf("%s\n", fileName);
					}

				}
			direc_entry = readdir(direc_stream);
		}
		closedir(direc_stream);
		printf("\n");
		return 1;
	}
	else
	{
		if(flags[1] == 1)
		{		
			struct stat fileStat;
			stat(temp, &fileStat);
			PrintFileInfo(fileStat);
		}		
		printf("%s\n", temp);
		return 1;
	}	
}

long long getTotalBlocks(char* temp)
{
	if(!(temp[0] == '.' || strlen(temp) == 1))
		printf("%s:\n", temp );
	struct dirent *direc_entry;
	DIR* direc_stream = opendir(temp);
	char full_path[1000];
	direc_entry = readdir(direc_stream);
	long long total_blocks = 0;
	while (direc_entry)
	{
			char* fileName = direc_entry->d_name;
					struct stat fileStat;
					sprintf(full_path, "%s/%s",temp,fileName); // Important to append Path
					lstat(full_path, &fileStat); // Important to put lstat
					total_blocks +=fileStat.st_blocks;
		direc_entry = readdir(direc_stream);
	}
	closedir(direc_stream);
	return total_blocks;
}

void PrintFileInfo(struct stat fileStat)
{
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
	printf("%lu ",fileStat.st_nlink);
	struct passwd *owner = getpwuid(fileStat.st_uid);
	struct group *group_own = getgrgid(fileStat.st_gid);
	printf("%s ", 	owner->pw_name);
	printf("%s ", 	group_own->gr_name);
	printf("%lu ", 	fileStat.st_size);
	struct tm* lastM_time;
	lastM_time = localtime(&fileStat.st_mtime);
	printf("%s ", 	getMonth(lastM_time->tm_mon));
	printf("%d ", 	lastM_time->tm_mday);
	printf("%d:", 	lastM_time->tm_hour);
	if(lastM_time->tm_min < 10)
		printf("0");
	printf("%d ", 	lastM_time->tm_min);
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

char* getMonth(int i)
{
	char *month;
    switch (i)
    {
    case 0:
        month = "Jan";
        break;
    case 1:
        month = "Feb";
        break;
    case 2:
        month = "Mar";
        break;
    case 3:
        month = "Apr";
        break;
    case 4:
        month = "May";
        break;
    case 5:
        month = "Jun";
        break;
    case 6:
        month = "Jul";
        break;
    case 7:
        month = "Aug";
        break;
    case 8:
        month = "Sep";
        break;
    case 9:
        month = "Oct";
        break;
    case 10:
        month = "Nov";
        break;
    case 11:
        month = "Dec";
        break;        
    default:
        month = "?";
        break;
    }
    return (month);
}