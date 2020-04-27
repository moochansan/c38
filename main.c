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
		for (VarList *v1 = func->locals; v1; v1 = v1->next)
		{
			offset += 8;
			v1->var->offset = offset;
		}
		func->stackSize = offset;
	}
	codegen(prog);

	return 0;
}

