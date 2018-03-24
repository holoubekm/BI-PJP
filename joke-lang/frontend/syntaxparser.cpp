#include <iostream>
#include <fstream>

#include "syntaxparser.h"

#define semanticError(type) comparisonError_(type, __LINE__);
#define comparisonError(type) comparisonError_(type, __LINE__);
#define expansionError(term, type) expansionError_(term, type, __LINE__);

using namespace std;

void semanticError_(const string& msg, int line)
{
	cerr << "Semantic error! \"" << msg << "\" Thrown at line: \"" << line << "\"" << endl;
	exit(2);
}

void comparisonError_(LexemType type, int line)
{
	cerr << "Comparison error! Expected lexem: \"" << getLexemName(type) << "\". Thrown at line: \"" << line << "\"" << endl;
	exit(2);
}

void comparisonError_(const string& msg, int line)
{
	cerr << "Comparison error! \"" << msg << "\" Thrown at line: \"" << line << "\"" << endl;
	exit(2);
}

void expansionError_(const string& nterm, LexemType type, int line)
{
	cerr << "Expansion error! Non terminal: \"" << nterm << "\", lexem: \"" << getLexemName(type) <<  "\". Thrown at line: \"" << line << "\"" << endl;
	exit(2);
}


SynParser::SynParser(const string& input) : curLexem(NULL), nextLexem(NULL),  prog(NULL), curFunc(NULL)
{
	in.open(input.c_str());
	if(in.is_open())
	{
    	lexer = new yyFlexLexer(&in, &cerr);
	}
	else
	{
		cerr << "Can't open input file: " << input << endl;
		exit(4);
	}
}

SynParser::~SynParser()
{
	if(curLexem)
		delete curLexem;
	curLexem = NULL;
	if(nextLexem)
		delete nextLexem;
	nextLexem = NULL;
	if(lexer)
		delete lexer;
	lexer = NULL;
	if(in.is_open())
	{
		in.close();	
	}
}

void SynParser::NextLexem()
{
	if(curLexem)
		delete curLexem;
	curLexem = nextLexem;

	lexer->yylex();

	//global extern variable;
	nextLexem = _curLexem;

	//if(curLexem != NULL)
		//cout << "curLexem: " << *curLexem << endl;
	//if(nextLexem != NULL)
		//cout << "nextLexem: " << *nextLexem << endl;
	
}

void SynParser::Comparison(LexemType type)
{
	//cout << "compared: " << *curLexem << endl;
	if (curLexem->type == type)
	{
		NextLexem();
	}
	else if(type != SYM_SEMIC)
		comparisonError(type);
}

void SynParser::ComparisonINT(int& out)
{
	bool minus = false;
	if(curLexem->type == OP_MIN)
	{
		minus = true;
		NextLexem();
	}	
		
	if (curLexem->type == CONST)
	{
		if(minus)
			curLexem->data.val *= -1;
		out = curLexem->data.val;
		NextLexem();
	}
	else
		comparisonError(CONST);
}

void SynParser::ComparisonIDENT(string& out)
{
	if (curLexem->type == IDENT)
	{
		out = curLexem->data.str;
		NextLexem();
	}
	else
		comparisonError(IDENT);
}

void SynParser::ComparisonSTRING(string& out)
{
	if (curLexem->type == STRING)
	{
		out = curLexem->data.str;
		NextLexem();
	}
	else
		comparisonError(IDENT);
}

TreeProg* SynParser::Parse()
{
	NextLexem();
	NextLexem();

	TreeProg* prog = DeclProgram();
	if(prog->main == NULL)
		semanticError("Program should have begin .. end");
	return prog;
}

TreeProg* SynParser::DeclProgram()
{
	switch(curLexem->type)
	{
		case KW_PROG:
		{
			string name;
			Comparison(KW_PROG);
			ComparisonIDENT(name);
			Comparison(SYM_SEMIC);
			prog = new TreeProg(name);
			return Decl(prog);
		}
		default:
			comparisonError(KW_PROG);
	}
	return NULL;
}

