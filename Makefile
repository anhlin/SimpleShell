CFLAGS=-Wall -Werror -Wextra -pedantic -O3 -Wshadow


myshell: lex.yy.c
	cc -c myshell.c $(CFLAGS)
	cc -o myshell myshell.c lex.yy.c -ll

lex.yy.c: lex.l
	flex lex.l 

clean: 
	rm lex.yy.c myshell *.o
