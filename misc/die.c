/* die.c -- die() and warn(), for exiting non interactive programs */

/* $Id$ */
/* Carlos Duarte <cgd@teleweb.pt>, 990603 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "die.h"

#define ASIZE 99 		/* for small buffers */
#define BSIZE 1024		/* for medim buffers */

static const char *null_str = "<null>"; 

/* die variables */
static struct die_stuff {
	char *file; 
	unsigned int line; 
	FILE *stream; 
	int exitcode; 
	char prog_name[ASIZE]; 
	void (*catcher)(char *);
	char *prefix; 
	char *suffix; 

	int inited; 
	int nr_calls; 
} ds; /* die stuff */

/* warn vars */
static struct warn_stuff {
	char *file; 
	unsigned int line; 
	FILE *stream; 
	char prog_name[ASIZE]; 
	void (*catcher)(char *);
	char *prefix; 
	char *suffix; 

	int inited; 
	int nr_calls;
} ws; /* warn stuff */

static void
init_die(void) {
	if (ds.inited)
		return; 
	ds.inited++; 
	ds.stream = stderr; 
	ds.exitcode = 2; 
	strcpy(ds.prog_name, null_str); 
}

static void
init_warn(void) {
	if (ws.inited)
		return; 
	ws.inited++; 
	ws.stream = stderr; 
	strcpy(ws.prog_name, null_str); 
}

/* str: "/opt/foo/bar" -> "bar" */
static void
basenamify(char *str) {
	char *s, *t; 
	s=t=str; 
	while (*s) {
		if (*s == '/')
			t=s; 
		s++; 
	}
	if (t != str) {
		s = str; 
		do *s++ = *++t; while (*t); 
	}
}

int
die_ctl(die_ctl_T cmd, ...) { 
	va_list ap; 

	init_die(); 

	va_start(ap, cmd); 
	switch (cmd) {
	case DIE_SET_FILE: 
		ds.file = va_arg(ap, char *); 
		break; 
	case DIE_SET_LINE: 
		ds.line = va_arg(ap, unsigned int); 
		break; 
	case DIE_SET_EXITCODE: 
		ds.exitcode = va_arg(ap, int); 
		break;
	case DIE_GET_EXITCODE: {
		int *codep = va_arg(ap, int *); 
		*codep = ds.exitcode; 
		break; 
	}
	case DIE_SET_STREAM: 
		ds.stream = va_arg(ap, FILE *); 
		break; 
	case DIE_GET_STREAM: {
		FILE **fp = va_arg(ap, FILE **); 
		*fp = ds.stream; 
		break; 
	}
	case DIE_SET_PROG_NAME: {
		char *s = va_arg(ap, char *); 
		if (s)
			strncpy(ds.prog_name, s, ASIZE); 
		ds.prog_name[ASIZE-1] = '\0'; 
		break; 
	}
	case DIE_CATCH: {
		void (*f)(char *) = va_arg(ap, void *); 
		ds.catcher = f; 
		break; 
	}
	case DIE_PREFIX: 
		ds.prefix = va_arg(ap, char *); 
		break; 
	case DIE_SUFFIX: 
		ds.suffix = va_arg(ap, char *); 
		break; 
	case DIE_USE_BASENAME: 
		basenamify(ds.prog_name); 
		break;
	} /* switch */
	va_end(ap); 
	return 0; 	/* returns ok */
}

int
warn_ctl(warn_ctl_T cmd, ...) { 
	va_list ap; 

	init_warn(); 

	va_start(ap, cmd); 
	switch (cmd) {
	case WARN_SET_FILE: 
		ws.file = va_arg(ap, char *); 
		break; 
	case WARN_SET_LINE: 
		ws.line = va_arg(ap, int); 
		break; 
	case WARN_SET_STREAM: 
		ws.stream = va_arg(ap, FILE *); 
		break; 
	case WARN_GET_STREAM: {
		FILE **fp = va_arg(ap, FILE **); 
		*fp = ws.stream; 
		break; 
	}
	case WARN_SET_PROG_NAME: {
		char *s = va_arg(ap, char *); 
		if (s)
			strncpy(ws.prog_name, s, ASIZE); 
		ws.prog_name[ASIZE-1] = '\0'; 
		break; 
	}
	case WARN_CATCH: {
		void (*f)(char *) = va_arg(ap, void *); 
		ws.catcher = f; 
		break; 
	}
	case WARN_PREFIX: 
		ws.prefix = va_arg(ap, char *); 
		break; 
	case WARN_SUFFIX: 
		ws.suffix = va_arg(ap, char *); 
		break; 
	case WARN_USE_BASENAME: 
		basenamify(ws.prog_name); 
		break;
	} /* switch */
	va_end(ap); 
	return 0; 	/* returns ok */
}

/*
 * do the following replacements, while copying SOURCE to DEST
 * 	$! 	errno string
 * 	$e	error number
 * 	$0	program name
 * 	$f	source file name
 * 	$l	source file number
 * 	$$	a single '$'
 * 
 * if from==0; use die variables
 * if from==1; use warn variables
 */
