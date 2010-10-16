// # .1. Numbered lists
// # TODO Support utf8

#include <err.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/*
	'n' is the number of an ordered list or the number of a header,
	or a flag for whether a '<li>' has been ommited in the LIST state.
	the typedef S is only for brevity in literal expressions.
*/
enum { NONE=0, P, LIST, OL, QUOTE, HEADER };
typedef struct state { int state, n; } S;
struct state State[9] = {0};
int Level = 0;

static void exitstate ()
{	switch (State[Level].state) {
	 case NONE: break;
	 case P: puts("</p>"); break;
	 case LIST:
		if (State[Level].n) puts("</li>");
		puts("</ul>");
		break;
	 case QUOTE: puts("</pre>"); break;
	 case HEADER: printf("</h%d>\n", State[Level].n); break; }
	State[Level].state = NONE; }

static void enterstate (int s, int n)
{	if (s==State[Level].state) return;
	exitstate();
	switch (s) {
	 case NONE: break;
	 case P: puts("<p>"); break;
	 case LIST: puts("<ul>"); break;
	 case QUOTE: puts("<pre>"); break;
	 case HEADER: printf("<h%d>\n", n); break; }
	State[Level] = (S){s, n}; }

/* Sets 'Level', handles <blockquote>s, returns l with tabs removed.  */
static char *indent (char *l)
{	int c;
	for (c=0; '\t'==*l; c++,l++);
	if (c>9) errx(1, "Only 9 levels of indentation are allowed.");
	while (Level>c) exitstate(),puts("</blockquote>"),Level--;
	while (Level<c) exitstate(),puts("<blockquote>"),Level++;
	return l; }

static void pedant (char *line)
{	char *c;
	int visual, l;
	c=line;
	{	l=strlen(line);
		if (l && isspace(line[l-1]))
			errx(1, "No spaces are allowed at the end of a line."); }
	{	while ('\t'==*c) c++;
		if ('|'==*c) return;
		for (; *c; c++)
			if ('\t'==*c)
				errx(1, "No tabs are allowed except at the beginning of lines."); }
	{	for (c=line,visual=0; *c; c++,visual++)
			if ('\t'==*c) visual+=7;
		if (visual>78)
			errx(1, "A line is visually longer than 78 characters.  "
		        	"Note that tabs count as 8 characters."); }}

static int count (char pre, char *l) {
	int x=0;
	while (pre==*l++) x++;
	return x; }

static void emit (char *l)
{	int len=strlen(l), br=(len>=2 && !strcmp(l+(len-2), "\\\\"));
	if (br) l[len-2]='\0';
	printf("%s%s\n", l, br?"<br>":""); }

static void input (char *l)
{	pedant(l);
	l=indent(l);
	switch(*l)
	{case '\0': enterstate(NONE, -1); break;
	 case '-': enterstate(LIST, 0); break;
	 case ' ': enterstate(LIST, 0); break;
	 case '|': enterstate(QUOTE, -1); break;
	 case '#': enterstate(HEADER, count('#', l)); break;
	 default: enterstate(P, -1); }

	switch (State[Level].state)
	{case NONE: return;
	 case LIST:
	 {	int *n = &State[Level].n;
		bool cont = (' '==*l++);
		if (' '!=*l++) errx(1, "bad list.");
		if (isspace(*l)) errx(1, "bad list.");
		if (!*n && cont) errx(1, "space w/o list");
		if (*n && !cont) puts("</li>");
		if (!cont) puts("<li>");
		*n=1,emit(l);
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
		puts(l);
		break; }}

int main (int argc, char *argv[])
{	puts("<html>\n<body>");
	char buf[80];
	while (buf[78] = '\0', fgets(buf, 80, stdin)) /* strlen(buf)>0 */
		if (buf[78] == '\n' || buf[78] == '\0')
		{	int l=strlen(buf);
			if ('\n'!=buf[l-1])
				errx(1, "The last character must be a newline.");
			buf[l-1]='\0';
			input(buf); }
		else errx(1, "A line is longer than 78 bytes.");
	input("");
	puts("</body>\n</html>");
	return 0; }
