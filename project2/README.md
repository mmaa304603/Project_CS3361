# CS 3361 Project 2 — Recursive Descent Parser

**Team:** Jaechang Kim, Masaki Kajikuri, Abenezer Setegn

Recursive descent parser in C11 for the calculator language, using the Project 1
DFA scanner extended to capture token values.

## Build and run

Requires GCC (MinGW GCC on Windows). Run commands from the directory containing
`parser.c`. From the repository root:

```sh
cd project2
```

Linux / macOS:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -o parser parser.c
./parser tests/test1_read.txt
```

Windows (Command Prompt or PowerShell):

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -o parser.exe parser.c
.\parser.exe tests\test1_read.txt
```

Replace the example path with your input file; quote paths containing spaces.
Input is calculator source text, such as `read A`. End-of-file supplies `$$`
automatically.

Valid input produces an XML-like parse tree with token types and values.
Lexical or syntax errors produce only `Error` and a nonzero exit status.

## Tests

Run all tests for manual inspection. Tests 1–5 should produce trees; tests 6–11
should print `Error`. See `report.txt` for test descriptions and rationale.

Linux / macOS:

```sh
for f in tests/*.txt; do echo "== $f"; ./parser "$f"; done
```

Windows PowerShell:

```powershell
Get-ChildItem tests\*.txt | ForEach-Object { Write-Output "== $($_.Name)"; .\parser.exe $_.FullName }
```
