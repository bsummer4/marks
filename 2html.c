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
	printf("%s%s\n",w.w,((!tagstr || 'h'==*tagstr)?"":"<br/>")); }

void hdrdepth(int i) {
	static char htag[3];
	snprintf(htag,3,"h%d",i);
	TAG(htag);
	for (;i;i--) printf("#"); printf(" "); }
