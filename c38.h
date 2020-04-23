#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
// tokenize.c
//

typedef enum {
	TK_RESERVED,
	TK_IDENT,
	TK_NUM,
	TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token
{
	TokenKind kind;
	Token *next;
	int val;
	char *str;
	int len;
};

extern Token *token;
extern char *user_input;

void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
int expect_number(void);
bool at_eof(void);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
char *starts_with_reserved(char *p);
int is_alnum(char c);
Token *tokenize(void);


//
// Parse.c
//

typedef struct LVar LVar;

struct LVar
{
	LVar *next;
	char *name;
	int len;
	int offset;
};

LVar *find_lvar(Token *tok);

typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_EQ,     // ==
	ND_NE,     // !=
	ND_LT,     // <
	ND_LE,     // <=
	ND_ASSIGN, // =
	ND_LVAR,   // ローカル変数
	ND_NUM,
	ND_RETURN,
	ND_IF,
	ND_WHILE,
	ND_FOR,
	ND_BLOCK,
} NodeKind;

typedef struct Node Node;

struct Node
{
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
	LVar *var;
	Node *next;

// for "if", "while" or "for"
	Node *cond;
	Node *then;
	Node *els;
	Node *init;
	Node *inc;

// for "{}"
	Node *body;
};

extern Node *code[];

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Node *stmt(void);
Node *expr(void);
Node *assign(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);

typedef struct Function Function;

struct Function
{
	Node *node;
	LVar *locals;
	int stackSize;
};

Function *program(void);

//
// codegen.c 
//
void gen_lval(Node *node);
void gen(Node *node);
void codegen(Function *prog);

