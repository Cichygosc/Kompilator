%option	noyywrap

%{
	#include "compiler.h"
	#include "comp.tab.h"
	#include <stdlib.h>

	#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}

%option yylineno
%x		COMMENT

%%
[0-9]+		{ if (DEBUG == true) printf("%s", yytext); yylval.val = strtoull(yytext, NULL, 10); return num; }
[_a-z]+		{ if (DEBUG == true) printf("%s", yytext); yylval.name = strdup(yytext); return pidentifier; }

VAR			{ if (DEBUG == true) printf("%s", yytext); return VAR; }
BEGIN		{ if (DEBUG == true) printf("%s", yytext); return Begin; }
END 		{ if (DEBUG == true) printf("%s", yytext); return END; }

READ		{ if (DEBUG == true) printf("%s", yytext); return READ; }
WRITE 		{ if (DEBUG == true) printf("%s", yytext); return WRITE; }

SKIP		{ if (DEBUG == true) printf("%s", yytext); return SKIP; }
IF 			{ if (DEBUG == true) printf("%s", yytext); return IF; }
THEN		{ if (DEBUG == true) printf("%s", yytext); return THEN; }
ELSE		{ if (DEBUG == true) printf("%s", yytext); return ELSE; }
ENDIF		{ if (DEBUG == true) printf("%s", yytext); return ENDIF; }

WHILE		{ if (DEBUG == true) printf("%s", yytext); return WHILE; }
DO 			{ if (DEBUG == true) printf("%s", yytext); return DO; }
ENDWHILE 	{ if (DEBUG == true) printf("%s", yytext); return ENDWHILE; }

FOR			{ if (DEBUG == true) printf("%s", yytext); return FOR; }
FROM		{ if (DEBUG == true) printf("%s", yytext); return FROM; }
TO 			{ if (DEBUG == true) printf("%s", yytext); return TO; }
ENDFOR		{ if (DEBUG == true) printf("%s", yytext); return ENDFOR; }
DOWNTO		{ if (DEBUG == true) printf("%s", yytext); return DOWNTO; }

":="		{ if (DEBUG) printf("%s", yytext); return ASSIGN; }
\+			{ if (DEBUG) printf("%s", yytext); return ADD; }
\-			{ if (DEBUG) printf("%s", yytext); return SUB; }
\*			{ if (DEBUG) printf("%s", yytext); return MUL; }
\/			{ if (DEBUG) printf("%s", yytext); return DIV; }
\%			{ if (DEBUG) printf("%s", yytext); return MOD; }

\=			{ if (DEBUG) printf("%s", yytext); return EQUAL; }
\<\>		{ if (DEBUG) printf("%s", yytext); return DIFFERENT; }
\>			{ if (DEBUG) printf("%s", yytext); return GREATER; }
\<			{ if (DEBUG) printf("%s", yytext); return LESS; }
\>\=		{ if (DEBUG) printf("%s", yytext); return GREATEROREQUAL; }
\<\=		{ if (DEBUG) printf("%s", yytext); return LESSOREQUAL; }

[[:space:]]+
\[			{ if (DEBUG == true) printf("%s", yytext); return LEFTBRACKET; }
\]			{ if (DEBUG == true) printf("%s", yytext); return RIGHTBRACKET; }
\;			{ if (DEBUG == true) printf("%s", yytext); return SEMICOLON; }

\{			{ if (DEBUG == true) printf("%s", yytext); BEGIN(COMMENT); }
<COMMENT>\} { if (DEBUG == true) printf("%s", yytext); BEGIN(INITIAL); }
<COMMENT>\n { if (DEBUG == true) printf("%s", yytext); BEGIN(INITIAL); }

.			{ if (DEBUG == true) printf("Unrecoginzed tokens %s", yytext); }
%%