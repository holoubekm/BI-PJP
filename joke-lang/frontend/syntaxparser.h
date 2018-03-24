#ifndef _SYNTAX_PARSER_H_
#define _SYNTAX_PARSER_H_

#include <fstream>
#include "FlexLexer.h"
#include "lextypes.h"
#include "symtable.h"
#include "astree.h"

void semanticError_(const string& msg, int line);
void comparisonError_(LexemType type, int line);
void comparisonError_(const string& msg, int line);
void expansionError_(const string& nterm, LexemType type, int line);

extern Lexem* _curLexem;

class SynParser
{
public:
	SynParser(const string&);
	~SynParser();
	TreeProg* Parse();
	void NextLexem();

	SymbolMember* FindSymbol(const string&);

	TreeProg* DeclProgram();
	TreeProg* Decl(TreeProg*);
	SymbolTable* DeclConst(SymbolTable*);
	SymbolTable* DeclConstRest(SymbolTable*);
	SymbolTable* DeclVar(SymbolTable*);
	SymbolTable* DeclVarRest(SymbolTable*);

	TreeProg* DeclFunc(TreeProg*);
	TreeProg* DeclProced(TreeProg*);

	SymbolTable* DeclParameter(SymbolTable*);
	SymbolTable* DeclLocals(SymbolTable* );
	TreeStatmList* DeclBody();
	TreeStatm* DeclCommand();

	TreeStatm* DeclIfStatement();
	TreeStatm* DeclForLoop();
	TreeStatm* DeclWhileLoop();
	TreeStatm* DeclCallOrAssig();
	
	TreeExpr* DeclLogExpr();
	TreeExpr* DeclLogExprOred();
	TreeExpr* DeclLogExprOredRest(TreeExpr*);
	TreeExpr* DeclLogExprAnded();
	TreeExpr* DeclLogExprAndedRest(TreeExpr*);
	TreeExpr* DeclLogExprUnary();

	TreeExpr* DeclArithExpr();
	TreeExpr* DeclArithExprAdd();
	TreeExpr* DeclArithExprAddRest(TreeExpr*);
	TreeExpr* DeclArithExprMult();
	TreeExpr* DeclArithExprMultRest(TreeExpr*);
	TreeExpr* DeclArithUnaryExpr();
	TreeExprList* DeclArithExprList();
	TreeExprList* DeclArithExprListRest();

private:
	void Comparison(LexemType);
	void ComparisonINT(int&);
	void ComparisonIDENT(string&);
	void ComparisonSTRING(string&);

	// SymbolTable* symbols;

	ifstream in;
	yyFlexLexer* lexer;
	Lexem* curLexem;
	Lexem* nextLexem;

public:
	TreeProg* prog;
	TreeFunc* curFunc;
};

#endif //_SYNTAX_PARSER_H_