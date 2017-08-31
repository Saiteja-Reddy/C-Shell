#include <stdio.h>
#include <stdlib.h>

void myfunction(int* ptr)
{
	*ptr = 50;
}

int main(int argc, char const *argv[])
{
	int *ptr = (int*)malloc(sizeof(int));
	*ptr = 20;
	printf("%d\n", *ptr);
	myfunction(ptr);
	printf("%d\n", *ptr);
	free(ptr);
	return 0;
}