static void
cvt_fmt(char *source, char *dest, int dlen, int from) {
#define VAR(x) 	(from==0?ds.x:ws.x)
#define _ADD(x) if (--dlen<=0) goto end; else *dest++ = (x) 
#define ADD(x)	do { if ((x) == '%') _ADD('%'); _ADD((x)); } while(0)
#define ADD_STR(s) \
		do {if(!s) s=null_str; while(*s) { ADD(*s); s++; } } while(0)
#define ADD_NUM(d) do {                                                      \
	int e = d;                                                           \
	int len = 1;                                                         \
	while (e>9) {                                                        \
		e /= 10;                                                     \
		len++;                                                       \
	}                                                                    \
	if (dlen>len)                                                        \
		sprintf(dest, "%d", d);                                      \
	else                                                                 \
		goto end;                                                    \
	dest += len;                                                         \
} while(0)

	const char *s; 
	int c, d; 
	char *orig_dest = dest; 

	while ((c = *source++) != 0) {
		if (c != '$') {
			_ADD(c);
			continue; 
		}
		c = *source++; 
		switch (c) {
		case '!': 
			s = strerror(errno); 
			ADD_STR(s); 
			break; 
		case 'e': 
			ADD_NUM(errno); 
			break; 
		case '0':
			s = VAR(prog_name); 
			ADD_STR(s); 
			break; 
		case 'f': 
			s = VAR(file); 
			ADD_STR(s); 
			break; 
		case 'l': 
			d = VAR(line); 
			ADD_NUM(d); 
			break; 
		case '$': 
			ADD('$'); 
			break; 
		case '\0':
			/* ignore "...$" */
			break; 
		default: 
			/* "... $x" gets replaced by literal $x */
			ADD('$'); 
			ADD(c); 
			break; 
		}
	}
end:
	/* strip tailing newlines */
	while (dest > orig_dest && dest[-1] == '\n')
		dest--; 
	*dest = '\0'; 
}

/* if form==0, do die; else do warn; */
static void 
dodie(char *fmt, va_list ap, int from) {
	char pre_suf[BSIZE] = ""; 
	char new_fmt[BSIZE]; 

	/* fmt -> prefix fmt suffix, if applicable */
	if (VAR(prefix) && strlen(VAR(prefix))+strlen(pre_suf)<BSIZE)
		strcat(pre_suf, VAR(prefix)); 
	if (fmt && strlen(fmt)+strlen(pre_suf)<BSIZE)
		strcat(pre_suf, fmt); 
	if (VAR(suffix) && strlen(VAR(suffix))+strlen(pre_suf)<BSIZE)
		strcat(pre_suf, VAR(suffix)); 
	if (fmt && pre_suf[0] != '\0')
		fmt = pre_suf; 

	if (fmt) {
		cvt_fmt(fmt, new_fmt, BSIZE, from); 
		fmt = new_fmt; 
	}

	/* is a catcher set, and is this a first call? */
	if (VAR(catcher) && VAR(nr_calls)==1) {
		/* yep: build fmt,ap buffer, and call catcher with it */
		char my_buf[BSIZE]; 
		char *buf; 

		if (fmt) {
#ifdef HAVE_VSNPRINTF
			vsnprintf(my_buf, BSIZE, new_fmt, ap);
#else
			/*  WARN: might cause buffer overflow */
			vsprintf(my_buf, new_fmt, ap);
#endif
			buf = my_buf; 
		} else
			buf = 0; 

		/* catcher called with NULL, or contructed buf */
		if (from==0)
			ds.catcher(buf); 
		else
			ws.catcher(buf);
		return; /* does not continue, after the catcher */
	}

	/* no catch done: just print the stuff */
	if (VAR(stream) && fmt) {
		vfprintf(VAR(stream), fmt, ap); 
		fprintf(VAR(stream), "\n"); 
		fflush(VAR(stream));
	}

	/* reset source line and file info of current call */
	ds.file = ws.file = 0; 
	ds.line = ws.line = 0; 

	/* exit if a die call */
	if (from==0)
		exit(ds.exitcode); 
}

void die(char *fmt,...) {
	va_list ap; 

	init_die(); 
	ds.nr_calls++; 
	va_start(ap, fmt); 
	dodie(fmt, ap, 0); 
	va_end(ap);
	ds.nr_calls--; 
}

void warn(char *fmt,...) {
	va_list ap; 

	init_warn(); 
	ws.nr_calls++; 
	va_start(ap, fmt); 
	dodie(fmt, ap, 1); 
	va_end(ap);
	ws.nr_calls--; 
}

#ifdef TEST
static void 
die_catcher(char *msg) {
	printf("custom die entered with msg: %s\n", msg); 
	/* ... */
	printf("calling die(0) now\n"); 
	die(0); 
	printf("should not arrive here\n"); 
}

static void
warn_catcher(char *msg) {
	printf("custom warn entered with msg: %s\n", msg); 
	/* ... */
	printf("calling warn to print real message...\n");
	warn(msg); 
	printf("should _arrive_ here\n"); 
}

int
main(int argc, char *argv[]) {
	int c; 
	FILE *f; 

	die_ctl(DIE_SET_PROG_NAME, argv[0]); 

	die_ctl(DIE_SUFFIX, ", at $f line $l"); 
	warn_ctl(WARN_SUFFIX, ", at $f line $l"); 

	if (argc != 2) 
		DIE("usage: $0 filename");

	if ((f = fopen(argv[1], "r")) == NULL)
		DIE("can't open %s for read: $!", argv[1]); 

	while ((c = getc(f)) != EOF)
		putchar(c); 

	if (ferror(f)) 
		DIE("error on %s stream: $!", argv[1]); 

	if (fclose(f) != 0) 
		WARN("could not close %s stream: $!", argv[1]); 

	warn_ctl(WARN_CATCH, warn_catcher);
	WARN("will use custom warn-- prog: $0, file: %s", argv[1]);

	warn_ctl(WARN_CATCH, 0); 
	WARN("a normal warning :)"); 

	die_ctl(DIE_CATCH, die_catcher);
	DIE("will use custom die-- prog: $0, file: %s", argv[1]); 

	return 0; /* ok */
}
#endif
