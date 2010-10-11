// - TODO - Lists
// - TODO 1. More lists
// - TODO explicit newlines \\

#include <err.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

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

int Level = 0;
int Inpara[10] = {0};
int Inpre[10] = {0};

static void endpre ();
static void startpre ();
static void endpara ();
static void startpara ();

static void endpre ()
{	if (Inpre[Level]) puts("</pre>"),Inpre[Level]=false; }
static void startpre ()
{	if (!Inpre[Level]) endpara(),puts("<pre>"),Inpre[Level]=true; }

static void endpara ()
{	if (Inpara[Level])endpre(),puts("</p>"),Inpara[Level]=false; }
static void startpara ()
{	if (!Inpara[Level]) endpre(),puts("<p>"),Inpara[Level]=true; }

static void pre (char *l) {
	if ('|'!=*l++) errx(1, "Invalid quotation.");
	if (!isspace(*l++)) errx(1, "Invalid quotation.");
	startpre();
	puts(l); }

static void header (char *l)
{	int c=0;
	for (; '#'==*l; c++,l++);
	if (' '!=(*l++)) errx(1, "Invalid header.");
	endpara();
	printf("<h%d>\n%s\n</h%d>\n", c, l, c); }

/* Sets 'Level', handles <blockquote>s, returns l with tabs removed.  */
static char *indent (char *l)
{	int c;
	for (c=0; '\t'==*l; c++,l++);
	/* Note that this next case should never happen because any line with 10+
		levels of indentation is over 78 characters wide. */
	if (c>9) errx(1, "Only 9 levels of indentation are allowed.");
	while (Level>c) puts("</blockquote>"),Level--;
	while (Level<c) puts("<blockquote>"),Level++,Inpara[Level]=true;
	return l; }

static void input (char *l)
{	pedant(l);
	l=indent(l);
	if (isspace(*l)) errx(1, "Lines must not start with spaces; only tabs.");
	if ('#'==*l) { header(l); return; }
	if ('|'==*l) { pre(l); return; }
	if (!*l) endpara(),endpre();
	if (*l) startpara();
	if (Inpara[Level]) puts(l); }

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
