/*
 * CS 3361 Project 2 - Recursive Descent Parser
 * Team: Jaechang Kim, Masaki Kajikuri, Abenezer Setegn
 *
 * Compile:  gcc -std=c11 -Wall -o parser parser.c
 * Run:      ./parser <input file name>
 *
 * The scan() function is the DFA-based scanner from Project 1 (scanner.c).
 * It was extended to also save the token value (lexeme) so the parse tree can
 * show both the token type and its content, e.g. <id> A </id>.
 *
 * Grammar:
 *   <program>   -> <stmt_list> $$
 *   <stmt_list> -> <stmt> <stmt_list> | e
 *   <stmt>      -> id assign <expr> | read id | write <expr>
 *   <expr>      -> <term> <term_tail>
 *   <term_tail> -> <add_op> <term> <term_tail> | e
 *   <term>      -> <factor> <fact_tail>
 *   <fact_tail> -> <mult_op> <factor> <fact_tail> | e
 *   <factor>    -> lparen <expr> rparen | id | number
 *   <add_op>    -> plus | minus
 *   <mult_op>   -> times | div
 *
 * Output: the XML-like parse tree, or only "Error" on a lexical/syntax error.
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TOK_END,        /* the pseudo-token $$ */
    TOK_ERROR,
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_TIMES,
    TOK_DIV,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_ID,
    TOK_NUMBER,
    TOK_READ,
    TOK_WRITE
} TokenType;

/* ------------------------------------------------------------------------ */
/* Error handling: print only "Error" and stop.                              */
/* ------------------------------------------------------------------------ */

static void fail(void)
{
    puts("Error");
    exit(EXIT_FAILURE);
}

/* ------------------------------------------------------------------------ */
/* Lexeme buffer (extension for Project 2): the characters of the token      */
/* that scan() most recently returned.                                       */
/* ------------------------------------------------------------------------ */

static char  *lexeme = NULL;
static size_t lex_len = 0;
static size_t lex_cap = 0;

static void lex_reset(void)
{
    lex_len = 0;
    if (lexeme != NULL)
        lexeme[0] = '\0';
}

static void lex_add(int c)
{
    if (lex_len + 1 >= lex_cap) {
        size_t new_cap = lex_cap ? lex_cap * 2 : 32;
        char *grown = realloc(lexeme, new_cap);
        if (grown == NULL)
            fail();
        lexeme = grown;
        lex_cap = new_cap;
    }
    lexeme[lex_len++] = (char)c;
    lexeme[lex_len] = '\0';
}

/* ------------------------------------------------------------------------ */
/* Scanner from Project 1                                                    */
/* ------------------------------------------------------------------------ */

/* Put back a character unless it is EOF. */
static int put_back(int c, FILE *input)
{
    return c == EOF || ungetc(c, input) != EOF;
}

/* Scan the next token using the DFA. Returns ONE token per call.
 * The token's characters are saved in lexeme. */
