%{
#include "y.tab.h"
#include "ast.h"
#include <string.h>
%}

%option yylineno
%option noyywrap

DIGIT       [0-9]
LETTER      [a-zA-Z]
IDREST      [a-zA-Z0-9_]
HEXNUM      0[xX][0-9a-fA-F]+
INTNUM      0|[1-9][0-9]*
REALNUM     [+-]?(({DIGIT}+\.{DIGIT}*|\.{DIGIT}+)([eE][+-]?{DIGIT}+)?|{DIGIT}+[eE][+-]?{DIGIT}+)
CHARVAL     \'[^'\n]\'
STRINGVAL   \"[^\n"]*\"

%%

"proc"          { return PROC; }
"func"          { return FUNC; }
"return"        { return RETURN; }
"var"           { return VAR; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }

"bool"          { yylval = mknode("bool", NULL, NULL); return TYPE; }
"char"          { yylval = mknode("char", NULL, NULL); return TYPE; }
"int"           { yylval = mknode("int", NULL, NULL); return TYPE; }
"real"          { yylval = mknode("real", NULL, NULL); return TYPE; }
"int*"          { yylval = mknode("int*", NULL, NULL); return TYPE; }
"char*"         { yylval = mknode("char*", NULL, NULL); return TYPE; }
"real*"         { yylval = mknode("real*", NULL, NULL); return TYPE; }
"string"        { return STRING_TYPE; }

"null"          { yylval = mknode("null", NULL, NULL); return NULL_LITERAL; }
"true"          { yylval = mknode("true", NULL, NULL); return BOOL_LITERAL; }
"false"         { yylval = mknode("false", NULL, NULL); return BOOL_LITERAL; }

"=="            { return EQ; }
">"             { return GT; }
"<"             { return LT; }
">="            { return GE; }
"<="            { return LE; }
"!="            { return NE; }
"&&"            { return AND; }
"||"            { return OR; }
"^"             { return DEREF; }
"&"             { return ADDRESS; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
"|"             { return BAR; }
":"             { return COLON; }
";"             { return SEMICOLON; }
","             { return COMMA; }
"="             { return ASSIGN; }
"!"             { return NOT; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return STAR; }
"/"             { return DIV; }

{REALNUM}       { yylval = mknode(yytext, NULL, NULL); return REAL_LITERAL; }
{INTNUM}        { yylval = mknode(yytext, NULL, NULL); return INT_LITERAL; }
{CHARVAL}       { yylval = mknode(yytext, NULL, NULL); return CHAR_LITERAL; }
{STRINGVAL}     { yylval = mknode(yytext, NULL, NULL); return STRING_LITERAL; }
{HEXNUM}        { yylval = mknode(yytext, NULL, NULL); return INT_LITERAL; }

{LETTER}{IDREST}*   { yylval = mknode(yytext, NULL, NULL); return ID; }

"/%"([^%]|"%"+[^/])*"%/"    ;
[ \t\r\n]+                  ;
.               { return yytext[0]; }

%%