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

static void input (char *buf) {
	int tabs=0, spaces=0;
	char *scan = buf;
	while (*scan == '\t') scan++, tabs++;
	while (*scan == ' ') scan++, spaces++;
	if (!tabs && !spaces) {
		if (*scan != '\n') errx(1, "Empty lines must be empty");
		inparagraph=0;
		return; }
	fprintf(stderr, "%d tabs and %d spaces\n", tabs, spaces); }

int main (int argc, char *argv[]) {
	char buf[80];
	while (buf[78] = '\0', fgets(buf, 80, stdin))
		if (buf[78] == '\n' || buf[78] == '\0') input(buf);
		else errx(1, "A line is longer than 78 characters");
	return 0; }
