
#include "builder.h"


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

using namespace std;


tree buildGlobalVariable(const string& id, bool var, int value)
{
	tree variable = build_decl(UNKNOWN_LOCATION, var ? VAR_DECL : CONST_DECL, get_identifier(id.c_str()), integer_type_node);
	TREE_ADDRESSABLE(variable) = true;
	TREE_USED(variable) = true;
	TREE_STATIC(variable) = true;
	TREE_PUBLIC(variable) = true;
	DECL_INITIAL(variable) = build_int_cst(integer_type_node, value);
	return variable;
}

tree buildGlobalArray(const string& id, int size)
{
	tree array = build_array_type(integer_type_node, build_index_type(size_int(size)));
	rest_of_type_compilation(array, 1);
	tree arr_decl = build_decl(UNKNOWN_LOCATION, VAR_DECL, get_identifier(id.c_str()), array);

	TREE_STATIC(arr_decl) = true;
	TREE_PUBLIC(arr_decl) = true;
	TREE_ADDRESSABLE(arr_decl) = true;
	TREE_USED(arr_decl) = true;
	rest_of_decl_compilation(arr_decl, 1, 0);
	return arr_decl;
}

tree buildLocalVariable(const string& id, bool var, int value, tree context)
{
	tree variable = build_decl(UNKNOWN_LOCATION, var ? VAR_DECL : CONST_DECL, get_identifier(id.c_str()), integer_type_node);
	TREE_ADDRESSABLE(variable) = true;
	TREE_USED(variable) = true;
	TREE_STATIC(variable) = true;
	TREE_PUBLIC(variable) = true;
	DECL_INITIAL(variable) = build_int_cst(integer_type_node, value);
	DECL_CONTEXT(variable) = context;

	return variable;
}

tree buildLocalArray(const string& id, int size, tree context)
{
	tree array = build_array_type(integer_type_node, build_index_type(size_int(size)));
	rest_of_type_compilation(array, 1);
	tree arr_decl = build_decl(UNKNOWN_LOCATION, VAR_DECL, get_identifier(id.c_str()), array);

	TREE_STATIC(arr_decl) = true;
	TREE_PUBLIC(arr_decl) = true;
	TREE_ADDRESSABLE(arr_decl) = true;
	TREE_USED(arr_decl) = true;
	rest_of_decl_compilation(arr_decl, 1, 0);
	DECL_CONTEXT(arr_decl) = context;
	return arr_decl;
}

tree buildConst(int val)
{
  return build_int_cst(integer_type_node, val);
}

tree buildConstString(const string& str, bool isConst = true)
{
  int length = str.length();
  tree index_type = build_index_type(size_int(length));
  tree const_char_type = isConst ? build_qualified_type(unsigned_char_type_node, TYPE_QUAL_CONST) : unsigned_char_type_node;
  tree string_type = build_array_type(const_char_type, index_type);
  TYPE_STRING_FLAG(string_type) = 1;
  tree res = build_string(length + 1, str.c_str());
  TREE_TYPE(res) = string_type;

  return res;
}

tree buildPrintf(location_t loc, const string& msg, tree int_expr) 
{
	tree str = buildConstString(msg.c_str(), true);

	tree* args_vec = XNEWVEC(tree, 2);
	args_vec[0] = build1(ADDR_EXPR, build_pointer_type(TREE_TYPE(str)), str);
	args_vec[1] = int_expr;

	tree params = NULL_TREE;
	chainon(params, tree_cons(NULL_TREE, TREE_TYPE(args_vec[0]), NULL_TREE));
	chainon(params, tree_cons(NULL_TREE, TREE_TYPE(args_vec[1]), NULL_TREE));

	tree param_decl = NULL_TREE;

	tree resdecl = build_decl(BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;

	tree fntype = build_function_type(TREE_TYPE(resdecl), params);
	tree fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier("printf"), fntype);
	DECL_ARGUMENTS(fndecl) = param_decl;

	DECL_RESULT(fndecl) = resdecl;

	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;
	DECL_EXTERNAL(fndecl) = true;

	tree call = build_call_expr_loc_array(loc, fndecl, 2, args_vec);
	SET_EXPR_LOCATION(call, loc);
	TREE_USED(call) = true;

	return call;
}


