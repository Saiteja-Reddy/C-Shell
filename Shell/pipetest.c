#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>



int main(int argc, char const *argv[])
{
		int i;
		char *args1[] = {"ls"  , 0};
		char *args[] = {"wc" , 0};
		int pipe_p[2];
		pipe(pipe_p);

		// printf("%s \n " ,args1[1]);
		pid_t npid = fork();
		// printf("Child\n");

		if(npid == 0)
		{
			close(pipe_p[0]);
			dup2(pipe_p[1],1);
			if(execvp(args1[0], args1) == -1)
			{
				// printf("Child\n");
				perror("Shell");
			}
			abort();
		} 

		close(pipe_p[1]);
		dup2(pipe_p[0], 0);

	if(execvp(args[0], args) == -1)
			perror("Shell");

}