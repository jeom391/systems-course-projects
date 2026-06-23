# miniC Lexical Analyzer

A C++ lexical analyzer for a simplified MiniC language.

This project analyzes MiniC source code and breaks it into tokens such as keywords, identifiers, operators, delimiters, integer literals, character literals, string literals, and double literals.
It was implemented by analyzing and extending a provided MiniC scanner structure as part of a formal language/compiler course project.

## Overview

The scanner reads MiniC source code character by character and classifies each lexeme into a corresponding token type.

The main goal of this project was to understand how lexical analysis works internally, especially how regular-expression-like token rules can be implemented using procedural code such as `fgetc()`, `switch`, `while`, and `ungetc()`.

## Features

### Basic Token Recognition

The scanner recognizes common MiniC tokens, including:

* Keywords
* Identifiers
* Integer literals
* Operators
* Delimiters
* Comments
* End-of-file token

### Extended Keyword Support

In addition to the original MiniC keywords, the scanner was extended to recognize the following keywords:

```text
char
double
for
do
goto
switch
case
break
default
```

These keywords are stored in the `keyword[]` array and mapped to their corresponding token numbers through the `tnum[]` array.

### Character Literal Support

The scanner supports character literals enclosed in single quotes.

Examples:

```c
'a'
'\n'
'\t'
'\\'
'\''
'\"'
```

The recognized character value is stored in `token.value.ch`.

Invalid character literals, such as missing closing quotes or unsupported escape sequences, are handled as lexical errors.

### String Literal Support

The scanner supports string literals enclosed in double quotes.

Examples:

```c
"hello"
"hello\nworld"
"quote: \""
"backslash: \\"
```

The recognized string value is stored in `token.value.str`.

The scanner supports normal characters and escape sequences inside strings.
If a string literal reaches a newline or EOF before a closing double quote, it is treated as an invalid string literal.

### Double Literal Support

The scanner was extended to recognize double literals in multiple formats.

Supported examples:

```c
3.14
0.123
.123
123.
123.456e10
123.456e+10
123.456e-10
.123e-2
123.e+5
```

The recognized double value is converted using `strtod()` and stored in `token.value.dnum`.

### Lexical Error Recovery

When the scanner encounters an invalid token or malformed literal, it prints a lexical error message but does not immediately terminate the scanning process.

Instead, the invalid token is discarded by setting the token number to `tnull`, allowing the scanner to continue recognizing the next valid token.

This makes it possible to observe multiple tokens and errors in a single input file.

## Project Structure

```text
minic-lexer/
├── README.md
├── src/
│   ├── Main.cpp
│   ├── Scanner.cpp
│   └── Scanner.h
└── examples/
    ├── example.mc
    ├── extended_test.mc
    └── ...
```

## Core Implementation

### `Scanner.h`

`Scanner.h` defines the token-related data structures and token symbols.

The `tokenType` structure stores both the token number and the token value.
To support extended literals, the value union was expanded to include:

```cpp
char ch;
double dnum;
char str[STRING_LENGTH];
```

The `enum tsymbol` was also extended with additional keyword tokens and literal tokens, including:

* Character literal token
* String literal token
* Double literal token

The order of this enum must match the order of the `tokenName[]` array in `Scanner.cpp`.

### `Scanner.cpp`

`Scanner.cpp` contains the main scanner logic.

Important components include:

| Component                | Description                                         |
| ------------------------ | --------------------------------------------------- |
| `keyword[]`              | Stores keyword strings                              |
| `tnum[]`                 | Maps each keyword to its token number               |
| `scanner()`              | Main lexical scanning function                      |
| `getNumberToken()`       | Distinguishes integer literals from double literals |
| `getCharacterLiteral()`  | Handles character literals and escape sequences     |
| `getStringLiteral()`     | Handles string literals and escape sequences        |
| `getDotStartingDouble()` | Handles double literals that start with `.`         |
| `lexicalError()`         | Prints lexical error messages                       |
| `printToken()`           | Prints recognized tokens and their values           |

## Token Recognition Flow

The scanner follows this general flow:

```text
Read character
     |
     v
Skip whitespace
     |
     v
Check token category
     |
     ├── Letter or '_'  -> Identifier or keyword
     ├── Digit          -> Integer or double literal
     ├── '\''           -> Character literal
     ├── '"'            -> String literal
     ├── '.' + digit    -> Dot-starting double literal
     ├── Operator       -> Operator token
     ├── Delimiter      -> Delimiter token
     └── Invalid input  -> Lexical error
```

## Example Input

```c
char c1 = 'a';
char c2 = '\n';

double d1 = 3.14;
double d2 = .123;
double d3 = 123.;
double d4 = 123.456e-10;

for do goto switch case break default

"hello"
"hello\nworld"
"quote: \""
"backslash: \\"
```

## Example Output Format

The scanner prints each recognized token with its token number and value when applicable.

Example format:

```text
Current Token --> number: 35(char)
Current Token --> number: 46, value: 'a'
Current Token --> number: 48, value: 3.140000
Current Token --> number: 47, value: "hello"
```

For identifiers and literals, the scanner prints the actual stored value.
For keywords, operators, and delimiters, it prints the token name.

## Error Handling

The scanner reports lexical errors for cases such as:

* Invalid characters
* Invalid character literals
* Invalid string literals
* Invalid double literals
* Too-long identifiers or string literals

Example:

```text
@error@
```

In this case, `@` is reported as an invalid character, while `error` can still be recognized as a valid identifier afterward.

## What I Learned

Through this project, I learned how a compiler front-end begins processing source code.

Before implementing this scanner extension, tokens such as `char`, `double`, `"hello"`, and `3.14` simply looked like ordinary programming syntax.
However, from the scanner's perspective, they are just raw text that must be classified according to lexical rules.

This project helped me understand how concepts from formal languages, such as regular expressions and finite automata, can be translated into actual scanner logic using C/C++ code.

Although the scanner was not implemented entirely from scratch, extending the provided structure gave me hands-on experience with compiler front-end internals and strengthened my understanding of how source code is transformed into token streams.

## Tech Stack

* Language: C++
* Concepts: Lexical analysis, tokenization, finite automata, compiler front-end
* Tools: Visual Studio / C++ compiler

## Build and Run

This project was originally developed in a Visual Studio environment.

If using a command-line C++ compiler, the project can be compiled in a structure similar to the following:

```bash
g++ -std=c++17 src/Main.cpp src/Scanner.cpp -o minic_lexer
```

Then run the scanner with a MiniC input file depending on the input handling implemented in `Main.cpp`.

```bash
./minic_lexer
```

## Repository Purpose

This repository is part of my systems and compiler course project collection.
It focuses on understanding how programming language source code is recognized and converted into tokens before parsing and semantic analysis.
