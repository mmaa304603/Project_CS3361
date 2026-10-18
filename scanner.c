#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TOK_END,
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

/* Put back a character unless it is EOF. */
static int put_back(int c, FILE *input)
{
    return c == EOF || ungetc(c, input) != EOF;
}

/* Scan the next token using the DFA. */
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

    char word[6] = {0};
    size_t length = 0;

    for (;;) {
        int c = fgetc(input);

        switch (state) {

        case START:
            if (c == EOF)
                return TOK_END;

            if (c == ' ' || c == '\t' ||
                c == '\n' || c == '\r')
                break;

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
            else if (isalpha((unsigned char)c)) {
                state = IDENTIFIER;
                word[0] = (char)c;
                length = 1;
            }
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
            if (c == '=')
                return TOK_ASSIGN;
            return TOK_ERROR;

        /* State 13: number starting with '.' */
        case LEADING_DOT:
            if (!isdigit((unsigned char)c))
                return TOK_ERROR;
            state = DECIMAL;
            break;

        /* State 14: integer */
        case INTEGER:
            if (c != EOF && isdigit((unsigned char)c)) {
                /* Remain in INTEGER state. */
            }
            else if (c == '.')
                state = DECIMAL;
            else {
                if (!put_back(c, input))
                    return TOK_ERROR;
                return TOK_NUMBER;
            }
            break;

        /* State 15: decimal number */
        case DECIMAL:
            if (c != EOF && isdigit((unsigned char)c)) {
                /* Remain in DECIMAL state. */
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

                if (length < 5)
                    word[length] = (char)c;

                if (length < 6)
                    ++length;
            }
            else {
                if (!put_back(c, input))
                    return TOK_ERROR;

                if (length == 4 &&
                    memcmp(word, "read", 4) == 0)
                    return TOK_READ;

                if (length == 5 &&
                    memcmp(word, "write", 5) == 0)
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
    default:         return NULL;
    }
}

/* Store tokens so nothing is printed before validation. */
static int append_token(TokenType **tokens,
                        size_t *count,
                        size_t *capacity,
                        TokenType token)
{
    if (*count == *capacity) {
        size_t new_capacity =
            *capacity ? *capacity * 2 : 32;

        if (new_capacity <= *capacity ||
            new_capacity > SIZE_MAX / sizeof(**tokens))
            return 0;

        TokenType *grown =
            realloc(*tokens,
                    new_capacity * sizeof(**tokens));

        if (grown == NULL)
            return 0;

        *tokens = grown;
        *capacity = new_capacity;
    }

    (*tokens)[(*count)++] = token;
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        puts("error");
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");

    if (input == NULL) {
        puts("error");
        return EXIT_FAILURE;
    }

    TokenType *tokens = NULL;
    size_t count = 0;
    size_t capacity = 0;
    int invalid = 0;

    for (;;) {
        /* Check EOF before calling scan(). */
        int c = fgetc(input);

        if (c == EOF)
            break;

        if (ungetc(c, input) == EOF) {
            invalid = 1;
            break;
        }

        TokenType token = scan(input);

        if (token == TOK_END)
            break;

        if (token == TOK_ERROR ||
            !append_token(&tokens, &count,
                          &capacity, token)) {
            invalid = 1;
            break;
        }
    }

    if (ferror(input))
        invalid = 1;

    if (fclose(input) != 0)
        invalid = 1;

    if (invalid) {
        puts("error");
        free(tokens);
        return EXIT_FAILURE;
    }

    printf("(");

    for (size_t i = 0; i < count; i++) {
        if (i > 0)
            printf(", ");

        printf("%s", token_name(tokens[i]));
    }

    printf(")\n");

    free(tokens);
    return EXIT_SUCCESS;
}
