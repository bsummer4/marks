all:V: marks
clean:V:
	rm marks

marks: marks.c
	gcc marks.c -o $target