TokenType scan(FILE *input)
{
    enum {
        START = 1,
        SLASH = 2,
        LINE_COMMENT = 3,
        BLOCK_COMMENT = 4,
        BLOCK_STARS = 5,
        COLON = 11,
        LEADING_DOT = 13,
        INTEGER = 14,
        DECIMAL = 15,
        IDENTIFIER = 16
    } state = START;

    lex_reset();

    for (;;) {
        int c = fgetc(input);

        switch (state) {

        case START:
            if (c == EOF)
                return TOK_END;

            if (c == ' ' || c == '\t' ||
                c == '\n' || c == '\r')
                break;

            lex_reset();        /* a new token starts here */
            lex_add(c);

            if (c == '/')
                state = SLASH;
            else if (c == '(')
                return TOK_LPAREN;
            else if (c == ')')
                return TOK_RPAREN;
            else if (c == '+')
                return TOK_PLUS;
            else if (c == '-')
                return TOK_MINUS;
            else if (c == '*')
                return TOK_TIMES;
            else if (c == ':')
                state = COLON;
            else if (c == '.')
                state = LEADING_DOT;
            else if (isdigit((unsigned char)c))
                state = INTEGER;
            else if (isalpha((unsigned char)c))
                state = IDENTIFIER;
            else
                return TOK_ERROR;
            break;

        /* State 2: division or start of comment */
        case SLASH:
            if (c == '/')
                state = LINE_COMMENT;
            else if (c == '*')
                state = BLOCK_COMMENT;
            else {
                if (!put_back(c, input))
                    return TOK_ERROR;
                return TOK_DIV;
            }
            break;

        /* State 3: // comment */
        case LINE_COMMENT:
            if (c == '\n')
                state = START;
            else if (c == EOF)
                return TOK_ERROR;
            break;

        /* State 4: inside block comment */
        case BLOCK_COMMENT:
            if (c == EOF)
                return TOK_ERROR;
            if (c == '*')
                state = BLOCK_STARS;
            break;

        /* State 5: possible end of block comment */
        case BLOCK_STARS:
            if (c == EOF)
                return TOK_ERROR;

            if (c == '/')
                state = START;
            else if (c != '*')
                state = BLOCK_COMMENT;
            break;

        /* States 11 and 12: := */
        case COLON:
            if (c == '=') {
                lex_add(c);
                return TOK_ASSIGN;
            }
            return TOK_ERROR;

        /* State 13: number starting with '.' */
        case LEADING_DOT:
            if (c == EOF || !isdigit((unsigned char)c))
                return TOK_ERROR;
            lex_add(c);
            state = DECIMAL;
            break;

        /* State 14: integer */
        case INTEGER:
            if (c != EOF && isdigit((unsigned char)c)) {
                lex_add(c);     /* Remain in INTEGER state. */
            }
            else if (c == '.') {
                lex_add(c);
                state = DECIMAL;
            }
            else {
                if (!put_back(c, input))
                    return TOK_ERROR;
                return TOK_NUMBER;
            }
            break;

        /* State 15: decimal number */
        case DECIMAL:
            if (c != EOF && isdigit((unsigned char)c)) {
                lex_add(c);     /* Remain in DECIMAL state. */
            }
            else {
                if (!put_back(c, input))
                    return TOK_ERROR;
                return TOK_NUMBER;
            }
            break;

        /* State 16: identifier or keyword */
        case IDENTIFIER:
            if (c != EOF &&
                (isalpha((unsigned char)c) ||
                 isdigit((unsigned char)c))) {
                lex_add(c);
            }
            else {
                if (!put_back(c, input))
                    return TOK_ERROR;

                if (strcmp(lexeme, "read") == 0)
                    return TOK_READ;

                if (strcmp(lexeme, "write") == 0)
                    return TOK_WRITE;

                return TOK_ID;
            }
            break;
        }
    }
}

/* Convert token types into output strings. */
static const char *token_name(TokenType token)
{
    switch (token) {
    case TOK_ASSIGN: return "assign";
    case TOK_PLUS:   return "plus";
    case TOK_MINUS:  return "minus";
    case TOK_TIMES:  return "times";
    case TOK_DIV:    return "div";
    case TOK_LPAREN: return "lparen";
    case TOK_RPAREN: return "rparen";
    case TOK_ID:     return "id";
    case TOK_NUMBER: return "number";
    case TOK_READ:   return "read";
    case TOK_WRITE:  return "write";
    case TOK_END:    return "$$";
    default:         return "error";
    }
}

/* ------------------------------------------------------------------------ */
/* Output buffer: the tree is printed only if parsing succeeds.              */
/* ------------------------------------------------------------------------ */

static char  *out = NULL;
static size_t out_len = 0;
static size_t out_cap = 0;
static int    depth = 0;           /* current tree level */

#define INDENT "   "               /* 3 spaces per level */

/* Append one line: (indent * level) + text + newline. */
static void emit(int level, const char *fmt, ...)
{
    char line[1024];
    int n = 0;
    va_list ap;

    for (int i = 0; i < level && n < 900; i++)
        n += snprintf(line + n, sizeof line - n, "%s", INDENT);

    va_start(ap, fmt);
    vsnprintf(line + n, sizeof line - n, fmt, ap);
    va_end(ap);

    size_t len = strlen(line);
    if (out_len + len + 2 > out_cap) {
        size_t new_cap = out_cap ? out_cap * 2 : 4096;
        while (new_cap < out_len + len + 2)
            new_cap *= 2;
        char *grown = realloc(out, new_cap);
        if (grown == NULL)
            fail();
        out = grown;
        out_cap = new_cap;
    }
    memcpy(out + out_len, line, len);
    out_len += len;
    out[out_len++] = '\n';
    out[out_len] = '\0';
}

static void open_nt(const char *name)
{
    emit(depth, "<%s>", name);
    depth++;
}

static void close_nt(const char *name)
{
    depth--;
    emit(depth, "</%s>", name);
}

/* ------------------------------------------------------------------------ */
/* Recursive descent parser                                                  */
/* ------------------------------------------------------------------------ */

static FILE     *input;
static TokenType lookahead;        /* current token type */
static char     *lookahead_value;  /* current token value */

/* Return a newly allocated copy of s. */
static char *copy_string(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p == NULL)
        fail();
    memcpy(p, s, n);
    return p;
}

/* Read the next token into lookahead / lookahead_value. */
static void next_token(void)
{
    lookahead = scan(input);
    if (lookahead == TOK_ERROR)
        fail();                    /* lexical error */
    free(lookahead_value);
    lookahead_value = copy_string(
        (lookahead == TOK_END || lexeme == NULL) ? "" : lexeme);
}

