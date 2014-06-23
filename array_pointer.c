#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _strcpy(char *s, char *t)
{
	while (*t++ = *s++)
		;
}

void _strncpy(char *s, char *t, int n)
{
	
	while(n) {
		*t++ = *s++;
		n--;
	}
	*t = '\0';
}


void _strcat(char *big, char *small)
{
	while (*big != '\0')
		big++;
	while ((*big++ = *small++) != '\0')
		;
}

int main(void)
{
	char a[] = "Hello world\n";
	char *p = a;
	int i = 0;
	while (p[i] != '\0') {
		printf("%c", p[i]);
		i++;
	}
	/* The source string can be const string ie, char * because we are only
	 * going to iterate over the string. The destination has to be a char
	 * array or a pointer to malloc array, ie, it should have the required
	 * amount of memory available with it because the data is going to be
	 * copied to its location
	 */
	char *s = "Hello";
	char t[6];
	_strncpy(s, t, 1);
	printf("%s\n", t);
	s = "World";
	/* Char * can point to a new location because the pointer
	 * wasn't a const, only the string was.
	*/
	
	/* Testing with malloc'd block of data. We need to use strlen on char *
	 * to get the string size. sizeof will give the size of pointer. Also
	 * strlen gives length by ignoring \0, we need additional space for that
	 * as well, thus the +1 
	 */
	char *mp = (char *)malloc(strlen(s) + 1);
	_strcpy(s, mp);
	printf("%s\n", mp);
	char big[12] = "Hello";
	char *small = " World";
	_strcat(big, small);
	printf("Concat: %s\n", big);
}
