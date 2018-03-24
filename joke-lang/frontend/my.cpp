#include "my.h"

SynParser* parser = NULL;

void parseFile(const char* input, vec<tree, va_gc>** context)
{
	parser = new SynParser(input);
	TreeProg* ex = parser->Parse();
	cout << endl << "Program AST dump" << endl;
	ex->Print();

	cout << "Compiling assembly" << endl;
	ex->context = context;
	ex->Compile();
	cout << "Finished..." << endl;
}
