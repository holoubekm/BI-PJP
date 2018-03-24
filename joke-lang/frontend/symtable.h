#ifndef _SYM_TABLE_H_
#define _SYM_TABLE_H_

#include <cstdlib>
#include <string>
#include <iostream>
#include <cstring>

using namespace std;
//enum DruhId {Nedef, IdProm, IdKonst};

#define declarationError(msg, name) declarationError_(msg, name, __LINE__);

inline void declarationError_(const string& msg, const string& name, int line)
{
	cerr << "Declaration error: \"" << msg << "\" of " << "\"" << name << "\" at line " << line << endl;
	exit(3);
}

inline void Spaces(int spaces)
{
	for (int x = 0; x < spaces * 2; x++)
		cout << " ";
}


union SymbolData
{
	char* str;
	int val;

	SymbolData() : str(NULL) {}

	~SymbolData(){}
};

enum SymbolType
{
	SM_UNDEFINED,
	SM_VAR,
	SM_PARAM,
	SM_STRING,
	SM_ARRAY,
	SM_CONST,
	SM_FUNC,
	SM_PROCED
};

static const char* symbolNames[] =
{
	"SM_UNDEFINED",
	"SM_VAR",
	"SM_PARAM",
	"SM_STRING",
	"SM_ARRAY",
	"SM_CONST",
	"SM_FUNC",
	"SM_PROCED"
};

inline const char* getSymbolName(SymbolType t)
{
	return symbolNames[(int) t];
}

#endif //_SYM_TABLE_H_

