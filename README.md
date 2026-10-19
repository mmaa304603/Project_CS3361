# CS3361 Project 1 - Scanner

Scanner for the calculator-language tokens (textbook p.54), based on the DFA on p.57.

## Files
- `scanner.c` - source code (contains the `scan` function)
- `report.txt` - project report
- `README.md` - this file

## How to compile
Requires gcc. From the folder containing `scanner.c`:

```
gcc -std=c11 -Wall -o scanner scanner.c
```

On Windows (MinGW) this produces `scanner.exe`.

## How to run
```
./scanner <input file name>
```
(Windows: `scanner.exe <input file name>`)

Example - `foo.txt`:
```
read
/* foo
bar */
*
five 5
```
```
$ ./scanner foo.txt
(read, times, id, number)
```

## Output
- Valid input: list of token types, e.g. `(read, times, id, number)`
- Any invalid token (or unclosed comment, or file cannot be opened): `error` only

Token types: `assign plus minus times div lparen rparen id number read write`.
Comments and whitespace are skipped and never printed.