TreeProg* SynParser::Decl(TreeProg* prog)
{
	TreeStatmList* main = NULL;
	switch(curLexem->type)
	{
		case KW_VAR:
			NextLexem();
			DeclVar(prog->globals);
			Decl(prog);
			break;
		case KW_CONST:
			DeclConst(prog->globals);
			Decl(prog);
			break;
		case KW_PROCED:
			DeclProced(prog);
			Decl(prog);
			break;
		case KW_FUNC:
			DeclFunc(prog);
			Decl(prog);
			break;
		case KW_BEGIN:
			NextLexem();
			main = DeclBody();
			Comparison(KW_END);
			Comparison(SYM_DOT);
			prog->main = main;
			break;
		default:
			;
	}

	return prog;
}


//	+------+--------+--------+--------+--------+--------+
//	|													|
//	+				Object Declarations					+
//	|													|
//	+------+--------+--------+--------+--------+--------+


SymbolTable* SynParser::DeclLocals(SymbolTable* lt)
{
	switch(curLexem->type)
	{
		case KW_VAR:
			NextLexem();
			DeclVar(lt);
			DeclLocals(lt);
			break;
		case KW_CONST:
			DeclConst(lt);
			DeclLocals(lt);
			break;
		default:
			;
	}
	return lt;
}

SymbolTable* SynParser::DeclVar(SymbolTable* st)
{
	if(curLexem->type == IDENT)
	{
		string id;
		ComparisonIDENT(id);
		switch(curLexem->type)
		{
			case SYM_COMMA:
				st->DeclVar(id);
				DeclVarRest(st);
				Comparison(SYM_COLON);
				break;
			case SYM_COLON:
				if(nextLexem->type == KW_INT)
				{
					NextLexem();
					st->DeclVar(id);
					break;
				}

				int start, end;
				Comparison(SYM_COLON);
				Comparison(KW_ARRAY);
				Comparison(SYM_LSBRC);
				ComparisonINT(start);
				Comparison(SYM_DDOT);
				ComparisonINT(end);
				Comparison(SYM_RSBRC);
				Comparison(KW_OF);
				st->DeclArray(id, start, end);
				break;
			default:
			;
		}
		Comparison(KW_INT);
		Comparison(SYM_SEMIC);
		DeclVar(st);
	}
	return st;
}

SymbolTable* SynParser::DeclVarRest(SymbolTable* st)
{
	if(curLexem->type == SYM_COMMA)
	{
		string id;
		Comparison(SYM_COMMA);
		ComparisonIDENT(id);
		st->DeclVar(id);
		DeclVarRest(st);
	}
	return st;
}

SymbolTable* SynParser::DeclConst(SymbolTable* st)
{
	int val;
	string id;
	NextLexem();
	ComparisonIDENT(id);
	Comparison(OP_EQ);
	ComparisonINT(val);
	Comparison(SYM_SEMIC);

	st->DeclConst(id, val);
	DeclConstRest(st);
	return st;
}

SymbolTable* SynParser::DeclConstRest(SymbolTable* st)
{
	if(curLexem->type == IDENT)
	{
		int val;
		string id;
		ComparisonIDENT(id);
		Comparison(OP_EQ);
		ComparisonINT(val);
		Comparison(SYM_SEMIC);

		st->DeclConst(id, val);
		DeclConstRest(st);
	}
	return st;
}

