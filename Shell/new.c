#include<stdio.h>
int main()
{
	char str[10000];
		int i = 0,state=0;
	while(1)
	{
		state = 0;
		i=0;
	scanf("%[^\n]%*c",str);
	printf("%s\n",str);
	while(str[i]!='\0')
	{
		switch(state)
		{
			case 0:
				switch(str[i])
				{
					case 34:
						state = 2;
						break;
					case 39:
						state = 3;
						break;
					case 32:
						break;
					default:
					printf("%c", now);
						state = 1;
						break;
				}
				break;
			case 1:
				switch(str[i])
				{
					case 34:
						state = 2;
						break;
					case 39:
						state = 3;
						break;
					case 32:
					printf("%c", now);
						state = 0;
						break;
					default:
						printf("%c",str[i]);
						break;
				}
				break;
			case 2:
				switch(str[i])
				{
					case 34:
						state = 1;
						break;
					default:
					printf("%c", now);
						break;
				}
				break;

			case 3:
				switch(str[i])
				{
					case 39:
						state = 1;
						break;
					default:
					printf("%c", now);
						break;
				}
				break;
		}
		iter = iter + 1;
		now = *(iter);

		i++;
	}
	printf("\n");
}
}

