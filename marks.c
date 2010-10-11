// - TODO | Literal quotation
// - TODO - Lists
// - TODO 1. More lists
// - TODO explicit newlines \\

#include <err.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

int Level = 0;
int Inpara[7] = {0};

static void header (char *l)
{	assert('#'==*l);
	int c;
	for (c=0; '#'==*l; c++,l++);
	if (' '!=(*l++)) errx(1, "Invalid Header.");
	printf("<h%d>%s</h%d>\n", c, l, c); }

static void endpara () { if (Inpara[Level]) puts("</p>"),Inpara[Level]=false; }
static void startpara () { if (!Inpara[Level]) puts("<p>"),Inpara[Level]=true; }

static void pedant (char *line)
{	for (char *c=line; *c; c++)
		 if ('\t'==*c)
			errx(1, "No tabs are allowed except at the beginning of lines.");
	{	int l = strlen(line);
		if (l && isspace(line[l-1]))
			errx(1, "No spaces are allowed at the end of a line."); }}

/* Sets 'Level', handles <blockquote>s, and removed tabs from l.  */
static void handlequote (char **l)
{	int c;
	for (c=0; '\t'==**l; c++,(*l)++);
	if (c>=7) errx(1, "Only 6 levels of indentation are allowed.");
	while (Level>c) puts("</blockquote>"),Level--;
	while (Level<c) puts("<blockquote>"),Level++,Inpara[Level]=true; }

static void input (char *l)
{	handlequote(&l);
	pedant(l);
	if (isspace(*l)) errx(1, "Lines must not start with spaces; only tabs.");
	if ('#'==*l) { endpara(),header(l); return; }
	if (!*l) endpara();
	if (*l) startpara();
	if (Inpara[Level]) puts(l); }

int main (int argc, char *argv[])
{	puts("<html><body>");
	char buf[80];
	while (buf[78] = '\0', fgets(buf, 80, stdin)) /* strlen(buf)>0 */
		if (buf[78] == '\n' || buf[78] == '\0')
		{	int l=strlen(buf);
			if ('\n'!=buf[l-1]) errx(1, "Last character must be a newline.");
			buf[l-1]='\0';
			input(buf); }
		else errx(1, "A line is longer than 78 characters.");
	input("");
	puts("</body></html>");
	return 0; }
