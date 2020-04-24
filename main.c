#include "c38.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
		return 1;
	}
	
	user_input = argv[1];
	tokenize();
	Function *prog = program();

	for (Function *func = prog; func; func = func->next)
	{
		// 変数分の領域を計算.
		int offset = 0;
		for (LVar *var = prog->locals; var; var = var->next)
		{
			offset += 8;
		}
		func->stackSize = offset;
	}
	codegen(prog);

	return 0;
}