TreeProg* SynParser::DeclProced(TreeProg* prog)
{	
	//function fibonacci(n : integer) : integer;
	curFunc = NULL;
	TreeStatmList* body = NULL;
	SymbolTable* locals = new SymbolTable();
	string id;
	NextLexem();
	ComparisonIDENT(id);
	
	int oldParamsCnt = -1;
	if (prog->globals->HasId(id))
	{
		curFunc = prog->FindFunc(id);
		if (!curFunc || (curFunc != NULL && curFunc->body != NULL))
			semanticError("Procedure redeclaration");

		oldParamsCnt = curFunc->locals->GetParamsCnt();
		delete curFunc->locals;
		curFunc->locals = locals;
	}
	else
	{
		prog->globals->DeclFunc(id, false);
		curFunc = new TreeFunc(id, NULL, prog->globals, locals, false);
	}

	Comparison(SYM_LBRAC);
	locals = DeclParameter(locals);
	Comparison(SYM_RBRAC);
	Comparison(SYM_SEMIC);

	int curParamsCnt = locals->GetParamsCnt();
	if(oldParamsCnt != -1 && curParamsCnt != oldParamsCnt)
		semanticError("Number of params doesn't match forward declaration.");

	if(curLexem->type == KW_FORW)
	{
		Comparison(KW_FORW);
		Comparison(SYM_SEMIC);

		prog->funcs = new TreeFuncList(curFunc, prog->funcs);
		return prog;
	}

	locals = DeclLocals(locals);

	Comparison(KW_BEGIN);

	body = DeclBody();
	TreeStatmList* cur = body;
	while (cur && cur->next)
		cur = cur->next;

	if(!cur)
		semanticError("Function should have body");
	
	// cur->next = new TreeStatmList(new TreeReturn(), NULL);

	Comparison(KW_END);
	Comparison(SYM_SEMIC);

	curFunc->body = body;
	prog->funcs = new TreeFuncList(curFunc, prog->funcs);
	return prog;
}

TreeProg* SynParser::DeclFunc(TreeProg* prog)
{	
	//function fibonacci(n : integer) : integer;
	curFunc = NULL;
	TreeStatmList* body = NULL;
	SymbolTable* locals = new SymbolTable();
	string id;
	NextLexem();
	ComparisonIDENT(id);
	
	int oldParamsCnt = -1;
	if (prog->globals->HasId(id))
	{
		curFunc = prog->FindFunc(id);
		if (!curFunc || (curFunc != NULL && curFunc->body != NULL))
			semanticError("Function redeclaration");

		oldParamsCnt = curFunc->locals->GetParamsCnt();
		delete curFunc->locals;
		curFunc->locals = locals;
	}
	else
	{
		prog->globals->DeclFunc(id, false);
		curFunc = new TreeFunc(id, NULL, prog->globals, locals, true);
	}

	Comparison(SYM_LBRAC);
	locals = DeclParameter(locals);
	locals->DeclVar("_ret");
	Comparison(SYM_RBRAC);
	Comparison(SYM_COLON);
	Comparison(KW_INT);
	Comparison(SYM_SEMIC);

	int curParamsCnt = locals->GetParamsCnt();
	if(oldParamsCnt != -1 && curParamsCnt != oldParamsCnt)
		semanticError("Number of params doesn't match forward declaration.");

	if(curLexem->type == KW_FORW)
	{
		Comparison(KW_FORW);
		Comparison(SYM_SEMIC);

		prog->funcs = new TreeFuncList(curFunc, prog->funcs);
		return prog;
	}

	locals = DeclLocals(locals);

	Comparison(KW_BEGIN);

	body = DeclBody();
	TreeStatmList* cur = body;

	while (cur && cur->next)
		cur = cur->next;

	if(!cur)
		semanticError("Function should have body");

	// cur->next = new TreeStatmList(new TreeReturn(), NULL);

	Comparison(KW_END);
	Comparison(SYM_SEMIC);

	curFunc->body = body;
	prog->funcs = new TreeFuncList(curFunc, prog->funcs);
	return prog;
}

SymbolTable* SynParser::DeclParameter(SymbolTable* tb)
{
	if(curLexem->type == IDENT)
	{
		string id;
		ComparisonIDENT(id);

		if (tb->HasId(id))
			semanticError("Param redeclaration");
		tb->DeclParam(id);

		Comparison(SYM_COLON);
		Comparison(KW_INT);

		if(curLexem->type == SYM_SEMIC)
		{
			Comparison(SYM_SEMIC);
			DeclParameter(tb);
		}
	}
	return tb;
}


