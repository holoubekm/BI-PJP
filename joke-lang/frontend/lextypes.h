
#ifndef _TYPES_H_
#define _TYPES_H_

#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class Lexem;
Lexem* readLexem();
void parseError(const string& message, int line = -1);
void parseError(const string& message, const string& lexem, int line = -1);

enum LexemType
{
	CONST,
	EOI,
	IDENT,
	STRING,
	UNKNOWN,

	KW_ARRAY,
	KW_BEGIN,
	KW_CONST,
	KW_DIV,
	KW_DO,
	KW_DOWNTO,
	KW_ELSE,
	KW_END,
	KW_EXIT,
	KW_FOR,
	KW_FORW,
	KW_FUNC,
	KW_IF,
	KW_INT,
	KW_OF,
	KW_PROCED,
	KW_PROG,
	KW_READ,
	KW_THEN,
	KW_TO,
	KW_VAR,
	KW_WHILE,
	KW_WRITE,
	KW_WRITELN,

	OP_ADD,
	OP_AND,
	OP_ASIG,
	OP_DIV,
	OP_EQ,
	OP_GE,
	OP_GT,
	OP_LE,
	OP_LT,
	OP_MIN,
	OP_MOD,
	OP_MUL,
	OP_NEQ,
	OP_OR,

	SYM_COLON,
	SYM_COMMA,
	SYM_DDOT,
	SYM_DOT,
	SYM_LBRAC,
	SYM_LSBRC,
	SYM_RBRAC,
	SYM_RSBRC,
	SYM_SEMIC
};

static const char* lexemNames[] = 
{
	"CONST",
	"EOI",
	"IDENT",
	"STRING",
	"UNKNOWN",

	"KW_ARRAY",
	"KW_BEGIN",
	"KW_CONST",
	"KW_DIV",
	"KW_DO",
	"KW_DOWNTO",
	"KW_ELSE",
	"KW_END",
	"KW_EXIT",
	"KW_FOR",
	"KW_FORW",
	"KW_FUNC",
	"KW_IF",
	"KW_INT",
	"KW_OF",
	"KW_PROCED",
	"KW_PROG",
	"KW_READ",
	"KW_THEN",
	"KW_TO",
	"KW_VAR",
	"KW_WHILE",
	"KW_WRITE",
	"KW_WRITELN",

	"OP_ADD",
	"OP_AND",
	"OP_ASIG",
	"OP_DIV",
	"OP_EQ",
	"OP_GE",
	"OP_GT",
	"OP_LE",
	"OP_LT",
	"OP_MIN",
	"OP_MOD",
	"OP_MUL",
	"OP_NEQ",
	"OP_OR",

	"SYM_COLON",
	"SYM_COMMA",
	"SYM_DDOT",
	"SYM_DOT",
	"SYM_LBRAC",
	"SYM_LSBRC",
	"SYM_RBRAC",
	"SYM_RSBRC",
	"SYM_SEMIC",
};

inline const string getLexemName(LexemType type)
{
	return lexemNames[(int) type];
}

union LexemData
{
	char* str;
	int val;

	LexemData() : str(NULL) {}
	~LexemData(){}
};

class Lexem
{
public:
	Lexem() : type(UNKNOWN) {}
	Lexem(LexemType ltype) : type(ltype), has_value(false) {}
	Lexem(LexemType ltype, int val) : type(ltype), has_value(true)
	{
		data.val = val;
	}
	Lexem(LexemType ltype, const char* ldata, int len) : type(ltype), has_value(true)
	{
		if(type != IDENT && type != STRING)
		{
			parseError("Only identifier should have string value");
		}

		data.str = new char[len + 1];
		strncpy(data.str, ldata, len + 1);
	} 

	~Lexem() 
	{
	}

	friend ostream& operator<<(ostream&, const Lexem&);

	LexemData data;
	LexemType type;
	bool has_value;
};

inline ostream& operator<<(ostream& os, const Lexem& l)
{
	os << "type: " << getLexemName(l.type);
	switch(l.type)
	{
		case IDENT:
			os << " : " << l.data.str;
			break;
		case CONST:
			os << " : " << l.data.val;
			break;
		default:
			break;
	}
	return os;
}

#endif /*_TYPES_H_*/