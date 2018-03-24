#include "astree.h"
#include "syntaxparser.h"

#include "config.h"
#include "system.h"
#include "ansidecl.h"
#include "coretypes.h"

#include "toplev.h"

#include "diagnostic-core.h"
#include "input.h"

#include "tm.h"

#include "hash-set.h"
#include "machmode.h"
#include "vec.h"
#include "double-int.h"
#include "input.h"
#include "alias.h"
#include "symtab.h"
#include "options.h"
#include "wide-int.h"
#include "inchash.h"

#include "tree.h"
#include "fold-const.h"
#include "tree-dump.h"
#include "tree-iterator.h"

#include "tree-ssa-operands.h"
#include "tree-pass.h"
#include "tree-ssa-alias.h"
#include "bitmap.h"
#include "symtab.h"

#include "hard-reg-set.h"
#include "function.h"
#include "langhooks-def.h"
#include "langhooks.h"
#include "stringpool.h"
#include "is-a.h"

#include "gimple-expr.h"

#include "predict.h"
#include "basic-block.h"
#include "gimple.h"
#include "gimplify.h"
#include "ipa-ref.h"
#include "lto-streamer.h"
#include "cgraph.h"
#include "opts.h"

#include "print-tree.h"
#include "tree-cfg.h"

#include "tree-iterator.h"
#include "print-tree.h"
#include "opts.h"
#include "stringpool.h"
#include "debug.h" 



extern SynParser* parser;


TreeNode::TreeNode()
{

}

TreeProg::TreeProg(const string& tname) : name(tname), funcs(NULL), main(NULL), globals(new SymbolTable()), context(NULL)
{
	globals->DeclFunc("main", true);
	globals->DeclFunc("inc", true);
	globals->DeclFunc("dec", true);
	globals->DeclFunc("readln", true);
	globals->DeclFunc("write", true);
	globals->DeclFunc("writeln", true);
/*
	TreeFunc* dec = new TreeFunc("dec", new TreeStatmList(NULL, NULL), globals, new SymbolTable(), true);
	dec->inter = true;
	TreeFunc* write = new TreeFunc("write", new TreeStatmList(NULL, NULL), globals, new SymbolTable(), true);
	write->inter = true;
	TreeFunc* writeln = new TreeFunc("writeln", new TreeStatmList(NULL, NULL), globals, new SymbolTable(), true);
	writeln->inter = true;
	TreeFunc* readln = new TreeFunc("readln", new TreeStatmList(NULL, NULL), globals, new SymbolTable(), true);
	readln->inter = true;

	TreeFuncList* newfuncs = new TreeFuncList(dec, funcs);
	newfuncs = new TreeFuncList(write, newfuncs);
	newfuncs = new TreeFuncList(writeln, newfuncs);
	funcs = new TreeFuncList(readln, newfuncs);*/
}

TreeFunc* TreeProg::FindFunc(const string& id)
{
	TreeFuncList* list = this->funcs;
	while (list != NULL)
	{
		if (list->id == id)
			return list;
		list = list->next;
	}
	return NULL;
}

TreeProg::~TreeProg()
{
	delete globals;
}

void TreeProg::Print(int depth)
{
	cout << "Program: " << name << endl;
	cout << "Global decls:" << endl;
	globals->Print(depth + 1);
	cout << endl;
	cout << "Functions:" << endl;
	TreeFuncList* func = funcs;
	while (func != NULL)
	{
		cout << endl;
		func->Print(depth + 1);
		func = func->next;
	}
	cout << "Main body:" << endl << "{" << endl;
	main->Print(depth + 1);
	cout << "}" << endl;
}

TreeNode* TreeProg::Optimize(){ return NULL; }

