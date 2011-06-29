/*
	# marks
	This is a translator from the marks language (as defined in
	README) to troff with the -ms macros.

	## Unicode support
	utf8 is used for input, output, and internal processing. Since
	there are no sane libraries for unicode, we have to handle
	everything ourselves. It's complicated and annoying.

	## TODO
	:TODO: Support utf8 better
		- :TODO: multicol chars
			Some unicode chars fill two or even zero
			columns. The current code assumes they all fill
			exactly one.

	:TODO: Reject unprintable characters (use `isprint(3)').
	:TODO: Handled `inline-quoted-text'.
	:TODO: Handle macro syntax
	:TODO: Separate processing and output code.
	:TODO: More-correct Output.
		Why are continuation lines and indented lines aligned
		in the output?
*/

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

#define UTF8MAX 6
#define BUFSIZE UTF8MAX*78+2
#define UTF8LEN(ch) ((unsigned char)ch>=0xFC ? 6 : \
                     ((unsigned char)ch>=0xF8 ? 5 : \
                      ((unsigned char)ch>=0xF0 ? 4 : \
                       ((unsigned char)ch>=0xE0 ? 3 : \
                        ((unsigned char)ch>=0xC0 ? 2 : 1)))))

/*
	'state.n' is a modifier for 'state.state', it means different
	things for different states.  For named and unordered lists it
	represents the number of characters before the text starts,
	and for headers it represents the header depth.  The typedef
	'S' is only for brevity in structure literals.
*/
enum { NONE=0, P, LIST, OL, QUOTE, HEADER };
typedef struct state { int state, n; } St;
S struct state State[9] = {0};
S int Level = 0;

S void exitstate () {
	switch (State[Level].state) {
	case QUOTE: puts(".P2"); break; }
	State[Level].state = NONE; }

S void enterstate (int s, int n) {
	if (s==State[Level].state) return;
	exitstate();
	switch (s) {
	case P: puts(".LP"); break;
	case QUOTE: puts(".P1"); break;
	case HEADER: printf(".NH %d\n", n); break; }
	State[Level] = (St){s, n}; }

/*
	Sets 'Level'; outputs indentation codes; returns 'l' with leading
	tabs removed.
*/
S char *indent (char *l) {
	int c;
	for (c=0; '\t'==*l; c++,l++);
	if (c>9) errx(1, "Only 9 levels of indentation are allowed.");
	while (Level>c) exitstate(),puts(".RE"),Level--;
	while (Level<c) exitstate(),puts(".RS"),Level++;
	return l; }

/*
	This sets *n to the number of characters before the non-key text
	starts, sets *key to the text of the key with leading and trailing
	'.'s removed, and sets *val to the beginning of the non-key text.
	Note that key and val will be pointers into buf, and buf will
	be modified in undefined ways.
*/
S void dotkey (char *buf, int *n, char **key, char **val) {
	char *bad = "Invalid ordered list %d";
	if ('.'!=*buf) errx(1, bad, 1);
	*key = buf+1;
	/* The length of the key.  Doesn't include the training space.  */
	int keylen;
	{
		char *w=strchr(*key, ' ');
		keylen=w?(w-*key):strlen(*key); }
	*n = keylen+2;
	/* 2 because 'keylen' doesn't include leading . or trailing ' '.  */
	assert(!(*key)[keylen] || ' '==(*key)[keylen]);
	if (keylen<=1) errx(1, bad, 2);
	if ('.'==**key) errx(1, bad, 3);
	if ((*key)[keylen-1]!='.') errx(1, bad, 4);
	/*
		Set 'val' to the text directly after the key.  Skip the
		separating space unless the line ends directly after
		the key.
	*/
	*val=(*key)+keylen;
	if (**val) (*val)++;
	if (isspace(**val)) errx(1, bad, 5);
	/* Nullify all '.'s at the end of the key.  */
	while (keylen && '.'==(*key)[keylen-1]) (*key)[--keylen]='\0'; }