//	+------+--------+--------+--------+--------+--------+
//	|													|
//	+				Command Expressions					+
//	|													|
//	+------+--------+--------+--------+--------+--------+

TreeStatmList* SynParser::DeclBody()
{
	TreeStatm* line;
	switch(curLexem->type)
	{
		case KW_IF:
		case KW_FOR:
		case KW_WHILE:
		case KW_READ:
		case KW_WRITE:
		case KW_WRITELN:
		case IDENT:
			line = DeclCommand();
			return new TreeStatmList(line, DeclBody());
		case KW_EXIT:
			Comparison(KW_EXIT);
			Comparison(SYM_SEMIC);
			return new TreeStatmList(new TreeReturn(), DeclBody());
		default:
			;
			//expansionError(getLexemName(curLexem->type), curLexem->type);
	}
	return NULL;
}

TreeStatm* SynParser::DeclCommand()
{
	switch(curLexem->type)
	{
		case KW_IF:
		{
			return DeclIfStatement();
		}
		case KW_FOR:
		{
			return DeclForLoop();
		}
		case KW_WHILE:
		{
			return DeclWhileLoop();
		}
		case IDENT:
		{
			TreeStatm* statm = DeclCallOrAssig();
			Comparison(SYM_SEMIC);
			return statm;
		}
		case KW_WRITE:
		{
			NextLexem();
			Comparison(SYM_LBRAC);
			TreeCall* call = new TreeCall("write", DeclArithExprList());
			Comparison(SYM_RBRAC);
			Comparison(SYM_SEMIC);
			return call;
		}
		case KW_WRITELN:
		{
			NextLexem();
			Comparison(SYM_LBRAC);
			TreeCall* call = new TreeCall("writeln", DeclArithExprList());
			Comparison(SYM_RBRAC);
			Comparison(SYM_SEMIC);
			return call;
		}
		case KW_READ:
		{
			NextLexem();
			Comparison(SYM_LBRAC);
			TreeCall* call = new TreeCall("readln", DeclArithExprList());
			Comparison(SYM_RBRAC);
			Comparison(SYM_SEMIC);
			return call;
		}
		case KW_EXIT:
		{
			Comparison(KW_EXIT);
			Comparison(SYM_SEMIC);
			return new TreeReturn();
		}
		default:
			comparisonError("Statement expected");
		;
	}
	return NULL;
}



//	+------+--------+--------+--------+--------+--------+
//	|													|
//	+				Control Statements					+
//	|													|
//	+------+--------+--------+--------+--------+--------+


TreeStatm* SynParser::DeclIfStatement()
{
	TreeStatm* posbranch = NULL;
	TreeStatm* negbranch = NULL;
	Comparison(KW_IF);
	TreeExpr* cond = DeclLogExpr();
	Comparison(KW_THEN);
	if (curLexem->type == KW_BEGIN)
	{
		Comparison(KW_BEGIN);
		posbranch = DeclBody();
		Comparison(KW_END);
		Comparison(SYM_SEMIC);
	}
	else
	{
		posbranch = DeclCommand();
	}
	if (curLexem->type == KW_ELSE)
	{
		Comparison(KW_ELSE);
		if (curLexem->type == KW_BEGIN)
		{
			Comparison(KW_BEGIN);
			negbranch = DeclBody();
			Comparison(KW_END);
			Comparison(SYM_SEMIC);
		}
		else
		{
			negbranch = DeclCommand();
		}
	}

	if (posbranch == NULL)
	{
		semanticError("If statement should have body");
	}

	return new TreeIf(cond, posbranch, negbranch);
}

