/* die_warn.c -- a simple die/warn function...  */

/* $Id$ */
/* Carlos Duarte <cgd@mail.teleweb.pt>, 990704 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

static char programname[88]; 

void set_progname(char *s) 
{
	char *p;

	if (!s)
		s = "<unknown>"; 

	p = strrchr(s, '/'); 
	if (p) {
		s = p+1; 
	}
	strncpy(programname, s, sizeof programname); 
	programname[sizeof(programname)-1] = '\0'; 
}

static void _die(int code, char *msg, va_list ap) 
{
#define sz 1024
	char mymsg[sz+2]; 	/* extra \n and \0 */
	int i; 
	char *str; 
	char *s = msg; 

	for (i=0; *s && i<sz; ) {
		if (s[0] != '$') {
literal: 
			mymsg[i++] = *s++;
			continue; 
		}
		switch (s[1]) {
		case '!': 
			str = strerror(errno); 
			break; 
		case '0': 
			str = programname; 
			break; 
		default: 
			goto literal; 
		}
		s += 2; 
		while (i<sz && *str) 
			mymsg[i++] = *str++; 
	}
	mymsg[i] = 0; 
	s = &mymsg[*mymsg ? strlen(mymsg)-1 : 0];
	if (*s != '\n') {
		*++s = '\n'; 
		*++s = '\0'; 
	}

	vfprintf(stderr, mymsg, ap); 
	if (code)
		exit(code);

#undef sz
}

void die(char *msg, ...) 
{
	va_list ap; 
	va_start(ap, msg); 
	_die(1, msg, ap); 
	va_end(ap); 
}

void warn(char *msg, ...) 
{
	va_list ap; 
	va_start(ap, msg); 
	_die(0, msg, ap); 
	va_end(ap); 
}

#ifdef TEST

int main(int argc, char *argv[]) 
{
	FILE *f; 
	int c; 

	set_progname(argv[0]); 
	if (argc != 2)
		die("usage: $0 file"); 
	if ((f = fopen(argv[1], "r")) == NULL)
		die("$0: %s: $!", argv[1]); 
	while ((c = getc(f)) != EOF) 
		putchar(c); 
	if (ferror(f))
		die("error on %s: $!", argv[1]); 
	if (fclose(f) != 0)
		warn("can't close %s: $!", argv[1]); 
	exit(0); 
	return 0; 
}

#endif