void TreeProg::Compile()
{
	SymbolMember* mem = NULL;
	globals->CompileGlobals(context);
	
	TreeFuncList* func = funcs;
	while (func != NULL)
	{

		parser->curFunc = func;
		mem = globals->SearchId(func->id);

		tree param_decl = NULL_TREE;
		tree params = NULL_TREE;
		
		SymbolMember* param = func->locals->head;
		while (param != NULL)
		{
			if(param->type == SM_PARAM)
			{
				tree number = build_decl(UNKNOWN_LOCATION, PARM_DECL, get_identifier(param->id.c_str()), integer_type_node);
				DECL_ARG_TYPE(number) = integer_type_node;
				param->ptr = number;
				param_decl = chainon(param_decl, number);

				params = chainon(params, tree_cons(NULL_TREE, TREE_TYPE(number), NULL_TREE));
			}
			param = param->next;
		}

		tree resdecl = NULL_TREE;
		if(func->isfunc)
			resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
		else
			resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, void_type_node);

		tree fntype = build_function_type(TREE_TYPE(resdecl), params);
		tree fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier(func->id.c_str()), fntype);
		DECL_ARGUMENTS(fndecl) = param_decl;

		DECL_RESULT(fndecl) = resdecl;

		TREE_STATIC(fndecl) = true;
		TREE_PUBLIC(fndecl) = true;
		mem->ptr = fndecl;

		func = func->next;

	}

	func = funcs;
	while (func != NULL)
	{
		parser->curFunc = func;
		tree vars = NULL_TREE;
		mem = globals->SearchId(func->id);
		tree ret = NULL_TREE;

		SymbolMember* param = func->locals->head;
		while (param != NULL)
		{
			if(param->type == SM_VAR)
			{
				tree var = build_decl (UNKNOWN_LOCATION, VAR_DECL, get_identifier(param->id.c_str()), integer_type_node);
				TREE_ADDRESSABLE(var) = true;
				TREE_USED(var) = true;
				func->locals->SearchId(param->id)->ptr = var;
				TREE_CHAIN(var) = vars;
				vars = var;
			}
			else if(param->type == SM_CONST)
			{
				tree con = build_decl (UNKNOWN_LOCATION, CONST_DECL, get_identifier(param->id.c_str()), integer_type_node);
				DECL_INITIAL(con) = build_int_cst(integer_type_node, param->data.val);
				TREE_ADDRESSABLE(con) = true;
				TREE_USED(con) = true;
				func->locals->SearchId(param->id)->ptr = con;
				TREE_CHAIN(con) = vars;
				vars = con;
			}
			else if(param->type == SM_ARRAY)
			{
				tree array = build_array_type(integer_type_node, build_index_type(size_int(param->end - param->start)));
				rest_of_type_compilation(array, 1);
				tree arr = build_decl(UNKNOWN_LOCATION, VAR_DECL, get_identifier(param->id.c_str()), array);

				TREE_PUBLIC(arr) = true;
				TREE_ADDRESSABLE(arr) = true;
				TREE_USED(arr) = true;
				rest_of_decl_compilation(arr, 1, 0);
				func->locals->SearchId(param->id)->ptr = arr;
				TREE_CHAIN(arr) = vars;
				vars = arr;
			}
			param = param->next;
		}

		tree body = alloc_stmt_list ();
		append_to_statement_list(func->body->Compile(), &body);

		if(func->isfunc)
		{
			ret = func->locals->SearchId("_ret")->ptr;
 			tree resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
 			tree return_stmt = build1(RETURN_EXPR, void_type_node, build2(MODIFY_EXPR, TREE_TYPE(integer_type_node), resdecl, ret));
 			append_to_statement_list(return_stmt, &body);
 		}

		tree block = build_block(vars, NULL_TREE, NULL_TREE, NULL_TREE);
		TREE_USED(block) = true;
		tree bind = build3(BIND_EXPR, void_type_node, BLOCK_VARS(block), NULL_TREE, block);

		BIND_EXPR_BODY(bind) = body;
		TREE_SIDE_EFFECTS(bind) = true;

		BLOCK_SUPERCONTEXT(block) = mem->ptr;
		DECL_INITIAL(mem->ptr) = block;
		DECL_SAVED_TREE(mem->ptr) = bind;

		registerGlobalFunction(context, mem->ptr);

		func = func->next;
	}

	parser->curFunc = NULL;

	tree body = main->Compile();
	mem = globals->SearchId("main");
	mem->ptr = buildMain(body);
	registerGlobalFunction(context, mem->ptr);
}

TreeFunc::TreeFunc(const string& fid, TreeStatmList* tbody, SymbolTable* tglobals, SymbolTable* tlocals, bool fisfunc) : id(fid), body(tbody), globals(tglobals), locals(tlocals), isfunc(fisfunc)
{

}
TreeFunc::~TreeFunc(){}
void TreeFunc::Print(int depth)
{
	cout << "\t" << id << endl;
	cout << "locals: " << endl;
	locals->Print(depth + 1);
	cout << "body: " << endl << "{" << endl;
	body->Print(depth + 1);
	cout << "}" << endl;
}