/*
	The number of columns taken by a UTF-8 string. We assume 8-column
	tabstops.

	:TODO: Tricky
		Make sure this `unsigned char' business is correct.
*/
SI unsigned cols (unsigned char *str) {
	char *utfe= "Unicode support is planned but not written.";
	unsigned chr,c=0;
	while (*str) {
		chr=*str;
		c += ('\t'!=chr?1:8-c%8);
		/*
			:TODO: Huge Fucking Security Hole!
				This *HAS* to be validated!
		*/
		str += UTF8LEN(*str);
	}
	while ((chr=*str++))
		if (127<chr) errx(1,utfe);
	return c; }

S void pedant (char *line) {
	char *nospaces = "No spaces are allowed at the end of a line.";
	char *notabs = "No tabs are allowed except at the beginning of lines.";
	char *c;
	int l;
	c=line;
	fprintf(stderr, "cols: %d len: %d\n", cols(line), strlen(line));
	if (78<cols(line)) errx(1,"All lines must fit within 78 columns");
	l=strlen(line);
	if (l && isspace(line[l-1]))
		errx(1, nospaces);
	while ('\t'==*c) c++;
	if ('|'==*c) return;
	for (; *c; c++) if ('\t'==*c) errx(1, notabs); }

S int count (char pre, char *l) {
	int x=0;
	while (pre==*l++) x++;
	return x; }

S void emit (char *l) {
	int len=strlen(l), br=(len>=2 && !strcmp(l+(len-2), "\\\\"));
	if (br) l[len-2]='\0';
	printf("%s%s\n", l, br?"\n.BR":""); }

S void input (char *l) {
	char *over = "Continuation line has too many leading spaces.";
	char *under = "Continuation line doesn't have enough leading spaces.";
	pedant(l);
	l=indent(l);
	bool cont=false;
	switch(*l) {
	case '\0': enterstate(NONE, -1); break;
	case '-': enterstate(LIST, 0); break;
	case '.': enterstate(OL, 0); break;
	case '|': enterstate(QUOTE, -1); break;
	case '#': enterstate(HEADER, count('#', l)); break;
	case ' ': {
		cont = true;
		int s = State[Level].state;
		if (LIST!=s && OL!=s) errx(1, "Bogus whitespace");
		for (int ii=0; ii<State[Level].n; ii++,l++)
			if (*l!=' ') errx(1, under);
		if (*l==' ') errx(1, over);
		break; }
	default: enterstate(P, -1); }

	switch (State[Level].state) {
	case NONE: return;
	case OL: {
		S const char *align =
		 "Text in ordered lists must be aligned.";
		int *n = &State[Level].n;
		char *key=NULL, *value=l;
		if (!cont) {
			int stash = *n;
			dotkey(l, n, &key, &value);
			if (stash && stash!=*n) errx(1, align); }
		if (!cont) printf(".IP %s) %d\n", key, *n-1);
		emit(value);
		break; }

	case LIST: {
		int *n = &State[Level].n;
		if (!cont) {
			if ('-'!=*l++) errx(1, "bad list.");
			if (' '!=*l++) errx(1, "bad list.");
			if (isspace(*l)) errx(1, "bad list.");
			/* TODO Should this go in enterstate()? */
			printf(".IP - 2\n");
			*n=2; }
		emit(l);
		break; }

	case P: emit(l); break;
	case HEADER:
		while ('#'==*l) l++;
		if (' '!=(*l++)) errx(1, "bad header.");
		puts(l),exitstate();
		break;

	case QUOTE:
		if ('|'!=*l++) errx(1, "bad quote.");
		if (!*l) { puts(""); break; }
		if (!isspace(*l++)) errx(1, "bad quote.");
		printf(" ");
		puts(l);
		break; }}

/*
	Returns a pointer to static memory.
*/
SI char *getline () {
	char *errlast = "The last character of every file must be a newline.";
	const int last = BUFSIZE-2;
	S char buf[BUFSIZE];
	buf[last]='\0';
	if (!fgets(buf, BUFSIZE, stdin)) return nil;
	assert(0<strlen(buf));
	if ('\n'!=buf[last] && '\0'!=buf[last])
		errx(1, "Lines must take less than %d bytes.", BUFSIZE-1);
	int l=strlen(buf);
	if ('\n'!=buf[l-1]) errx(1, errlast);
	buf[l-1]='\0';
	return buf; }

int main (int argc, char *argv[]) {
	char *line;
	while ((line=getline())) input(line);
	input("");
	return 0; }
