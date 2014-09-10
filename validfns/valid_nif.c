/* valid_nif.c -- checks for NIF validity */
/* $Id$ */
/* Carlos Duarte <cgd@teleweb.pt>, 000212/001005 */

#include <ctype.h>
#include <string.h>

int valid_nif(char *str)
{
	int sum,r; 

	if (strlen(str) != 9)
		return 0; 

	sum = 0; 
	for (r=0; r<8; r++) {
		unsigned char c = str[r]; 
		if (!isdigit(c)) return 0; 
		c -= '0'; 
		sum += c * (10 - r - 1); 
	}
	r = sum%11;
	r = r < 2 ? 0 : 11-r; 
	return str[8] == r+'0'; 
}


#ifdef TEST

#include <stdio.h>
int main(int argc, char *argv[])
{
	int i; 
	for (i=1; i<argc; i++) {
		int s = valid_nif(argv[i]); 
		printf("%s: %s\n", argv[i], s ? "valid" : "not valid"); 
	}
	return 0; 
}

#endif
