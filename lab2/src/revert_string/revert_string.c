#include "revert_string.h"
#include "string.h"

void RevertString(char *str) {
	char* start = str;
	char* end = str + strlen(str) - 1;
	
	while (start < end) {
		char temp = *start;
		*start = *end;
		*end = temp;

		start++;
		end--;
	}
}


