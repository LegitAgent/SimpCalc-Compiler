#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
enum Token
{
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
    PRINT,
    ERROR,
    // Unique Identifier Tokens
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

enum FinalState gettoken();

#endif
