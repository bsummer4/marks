/*
	# marks
	This is a translator from the marks language (as defined in
	README) to troff with the -ms macros.
*/

// :TODO: Support utf8
// :TODO: Handle macro syntax
// :TODO: @ Handle -- phrase lists
// :TODO: More-correct Output.
// 	Why are continuation lines and indented lines aligned in the
// 	output?

#include <err.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/*
	'state.n' is a modifier for 'state.state', it means different
	things for different states.  For named and unordered lists it
	represents the number of characters before the text starts,
	and for headers it represents the header depth.  The typedef
	'S' is only for brevity in structure literals.
*/
enum { NONE=0, P, LIST, OL, QUOTE, HEADER };
typedef struct state { int state, n; } S;
struct state State[9] = {0};
int Level = 0;

static void exitstate () {
	switch (State[Level].state) {
	case QUOTE: puts(".P2"); break; }
	State[Level].state = NONE; }

static void enterstate (int s, int n) {
	if (s==State[Level].state) return;
	exitstate();
	switch (s) {
	case P: puts(".LP"); break;
	case QUOTE: puts(".P1"); break;
	case HEADER: printf(".NH %d\n", n); break; }
	State[Level] = (S){s, n}; }

/*
	Sets 'Level'; outputs indentation codes; returns 'l' with leading
	tabs removed.
*/
static char *indent (char *l) {
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
static void dotkey (char *buf, int *n, char **key, char **val) {
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

static void pedant (char *line) {
	char *nospaces = "No spaces are allowed at the end of a line.";
	char *notabs = "No tabs are allowed except at the beginning of lines.";
	char *c;
	int visual, l;
	c=line;
	{
		l=strlen(line);
		if (l && isspace(line[l-1]))
			errx(1, nospaces); }
	{
		while ('\t'==*c) c++;
		if ('|'==*c) return;
		for (; *c; c++)
			if ('\t'==*c)
				errx(1, notabs); }
	{
		for (c=line,visual=0; *c; c++,visual++)
			if ('\t'==*c) visual+=7;
		if (visual>78)
			errx(1, "A line is visually longer than 78 characters."
			        "Note that tabs count as 8 characters."); }}

static int count (char pre, char *l) {
	int x=0;
	while (pre==*l++) x++;
	return x; }

static void emit (char *l) {
	int len=strlen(l), br=(len>=2 && !strcmp(l+(len-2), "\\\\"));
	if (br) l[len-2]='\0';
	printf("%s%s\n", l, br?"\n.BR":""); }

static void input (char *l) {
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
		static const char *align =
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

int main (int argc, char *argv[]) {
	char *last = "The last character must be a newline.";
	char buf[80];
	while (buf[78] = '\0', fgets(buf, 80, stdin))
		assert(strlen(buf)>0);
		if (buf[78] == '\n' || buf[78] == '\0') {
			int l=strlen(buf);
			if ('\n'!=buf[l-1])
				errx(1, last);
			buf[l-1]='\0';
			input(buf); }
		else errx(1, "A line is longer than 78 bytes.");
	input("");
	return 0; }
