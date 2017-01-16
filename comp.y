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
|	IF { ifCount++; isIf = true; } condition THEN commands { afterFirstCond(); } ELSE commands { afterSecondCond(); ifCount--; } ENDIF { changeLabels(); }			
|	WHILE condition DO commands ENDWHILE
|	FOR pidentifier FROM value TO value DO commands ENDFOR
|	FOR pidentifier FROM value DOWNTO value DO commands ENDFOR
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
	value EQUAL value 			{ equal($1, $3); }
|	value DIFFERENT value 		{ different($1, $3); }
|	value LESS value 			{ greater($3, $1); }
|	value GREATER value 		{ greater($1, $3); }
|	value LESSOREQUAL value 	{ greaterOrEqual($3, $1); }
|	value GREATEROREQUAL value 	{ greaterOrEqual($1, $3); }
 
value:
	num 		{ $$ = createValue($1); }
|	identifier	{ $$ = $1; }

identifier:
	pidentifier											{ $$ = getVariable($1); }
|	pidentifier LEFTBRACKET pidentifier RIGHTBRACKET	{ $$ = getVariableFromTable($1, getVariableValue($3)); }
|	pidentifier LEFTBRACKET num RIGHTBRACKET			{ $$ = getVariableFromTable($1, $3); }
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
	changeLabels();

	writeCommands();

	return ret;
}

void yyerror(char const * s)
{
	fprintf(stderr, "At line %d %s\n", yylloc.first_line, s);
}