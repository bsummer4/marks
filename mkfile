CFLAGS = -O0 -g -std=c99

all:V: marks example.html
show:V: example.html
	uzbl example.html &

clean:V:
	rm -f marks *.html *.pdf


marks: marks.c
	gcc $CFLAGS marks.c -o $target


%.html %.pdf:D: % marks
	rofftmp=$(mktemp)
	cat $stem | sed 's/\.\([^.]*\)\. /- \1) /' | ./marks >$rofftmp
	9 htmlroff -ms <$rofftmp >$stem.html
	9 troff -ms <$rofftmp | 9 tr2post | 9 psfonts | ps2pdf - - >$stem.pdf