TreeNode* TreeFunc::Optimize(){ return NULL; }
tree TreeFunc::Compile()
{
	return NULL_TREE;
}



TreeFuncList::TreeFuncList(TreeFunc* tcur, TreeFuncList* tnext) : TreeFunc(*tcur), next(tnext)
{

}
TreeFuncList::~TreeFuncList(){}
void TreeFuncList::Print(int depth){ TreeFunc::Print(depth);  }
TreeNode* TreeFuncList::Optimize(){ return NULL; }





TreeArray::TreeArray(const string& aid, TreeExpr* aindex) : id(aid), index(aindex)
{

}
TreeArray::~TreeArray(){}
void TreeArray::Print(int depth)
{
	cout << id << "[";
	index->Print(depth + 1);
	cout << "]";
}
TreeNode* TreeArray::Optimize(){ return NULL; }
tree TreeArray::Compile()
{
	SymbolMember* mem = parser->FindSymbol(id);
	if(mem)
	{
		tree ind = build2(MINUS_EXPR, integer_type_node, index->Compile(), buildConst(mem->start));
		return build4(ARRAY_REF, TREE_TYPE(TREE_TYPE(mem->ptr)), mem->ptr, ind, NULL_TREE, NULL_TREE);
		// return build4(ARRAY_REF, TREE_TYPE(TREE_TYPE(mem->ptr)), mem->ptr, ind, NULL_TREE, NULL_TREE);
	}

	cerr << "Undefined array variable" << endl;
	exit(5);
	return NULL_TREE; 
}


TreeVar::TreeVar(const string& aid) : id(aid)
{

}
TreeVar::~TreeVar(){}
void TreeVar::Print(int)
{
	cout << id;
}
TreeNode* TreeVar::Optimize(){ return NULL; }
tree TreeVar::Compile()
{
	SymbolMember* mem = parser->FindSymbol(id);
	if(mem)
		return mem->ptr;

	cerr << "Undefined variable" << endl;
	exit(5);
	return NULL_TREE; 
}


TreeAssign::TreeAssign(TreeMutable* tdest, TreeExpr* tsource) : dest(tdest), source(tsource)
{

}
TreeAssign::~TreeAssign(){}
void TreeAssign::Print(int depth)
{
	Spaces(depth);
	dest->Print(depth + 1);
	cout << " := ";
	source->Print(depth + 1);
	cout << ";";
}
TreeNode* TreeAssign::Optimize(){ return NULL; }
tree TreeAssign::Compile()
{
	tree des = dest->Compile();
	tree src = source->Compile();
	return build2(MODIFY_EXPR, TREE_TYPE(des), des, src);
}

TreeReturn::TreeReturn()
{

}
TreeReturn::~TreeReturn(){}
void TreeReturn::Print(int depth)
{
	Spaces(depth);
	cout << "return";
}
TreeNode* TreeReturn::Optimize(){ return NULL; }
tree TreeReturn::Compile()
{
	if(parser->curFunc->isfunc)
	{
		SymbolMember* mem = parser->FindSymbol("_ret");
		if(mem)
		{
			tree resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
 			return build1(RETURN_EXPR, void_type_node, build2(MODIFY_EXPR, TREE_TYPE(integer_type_node), resdecl, mem->ptr));
		}
		cerr << "Undefined variable" << endl;
		exit(5);
	}
	else
	{
		return build1(RETURN_EXPR, void_type_node, NULL_TREE);
	}
	
	return NULL_TREE; 

}



