CS 3361 Project 2 - Recursive Descent Parser
Team: Jaechang Kim, Masaki Kajikuri, Abenezer Setegn

Language: C (C11). The scan() function is our Project 1 scanner, extended to
also return the token value (lexeme).

How to compile (gcc)
--------------------
    gcc -std=c11 -Wall -o parser parser.c

On Windows (MinGW) this creates parser.exe.

How to run
----------
Linux / macOS:
    ./parser <input file name>
    ./parser tests/test1_read.txt

Windows (cmd or PowerShell):
    parser.exe tests\test1_read.txt

Output
------
The XML-like parse tree is printed to the console. If a lexical or syntax
error is found, only "Error" is printed and the program stops.

Run all test cases
------------------
Linux / macOS:
    for f in tests/*.txt; do echo "== $f"; ./parser "$f"; done
Windows (cmd):
    for %f in (tests\*.txt) do @echo == %f & parser.exe %f
