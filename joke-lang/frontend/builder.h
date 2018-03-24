#ifndef _BUILDER_H_
#define _BUILDER_H_

#include <iostream>
#include <string>

#include "config.h"
#include "system.h"
#include "ansidecl.h"
#include "coretypes.h"

#include "diagnostic-core.h"
#include "input.h"

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


using namespace std;
tree buildLocalVariable(const string&, bool, int, tree);
tree buildLocalArray(const string&, int, tree);
tree buildGlobalVariable(const string&, bool var = true, int value = 0);
tree buildGlobalArray(const string&, int);
tree buildConst(int);
tree buildConstString(const string&, bool);
tree buildPrintf(location_t, const string&, tree);
tree buildScanf(location_t, tree);
tree buildMain(tree);
tree buildFunction(const string&, tree);
tree buildFunctionDeclaration(const string&, tree, tree, tree, tree, tree, bool);
tree buildCall(location_t, tree, tree*, unsigned);

tree buildInc(location_t, tree);
tree buildDec(location_t, tree);

void registerGlobalVariable(vec<tree, va_gc>**, tree);
void registerGlobalFunction(vec<tree, va_gc>**, tree);
void treeDump(tree);

#endif //_BUILDER_H_