tree buildScanf(location_t loc, tree var_expr) 
{
	tree str = buildConstString("%d", true);

	tree * args_vec = XNEWVEC(tree, 2);
	args_vec[0] = build1(ADDR_EXPR, build_pointer_type(TREE_TYPE(str)), str);
	args_vec[1] = build1(ADDR_EXPR, build_pointer_type(TREE_TYPE(var_expr)), var_expr);

	tree params = NULL_TREE;
	chainon(params, tree_cons(NULL_TREE, TREE_TYPE(args_vec[0]), NULL_TREE));
	chainon(params, tree_cons(NULL_TREE, TREE_TYPE(args_vec[1]), NULL_TREE));

	// function parameters
	tree param_decl = NULL_TREE;

	tree resdecl = build_decl(BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;

	tree fntype = build_function_type(TREE_TYPE(resdecl), params);
	tree fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier("scanf"), fntype);
	DECL_ARGUMENTS(fndecl) = param_decl;

	DECL_RESULT(fndecl) = resdecl;

	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;
	DECL_EXTERNAL(fndecl) = true;

	tree call = build_call_expr_loc_array(loc, fndecl, 2, args_vec);
	SET_EXPR_LOCATION(call, loc);
	TREE_USED(call) = true;

	return call;
}

tree buildDec(location_t, tree var_expr) 
{
	tree expr = build2(MINUS_EXPR, TREE_TYPE(var_expr), var_expr, buildConst(1));
	return build2(MODIFY_EXPR, TREE_TYPE(var_expr), var_expr, expr);
}

tree buildInc(location_t, tree var_expr) 
{
	tree expr = build2(PLUS_EXPR, TREE_TYPE(var_expr), var_expr, buildConst(1));
	return build2(MODIFY_EXPR, TREE_TYPE(var_expr), var_expr, expr);
}

tree buildMain(tree body)
{ 
	tree resdecl = build_decl(BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);

	tree fntype = build_function_type(TREE_TYPE(resdecl), NULL_TREE);
	tree fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier("main"), fntype);
	DECL_ARGUMENTS(fndecl) = NULL_TREE;
	DECL_RESULT(fndecl) = resdecl;
	TREE_STATIC(fndecl) = true;
	TREE_PUBLIC(fndecl) = true;

	tree block = build_block(NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE);
	TREE_USED(block) = true;

	tree return_stmt = build1(RETURN_EXPR, void_type_node, build2(MODIFY_EXPR, TREE_TYPE(integer_type_node), resdecl, buildConst(0)));
	append_to_statement_list(return_stmt, &body);

	tree bind = build3(BIND_EXPR, void_type_node, BLOCK_VARS(block), NULL_TREE, block);
	BIND_EXPR_BODY(bind) = body;
	TREE_SIDE_EFFECTS(bind) = true;

	BLOCK_SUPERCONTEXT(block) = fndecl;
	DECL_INITIAL(fndecl) = block;
	DECL_SAVED_TREE(fndecl) = bind;

	return fndecl;
}

tree buildFunction(tree fndecl, tree body)
{
	// expand_return(build(MODIFY_EXPR, integer_type_node, DECL_RESULT(fndecl), integer_zero_node));
	// expand_end_bindings(NULL_TREE, 1, 0);
	// poplevel(1, 0, 1);
	// expand_function_end(input_file_name, 1, 0);
	// rest_of_compilation(fndecl);
	// current_function_decl = 0;
	// permanent_allocation(1);
	return fndecl;
}

