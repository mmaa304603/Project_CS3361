# Project_CS3361

## scanner for the tokens specified by the regular expression

### tokens specified by the regular expression (including comment) given in page 54 of the textbook
assign −→ :=
plus −→ +
minus −→ -
times −→ *
div −→ /
lparen −→ (
rparen −→ )
id −→ letter (letter | digit )* except for read and write
number −→ digit digit * |digit *(. digit | digit . ) digit *
comment −→ /* (non-* | * non-/)* *+/| // (non-newline)* newline

## DFA transition table

The scanner uses the following runtime states. `Stay` means that the scanner
continues reading characters in the same state. A restored lookahead character
is available to the next token scan.

| State | Input | Action / next state |
|---|---|---|
| 1 (start) | EOF | Return `END` |
| 1 | Whitespace | Stay in 1 |
| 1 | `/` | Go to 2 |
| 1 | `(`, `)`, `+`, `-`, `*` | Return `lparen`, `rparen`, `plus`, `minus`, or `times` |
| 1 | `:` | Go to 11 |
| 1 | `.` | Go to 13 |
| 1 | Digit | Go to 14 |
| 1 | Letter | Save the first identifier character; go to 16 |
| 1 | Any other character | Return `ERROR` |
| 2 (slash) | `/` | Go to 3; begin line comment |
| 2 | `*` | Go to 4; begin block comment |
| 2 | Other character, including EOF | Restore lookahead; return `div` |
| 3 (line comment) | Newline | Go to 1 |
| 3 | EOF | Return `ERROR` |
| 3 | Any other character | Stay in 3 |
| 4 (block comment) | EOF | Return `ERROR` |
| 4 | `*` | Go to 5 |
| 4 | Any other character | Stay in 4 |
| 5 (possible comment end) | EOF | Return `ERROR` |
| 5 | `/` | Go to 1; finish comment |
| 5 | `*` | Stay in 5 |
| 5 | Any other character | Go to 4 |
| 11 (colon) | `=` | Return `assign` |
| 11 | Other character, including EOF | Return `ERROR` |
| 13 (leading dot) | Digit | Go to 15 |
| 13 | Other character, including EOF | Return `ERROR` |
| 14 (integer) | Digit | Stay in 14 |
| 14 | `.` | Go to 15 |
| 14 | Other character, including EOF | Restore lookahead; return `number` |
| 15 (decimal) | Digit | Stay in 15 |
| 15 | Other character, including EOF | Restore lookahead; return `number` |
| 16 (identifier) | Letter or digit | Save the character when within the five-character prefix; stay in 16 |
| 16 | Other character, including EOF | Restore lookahead; return `read` or `write` for an exact keyword; otherwise return `id` |

### requirement
write a scanner using an imperative language
Your program must have the scan function:
1. Nameofthe function: scan.
2. Its precondition is that the current pointer of the input file is NOT at the end of file (EOF).
3. Input. You may use the (handle of the) input file as an explicit input or use the (handle of
the) input file as a global variable (and thus there is no explicit input).
4. output. If there is a valid token starting from the current pointer of the input file, the output
is the token type (i.e., the left hand side symbol of the corresponding production rule. E.g.,
for production rule id-> letter letter∧,thetokentypeof aninput ”abc” would be
id); otherwise, the output is an error flag.
5. Algorithm requirement. When deciding a token, you must use the “longest possible token” rule. Your scanner should also deal with the read and write token correctly.

Your program takes the name of a text file from the command line. It outputs to the console error if there is any non-valid token in the input file; the list of tokens otherwise.  Your implementation should be based on the automata in on Page 57 of the textbook.
Command line Invocation: scanner <input file name>
If there is any non-token string in the input file, your program should exit and output to the console only: "error".
Otherwise, your program ouputs only the list of tokens in the form of (token1, token2, ...).

Example 
- Input file (foo.txt):
read
/* foo
bar */
*
five 5
- Output: (read, times, id, number)
Note that your scanner does NOT output the token of comment. In face, your function scan should completely ignore all comments in the input. In the example, the next token of times is id, instead of comments.

### notes
Note that the DFA does not fully respect the production rule on id (i.e., the DFA will take read and write as id while they should be taken as a token of read and a token of write respectively). So, your scanner returns an ID token only if the id is not read or write. When the id is read or write, your scanner should return the token read and write respectively.

### submission: Submit a zip file containing the following to Canvas by the project due date.
1. Your report (either PDF or text files). The name of your file must start with “report”.
2. The source code for the scanner which must be compilable to an executable file using
command line instead of IDE. For C/C++: source code must be gcc/g++ compilable.
3. Areadme file on howtocompile the source code and how to run the executable.
4. Programming language to use: python/C/C++. Talk to TA if you have to use a different language