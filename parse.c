#include "c38.h"

LVar *locals;

char *strndup(const char *s, size_t n)
{
	char *p = memchr(s, '\0', n);
	if (p != NULL)
		n = p - s;
	p = malloc(n + 1);
	if (p != NULL)
	{
		memcpy(p, s, n);
		p[n] = '\0';
	}
	return p;
}

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

// program = stmt*
Function *program()
{
	locals = NULL;

	Node head = {};
	Node *cur = &head;

	while (!at_eof())
	{
		cur->next = stmt();
		cur = cur->next;
	}

	Function *prog = calloc(1, sizeof(Function));
	prog->node = head.next;
	prog->locals = locals;
	return prog;
}

// stmt = expr ";" |
//        "return" expr ";" |
//        "if" "(" expr ")" stmt ("else" stmt)? |
//        "while" "(" expr ")" stmt |
//        "for" "(" expr? ";" expr? ";" expr? ")" stmt |
//        "{" stmt* "}"
Node *stmt()
{
	Node *node;
	if (consume("return"))
	{
		node = calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	}
	else if (consume("{"))
	{
		node = calloc(1, sizeof(Node));
		node->kind = ND_BLOCK;

		Node head = {};
		Node *block = &head;
		while (!consume("}"))
		{
			Node *n = stmt();
			block->next = n;
			block = n;
		}

		node->body = head.next;
		return node;
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
	else if (consume("while"))
	{
		node = calloc(1, sizeof(Node));
		node->kind = ND_WHILE;
		expect("(");
		node->cond = expr();
		expect(")");
		node->then = stmt();
		return node;
	}
	else if (consume("for"))
	{
		node = calloc(1, sizeof(Node));
		node->kind = ND_FOR;
		expect("(");
		if (!consume(";"))
		{
			node->init = expr();
			expect(";");
		}
		if (!consume(";"))
		{
			node->cond = expr();
			expect(";");
		}
		if (!consume(")"))
		{
			node->inc = expr();
			expect(")");
		}
		node->then = stmt();
		return node;
	}
	else
	{
		node = expr();
	}

	expect(";");
	return node;
}


// expr = assign
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

// primary = "(" expr ")" | num | ident | ident ( "(" ")" )?
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
		// func call
		if (consume("("))
		{
			expect(")");
			Node *node = calloc(1, sizeof(Node));
			node->kind = ND_CALLFUNC;
			node->funcname = strndup(tok->str, tok->len);
			return node;
		}

		// variable
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(tok);
		if (lvar)
		{
			node->var = lvar;
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
			locals = lvar;
			node->var = lvar;
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

