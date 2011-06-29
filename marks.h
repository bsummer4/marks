#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define S static
#define SI static inline
#define nil NULL
#define CASE(i,e) case i: e; break;
#define BAD(x) errx(1,"Error on line %d:\n\t%s",Line,x)
#define BUG BAD("You've been attacked by a vicious bug!")

/*
	## Output Module
	Output modules implement this interface. These functions behave
	similar to the way the grammar works. See `SPEC'.
*/
typedef enum {QUOTED,PLAIN,LINK,GROUP,ENDGROUP,WS} Wtype;
typedef struct { Wtype t; char *w; } Word;
void BEGIN(); void END();
void indent(); void undent(); void hdr(); void para();
void blist(); void nlist(); void lit(); void anno();
void item(char*); void bullet(); void hdrdepth(int); void word(Word);
void qline(char*);
