#include "marks.h"

char *tagstr=nil;
void tag() { if (tagstr) printf("<%s>\n", tagstr); }
void endtag() { if (tagstr) printf("</%s>\n", tagstr); tagstr=nil; }
void BEGIN() {}
void END() { endtag(); }
void indent() { endtag(); puts("<blockquote>"); }
void undent() { endtag(); puts("</blockquote>"); }

#define TAG(s) endtag(); tagstr=s; tag();
void blist() { TAG("ul"); }
void lit() { TAG("pre"); }
void para() { TAG("p"); }
void hdr() {}
void nlist() { TAG("ol"); }
void anno() { TAG("pre"); }

void item(char *l) { printf("<li>"); }
void bullet() { printf("<li>"); }
void qline(char *line) { printf("| <b><font color=red>%s</font></b>\n",line); }
void word(Word w) {
	static int gdepth=0;
	switch (w.t) {
	case LINK:
		printf("[<a href=\"%s\">%s</a>]",w.w,w.w);
		break;

	case GROUP:
		printf("<tt>{</tt>");
		switch (gdepth++) {
		case 0: printf("<u>"); break;
		case 1: printf("<i>"); break;
		case 2: printf("<b>"); break; }
		break;

	case WS: printf(" "); break;
	case ENDGROUP:
		switch (--gdepth) {
		case 0: printf("</u>"); break;
		case 1: printf("</i>"); break;
		case 2: printf("</b>"); break; }
		printf("<tt>}</tt>"); break;
	case QUOTED:
		printf("<tt>`<u>");
		for (char *c=w.w; *c; c++)
			if (' '==*c) printf("&nbsp;");
			else putchar(*c);
		printf("</u>'</tt>");
		break;
	default: printf("%s",w.w); }}

void hdrdepth(int i) {
	static char htag[3];
	snprintf(htag,3,"h%d",i);
	TAG(htag);
	printf("<tt>");
	for (;i;i--) printf("#"); printf("</tt> "); }