TreeIf::TreeIf(TreeExpr* texpr, TreeStatm* tpos, TreeStatm* tneg) : expr(texpr), posBranch(tpos), negBranch(tneg)
{

}
TreeIf::~TreeIf(){}
void TreeIf::Print(int depth)
{
	Spaces(depth);
	cout << "if ";
	expr->Print(depth + 1);
	cout << endl;
	Spaces(depth); 
	cout <<"{" << endl;
	posBranch->Print(depth + 1);
	Spaces(depth);
	cout << endl;
	Spaces(depth);
	cout << "}" << endl;

	if (negBranch)
	{
		Spaces(depth);
		cout << "else " << endl;
		Spaces(depth);
		cout << "{" << endl;
		negBranch->Print(depth + 1);
		cout << endl;
		Spaces(depth);
		cout << "}" << endl;
	}
	
}
TreeNode* TreeIf::Optimize(){ return NULL; }
tree TreeIf::Compile()
{
	tree pos = NULL_TREE;
	if(posBranch)
		pos = posBranch->Compile();
	tree neg = NULL_TREE;
	if(negBranch)
		neg = negBranch->Compile();

	return build3(COND_EXPR, void_type_node, expr->Compile(), pos, neg);
}



TreeWhile::TreeWhile(TreeExpr* texpr, TreeStatm* tbody) : expr(texpr), body(tbody)
{

}
TreeWhile::~TreeWhile(){}
void TreeWhile::Print(int depth)
{
	Spaces(depth);
	cout << "while ";
	expr->Print(depth + 1);
	cout << endl;
	Spaces(depth); 
	cout << "{" << endl;
	body->Print(depth + 1);
	Spaces(depth);
	cout << "}" << endl;
}

TreeNode* TreeWhile::Optimize(){ return NULL; }
tree TreeWhile::Compile()
{
	tree block = alloc_stmt_list();
	tree condition = expr->Compile();
	tree exit_stmt = build1(EXIT_EXPR, void_type_node, build2(EQ_EXPR, integer_type_node, buildConst(0), condition));
	append_to_statement_list(exit_stmt, &block);
	append_to_statement_list(body->Compile(), &block);
/*s
	tree loop = build3(BIND_EXPR, void_type_node, NULL_TREE, NULL_TREE, NULL_TREE);
	BIND_EXPR_BODY(loop) = block;*/
	TREE_SIDE_EFFECTS(block) = true;
	return build1(LOOP_EXPR, void_type_node, block);
}


TreeFor::TreeFor(TreeExpr* tvar, TreeExpr* tfrom, TreeExpr* tto, TreeStatm* tbody, bool tupto) : var(tvar), from(tfrom), to(tto), body(tbody), upto(tupto)
{

}
TreeFor::~TreeFor(){}
void TreeFor::Print(int depth)
{
	Spaces(depth);
	cout << "for ";
	var->Print(depth + 1);
	cout << " from ";
	from->Print(depth + 1);

	if(upto)
		cout << " to ";
	else
		cout << " downto";
	to->Print(depth + 1);
	cout << endl;
	Spaces(depth - 1); 
	cout << "{" << endl;
	body->Print(depth + 1);
	Spaces(depth - 1);
	cout << "}" << endl;
}
TreeNode* TreeFor::Optimize(){ return NULL; }
tree TreeFor::Compile()
{
	tree control = var->Compile();
	tree start = from->Compile();
	tree end = to->Compile();

	tree outer = alloc_stmt_list();
	append_to_statement_list(build2(MODIFY_EXPR, TREE_TYPE(control), control, start), &outer);

	tree block = alloc_stmt_list();
	tree exit_stmt = NULL_TREE;
	if(upto)
		exit_stmt = build1(EXIT_EXPR, void_type_node, build2(GT_EXPR, integer_type_node, control, end));
	else
		exit_stmt = build1(EXIT_EXPR, void_type_node, build2(LT_EXPR, integer_type_node, control, end));
	
	append_to_statement_list(exit_stmt, &block);
	append_to_statement_list(body->Compile(), &block);

	if(!upto)
	{
		cout << "Down" << endl;
		tree stm = build2(PREDECREMENT_EXPR, TREE_TYPE(control), control, build_int_cst(integer_type_node, 1));
		append_to_statement_list(stm, &block);
	}
	else
	{
		cout << "Not Down" << endl;
		tree stm = build2(PREINCREMENT_EXPR, TREE_TYPE(control), control, build_int_cst(integer_type_node, 1));
		append_to_statement_list(stm, &block);
	}
/*
	tree loop = build3(BIND_EXPR, void_type_node, NULL_TREE, NULL_TREE, NULL_TREE);
	BIND_EXPR_BODY(loop) = block;*/
	TREE_SIDE_EFFECTS(block) = true;
	append_to_statement_list(build1(LOOP_EXPR, void_type_node, block), &outer);

	return outer;
}



