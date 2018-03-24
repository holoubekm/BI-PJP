#include "astree.h"


SymbolMember::SymbolMember() : type(SM_UNDEFINED), next(NULL), intern(false), ptr(NULL)
{
}

SymbolMember::SymbolMember(SymbolType stype, const string& sid, const char* sval) : id(sid), type(stype), next(NULL), intern(false), ptr(NULL)
{
	if(stype != SM_STRING)
	{
		declarationError("Initialization not allowed", sid);
	}

	int len = strlen(sval);
	strncpy(data.str, sval, len);
}

SymbolMember::SymbolMember(SymbolType stype, const string& sid, int sval) : id(sid), type(stype), next(NULL), intern(false), ptr(NULL)
{
	if(stype != SM_CONST)
	{
		declarationError("Initialization not allowed", sid);
	}

	data.val = sval;
}

SymbolMember::SymbolMember(SymbolType stype, const string& sid) : id(sid), type(stype), next(NULL), intern(false), ptr(NULL)
{
}

SymbolMember::SymbolMember(SymbolType stype, const string& sid, int sstart, int send) : id(sid), type(stype), next(NULL), start(sstart), end(send), intern(false), ptr(NULL)
{
	if (end - start <= 0)
	{
		declarationError("Array size must be positive value!", sid);
	}
}

SymbolMember::~SymbolMember()
{
}

tree SymbolMember::Compile()
{
	return NULL_TREE;
}

void SymbolMember::Print(int)
{
	cout << getSymbolName(type) << " : " << id;
	switch (type)
	{
		case SM_CONST:
			cout << " : " << data.val;
			break;
		case SM_STRING:
			cout << " : " << data.str;
			break;
		case SM_ARRAY:
			cout << " : " << start << " - " << end;
		default:
			;
	}
}

SymbolTable::SymbolTable() : head(NULL), last(NULL)
{

}

void SymbolTable::Print(int depth)
{
	SymbolMember* mem = head;
	while (mem != NULL)
	{
		if (!mem->intern)
		{
			mem->Print(depth + 1);
			cout << endl;
		}
		mem = mem->next;
	}
}

void SymbolTable::CompileGlobals(vec<tree, va_gc>** context)
{
	SymbolMember* mem = head;
	while (mem != NULL)
	{
		switch(mem->type)
		{
			case SM_CONST:
				mem->ptr = buildGlobalVariable(mem->id, false, mem->data.val);
				registerGlobalVariable(context, mem->ptr);
				break;
			case SM_VAR:
				mem->ptr = buildGlobalVariable(mem->id, true);
				registerGlobalVariable(context, mem->ptr);
				break;
			case SM_ARRAY:
				mem->ptr = buildGlobalArray(mem->id, mem->end - mem->start + 1);
				registerGlobalVariable(context, mem->ptr);
				break;
			default:
				break;
		}
		mem = mem->next;
	}
}

void SymbolTable::CompileLocals(vec<tree, va_gc>** context)
{
	SymbolMember* mem = head;
	while (mem != NULL)
	{
		switch(mem->type)
		{
			case SM_CONST:
				mem->ptr = buildGlobalVariable(mem->id, false, mem->data.val);
				registerGlobalVariable(context, mem->ptr);
				break;
			case SM_VAR:
				mem->ptr = buildGlobalVariable(mem->id, true);
				registerGlobalVariable(context, mem->ptr);
				break;
			case SM_ARRAY:
				mem->ptr = buildGlobalArray(mem->id, mem->end - mem->start + 1);
				registerGlobalVariable(context, mem->ptr);
				break;
			default:
				break;
		}
		mem = mem->next;
	}
}

tree SymbolTable::Compile()
{
	return NULL_TREE;
}

int SymbolTable::GetParamsCnt()
{
	int cnt = 0;
	SymbolMember* cur = head;
	while(cur != NULL)
	{
		cnt++;
		cur = cur->next;
	}
	return cnt;
}


SymbolTable::~SymbolTable()
{
	SymbolMember* cur = head;
	SymbolMember* next = NULL;
	while(cur != NULL)
	{
		next = cur->next;
		delete cur;
		cur = next;
	}
}

void SymbolTable::DeclConst(const string& id, int val)
{
	SymbolMember* mem = SearchId(id);
	if(mem != NULL)
	{
		declarationError("Double const declaration", id);
	}

	AddSymbol(new SymbolMember(SM_CONST, id, val));
}

void SymbolTable::DeclVar(const string& id, bool intern)
{
	SymbolMember* mem = SearchId(id);
	if (mem != NULL)
	{
		declarationError("Double variable declaration", id);
	}

	mem = new SymbolMember(SM_VAR, id);
	mem->intern = intern;
	AddSymbol(mem);
}

void SymbolTable::DeclParam(const string& id)
{
	SymbolMember* mem = SearchId(id);
	if (mem != NULL)
	{
		declarationError("Double variable declaration", id);
	}

	AddSymbol(new SymbolMember(SM_PARAM, id));
}

void SymbolTable::DeclArray(const string& id, int start, int end)
{
	SymbolMember* mem = SearchId(id);
	if (mem != NULL)
	{
		declarationError("Double variable declaration", id);
	}

	AddSymbol(new SymbolMember(SM_ARRAY, id, start, end));
}

void SymbolTable::DeclFunc(const string& id, bool intern)
{
	SymbolMember* mem = SearchId(id);
	if(mem != NULL)
	{
		declarationError("Double function declaration", id);
	}

	mem = new SymbolMember(SM_FUNC, id);
	mem->intern = intern;
	AddSymbol(mem);
}

void SymbolTable::DeclProced(const string& id)
{
	SymbolMember* mem = SearchId(id);
	if(mem != NULL)
	{
		declarationError("Double precedure declaration", id);
	}

	AddSymbol(new SymbolMember(SM_PROCED, id));
}

void SymbolTable::AddSymbol(SymbolMember* symbol)
{
	if(head == NULL)
	{
		head = symbol;
	}	
	else
	{
		last->next = symbol;
	}
	last = symbol;
}

SymbolMember* SymbolTable::SearchId(const string& id)
{
	SymbolMember* cur = head;
	while(cur != NULL)
	{
		if(cur->id == id)
			return cur;
		cur = cur->next;
	}
	return NULL;
}

bool SymbolTable::HasId(const string& id)
{
	SymbolMember* cur = head;
	while (cur != NULL)
	{
		if (cur->id == id)
			return true;
		cur = cur->next;
	}
	return false;
}

