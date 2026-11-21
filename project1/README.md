CS3361 Project 1 - Scanner
Project overview
This project implements a scanner (lexical analyzer) in C for the calculator language described in the course textbook (token rules on page 54 and DFA on page 57). A scanner reads a program character by character and groups characters into tokens such as id, number, and assign. It does not calculate expressions or check the grammar of entire statements.

For example, the input read score; is invalid for this scanner because ; is not a supported character. In contrast, read score := 10 produces (read, id, assign, number).
Files
- scanner.c — the C source code, containing the DFA-based scan() function and program entry point main().
- pseudocode.txt — a human-readable outline of the program with comments explaining each step.
- report.txt — the project report with token rules, DFA states, and tests.
- foo.txt — sample input file.
- README.md — overview, build instructions, and explanation.
Recognized tokens
Input	Printed token name	Meaning
:=	assign	Assignment operator
+	plus	Addition operator
-	minus	Subtraction operator
*	times	Multiplication operator
/	div	Division operator, unless followed by / or * to begin a comment
(	lparen	Left parenthesis
)	rparen	Right parenthesis
Letters followed by letters/digits, e.g. score2	id	Identifier (except the exact keywords below)
Digits and supported decimal forms, e.g. 10, .5, 5., 12.34	number	Number
read	read	Reserved word
write	write	Reserved word


Spaces, tabs, line breaks, /* ... */ comments, and // ... comments ending in a newline are skipped; they do not appear in the token output. The code treats keywords as case-sensitive: Read is id, but read is read.
How the scanner works
1. main() checks that exactly one file name was supplied and opens that file.
2. main() repeatedly calls scan(input) to recognize the next token.
3. scan() reads one character with fgetc() and uses the current DFA state to decide what that character means.
4. When a token is complete, scan() returns its TokenType. If it has looked one character too far, it uses put_back() (which calls ungetc()) so that character can start the next token.
5. main() stores token types in a dynamically allocated array through append_token() rather than printing them immediately.
6. After the entire file is successfully scanned, token_name() converts token types to words like id and number, and main() prints them in parentheses.
7. If a lexical error, file error, or storage failure occurs, the program prints only error rather than a partial token list.
Why DFA states matter
A DFA is a set of states and rules for moving between states based on incoming characters. In scanner.c, a switch (state) represents those rules. Important states include:
- START (1): skips whitespace and determines what kind of token is beginning.
- SLASH (2): determines whether / means division or starts a comment.
- LINE_COMMENT (3): ignores characters until a newline; EOF before that newline is an error in this implementation.
- BLOCK_COMMENT (4) and BLOCK_STARS (5): find the ending */; EOF before the end is an error.
- COLON (11): expects = to make the assignment operator :=.
- LEADING_DOT (13): expects a digit after an initial ..
- INTEGER (14) and DECIMAL (15): consume a complete number.
- IDENTIFIER (16): consumes letters/digits, then distinguishes read and write from ordinary identifiers.
Some single-character tokens return immediately and do not require storing a separate state in the C program.
Walkthrough of one input
Input:
read score := 10 + 5
Processing:
1. read becomes read because it matches the reserved word exactly.
2. score becomes id because it is an identifier, not a reserved word.
3. := becomes assign because the colon is followed by =.
4. 10 becomes number because the scanner consumes the entire number before returning.
5. + becomes plus.
6. 5 becomes number.
Output:
(read, id, assign, number, plus, number)
Important implementation details
Longest match: When reading a number or identifier, the scanner keeps consuming characters that are part of that token. This means read1 is one id, not read followed by number.

One-character lookahead: The program may read one character that belongs to the following token. put_back() returns that character to the input stream so the next scan() call can read it.

Keyword buffer: word[6] saves up to the first five identifier characters, and length distinguishes identifiers longer than a keyword. This is enough to decide whether the complete identifier is exactly read or write.

All-or-nothing output: append_token() stores tokens temporarily. Printing happens only after the file passes validation; therefore read x @ produces error, not a partial list followed by error.

Token names vs. token values: The output shows token types, not the original variable names or numbers. For example, score becomes id and 10 becomes number.
How to compile
Requires GCC. Run the following from the folder containing scanner.c:
gcc -std=c11 -Wall -o scanner scanner.c
On Windows (MinGW), the output executable is scanner.exe.
How to run
On macOS/Linux:
./scanner foo.txt
On Windows (PowerShell):
.\scanner.exe foo.txt
Example foo.txt:
read
/* foo
bar */
*
five 5
Expected output:
(read, times, id, number)
Errors and special cases
- x : 2 -> error, because : must be followed by =.
- . -> error, because a number starting with . needs a following digit.
- x @ 2 -> error, because @ is not a supported character.
- /* unfinished -> error, because the block comment never closes.
- // unfinished with no final newline -> error in this implementation.
- An empty file -> ().
Main functions in scanner.c
Function	Responsibility
scan(FILE *input)	Read input using DFA states and return the next token, TOK_END, or TOK_ERROR.
put_back(int c, FILE *input)	Restore one lookahead character to the input stream (except EOF).
token_name(TokenType token)	Convert an internal token enum to its printable name.
append_token(...)	Grow the token array if necessary and store the next recognized token.
main(int argc, char *argv[])	Validate the command line, open the file, collect tokens, detect errors, print final output, and release memory.


Team review / interview preparation
Be prepared to explain (1) what a token is, (2) how switch (state) implements the DFA, (3) why / requires lookahead, (4) why read1 is an id, (5) how invalid input is handled, and (6) why tokens are saved before printing. The annotated pseudocode file follows the same execution order as the source code.
