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

void scanner(FILE* read, FILE* write);

typedef struct {
    enum TokenType type;
    char lexeme[256]; // multiple attribs per token, i.e. foo -> identifier, lexeme = foo
} Token;

Token getToken(void);

#endif