/* Consume the lookahead if it is the expected token; print it. */
static void match(TokenType expected)
{
    if (lookahead != expected)
        fail();                    /* syntax error */
    emit(depth, "<%s>", token_name(lookahead));
    emit(depth + 1, "%s", lookahead_value);
    emit(depth, "</%s>", token_name(lookahead));
    next_token();
}

static void stmt_list(void);
static void stmt(void);
static void expr(void);
static void term_tail(void);
static void term(void);
static void fact_tail(void);
static void factor(void);
static void add_op(void);
static void mult_op(void);

/* <program> -> <stmt_list> $$ */
static void program(void)
{
    open_nt("Program");
    stmt_list();
    if (lookahead != TOK_END)
        fail();
    close_nt("Program");
}

/* <stmt_list> -> <stmt> <stmt_list> | e        FOLLOW = { $$ } */
static void stmt_list(void)
{
    open_nt("stmt_list");
    if (lookahead == TOK_ID || lookahead == TOK_READ ||
        lookahead == TOK_WRITE) {
        stmt();
        stmt_list();
    }
    else if (lookahead == TOK_END) {
        /* epsilon */
    }
    else
        fail();
    close_nt("stmt_list");
}

/* <stmt> -> id assign <expr> | read id | write <expr> */
static void stmt(void)
{
    open_nt("stmt");
    if (lookahead == TOK_ID) {
        match(TOK_ID);
        match(TOK_ASSIGN);
        expr();
    }
    else if (lookahead == TOK_READ) {
        match(TOK_READ);
        match(TOK_ID);
    }
    else if (lookahead == TOK_WRITE) {
        match(TOK_WRITE);
        expr();
    }
    else
        fail();
    close_nt("stmt");
}

/* <expr> -> <term> <term_tail> */
static void expr(void)
{
    open_nt("expr");
    term();
    term_tail();
    close_nt("expr");
}

/* <term_tail> -> <add_op> <term> <term_tail> | e
 * FOLLOW = { rparen, id, read, write, $$ } */
static void term_tail(void)
{
    open_nt("term_tail");
    if (lookahead == TOK_PLUS || lookahead == TOK_MINUS) {
        add_op();
        term();
        term_tail();
    }
    else if (lookahead == TOK_RPAREN || lookahead == TOK_ID ||
             lookahead == TOK_READ || lookahead == TOK_WRITE ||
             lookahead == TOK_END) {
        /* epsilon */
    }
    else
        fail();
    close_nt("term_tail");
}

/* <term> -> <factor> <fact_tail> */
static void term(void)
{
    open_nt("term");
    factor();
    fact_tail();
    close_nt("term");
}

/* <fact_tail> -> <mult_op> <factor> <fact_tail> | e
 * FOLLOW = { plus, minus, rparen, id, read, write, $$ } */
static void fact_tail(void)
{
    open_nt("fact_tail");
    if (lookahead == TOK_TIMES || lookahead == TOK_DIV) {
        mult_op();
        factor();
        fact_tail();
    }
    else if (lookahead == TOK_PLUS || lookahead == TOK_MINUS ||
             lookahead == TOK_RPAREN || lookahead == TOK_ID ||
             lookahead == TOK_READ || lookahead == TOK_WRITE ||
             lookahead == TOK_END) {
        /* epsilon */
    }
    else
        fail();
    close_nt("fact_tail");
}

/* <factor> -> lparen <expr> rparen | id | number */
static void factor(void)
{
    open_nt("factor");
    if (lookahead == TOK_LPAREN) {
        match(TOK_LPAREN);
        expr();
        match(TOK_RPAREN);
    }
    else if (lookahead == TOK_ID)
        match(TOK_ID);
    else if (lookahead == TOK_NUMBER)
        match(TOK_NUMBER);
    else
        fail();
    close_nt("factor");
}

/* <add_op> -> plus | minus */
static void add_op(void)
{
    open_nt("add_op");
    if (lookahead == TOK_PLUS)
        match(TOK_PLUS);
    else if (lookahead == TOK_MINUS)
        match(TOK_MINUS);
    else
        fail();
    close_nt("add_op");
}

/* <mult_op> -> times | div */
static void mult_op(void)
{
    open_nt("mult_op");
    if (lookahead == TOK_TIMES)
        match(TOK_TIMES);
    else if (lookahead == TOK_DIV)
        match(TOK_DIV);
    else
        fail();
    close_nt("mult_op");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        puts("Usage: parser <input file name>");
        return EXIT_FAILURE;
    }

    input = fopen(argv[1], "r");
    if (input == NULL)
        fail();

    next_token();       /* read the first lookahead token */
    program();

    fclose(input);
    fputs(out, stdout); /* print the whole tree only after success */

    free(out);
    free(lexeme);
    free(lookahead_value);
    return EXIT_SUCCESS;
}
