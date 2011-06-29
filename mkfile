all:V: o.marks2tokens README.toks SPEC.toks INTRO.toks
clean:V:
	rm -f marks *.toks *.o o.*

o.marks2%: marks.c 2%.c
	c99 marks.c 2$stem.c -o $target

%.toks: o.marks2tokens %
	./o.marks2tokens <$stem >$target
