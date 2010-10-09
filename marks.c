// - whitespace at the end of a line is not legal
// - indentation must be of the form /^\t*((   )|(  )|())/
// /^##* (.*)/ -> header \1
// /^[-*%@] ([^ ].*)/ -> set \1
// /^([0-9]*|[a-z]*|[A-Z]*|[IVXC]*)\. (.*)/ -> list \1
// /^  ([^ ].*)/ -> setc \1
// /^  

#include <err.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

static void header (char *l) {
	int c;
	for (c=0; '#'==*l; c++,l++);
	if (' '!=(*l++)) errx(1, "Invalid Header");
	printf("<h%d>%s</h%d>\n", c, l, c); }

static void input (char *l) {
	if (isspace(*l)) errx(1, "No indentation is allowed");
	static bool inpara = false;
	if ('#'==*l) { header(l),inpara=false; return; }
	if (!*l && inpara) puts("</p>"), inpara=false;
	if (*l && inpara) puts(l);
	if (*l && !inpara) puts("<p>"), puts(l), inpara=true; }

int main (int argc, char *argv[]) {
	puts("<html><body>");
	char buf[80];
	while (buf[78] = '\0', fgets(buf, 80, stdin)) /* strlen(buf)>0 */
		if (buf[78] == '\n' || buf[78] == '\0') {
			int l=strlen(buf);
			if ('\n'!=buf[l-1]) errx(1, "Last character must be a newline.  ");
			buf[l-1]='\0';
			input(buf); }
		else errx(1, "A line is longer than 78 characters");
	input("");
	puts("</body></html>");
	return 0; }
