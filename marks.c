#include "marks.h"
int Line=0;
int TODO=9000;

/*
	## IO and String Utillities
*/
#define UTF8MAX 6
#define BUFSIZE UTF8MAX*78+2
#define UTF8LEN(ch) ((unsigned char)ch>=0xFC ? 6 : \
                     ((unsigned char)ch>=0xF8 ? 5 : \
                      ((unsigned char)ch>=0xF0 ? 4 : \
                       ((unsigned char)ch>=0xE0 ? 3 : \
                        ((unsigned char)ch>=0xC0 ? 2 : 1)))))

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

/*
	Returns a pointer to static memory or `nil' on EOF. Lines with
	more than 80 unicode characters may-or-may-not exit on error.
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
	Line++;
	return buf; }

S void pedant (char *line) {
	char *nospaces = "No spaces are allowed at the end of a line.";
	char *notabs = "No tabs are allowed except at the beginning of lines.";
	char *c;
	int l;
	c=line;
	// fprintf(stderr, "cols: %d len: %d\n", cols(line), strlen(line));
	if (78<cols(line)) errx(1,"All lines must fit within 78 columns");
	l=strlen(line);
	if (l && isspace(line[l-1]))
		errx(1, nospaces);
	while ('\t'==*c) c++;
	if ('|'==*c) return;
	for (; *c; c++) if ('\t'==*c) errx(1, notabs); }

SI int count (char pre, char *l) {
	int x=0;
	while (pre==*l++) x++;
	return x; }

SI int tabs (char **L) {
	int c=count('\t',*L);
	if (c>9) BAD("Only 9 levels of indentation are allowed.");
	*L+=c; return c; }

/*
	## Parsing
*/
enum {BLANK,BLIST,NLIST,P,LIT,HDR,ANNO};
typedef struct { int depth,state,align; } St;
St Cur = {0,BLANK,0};
#define TR(a,b,c) transition((St){a,b,c})
const char *overalign =
	"Continuation line has too many leading spaces.";
const char *underalign =
	"Continuation line doesn't have enough leading spaces.";

void words(char *line) { word((Word){PLAIN,line}); }

/*
	Returns a pointer to the key. This will be a pointer into the
	buffer, but a null byte will be added so that it's just the key.
	`*after' will be modified to point to the text of the list.
*/
S char *dotkey (char *buf, char **after) {
	const char *bad = "Invalid ordered list %d";
	if ('.'!=*buf++) errx(1,bad,1);
	/* The length of the key. Doesn't include the training space.  */
	int keylen;
	{ char *w=strchr(buf,' '); keylen=w?(w-buf):strlen(buf); keylen--; }
	if (keylen<1) errx(1,bad,2);
	if ('.'==*buf) errx(1,bad,3);
	if (buf[keylen]!='.') errx(1,bad,4);
	*after = buf+keylen+1;
	while ('.'==buf[keylen]) buf[keylen--]='\0';
	return buf; }

void transition (St new) {
	if (BLANK==Cur.state && BLANK==new.state)
		BAD("Blank lines may never occur in pairs");
	if (new.state==Cur.state&&new.depth==Cur.depth&&new.align==Cur.align)
		return;
	if (HDR==Cur.state && BLANK==new.state)
		BAD("Headers may not preced blank lines.");
	if (new.depth!=Cur.depth && new.state==BLANK) BUG;

	if (new.depth > Cur.depth) {
		if (Cur.state==BLANK)
			BAD("Indented sections must come directly after less-indented ones.");
		if (1+Cur.depth!=new.depth)
			BAD("Indentation must be increased one tab at a time.");
		indent(); }

	if (new.depth < Cur.depth) {
		if (Cur.state!=BLANK && Cur.state!=HDR)
			BAD("Reducing indentation must be proceded by a blank line.");
		for (int i=Cur.depth-new.depth; i; i--) undent(); }

	if (new.depth == Cur.depth)
		if (BLANK!=new.state && BLANK!=Cur.state && HDR!=Cur.state)
			BAD("Aligned chunks must be separated by a blank line.");

	Cur=new;

	switch (Cur.state) {
	CASE(BLIST,blist()) CASE(NLIST,nlist()) CASE(P,para())
	CASE(LIT,lit()) CASE(HDR,hdr()) CASE(ANNO,anno()) }}

/*
	- Takes the post-tabs section of a file and does a state-change
	  based on the first character of the file.
	- Transitions to the new state with `transition'.
	- Validates lexical syntax of line prefixes.
*/
S void recognize (int tabs, char *l) {
	switch (*l) {
	case '\0': TR(Cur.depth,BLANK,0); break;
	case '-': TR(tabs,BLIST,2); break;
	case '.': TR(tabs,NLIST,TODO); break;
	case ':': TR(tabs,ANNO,TODO); break;
	case '|': TR(tabs,LIT,0); break;
	case '#': TR(tabs,HDR,0); break;
	case ' ': {
		if (!Cur.align) BAD("Bogus whitespace");
		int align = count(' ',l);
		if (align > Cur.align) BAD(overalign);
		if (align < Cur.align) BAD(underalign);
		break; }
	default: TR(tabs,P,1); }}

S void input (char *l) {
	pedant(l);
	int depth = tabs(&l);
	recognize(depth,l);
	switch (Cur.state) {
	case BLANK: break;
	case BLIST: if (*l=='-') bullet(); words(l+Cur.align); break;
	case NLIST: {
		char *body;
		if (*l=='.') item(dotkey(l,&body)); words(l+1+Cur.align);
		break; }
	case P: words(l); break;
	case LIT: qline(l+2); break;
	case HDR: { int c=count('#',l); hdrdepth(c); words(l+c+1); break; }
	case ANNO: words(l); break; }}

int main (int argc, char *argv[]) {
	char *line;
	BEGIN();
	while ((line=getline())) input(line);
	input("");
	END();
	return 0; }
