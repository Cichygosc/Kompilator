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
	char sign;
	symrec * variable;
}

%token LEFTBRACKET
%token RIGHTBRACKET
%token <sign> SEMICOLON

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

%token SKIP

%token <name> pidentifier
%token <val> num

%type <variable> identifier
%type <variable> value

%left '-' '+'
%left '*' '/' '%'

%%

program:
	VAR vdeclarations Begin commands END

vdeclarations:
	vdeclarations pidentifier	{ addVariable($2); }
|	vdeclarations pidentifier LEFTBRACKET num RIGHTBRACKET
|	%empty

commands:
	commands command
|	command

command:
	identifier ":=" expression
|	IF condition THEN commands ELSE commands ENDIF
|	WHILE condition DO commands ENDWHILE
|	FOR pidentifier FROM value TO value DO commands ENDFOR
|	FOR pidentifier FROM value DOWNTO value DO commands ENDFOR
|	READ identifier SEMICOLON	{ readVariable($2); }
|	WRITE value SEMICOLON		{ writeVariable($2); }
|	SKIP;

expression:
	value
|	value '+' value
|	value '-' value
|	value '*' value
|	value '/' value
|	value '%' value

condition:
	value '=' value
|	value "<>" value
|	value '<' value
|	value '>' value
|	value "<=" value
|	value ">= "value

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
	initTests();

	

	int ret = yyparse();
	clearAll();

	return ret;
}

void yyerror(char const * s)
{
	fprintf(stderr, "At line %d %s\n", yylloc.first_line, s);
}