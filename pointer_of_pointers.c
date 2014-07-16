#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 50

char *buffer[MAXLINE];

int readlines(char **lineptr, int maxlines)
{
	char *p, line[80];
	int i = 0;
	while (i < 5) {
		scanf("%s", line);
		/* 
		 * Allocating memory to this pointer. This will result in this
		 * pointer getting memory in the heap and hence returning from
		 * this function won't make this memory garbage.
		 */
		p = malloc(80);
		/*Saving the string in heap memory and saving the heap memory's
		 * pointer in the pointer array
		 */
		strcpy(p, line);
		lineptr[i] = p;
		i++;
	}
	return i;
}

int main()
{
	int nlines = readlines(buffer, MAXLINE);
	int i = 0;
	while(i < nlines) {
		printf("%d : %s\n", i+1, buffer[i]);
		i++;
	}
}