tree buildFunctionDeclaration(const string& id, tree params_decl, tree params, tree vars, tree body, tree ret, bool is_func)
{
	/*
  tree resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);

  // then the type of the function and its declaration -- external - the code for it is somewhere else and 
  tree fntype = build_function_type( TREE_TYPE(resdecl), params );
  tree fndecl = build_decl( UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier(id.c_str()), fntype );
  DECL_ARGUMENTS(fndecl) = params_decl;
  
  DECL_RESULT( fndecl ) = resdecl;
  
  TREE_STATIC(fndecl) = true;
  TREE_PUBLIC( fndecl ) = true;
  
  tree block = build_block(vars, NULL_TREE, NULL_TREE, NULL_TREE);
  TREE_USED(block) = true;
*/







  // allocate the statement list
  //tree stmts = alloc_stmt_list ();
  /*
    tree then_decls = NULL_TREE;
    
    tree then_stmts = alloc_stmt_list ();
    
    tree then_return = build1(RETURN_EXPR, void_type_node, build2(MODIFY_EXPR, TREE_TYPE(integer_type_node), resdecl, build_int_cst(integer_type_node, 1)));
    append_to_statement_list(then_return, &then_stmts);
    
    // bind main block with statements
    tree then_bind = build3( BIND_EXPR, void_type_node, then_decls, NULL_TREE, NULL_TREE );
    BIND_EXPR_BODY(then_bind) = then_stmts;
    TREE_SIDE_EFFECTS(then_bind) = true;
    
  tree cond = build2(LE_EXPR, integer_type_node, number, build_int_cst(integer_type_node, 1));
  
  tree if_stmt = build3(COND_EXPR, void_type_node, cond, then_bind, NULL_TREE);
  append_to_statement_list(if_stmt, &stmts);
  
  tree * args_vec = XNEWVEC( tree, 1 );
  args_vec[0] = build2(MINUS_EXPR, integer_type_node, number, build_int_cst(integer_type_node, 1));

  tree call = build_call_expr_loc_array( UNKNOWN_LOCATION, fndecl, 1, args_vec );
  SET_EXPR_LOCATION(call, UNKNOWN_LOCATION);
  TREE_USED(call) = true;
  
  tree main_return = build1(RETURN_EXPR, void_type_node, build2(MODIFY_EXPR, TREE_TYPE(integer_type_node), resdecl, build2(MULT_EXPR, integer_type_node, number, call)));
  append_to_statement_list(main_return, &stmts);
  */
  // bind main block with statements

  // tree return_stmt = build1(RETURN_EXPR, void_type_node, build2(MODIFY_EXPR, TREE_TYPE(integer_type_node), resdecl, ret));
  // append_to_statement_list(return_stmt, &body);






  // tree bind = build3( BIND_EXPR, void_type_node, BLOCK_VARS(block), NULL_TREE, block );
  // BIND_EXPR_BODY(bind) = body;
  // TREE_SIDE_EFFECTS(bind) = true;

  // BLOCK_SUPERCONTEXT(block) = fndecl;
  // DECL_INITIAL(fndecl) = block;
  // DECL_SAVED_TREE(fndecl) = bind;
  
  // return fndecl;


  
  tree resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);

  // then the type of the function and its declaration -- external - the code for it is somewhere else and 
  tree fntype = build_function_type(TREE_TYPE(resdecl), params);
  tree fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier(id.c_str()), fntype);
  DECL_ARGUMENTS(fndecl) = params_decl;
  
  DECL_RESULT(fndecl) = resdecl;
  
  TREE_STATIC(fndecl) = true;
  TREE_PUBLIC(fndecl) = true;
  
  tree block = build_block(vars, NULL_TREE, NULL_TREE, NULL_TREE);
  TREE_USED(block) = true;


  tree bind = build3(BIND_EXPR, void_type_node, BLOCK_VARS(block), NULL_TREE, block);

  BIND_EXPR_BODY(bind) = body;
  TREE_SIDE_EFFECTS(bind) = true;

  BLOCK_SUPERCONTEXT(block) = fndecl;
  DECL_INITIAL(fndecl) = block;
  DECL_SAVED_TREE(fndecl) = bind;
  
  return fndecl;
}


tree buildCall(location_t loc, tree fndecl, tree* params, unsigned cnt)
{
	tree call = build_call_expr_loc_array(loc, fndecl, cnt, params);
	SET_EXPR_LOCATION(call, loc);
	TREE_USED(call) = true;
	return call;
}



void registerGlobalFunction(vec<tree, va_gc>** context, tree function) 
{
	vec_safe_push(*context, function);
	treeDump(function);
	gimplify_function_tree(function);
	cgraph_node::finalize_function(function, false);
}

void registerGlobalVariable(vec<tree, va_gc>** context, tree variable) 
{
	vec_safe_push(*context, variable);
}

void treeDump(tree fndecl) 
{
	FILE *dump_orig;
	int local_dump_flags;
	struct cgraph_node *cgn;
	
	// dump_orig = dump_begin(TDI_original, &local_dump_flags);
	dump_orig = stdout;
	if(dump_orig) 
	{
		fprintf(dump_orig, "\n;; Function %s", lang_hooks.decl_printable_name(fndecl, 2));
		fprintf(dump_orig, "(%s)\n",(!DECL_ASSEMBLER_NAME_SET_P(fndecl) ? "null" : IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(fndecl))));
		fprintf(dump_orig, ";; enabled by -%s\n", dump_flag_name(TDI_original));
		fprintf(dump_orig, "\n");
		
		if(local_dump_flags & TDF_RAW) 
			dump_node(DECL_SAVED_TREE(fndecl), TDF_SLIM | local_dump_flags, dump_orig);
		else 
		{
			struct function fn;
			fn.decl = fndecl;
			fn.curr_properties = 0;
			fn.cfg = NULL;
			DECL_STRUCT_FUNCTION(fndecl) = &fn;
			dump_function_to_file(fndecl, dump_orig, 0);
			DECL_STRUCT_FUNCTION(fndecl) = NULL;
		}
		fprintf(dump_orig, "\n");		
		dump_end(TDI_original, dump_orig);
	}
	cgn = cgraph_node::get_create(fndecl);
	for(cgn = cgn->nested; cgn ; cgn = cgn->next_nested) 
		treeDump(cgn->decl);
}
