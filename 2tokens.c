#include "marks.h"

void BEGIN() { puts("BEGIN"); }
void END() { puts("END"); }
void indent() { puts("INDENT"); }
void undent() { puts("UNDENT"); }

void blist() { puts("BLIST"); }
void lit() { puts("LIT"); }
void para() { puts("PARA"); }
void hdr() { puts("HDR"); }
void nlist() { puts("NLIST"); }
void anno() { puts("ANNO"); }

void item(char *l) { printf("ITEM: %s\n", l); }
void bullet() { puts("BULLET"); }
void hdrdepth(int i) { printf("HDRDEPTH: %d\n",i); }
void qline(char *line) { printf("\t%s\n",line); }
void word(Word w) {
	char *pre="";
	switch(w.t) {
	case QUOTED: pre="QUOTED WORD\n\t"; break;
	case PLAIN: pre="PLAIN WORD\n\t"; break;
	case LINK: pre="LINK\n\t"; break;
	case GROUP: w.w=""; break;
	case ENDGROUP: w.w=""; break;
	case WS: w.w="SPACE"; break; }
	printf("%s%s\n", pre, w.w); }
