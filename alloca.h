#include <alloca.h>

int main()
{
	int i=0;
	int *int_ptr = alloca(10);
	for(; i < 10; i++)
		i[int_ptr] = (i*3) - (i/3);

	for(i = 0; i < 10; i++)
		printf("%d\n", int_ptr[i]);
	return 0;
}
