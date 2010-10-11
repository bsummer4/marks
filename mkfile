CFLAGS = -O0 -g -std=c99

all:V: marks example.html
show:V: example.html
	uzbl example.html &

clean:V:
	rm -f marks *.html


marks: marks.c
	gcc $CFLAGS marks.c -o $target


%.html:D: % marks
	./marks <$stem >$stem.html
