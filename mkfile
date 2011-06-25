all:V: marks example.html
show:V: example.html
	psv example.ps &
	#web file://`pwd`/example.html &
	less example.ftx

clean:V:
	rm -f marks *.html *.ps *.ftx

marks: marks.c
	c99 marks.c -o $target

%.ftx %.html %.ps:D: % marks
	touch $target # So mk can delete it if something fails.
	rofftmp=$(mktemp)
	./marks <$stem >$rofftmp
	9 troff -ms <$rofftmp | 9 tr2post | 9 psfonts >$stem.ps
	9 nroff -ms <$rofftmp >$stem.ftx
	./htmlize <$stem >$stem.html
	rm $rofftmp
