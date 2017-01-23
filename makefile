comp:	comp.y comp.l compiler.c utilities.c test.c
		bison -d comp.y
		flex comp.l
		gcc comp.tab.c lex.yy.c utilities.c compiler.c test.c -lcunit -o comp
