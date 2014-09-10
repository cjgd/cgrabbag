/* die.h -- exiting non interactive programs */

/* $Id$ */
/* Carlos Duarte <cgd@teleweb.pt>, 990603/990610 */

/*
 * usage:
 *     	die_ctl(DIE_SET_PROG_NAME, argv[0]); 
 * 	...
 * 	if (someerror(s))
 * 		die("$0: some error on %s: $!", s); 
 * 
 * same for warning
 * available substitutions:
 * 	$0 - program name
 * 	$! - errno string
 * 	$e - error number
 * 	 
 * 	$$ - a single $$
 * 	 
 * 	$f - source file name
 * 	$l - source file number
 */

#ifndef DIE_H
#define DIE_H 

typedef enum { 
	DIE_SET_FILE,		/* char *fn: set die report filename as FN */
	DIE_SET_LINE,		/* int ln: set die report line as LN */
	DIE_SET_STREAM,		/* FILE *f: F is the stream die write to */
	DIE_GET_STREAM,		/* FILE **fp: fill *FP with current stream */
	DIE_SET_EXITCODE,	/* int code: die will exit with code CODE */
	DIE_GET_EXITCODE,	/* int *codep: get current code */
	DIE_SET_PROG_NAME,	/* char *fn: set prog name */
	DIE_CATCH,		/* void (*f)(char *): trap die to call f */
	DIE_PREFIX,		/* char *p: print p on beg of all msgs */
	DIE_SUFFIX,		/* char *s: print s at end of all msgs */
	DIE_USE_BASENAME	/* flag: use basename of prog_name */
} die_ctl_T; 

typedef enum {
	WARN_SET_FILE=99,	/* char *fn: set warn report filename */
	WARN_SET_LINE, 		/* int ln: set warn report line number */
	WARN_SET_STREAM,	/* FILE *f: set warn stream to write to */
	WARN_GET_STREAM,	/* FILE **fp: get warn stream */
	WARN_SET_PROG_NAME,	/* char *fn: set prog name */
	WARN_CATCH,		/* void (*f)(char *): trap warn to call f */
	WARN_PREFIX,		/* char *p: print p on beg of all msgs */
	WARN_SUFFIX,		/* char *s: print s at end of all msgs */
	WARN_USE_BASENAME	/* flag: use basename of prog_name */
} warn_ctl_T; 

#define DIE \
	die_ctl(DIE_SET_FILE, __FILE__),\
	die_ctl(DIE_SET_LINE, __LINE__),\
	die

#define WARN \
	warn_ctl(WARN_SET_FILE, __FILE__),\
	warn_ctl(WARN_SET_LINE, __LINE__),\
	warn


/* custom macro for setting up our die/warn behavior */
#ifndef DIE_SETUP

#define DIE_SETUP(argv0) do {                                                \
	die_ctl(DIE_SET_PROG_NAME, argv0);                                   \
	warn_ctl(WARN_SET_PROG_NAME, argv0);                                 \
                                                                             \
	/* \
	die_ctl(DIE_PREFIX, "$0: ");                                         \
	warn_ctl(WARN_PREFIX, "$0: ");                                       \
	*/ \
                                                                             \
	die_ctl(DIE_SUFFIX, ", at $f line $l");                              \
	warn_ctl(WARN_SUFFIX, ", at $f line $l");                            \
                                                                             \
	die_ctl(DIE_USE_BASENAME);                                           \
	warn_ctl(WARN_USE_BASENAME);                                         \
} while(0)

#endif

/* die.c */
int die_ctl(die_ctl_T cmd, ...);
int warn_ctl(warn_ctl_T cmd, ...);
void die(char *fmt, ...);
void warn(char *fmt, ...);

#endif /* DIE_H */

