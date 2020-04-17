#include "c38.h"

Token *token;
char *user_input;

void error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号(引数)のときは
// トークンを1つ読み進めて true を返す.
// それ以外の場合 false を返す.
bool consume(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

// 次のトークンが変数であればそのポインタを返す.
// それ以外の場合、NULLを返す.
Token *consume_ident()
{
	if (token->kind != TK_IDENT)
		return NULL;
	Token *tok = token;
	token = token->next;
	return tok;
}

// 次のトークンが期待している記号(引数)のときは
// トークンを1つ読み進める.
// それ以外の場合、エラーを報告する.
void expect(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		error_at(token->str, "expected '%c'", op);
	token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す.
// それ以外の場合にはエラーを報告する.
int expect_number()
{
	if (token->kind != TK_NUM)
		error_at(token->str, "expected a number");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof()
{
	return token->kind == TK_EOF;
}

// 新しいトークンを作成して cur に繋げる. (リンクリストの作成)
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p, char *q)
{
	return memcmp(p, q, strlen(q)) == 0;
}

char *starts_with_reserved(char *p)
{
	static char *keyWord[] = {"return", "if", "else", "while"};

	for (int i = 0; i < sizeof(keyWord) / sizeof(*keyWord); ++i)
	{
		int len = strlen(keyWord[i]);
		if (startswith(p, keyWord[i]) && !is_alnum(len))
			return keyWord[i];
	}

	static char *op[] = {"==", "!=", "<=", ">="};

	for (int i = 0; i < sizeof(op) / sizeof(*op); ++i)
	{
		if (startswith(p, op[i]))
			return op[i];
	}

	return NULL;
}

int is_alnum(char c)
{
	return ('a' <= c && c <= 'z') ||
			('A' <= c && c <= 'Z') ||
			('0' <= c && c <= '9') ||
			(c == '_');
}

// 入力文字列をトークナイズして、先頭のトークンを返す.
Token *tokenize()
{
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p)
	{
		if (isspace(*p))
		{
			p++;
			continue;
		}

		char *rsv = starts_with_reserved(p);
		if (rsv)
		{
			int len = strlen(rsv);
			cur = new_token(TK_RESERVED, cur, p, len);
			p += len;
			continue;
		}

		if (strchr("+-*/()<>=;", *p))
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6]))
		{
			cur = new_token(TK_RESERVED, cur, p, 6);
			p += 6;
			continue;
		}

		if ('a' <= *p && *p <= 'z' ||
			'A' <= *p && *p <= 'Z' ||
			*p == '_')
		{
			int i = 1;
			while (is_alnum(p[i]))
				++i;
			cur = new_token(TK_IDENT, cur, p, i);
			p += i;
			continue;
		}

		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	token = head.next;
}

