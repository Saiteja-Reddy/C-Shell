#include "echo_implement.h"
#include <string.h>

void checkEcho(char *buffer)
{
	unsigned long bufsize = 0;
	char *iter = buffer;
	iter = iter + 5;
	int pbufsize = 10;
	int *statePtr = malloc(sizeof(int));
	*statePtr = 0;
	char *printBuffer;
	printBuffer = echoOutLine(statePtr, iter);
	// printf("%s", printBuffer);

	char **allOut = (char**)malloc(sizeof(char*)*2);
	int all_bufsize = 2;
	int position = 0;
	allOut[position++] = printBuffer;

	// printf("%d\n", state);
	while(*statePtr == 2 || *statePtr == 3)
	{
		printf(">");
		getline(&buffer, &bufsize, stdin);
		printBuffer = echoOutLine(statePtr, buffer);	

		allOut[position++] = printBuffer;		
		if(position >= all_bufsize)
		{
			all_bufsize = position + all_bufsize;
			// printf("%d - Updated All buf\n", all_bufsize );
      		allOut = (char **)realloc(allOut, all_bufsize * sizeof(char*));
			if (!allOut) {
			    fprintf(stderr, "Shell: allOut reallocation error\n");
			    exit(EXIT_FAILURE);
			  }      		
		}
	}	
	int i;
	for (i = 0; i < position; ++i)
	{
		printf("%s", allOut[i]);
		free(allOut[i]);
	}
}

char* echoOutLine(int *statePtr, char *iter)
{
	char *printBuffer = (char*)malloc(sizeof(char)*1000);
	int *position = (int *)malloc(sizeof(int));
	int state = *statePtr;
	*position = 0;
	char now = *(iter);
	while(now != '\0')
	{
		switch(state)
		{
			case 0:
				switch(now)
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
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						state = 1;
						break;
				}
				break;
			case 1:
				switch(now)
				{
					case 34:
						state = 2;
						break;
					case 39:
						state = 3;
						break;
					case 32:
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						state = 0;
						break;
					default:
					printBuffer = addToPrintBuf(printBuffer, now, position);
						// printf("%c",now);
						break;
				}
				break;
			case 2:
				switch(now)
				{
					case 34:
						state = 1;
						break;
					default:
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						break;
				}
				break;

			case 3:
				switch(now)
				{
					case 39:
						state = 1;
						break;
					default:
					printBuffer = addToPrintBuf(printBuffer, now, position);
					// printf("%c", now);
						break;
				}
				break;
		}
		iter = iter + 1;
		now = *(iter);
		// printf("%d\n", strlen(printBuffer) );
	}
	// return state;
	*(statePtr) = state;
	printBuffer[*(position)] = '\0';
	return printBuffer;
}

char* addToPrintBuf(char *printBuffer, char now, int *position)
{
	int pbufsize = 1000;
	char c = now;
    printBuffer[*(position)] = c;
	*(position) = *(position) + 1;
    // If we have exceeded the buffer, reallocate.
    if (*(position) >= pbufsize) {
      pbufsize = *(position) + pbufsize;
      printBuffer = (char*)realloc(printBuffer, pbufsize*sizeof(char));
      if (!printBuffer) {
        fprintf(stderr, "Shell: printBuffer reallocation error\n");
        exit(EXIT_FAILURE);
      }
    }	
    return printBuffer;
}

int run_echo(char **args)
{
	int i;
	int *count = (int*)malloc(sizeof(int));
	*count = 0;
	char *buffer = (char*)malloc(sizeof(char) * 1000);
	strcpy(buffer,"echo");
	// printf("IN Echo : %s\n", args[0]);
	for ( i = 1; args[i] != NULL; ++i)
	{
		*count = *count + 1;
		strcat(buffer," ");
		strcat(buffer,args[i]);
		// printf("%s\n", args[i]);
	}
	strcat(buffer,"\n");
	// printf("%d\n", *count);
	// printf("%s\n", buffer);
	checkEcho(buffer);

	// printf("Done in Echo\n" );
	return 1;
}