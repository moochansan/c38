#include "c38.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
		return 1;
	}
	
	user_input = argv[1];
	token = tokenize(argv[1]);
	Node *node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	gen(node);

	// スタックトップをRAXにロード.
	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}

