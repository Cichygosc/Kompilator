%{
	#include <stdio.h>
	#include <math.h>
	#include "compiler.h"
	#include "test.h"
	void yyerror(char const *);
%}

%locations

%union {
	ull val;
	char * name;
	symrec * variable;
}

%token LEFTBRACKET
%token RIGHTBRACKET
%token SEMICOLON

%token VAR
%token Begin
%token END

%token IF
%token THEN
%token ELSE
%token ENDIF

%token WHILE
%token DO
%token ENDWHILE

%token FOR
%token FROM
%token TO
%token ENDFOR
%token DOWNTO

%token READ
%token WRITE

%token ASSIGN
%token ADD
%token SUB
%token MUL
%token DIV
%token MOD

%token EQUAL
%token DIFFERENT
%token GREATER
%token LESS
%token LESSOREQUAL
%token GREATEROREQUAL

%token SKIP

%token <name> pidentifier
%token <val> num

%type <variable> identifier
%type <variable> value
%type <variable> expression

%left '-' '+'
%left '*' '/' '%'

%%

program:
	VAR vdeclarations Begin commands END 						{ clearAll(); }

vdeclarations:
	vdeclarations pidentifier									{ addVariable($2); }
|	vdeclarations pidentifier LEFTBRACKET num RIGHTBRACKET		{ addTable($2, $4); }
|	%empty

commands:
	commands command
|	command

command:
	identifier ASSIGN expression SEMICOLON						{ assignVariable($1, $3); }
|	IF { ifCount++; isIf = true; isWhile = false; } condition THEN commands { afterFirstCond(); } ELSE commands { afterSecondCond(); ifCount--; } ENDIF { changeLabels(false); }			
|	WHILE { onWhile(); } condition DO commands { afterWhile(); } ENDWHILE { changeLabels(false); }
|	FOR pidentifier FROM value TO value { forCount++; forCond($2, $4, $6, false); } DO commands { afterFor($2, false); forCount--; } ENDFOR
|	FOR pidentifier FROM value DOWNTO value { forCount++; forCond($2, $4, $6, true); } DO commands { afterFor($2, true); forCount--; } ENDFOR
|	READ identifier SEMICOLON									{ readVariable($2); }
|	WRITE value SEMICOLON										{ writeVariable($2); }
|	SKIP SEMICOLON

expression:
	value 				{ $$ = $1; }
|	value ADD value 	{ $$ = performAddition($1, $3); }
|	value SUB value 	{ $$ = performSubtraction($1, $3); }
|	value MUL value 	{ $$ = performMultiplication($1, $3); }
|	value DIV value 	{ $$ = performDivision($1, $3, false); }
|	value MOD value 	{ $$ = performDivision($1, $3, true); }

condition:
	value EQUAL value 			{ performCondCheck($1, $3, 3); }
|	value DIFFERENT value 		{ performCondCheck($1, $3, 2); }
|	value LESS value 			{ performCondCheck($3, $1, 1); }
|	value GREATER value 		{ performCondCheck($1, $3, 1); }
|	value LESSOREQUAL value 	{ performCondCheck($3, $1, 4); }
|	value GREATEROREQUAL value 	{ performCondCheck($1, $3, 4); }
 
value:
	num 		{ $$ = createValue($1); }
|	identifier	{ $$ = $1; }

identifier:
	pidentifier											{ $$ = getVariable($1); readTab = false; }
|	pidentifier LEFTBRACKET pidentifier RIGHTBRACKET	{ $$ = getVariableFromTable($1, $3); readTab = false; }
|	pidentifier LEFTBRACKET num RIGHTBRACKET			{ $$ = getVariableFromTableByValue($1, $3); readTab = false; }
%%

int main(int argc, char ** argv)
{
	extern FILE * yyin;

	if (argc == 2)
	{
		yyin = fopen(argv[1], "r");

	}
	else return 0;

	initRegisters();

	int ret = yyparse();
	changeLabels(true);

	writeCommands();

	return ret;
}

void yyerror(char const * s)
{
	fprintf(stderr, "At line %d %s\n", yylloc.first_line, s);
}