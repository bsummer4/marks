MKSHELL = /opt/plan9/bin/rc
backends = tokens html
docs = `{ls | egrep '^[A-Z]+$'}
targets = `{echo o.marks2$backends; for (d in $docs) echo $d.$backends}

all:V: $targets
clean:V:
	rm -f $targets *.o

o.marks2%: marks.o 2%.o
%.o: %.c
	c99 -c $stem.c
o.%:
	c99 $prereq -o $target
([A-Z]+)\.([a-z]+):R: o.marks2\2 \1
	./o.marks2$stem2 <$stem1 >$target
