%option noyywrap
%{

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <sstream>
#include "lextypes.h"

using namespace std;

Lexem* _curLexem = NULL;
int curLine = 0;

#define SET_LEXEM(type) _curLexem = new Lexem(type);
#define SET_LEXEM_INT(type, data) _curLexem = new Lexem(type, data);
#define SET_LEXEM_IDENT(type, text, len) _curLexem = new Lexem(type, text, len);

%}

%x COMMENT

white_space       [ \t]*
digit             [0-9]
alpha             [A-Za-z_]
alpha_num         ({alpha}|{digit})
hex_digit         [0-9A-Fa-f]
oct_digit         [0-7]
identifier        {alpha}{alpha_num}*
integer           {digit}+
hex_integer       ${hex_digit}{hex_digit}*
oct_integer       &{oct_digit}{oct_digit}*
string			  '.*'

%%

"{"                  BEGIN(COMMENT);
<COMMENT>[^}\n]+
<COMMENT>\n            ++curLine;
<COMMENT><<EOF>>    parseError("EOF in comment", curLine);
<COMMENT>"}"        BEGIN(INITIAL);


"and"					{ SET_LEXEM(OP_AND); 		return 1; }
"or"					{ SET_LEXEM(OP_OR); 		return 1; }
"+"						{ SET_LEXEM(OP_ADD); 		return 1; }
"-"						{ SET_LEXEM(OP_MIN); 		return 1; }
"*"						{ SET_LEXEM(OP_MUL); 		return 1; }
"/"						{ SET_LEXEM(OP_DIV); 		return 1; }
"div"					{ SET_LEXEM(OP_DIV); 		return 1; }
"%"						{ SET_LEXEM(OP_MOD); 		return 1; }
"mod"					{ SET_LEXEM(OP_MOD); 		return 1; }
":="					{ SET_LEXEM(OP_ASIG); 		return 1; }
"<>"					{ SET_LEXEM(OP_NEQ); 		return 1; }
"="						{ SET_LEXEM(OP_EQ); 		return 1; }
"<"						{ SET_LEXEM(OP_LT); 		return 1; }
"<="					{ SET_LEXEM(OP_LE); 		return 1; }
">"						{ SET_LEXEM(OP_GT); 		return 1; }
">="					{ SET_LEXEM(OP_GE); 		return 1; }
"("						{ SET_LEXEM(SYM_LBRAC);		return 1; }
")"						{ SET_LEXEM(SYM_RBRAC);		return 1; }
"["						{ SET_LEXEM(SYM_LSBRC); 	return 1; }
"]"						{ SET_LEXEM(SYM_RSBRC); 	return 1; }
";"						{ SET_LEXEM(SYM_SEMIC); 	return 1; }
":"						{ SET_LEXEM(SYM_COLON); 	return 1; }
","						{ SET_LEXEM(SYM_COMMA); 	return 1; }
".."					{ SET_LEXEM(SYM_DDOT); 		return 1; }
"."						{ SET_LEXEM(SYM_DOT); 		return 1; }

"var"					{ SET_LEXEM(KW_VAR); 		return 1; }
"exit"					{ SET_LEXEM(KW_EXIT); 		return 1; }
"while"					{ SET_LEXEM(KW_WHILE); 		return 1; }
"to"					{ SET_LEXEM(KW_TO); 		return 1; }
"then"					{ SET_LEXEM(KW_THEN); 		return 1; }
"program"				{ SET_LEXEM(KW_PROG); 		return 1; }
"function"				{ SET_LEXEM(KW_FUNC); 		return 1; }
"forward"				{ SET_LEXEM(KW_FORW); 		return 1; }
"procedure"				{ SET_LEXEM(KW_PROCED); 	return 1; }
"array"					{ SET_LEXEM(KW_ARRAY); 		return 1; }
"begin"					{ SET_LEXEM(KW_BEGIN); 		return 1; }
"end"					{ SET_LEXEM(KW_END); 		return 1; }
"do"					{ SET_LEXEM(KW_DO); 		return 1; }
"for"					{ SET_LEXEM(KW_FOR); 		return 1; }
"if"					{ SET_LEXEM(KW_IF); 		return 1; }
"of"					{ SET_LEXEM(KW_OF); 		return 1; }
"integer"				{ SET_LEXEM(KW_INT); 		return 1; }
"const"					{ SET_LEXEM(KW_CONST); 		return 1; }
"downto"				{ SET_LEXEM(KW_DOWNTO); 	return 1; }
"else"					{ SET_LEXEM(KW_ELSE); 		return 1; }
"write"					{ SET_LEXEM(KW_WRITE);		return 1; }
"writeln"				{ SET_LEXEM(KW_WRITELN);	return 1; }
"readln"				{ SET_LEXEM(KW_READ); 		return 1; }

{oct_integer}			{ int v; istringstream str(YYText() + 1); str >> oct >> v; SET_LEXEM_INT(CONST, v); 	return 1; }
{hex_integer}			{ int v; istringstream str(YYText() + 1); str >> hex >> v; SET_LEXEM_INT(CONST, v); 	return 1; }
{integer} 				{ int v; istringstream str(YYText()); str >> v; SET_LEXEM_INT(CONST, v); 				return 1; }
{identifier}			{ SET_LEXEM_IDENT(IDENT, YYText(), YYLeng()); 											return 1; }

{string}				{ SET_LEXEM_IDENT(STRING, YYText(), YYLeng()); 										return 1; }

{white_space}			/* nothing */
\n						{ curLine += 1; }
.						{ parseError("Illegal input character", YYText(), curLine); }
<<EOF>>					{ SET_LEXEM(EOI); return 0; }
%%

/*
static yyFlexLexer Lexer;
Lexem* readLexem()
{
	Lexer.yylex();
	return _curLexem;
}*/

void parseError(const string& message, int line)
{
   cerr << "Error: \"" << message << "\" at line " << line << endl;
   exit(1);
}

void parseError(const string& message, const string& lexem, int line)
{
   cerr << "Error: \"" << message << "\" at line " << line << ". Lexem = \"" << lexem << "\"" << endl;
   exit(1);
}


/*
int main(int argc, char* argv[]) 
{
	Lexem* cLexem = NULL;
	int val;

	while((cLexem = nextLexem()) != NULL)
	{
		cout << *cLexem << endl;
		delete cLexem;
	}
	return 0;
}*/
