/* valid_nib.c -- checks for (portuguese only?) NIB validity */
/* $Id$ */
/* Carlos Duarte <cgd@teleweb.pt>, 000212 */


#include <ctype.h>
#include <string.h>

int valid_nib(char *str) 
{
	static int wtx_data[] = { 
		73,17,89,38,62,45,53,15,50,5,49,34,81,76,27,90,9,30,3
	}; 
	int wtt, i; 

	if (strlen(str) != 21) 
		return 0; 

	wtt = 0; 
	for (i=0; i<19; i++) {
		unsigned char c = str[i]; 
		if (!isdigit(c)) return 0; 
		c -= '0'; 
		wtt += c * wtx_data[i]; 
	}
	return 98 - wtt%97 == (str[19]-'0')*10+(str[20]-'0'); 
}

#ifdef TEST
#include <stdio.h>

int main(int argc, char *argv[])
{
	int i; 
	for (i=1; i<argc; i++) {
		char *s = argv[i]; 
		printf("%s: %s\n", s, valid_nib(s) ? "valid" : "not valid"); 
	}
	return 0; 
}

#endif
