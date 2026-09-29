#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
enum Token {
    Identifier,
    Number,
    String,
    Assign,
    Semicolon,
    Colon,
    Comma,
    LeftParen,
    RightParen,
    Plus,
    Minus,
    Multiply,
    Divide,
    Raise,
    LessThan,
    Equal,
    GreaterThan,
    LTEqual,
    GTEqual,
    NotEqual,
    ERROR,
    // Unique Identifier Tokens
    PRINT,
    IF,
    ELSE,
    ENDIF,
    SQRT,
    AND,
    OR,
    NOT,
    EndofFile
};

void scanner(FILE* read, FILE* write);

enum Token getToken();

#endif
