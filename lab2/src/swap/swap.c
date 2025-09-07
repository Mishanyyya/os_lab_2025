#include "swap.h"

void Swap(char *left, char *right)
{
	char c=' ';
	c = *left;
	*left = *right;
	*right = c;
}