TreeStatm* SynParser::DeclForLoop()
{
	//for I := 1 to 20 do begin
	string id;
	bool raise = true;
	NextLexem();
	ComparisonIDENT(id);
	TreeVar* var = new TreeVar(id);

	Comparison(OP_ASIG);
	TreeExpr* start = DeclArithExpr();
	if (curLexem->type == KW_TO)
		raise = true;
	else if (curLexem->type == KW_DOWNTO)
		raise = false;
	else
		comparisonError("Expected KW_TO, KW_DOWNTO");
	NextLexem();
	
	TreeExpr* end = DeclArithExpr();
	
	Comparison(KW_DO);

	TreeStatm* body = NULL;
	if(curLexem->type == KW_BEGIN)
	{
		Comparison(KW_BEGIN);
		body = DeclBody();
		Comparison(KW_END);
		Comparison(SYM_SEMIC);
	}
	else
	{
		body = DeclCommand();
	}

	if (body == NULL)
	{
		semanticError("For loop should have body.");
	}

	return new TreeFor(var, start, end, body, raise);
}

TreeStatm* SynParser::DeclWhileLoop()
{
	Comparison(KW_WHILE);
	TreeExpr* cond = DeclLogExpr();
	Comparison(KW_DO);

	TreeStatm* body = NULL;
	if(curLexem->type == KW_BEGIN)
	{
		Comparison(KW_BEGIN);
		body = DeclBody();
		Comparison(KW_END);
		Comparison(SYM_SEMIC);
	}
	else
	{
		body = DeclCommand();
	}

	if (body == NULL)
	{
		semanticError("While Loop should have body.");
	}
	return new TreeWhile(cond, body);
}

TreeStatm* SynParser::DeclCallOrAssig()
{
	string id;
	TreeStatm* callofasig = NULL;
	ComparisonIDENT(id);

	SymbolMember* sym = FindSymbol(id);
	if(curLexem->type == SYM_LSBRC)
	{
		//Array assignment
		Comparison(SYM_LSBRC);
		TreeExpr* index = DeclArithExpr();
		Comparison(SYM_RSBRC);
		Comparison(OP_ASIG);
		TreeExpr* rexpr = DeclArithExpr();
		
		if (!sym || sym->type != SM_ARRAY)
			semanticError("Indexed variable must be array");
		
		TreeConst* ind = dynamic_cast<TreeConst*>(index);
		if (ind && (ind->val < sym->start || ind->val > sym->end))
			semanticError("Array index out of bounds");
		
		callofasig =  new TreeAssign(new TreeArray(id, index), rexpr);
	}
	else if(curLexem->type == SYM_LBRAC)
	{
		//Function call
		Comparison(SYM_LBRAC);
		TreeExprList* params = DeclArithExprList();
		Comparison(SYM_RBRAC);

		if (!sym || sym->type != SM_FUNC)
			semanticError("Only function can be called");

		callofasig = new TreeCall(id, params);
	}
	else if (curLexem->type == OP_ASIG)
	{
		//Variable assignment
		Comparison(OP_ASIG);
		TreeExpr* rside = DeclArithExpr();

		if (!sym || (sym->type != SM_VAR && sym->type != SM_FUNC && sym->type != SM_PARAM))
			semanticError("Left side must be variable or static array");

		if (sym->type == SM_FUNC)
			callofasig = new TreeAssign(new TreeVar("_ret"), rside);
		else
			callofasig = new TreeAssign(new TreeVar(id), rside);
	}
	Comparison(SYM_SEMIC);
	return callofasig;
}




//	+------+--------+--------+--------+--------+--------+
//	|													|
//	+				Logical Expressions					+
//	|													|
//	+------+--------+--------+--------+--------+--------+


TreeExpr* SynParser::DeclLogExpr()
{
	return DeclLogExprOred();
}

TreeExpr* SynParser::DeclLogExprOred()
{
	//ored-logical-expr    ::= <anded-logical-expr> ( <OR> <anded-logical-expr> ) *
	TreeExpr* lexpr = DeclLogExprAnded();
	return DeclLogExprOredRest(lexpr);
}

TreeExpr* SynParser::DeclLogExprOredRest(TreeExpr* du)
{
	if(curLexem->type == OP_OR)
	{
		NextLexem();
		TreeExpr* lexpr = DeclLogExprAnded();
		TreeExpr* rexpr = DeclLogExprOredRest(lexpr);
		return new TreeBinOp(du, BO_OR, rexpr);
	}
	return du;
}

