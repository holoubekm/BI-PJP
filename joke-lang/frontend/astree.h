#ifndef _TREE_H_
#define _TREE_H_

#include <string>
#include "symtable.h"
#include "builder.h"

using namespace std;

class TreeArray;
class TreeAssign;
class TreeBinOp;
class TreeCall;
class TreeConst;
class TreeDeclList;
class TreeExpr;
class TreeExprList;
class TreeFor;
class TreeFunc;
class TreeFuncList;
class TreeIf;
class TreeMutable;
class TreeReturn;
class TreeStatm;
class TreeStatmList;
class TreeUnaryMin;
class TreeVar;
class TreeWhile;
class SymbolTable;

enum BinaryOp
{
	BO_ADD,
	BO_MIN,
	BO_MUL,
	BO_DIV,
	BO_MOD,

	BO_OR,
	BO_AND,

	BO_GT,
	BO_GE,
	BO_LT,
	BO_LE,
	BO_EQ,
	BO_NEQ
};


class TreeNode
{
public:
	TreeNode();
	virtual TreeNode* Optimize() { return this; };
	virtual void Print(int) = 0;
	virtual ~TreeNode() {};
};

class TreeProg : public TreeNode
{
public:
	TreeProg(const string&);
	TreeFunc* FindFunc(const string& );
	virtual TreeNode* Optimize();
	void Compile();
	virtual void Print(int = 0);
	virtual ~TreeProg();

	const string name;
	TreeFuncList* funcs;
	TreeStatmList* main;
	SymbolTable* globals;

	vec<tree, va_gc>** context;
};

class TreeFunc : public TreeNode
{
public:
	TreeFunc(const string&, TreeStatmList*, SymbolTable*, SymbolTable*, bool);
	virtual ~TreeFunc();
	virtual TreeNode* Optimize();
	tree Compile();
	virtual void Print(int);

	bool inter;
	const string id;
	TreeStatmList* body;
	SymbolTable* globals;
	SymbolTable* locals;
	bool isfunc;
};


class TreeFuncList : public TreeFunc
{
public:
	TreeFuncList(TreeFunc*, TreeFuncList*);
	virtual ~TreeFuncList();
	virtual TreeNode* Optimize();
	virtual void Print(int);

	TreeFuncList* next;
};







class TreeStatm : public TreeNode
{
public:
	TreeStatm();
	virtual ~TreeStatm();
	virtual tree Compile() = 0;
	virtual void Print(int);
	virtual TreeNode* Optimize(){ return this; }

};


class TreeAssign : public TreeStatm
{
public: 
	TreeAssign(TreeMutable*, TreeExpr*);
	virtual ~TreeAssign();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	TreeMutable* dest;
	TreeExpr* source;
};

class TreeReturn : public TreeStatm
{
public:
	TreeReturn();
	virtual ~TreeReturn();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);
};


class TreeIf : public TreeStatm
{
public:
	TreeIf(TreeExpr*, TreeStatm*, TreeStatm*);
	virtual ~TreeIf();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	TreeExpr* expr;
	TreeStatm* posBranch;
	TreeStatm* negBranch;
};


class TreeWhile : public TreeStatm
{
public:
	TreeWhile(TreeExpr*, TreeStatm*);
	virtual ~TreeWhile();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	TreeExpr* expr;
	TreeStatm* body;
};

class TreeFor : public TreeStatm
{
public:
	TreeFor(TreeExpr*, TreeExpr*, TreeExpr*, TreeStatm*, bool);
	virtual ~TreeFor();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	TreeExpr* var;
	TreeExpr* from;
	TreeExpr* to;
	TreeStatm* body;
	bool upto;
};


class TreeStatmList : public TreeStatm
{
public:
	TreeStatmList(TreeStatm*, TreeStatmList*);
	virtual ~TreeStatmList();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	TreeStatm* head;
	TreeStatmList* next;
};







class TreeExpr : public TreeNode
{
public:
	virtual tree Compile() = 0;
	virtual int Length() { return 1; }
};

class TreeConst : public TreeExpr
{
public:
	TreeConst(int);
	virtual ~TreeConst();
	virtual void Print(int);
	virtual TreeNode* Optimize(){ return this; };
	virtual tree Compile();
	int val;
};


class TreeString : public TreeExpr
{
public:
	TreeString(const string&);
	virtual ~TreeString();
	virtual void Print(int);
	virtual TreeNode* Optimize(){ return this; };
	virtual tree Compile();

	const string str;
};

class TreeCall : public TreeExpr, public TreeStatm
{
public:
	TreeCall(const string&, TreeExprList*);
	virtual ~TreeCall();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	const string id;
	TreeExprList* params;
};

class TreeExprList : public TreeExpr
{
public:
	TreeExprList(TreeExpr*, TreeExprList*);
	virtual ~TreeExprList();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);
	virtual int Length() 
	{ 
		int cnt = 0;
		TreeExprList* cur = this;
		while(cur != NULL)
		{
			cur = cur->next;
			cnt++;
		} 
		return cnt;
	}


	TreeExpr* head;
	TreeExprList* next;
};


class TreeBinOp : public TreeExpr
{
public:
	TreeBinOp(TreeExpr*, BinaryOp, TreeExpr*);
	virtual ~TreeBinOp();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	TreeExpr* lexpr;
	BinaryOp op;
	TreeExpr* rexpr;
};

class TreeUnaryMin : public TreeExpr
{
public:
	TreeUnaryMin(TreeExpr*);
	virtual ~TreeUnaryMin();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);


	TreeExpr* expr;
};

class TreeMutable : public TreeExpr
{

};

class TreeArray : public TreeMutable
{
public:
	TreeArray(const string&, TreeExpr*);
	virtual ~TreeArray();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	const string id;
	TreeExpr* index;
};


class TreeVar : public TreeMutable
{
public:
	TreeVar(const string&);
	virtual ~TreeVar();
	virtual TreeNode* Optimize();
	virtual tree Compile();
	virtual void Print(int);

	const string id;
};






class SymbolMember : TreeNode
{
public:
	SymbolMember();
	SymbolMember(SymbolType stype, const string& sid);
	SymbolMember(SymbolType stype, const string& sid, const char* sval);
	SymbolMember(SymbolType stype, const string& sid, int sval);
	SymbolMember(SymbolType stype, const string& sid, int start, int end);

	virtual ~SymbolMember();
	virtual void Print(int);
	virtual TreeNode* Optimize(){ return this; };
	virtual tree Compile();

	string id;
	SymbolType type;
	SymbolData data;
	SymbolMember* next;
	int start;
	int end;
	bool intern;
	tree ptr;
};

class SymbolTable : public TreeNode
{
public:
	SymbolTable();
	virtual void Print(int);
	virtual TreeNode* Optimize(){ return this; }
	virtual tree Compile();
	void CompileGlobals(vec<tree, va_gc>**);
	void CompileLocals(vec<tree, va_gc>**);
	virtual ~SymbolTable();
	void AddSymbol(SymbolMember* symbol);
	void DeclArray(const string& id, int start, int end);
	void DeclConst(const string& id, int val);
	void DeclVar(const string& id, bool fintern = false);
	void DeclParam(const string& id);
	void DeclFunc(const string& id, bool fintern = false);
	void DeclProced(const string& id);

	int GetParamsCnt();

	SymbolMember* SearchId(const string& id);
	bool HasId(const string& id);
public:

	SymbolMember* head;
	SymbolMember* last;
};

#endif //_TREE_H_
