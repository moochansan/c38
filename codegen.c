#include "c38.h"

static int labelseq = 1;

void gen_lval(Node *node)
{
	if (node->kind != ND_LVAR)
	{
		fprintf(stderr, "lvalue of assignment is not a variable");
		exit(1);
	}

	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", node->var->offset);
	printf("  push rax\n");
}

static char *argregs[] = {"rdi", "rsi", "rdx", "rcx","r8", "r9"}; 

void gen(Node *node)
{
	switch (node->kind)
	{
	case ND_CALLFUNC:
	{
		int numargs = 0;
		for (Node *arg = node->args; arg; arg = arg->next)
		{
			gen(arg);
			numargs++;
		}

		for (int i = numargs - 1; i >= 0; i--)
		{
			printf("  pop %s\n", argregs[i]);
		}

		printf("  call %s\n", node->funcname);
		printf("  push rax\n");
		return;
	}
	case ND_NUM:
		printf("  push %d\n", node->val);
		return;
	case ND_LVAR:
		gen_lval(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	case ND_ASSIGN:
		gen_lval(node->lhs);
		gen(node->rhs);
		printf("  pop rdi\n");
		printf("  pop rax\n");
		printf("  mov [rax], rdi\n");
		return;
	case ND_IF:
	{
		int seq = labelseq++;
		if (node->els)
		{
			gen(node->cond);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je  .L.else.%d\n", seq);
			gen(node->then);
			printf("  jmp .L.end.%d\n", seq);
			printf(".L.else.%d:\n", seq);
			gen(node->els);
			printf(".L.end.%d:\n", seq);
		}
		else
		{
			gen(node->cond);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je  .L.end.%d\n", seq);
			gen(node->then);
			printf(".L.end.%d:\n", seq);
		}

		return;
	}
	case ND_WHILE:
	{
		int seq = labelseq++;
		printf(".L.cond.%d:\n", seq);
		gen(node->cond);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je  .L.end.%d\n", seq);
		gen(node->then);
		printf("  jmp .L.cond.%d\n", seq);
		printf(".L.end.%d:\n", seq);
		return;
	}
	case ND_FOR:
	{
		int seq = labelseq++;
		if (node->init)
			gen(node->init);
		printf(".L.cond.%d:\n", seq);
		if (node->cond)
		{
			gen(node->cond);
			printf("  pop rax\n");
			printf("  cmp rax, 0\n");
			printf("  je  .L.end.%d\n", seq);
		}
		gen(node->then);
		if (node->inc)
			gen(node->inc);
		printf("  jmp .L.cond.%d\n", seq);
		printf(".L.end.%d:\n", seq);
		return;
	}
	case ND_BLOCK:
		if (node->body)
		{
			for (Node *n = node->body; n; n = n->next)
				gen(n);
		}
		return;
	case ND_RETURN:
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  jmp .L.return\n");
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind)
	{
	case ND_ADD:
		printf("  add rax, rdi\n");
		break;
	case ND_SUB:
		printf("  sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("  imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("  cqo\n");
		printf("  idiv rdi\n");
		break;
	case ND_EQ:
		printf("  cmp rax, rdi\n");
		printf("  sete al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_NE:
		printf("  cmp rax, rdi\n");
		printf("  setne al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LT:
		printf("  cmp rax, rdi\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LE:
		printf("  cmp rax, rdi\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
		break;
	}

	printf("  push rax\n");
}

void codegen(Function *prog)
{
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// prologue
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, %d\n", prog->stackSize);

	for (Node *node = prog->node; node; node = node->next)
	{
		gen(node);
	}
	printf("  pop rax\n");

	// epilogue
	printf(".L.return:\n");
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
}