TreeExpr* SynParser::DeclLogExprAnded()
{
	//anded-logical-expr   ::= <unary-logical-expr> ( <AND> <unary-logical-expr> ) *
	TreeExpr* lexpr = DeclLogExprUnary();
	return DeclLogExprAndedRest(lexpr);
}

TreeExpr* SynParser::DeclLogExprAndedRest(TreeExpr* du)
{
	if(curLexem->type == OP_AND)
	{
		NextLexem();
		TreeExpr* lexpr = DeclLogExprUnary();
		TreeExpr* rexpr = DeclLogExprAndedRest(lexpr);
		return new TreeBinOp(du, BO_AND, rexpr);
	}
	return du;
}

TreeExpr* SynParser::DeclLogExprUnary()
{
	//unary-logical-expr   ::= <arith-expr> [ <logical-operator> <arith-expr> ] 
	TreeExpr* lexpr = DeclArithExpr();
	TreeExpr* rexpr = NULL;
	switch(curLexem->type)
	{
		case OP_GT:
			NextLexem();
			rexpr = DeclArithExpr();
			return new TreeBinOp(lexpr, BO_GT, rexpr);
		case OP_GE:
			NextLexem();
			rexpr = DeclArithExpr();
			return new TreeBinOp(lexpr, BO_GE, rexpr);
		case OP_LT:
			NextLexem();
			rexpr = DeclArithExpr();
			return new TreeBinOp(lexpr, BO_LT, rexpr);
		case OP_LE:
			NextLexem();
			rexpr = DeclArithExpr();
			return new TreeBinOp(lexpr, BO_LE, rexpr);
		case OP_EQ:
			NextLexem();
			rexpr = DeclArithExpr();
			return new TreeBinOp(lexpr, BO_EQ, rexpr);
		case OP_NEQ:
			NextLexem();
			rexpr = DeclArithExpr();
			return new TreeBinOp(lexpr, BO_NEQ, rexpr);
		default:
			;
	}
	return lexpr;
}


//	+------+--------+--------+--------+--------+--------+
//	|													|
//	+	          Arithmetical Expressions				+
//	|													|
//	+------+--------+--------+--------+--------+--------+


TreeExpr* SynParser::DeclArithExpr()
{
	return DeclArithExprAdd();
}

TreeExpr* SynParser::DeclArithExprAdd()
{
	//<multive-expr> ( <"+"|"-"> multive-expr )*
	TreeExpr* lexpr = DeclArithExprMult();
	return DeclArithExprAddRest(lexpr);
}

TreeExpr* SynParser::DeclArithExprAddRest(TreeExpr* du)
{
	switch(curLexem->type)
	{
		case OP_ADD:
		{
			NextLexem();
			TreeExpr* lexpr = DeclArithExprMult();
			TreeExpr* rexpr = DeclArithExprAddRest(lexpr);
			return new TreeBinOp(du, BO_ADD, rexpr);

		}
		case OP_MIN:
		{
			NextLexem();
			TreeExpr* lexpr = DeclArithExprMult();
			TreeExpr* rexpr = DeclArithExprAddRest(lexpr);
			return new TreeBinOp(du, BO_MIN, rexpr);
		}
		default:
			;
	}
	return du;
}

TreeExpr* SynParser::DeclArithExprMult()
{
	//<unary-arith-expr> ( <"*"|"/"> <unary-arith-expr> ) *
	TreeExpr* lexpr = DeclArithUnaryExpr();
	return DeclArithExprMultRest(lexpr);
	//return NULL;
}

