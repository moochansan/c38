#include "c38.h"

LVar *locals;

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *code[100];

// program = stmt*
Node *program()
{
	int i = 0;
	while (!at_eof())
		code[i++] = stmt();
	code[i] = NULL;
}

// stmt = expr ";" |
//        "return" expr ";" |
//        "if" "(" expr ")" stmt ("else" stmt)?
Node *stmt()
{
	Node *node;
	if (consume("return"))
	{
		node = calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	}
	else if (consume("if"))
	{
		node = calloc(1, sizeof(Node));
		node->kind = ND_IF;
		expect("(");
		node->cond = expr();
		expect(")");
		node->then = stmt();
		if (consume("else"))
			node->els = stmt();
		return node;
	}
	else
	{
		node = expr();
	}

	expect(";");
	return node;
}


// expr = mul ("+" mul | "-" mul)*
Node *expr()
{
	return assign();
}

// assign = equality ("=" assign)?
Node *assign()
{
	Node *node = equality();

	if (consume("="))
		return new_node(ND_ASSIGN, node, assign());
	else
		return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality()
{
	Node *node = relational();

	for (;;)
	{
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else
			return node;
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
	Node *node = add();

	for (;;)
	{
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
	Node *node = mul(); 

	for (;;)
	{
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else 
			return node;
	}
}

// mul = unary ("*" unary | "/" unary)*
Node *mul()
{
	Node *node = unary();

	for (;;)
	{
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

// unary = ("+" | "-")? primary
Node *unary()
{
	if (consume("+"))
		return primary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

// primary = "(" expr ")" | num | ident
Node *primary()
{
	if (consume("("))
	{
		Node *node = expr();
		expect(")");
		return node;
	}
	
	Token *tok = consume_ident();
	if (tok)
	{
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(tok);
		if (lvar)
		{
			node->offset = lvar->offset;
		}
		else
		{
			lvar = calloc(1, sizeof(LVar));
			if (locals)
				lvar->offset = locals->offset + 8;
			else
				lvar->offset = 8;
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			node->offset = lvar->offset;
			locals = lvar;
		}
		return node;
	}
	else
		return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok)
{
	for (LVar *var = locals; var; var = var->next)
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
			return var;
	return NULL;
}

