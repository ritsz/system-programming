#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 50

char *buffer[MAXLINE];

char *__read()
{
	printf("Enter string : ");
	char *p, line[80];
	scanf("%s", line);
	p = malloc(80);
	strcpy(p, line);
	printf("string saved at heap address %p", p);
	/* We are returning malloced pointer. Since the pointer points to a
	 * memory address in the heap, the memory won't have garbage when
	 * accessed post return. That happens with pointers on stack as the
	 * stack of the funtion is cleared on return.
	 */
	return p;
}

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
		p = (char *)malloc(80);
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
	char *buff = __read();
	/* The heap addresses should match */
	printf("\n%s was read at %p\n", buff, buff);

	int nlines = readlines(buffer, MAXLINE);
	int i = 0;
	while(i < nlines) {
		printf("%d : %s\n", i+1, buffer[i]);
		i++;
	}
}