TreeExpr* SynParser::DeclArithExprMultRest(TreeExpr* du)
{
	switch(curLexem->type)
	{
		case OP_MUL:
		{
			NextLexem();
			TreeExpr* lexpr = DeclArithUnaryExpr();
			TreeExpr* rexpr = DeclArithExprMultRest(lexpr);
			return new TreeBinOp(du, BO_MUL, rexpr);
		}
		case OP_DIV:
		{
			NextLexem();
			TreeExpr* lexpr = DeclArithUnaryExpr();
			TreeExpr* rexpr = DeclArithExprMultRest(lexpr);
			return new TreeBinOp(du, BO_DIV, rexpr);
		}
		case OP_MOD:
		{
			NextLexem();
			TreeExpr* lexpr = DeclArithUnaryExpr();
			TreeExpr* rexpr = DeclArithExprMultRest(lexpr);
			return new TreeBinOp(du, BO_MOD, rexpr);
		}
		default:
			;
	}
	return du;
}

TreeExpr* SynParser::DeclArithUnaryExpr()
{
	//::= "(" <logical-expr> ")" | ("+" | "-")  <unary-arith-expr> | <INTEGER>
	switch(curLexem->type)
	{
		case SYM_LBRAC:
		{
			Comparison(SYM_LBRAC);
			TreeExpr* expr = DeclLogExpr();
			Comparison(SYM_RBRAC);
			return expr;
		}
		case OP_ADD:
			NextLexem();
			return DeclArithUnaryExpr();
		case OP_MIN:
			NextLexem();
			return new TreeUnaryMin(DeclArithUnaryExpr());
		case CONST:
		{
			int val;
			ComparisonINT(val);
			return new TreeConst(val);
		}
		case STRING:
		{
			string id;
			ComparisonSTRING(id);
			return new TreeString(id);
			break;
		}
		case KW_READ:
		{
			Comparison(KW_READ);
			return new TreeCall("readln", NULL);
		}
		case IDENT:
		{
			string id;
			ComparisonIDENT(id);
			SymbolMember* sym = FindSymbol(id);
			switch (curLexem->type)
			{
				case SYM_LBRAC:
				{
					if (!sym || (sym->type != SM_FUNC && sym->type != SM_PROCED))
						semanticError("Only function or procedure may be called");
					TreeExprList* params;
					Comparison(SYM_LBRAC);
					params = DeclArithExprList();
					Comparison(SYM_RBRAC);
					return new TreeCall(id, params);
				}
				case SYM_LSBRC:
				{
					Comparison(SYM_LSBRC);
					TreeExpr* index = DeclArithExpr();
					Comparison(SYM_RSBRC);

					if (!sym || sym->type != SM_ARRAY)
						semanticError("Indexed variable must be array");

					TreeConst* ind = dynamic_cast<TreeConst*>(index);
					if (ind && (ind->val < sym->start || ind->val > sym->end))
						semanticError("Array index out of bounds");


					return new TreeArray(id, index);
				}
				default:
				{
					if (!sym || (sym->type != SM_VAR && sym->type != SM_PARAM && sym->type != SM_CONST && sym->type != SM_FUNC))
						semanticError("Undefined variable");
					if (sym->type == SM_FUNC)
						return new TreeVar("_ret");
					return new TreeVar(id);
				}
			}
			break;
		}
		default:
			;
			//expansionError(getLexemName(curLexem->type), curLexem->type);
	}
	return NULL;
}

TreeExprList* SynParser::DeclArithExprList()
{
	TreeExpr* expr = DeclArithExpr();
	return new TreeExprList(expr, DeclArithExprListRest());
}

TreeExprList* SynParser::DeclArithExprListRest()
{
	if (curLexem->type == SYM_COMMA)
	{
		Comparison(SYM_COMMA);
		TreeExpr* expr = DeclArithExpr();
		return new TreeExprList(expr, DeclArithExprListRest());
	}
	return NULL;
}



SymbolMember* SynParser::FindSymbol(const string& id)
{
	if (curFunc && curFunc->locals)
	{
		SymbolMember* cur = curFunc->locals->SearchId(id);
		cout << "local " << id << endl;
		if (cur)
			return cur;
	}

	if (prog && prog->globals)
	{
		SymbolMember* cur = prog->globals->SearchId(id);
		cout << "global " << id << endl;
		if (cur)
			return cur;
	}

	return NULL;
}