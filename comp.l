%option	noyywrap

%{
	#include "compiler.h"
	#include "comp.tab.h"
	#include <stdlib.h>

	#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}

%option yylineno

%%
[0-9]+		{ if (DEBUG == true) printf("%s", yytext); yylval.val = strtoull(yytext, NULL, 10); return num; }
[_a-z]+		{ if (DEBUG == true) printf("%s", yytext); yylval.name = yytext; return pidentifier; }

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

[+*/%-]		{ if (DEBUG == true) printf("%s", yytext); return yytext[0]; }
[[:space:]]+
\[			{ if (DEBUG == true) printf("%s", yytext); return LEFTBRACKET; }
\]			{ if (DEBUG == true) printf("%s", yytext); return RIGHTBRACKET; }
\;			{ if (DEBUG == true) printf("%s", yytext); return SEMICOLON; }

.			{ if (DEBUG == true) printf("Unrecoginzed tokens %s", yytext); }
%%