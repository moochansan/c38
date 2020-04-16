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
	program();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// prologue
	// 1文字変数(a - z)文の領域を確保.
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, 208\n");

	for (int i = 0; code[i]; i++)
	{
		gen(code[i]);

		// 式の評価結果がスタックに残る.
		// 不要、スタックオーバーフローを避けるためにポップ.
		printf("  pop rax\n");
	}

	// epilogue
	printf(".L.return:\n");
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");

	// 最後の式の結果が rax に残っている.
	printf("  ret\n");
	return 0;
}