TreeStatm::TreeStatm(){}
TreeStatm::~TreeStatm(){}
void TreeStatm::Print(int) 
{
}


TreeStatmList::TreeStatmList(TreeStatm* thead, TreeStatmList* tnext) : head(thead), next(tnext)
{

}


TreeStatmList::~TreeStatmList(){}
void TreeStatmList::Print(int depth)
{
	TreeStatmList* cur = this;
	while (cur != NULL)
	{
		if (cur->head)
		{
			cur->head->Print(depth + 1);
			cout << endl;
		}
		cur = cur->next;
	}
}
TreeNode* TreeStatmList::Optimize(){ return NULL; }
tree TreeStatmList::Compile()
{
	tree stmts = alloc_stmt_list();
	TreeStatmList* cur = this;
	while (cur != NULL)
	{
		if (cur->head)
			append_to_statement_list(cur->head->Compile(), &stmts);
		cur = cur->next;
	}
	return stmts;
}



TreeConst::TreeConst(int tval) : val(tval)
{

}
TreeConst::~TreeConst(){}
void TreeConst::Print(int)
{
	cout << val;
}
tree TreeConst::Compile()
{
	return buildConst(val);
}


TreeString::TreeString(const string& sstr) : str(sstr)
{

}
TreeString::~TreeString(){}
void TreeString::Print(int)
{
	cout << str;
}
tree TreeString::Compile()
{
	return buildConstString(str, true);
}



TreeCall::TreeCall(const string& fid, TreeExprList* fparams) : id(fid), params(fparams)
{

}
TreeCall::~TreeCall(){}
void TreeCall::Print(int depth)
{
	//Spaces(depth);
	cout << id << "(";
	params->Print(depth + 1);
	cout << ")";
}
TreeNode* TreeCall::Optimize(){ return NULL; }
tree TreeCall::Compile()
{
	SymbolMember* func = parser->prog->globals->SearchId(id);
	int cnt = params->Length();
	if(!params->head)
		cnt = 0;

	tree * args = XNEWVEC(tree, cnt);
	TreeExprList* cur = params;
	for(int x = 0; x < cnt; x++)
	{
		args[x] = cur->head->Compile();
		cur = cur->next;
	}
	
	cout << "Calling" << endl;
	func->Print(0); cout << endl;
	cout << endl;
	cout << "After call" << endl;

	if(id == "writeln" || id == "write")
	{
		if(cnt != 1)
		{
			cerr << "Writeln wrong parameter count." << endl;
			exit(6);
		}

		if(params && params->head && dynamic_cast<TreeString*>(params->head) != NULL)
		{
			tree strptr = build1(ADDR_EXPR, build_pointer_type(TREE_TYPE(args[0])), args[0]);
			return buildPrintf(UNKNOWN_LOCATION, id == "writeln" ? "%s\n" : "%s", strptr); 
		}
		else
		{
			return buildPrintf(UNKNOWN_LOCATION, id == "writeln" ? "%d\n" : "%d", args[0]); 
		}
	}
	else if(id == "read" || id == "readln")
	{
		if(cnt != 1)
		{
			cerr << "Readln wrong parameter count." << endl;
			exit(6);
		}

		if(!dynamic_cast<TreeMutable*>(params->head))
		{
			cerr << "Readln can't load value to static object." << endl;
			exit(6);
		}
		return buildScanf(UNKNOWN_LOCATION, args[0]);
	}
	else if(id == "dec")
	{
		if(cnt != 1)
		{
			cerr << "Dec wrong parameter count." << endl;
			exit(6);
		}

		if(!dynamic_cast<TreeMutable*>(params->head))
		{
			cerr << "Can't decrease static object." << endl;
			exit(6);
		}
		return buildDec(UNKNOWN_LOCATION, args[0]);
	}
	else if(id == "inc")
	{
		if(cnt != 1)
		{
			cerr << "Inc wrong parameter count." << endl;
			exit(6);
		}

		if(!dynamic_cast<TreeMutable*>(params->head))
		{
			cerr << "Can't increase static object." << endl;
			exit(6);
		}
		return buildInc(UNKNOWN_LOCATION, args[0]);
	}

	
	if(func->type != SM_FUNC)
	{
		cerr << "You can't call static object." << endl;
		exit(6);
	}

	location_t loc = UNKNOWN_LOCATION;
	cout << endl << "Really called" << endl;
	tree call = build_call_expr_loc_array(loc, func->ptr, cnt, args);
	SET_EXPR_LOCATION(call, loc);
	TREE_USED(call) = true;
	return call;
}


