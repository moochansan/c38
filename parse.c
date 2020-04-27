#include "c38.h"

static VarList *locals;

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

LVar *find_lvar(Token *tok)
{
	for (VarList *v1 = locals; v1; v1 = v1->next)
	{
		LVar *var = v1->var;
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
			return var;
	}
	return NULL;
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

// program = function*
Function *program()
{
	Function head = {};
	Function *cur = &head;

	while (!at_eof())
	{
		cur->next = function();
		cur = cur->next;
	}
	return head.next;
}

// function = ident "(" ")" "{" stmt* "}"
Function *function()
{
	locals = NULL;

	char *name = expect_ident();
	expect("(");
	expect(")");
	expect("{");
	
	Node head = {};
	Node *cur = &head;

	while (!consume("}"))
	{
		cur->next = stmt();
		cur = cur->next;
	}

	Function *func = calloc(1, sizeof(Function));
	func->name = name;
	func->node = head.next;
	func->locals = locals;
	return func;
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

// primary = "(" expr ")" | num | ident func_args?
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
			Node *node = calloc(1, sizeof(Node));
			node->kind = ND_CALLFUNC;
			node->funcname = strndup(tok->str, tok->len);
			node->args = func_args();
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
			lvar->name = tok->str;
			lvar->len = tok->len;
			node->var = lvar;

			VarList *v1 = calloc(1, sizeof(VarList));
			v1->var = lvar;
			v1->next = locals;
			locals = v1;
		}
		return node;
	}
	else
		return new_node_num(expect_number());
}

// func_args = "(" (assign ("," assign)*)? ")"
Node *func_args()
{
	if (consume(")"))
		return NULL;

	Node head = {};
	Node *cur = &head;
	cur->next = assign();

	while (consume(","))
	{
		cur = cur->next;
		cur->next = assign();
	}

	expect(")");
	return head.next;
}

