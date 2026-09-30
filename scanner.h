#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

enum TokenType {
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
    EndofFile,
    // Unique Identifier Tokens
    PRINT,
    IF,
    ELSE,
    ENDIF,
    SQRT,
    AND,
    OR,
    NOT,
};

typedef struct {
    enum TokenType type;
    char lexeme[4096]; // multiple attribs per token, i.e. foo -> identifier, lexeme = foo
    int line;
} Token;

Token gettoken(FILE* read, FILE* write);
const char *tokenTypetoString(enum TokenType type);

#endif