TreeExprList::TreeExprList(TreeExpr* thead, TreeExprList* tnext) : head(thead), next(tnext)
{

}
TreeExprList::~TreeExprList(){}
void TreeExprList::Print(int depth)
{
	TreeExprList* cur = this;
	while (cur != NULL)
	{
		if (cur->head)
		{
			cur->head->Print(depth + 1);
			if (cur->next)
				cout << ", ";
		}
		cur = cur->next;
	}
}
TreeNode* TreeExprList::Optimize(){ return NULL; }
tree TreeExprList::Compile()
{
	return NULL_TREE;
}

TreeBinOp::TreeBinOp(TreeExpr* tlexpr, BinaryOp top, TreeExpr* trexpr) : lexpr(tlexpr), op(top), rexpr(trexpr)
{

}
TreeBinOp::~TreeBinOp(){}
void TreeBinOp::Print(int depth)
{
	cout << "(";
	lexpr->Print(depth + 1);
	cout << ") ";
	switch (op)
	{
	case BO_ADD:
		cout << "+";
		break;
	case BO_MIN:
		cout << "-";
		break;
	case BO_MUL:
		cout << "*";
		break;
	case BO_DIV:
		cout << "/";
		break;
	case BO_OR:
		cout << "or";
		break;
	case BO_AND:
		cout << "and";
		break;
	case BO_MOD:
		cout << "%";
		break;
	case BO_EQ:
		cout << "==";
		break; 
	case BO_NEQ:
		cout << "<>";
		break; 
	case BO_LT:
		cout << "<";
		break; 
	case BO_LE:
		cout << "<=";
		break; 
	case BO_GT:
		cout << ">";
		break; 
	case BO_GE:
		cout << ">=";
		break;
	}

	cout << " (";
	rexpr->Print(depth + 1);
	cout << ")";
}
TreeNode* TreeBinOp::Optimize(){ return NULL; }
tree TreeBinOp::Compile()
{
	tree lex = lexpr->Compile();
	tree rex = rexpr->Compile();
	switch (op)
	{
		case BO_ADD:
			return build2(PLUS_EXPR, integer_type_node, lex, rex);
		case BO_MIN:
			return build2(MINUS_EXPR, integer_type_node, lex, rex);
		case BO_MUL:
			return build2(MULT_EXPR, integer_type_node, lex, rex);
		case BO_DIV:
			return build2(TRUNC_DIV_EXPR, integer_type_node, lex, rex);
		case BO_MOD:
			return build2(TRUNC_MOD_EXPR, integer_type_node, lex, rex);
		case BO_OR:
			return build2(TRUTH_ORIF_EXPR, integer_type_node, lex, rex);
		case BO_AND:
			return build2(TRUTH_ANDIF_EXPR, integer_type_node, lex, rex);
		case BO_EQ:
			return build2(EQ_EXPR, integer_type_node, lex, rex);
		case BO_NEQ:
			return build2(NE_EXPR, integer_type_node, lex, rex);
		case BO_LT:
			return build2(LT_EXPR, integer_type_node, lex, rex);
		case BO_LE:
			return build2(LE_EXPR, integer_type_node, lex, rex);
		case BO_GT:
			return build2(GT_EXPR, integer_type_node, lex, rex);
		case BO_GE:
			return build2(GE_EXPR, integer_type_node, lex, rex);
		default:
		;
	}
	return NULL_TREE;
}


TreeUnaryMin::TreeUnaryMin(TreeExpr* texpr) : expr(texpr)
{

}
TreeUnaryMin::~TreeUnaryMin(){}
void TreeUnaryMin::Print(int depth)
{
	cout << "-";
	expr->Print(depth + 1);
}
TreeNode* TreeUnaryMin::Optimize(){ return NULL; }
tree TreeUnaryMin::Compile()
{
	return build1(NEGATE_EXPR, integer_type_node, expr->Compile());
}
