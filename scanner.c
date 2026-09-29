#include <stdio.h>
#include <string.h>
#include "scanner.h"

enum State {
    A,
    D,
    DOT,
    F,
    E,
    EXP,
    SIGN,
    STR,
    LT,
    I,
    COM,
    DIV,
    GT,
    COL,
    NEQ,
    MULT,
    ERRORs,
    EOFs
};

typedef struct {
    enum Token type;
    char lexeme[256]; // multiple attribs per token, i.e. foo -> identifier, lexeme = foo
} Token;

Token tokenList[10000];
int tokenIdx = 0;

// character buffer for stmt (string)
char buffer[4096]; // so max stmt len would be 4095, + 1 for null terminator
int bufferIdx = 0;
enum State state = A; // DEFAULT STATE

void setDefaultStates() {
    bufferIdx = 0;
    state = A;
}

// Adds a specific token to the token list, with a token type and token contents
void addToken(enum Token type, const char *lexeme) {
    tokenList[tokenIdx].type = type;
    strcpy(tokenList[tokenIdx].lexeme, lexeme);
    tokenIdx++;

    setDefaultStates();
}

void finishScanLine(FILE* file_ptr_w, enum Token type, const char *scanType) {
    buffer[bufferIdx] = '\0';
    fprintf(file_ptr_w, "%s\t%s\n", scanType, buffer);
    addToken(type, buffer);

}

void scanner(FILE* file_ptr_r, FILE* file_ptr_w) {
    
    int c; // cur char
    setDefaultStates();
    // https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    // iterate over file character by character
    
    while ((c = fgetc(file_ptr_r)) != EOF) {
        if (state == ERRORs) {
            printf("Error.");
            break;
        }

        if (tokenIdx > 10000) {
            fputs("Exceeded token limit.", file_ptr_w);
            addToken(ERROR, "Exceeded token limit");
            break;
        }

        if (bufferIdx > 4096) {
            fputs("Exceeded statement length.", file_ptr_w);
            addToken(ERROR, "Exceeded statement length.");
            break;
        }

        switch(state) {
            case A:
                if (c == '<') state = LT;
                else if (c == '>') state = GT;
                else if (c == '/') state = DIV;
                else if (c == ':') state = COL;
                else if (c == '!') state = NEQ;
                else if (c == '*') state = MULT;
                else if (c == '(') {
                    fputs("LeftParen\t(\n", file_ptr_w); // put in scanner file
                    addToken(LeftParen, "("); // put in token list
                } else if (c == ')') {
                    fputs("RightParen\t)\n", file_ptr_w);
                    addToken(RightParen, ")");
                } else if (c == '-') {
                    fputs("Minus\t-\n", file_ptr_w);
                    addToken(Minus, "-");
                } else if (c == '+') {
                    fputs("Plus\t+\n", file_ptr_w);
                    addToken(Plus, "+");
                } else if (c == ';') {
                    fputs("Semicolon\t;\n", file_ptr_w);
                    addToken(Semicolon, ";");
                } else if (c == ',') {
                    fputs("Comma\t,\n", file_ptr_w);
                    addToken(Comma, ",");
                } else if (c == '"') {
                    state = STR;
                    buffer[bufferIdx++] = c;
                } else if (c >= '0' && c <= '9') {
                    state = D;
                    buffer[bufferIdx++] = c;
                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                    state = I;
                    buffer[bufferIdx++] = c;
                }
                break;
            case D:
                if (c >= '0' && c <= '9') {
                    state = D;

                    buffer[bufferIdx++] = c;
                } else if (c == '.') {
                    state = DOT;

                    buffer[bufferIdx++] = c;
                } else if (c == 'e' || c == 'E') {
                    state = E;

                    buffer[bufferIdx++] = c;
                } else {
                    finishScanLine(file_ptr_w, Number, "Number");

                    ungetc(c, file_ptr_r);
                }
                break;
            case DOT:
                if (c >= '0' && c <= '9') {
                    state = F;
                    buffer[bufferIdx++] = c;
                } else {
                    state = ERRORs;
                    
                    addToken(ERROR, "Error.");
                }
                break;
            case F:
                if (c >= '0' && c <= '9') {
                    state = F;
                    buffer[bufferIdx++] = c;
                } else if (c == 'e' || c == 'E') {
                    state = E;
                    buffer[bufferIdx++] = c;
                } else {
                    finishScanLine(file_ptr_w, Number, "Number");
                
                    ungetc(c, file_ptr_r);
                }
                break;
            case E:
                if (c == '+' || c == '-') {
                    state = SIGN;
                    buffer[bufferIdx++] = c;
                } else if (c >= '0' && c <= '9') {
                    state = EXP;
                    buffer[bufferIdx++] = c;
                } else {
                    state = ERRORs;
                    addToken(ERROR, "Error.");
                }
                break;
            case EXP:
                if (c >= '0' && c <= '9') {
                    state = EXP;
                    buffer[bufferIdx++] = c;
                } else {
                    finishScanLine(file_ptr_w, Number, "Number");

                    ungetc(c, file_ptr_r);
                }
                break;
            case SIGN:
                if (c >= '0' && c <= '9') {
                    state = EXP;
                    buffer[bufferIdx++] = c;
                } else {
                    state = ERRORs;
                    addToken(ERROR, "Error.");
                }
                break;
            case STR:
                if (c == '"') {
                    finishScanLine(file_ptr_w, String, "String");
                } else if (c == '\n') {
                    state = ERRORs;
                    addToken(ERROR, "Error.");
                } else {
                    state = STR;
                    buffer[bufferIdx++] = c;
                }
                break;
            case LT:
                // this fails in the case of <=h, should be not valid, but with this, it will still recognize LTE
                if (c == '=') {
                    // reset state and push to tokenList the token LTEqual
                    fputs("LessThan\t<=\n", file_ptr_w);

                    addToken(LTEqual, "<=");
                } else {
                    // reset state and push to tokenList the token LessThan
                    // do push back
                    fputs("LessThan\t<\n", file_ptr_w);

                    addToken(LessThan, "<");
                    ungetc(c, file_ptr_r);
                }
                break;
            case I:
                if ((c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_') {
                    state = I;
                    buffer[bufferIdx++] = c;
                } else {
                    finishScanLine(file_ptr_w, Identifier, "Identifier");

                    ungetc(c, file_ptr_r);
                }
                break;
            case COM:
                if (c == '\n')
                    state = A; // reset state if newline
                break;
            case DIV:
                if (c == '/') {
                    // set state to COM
                    state = COM;
                } else {
                    fputs("Divide\t/\n", file_ptr_w);

                    addToken(Divide, "/");
                    ungetc(c, file_ptr_r);
                }
                break;
            case GT:
                if (c == '=') {
                    // reset state and push to tokenList the token GTEqual
                    fputs("GreaterThan\t>=\n", file_ptr_w);

                    addToken(GTEqual, ">=");
                } else {
                    fputs("GreaterThan\t>\n", file_ptr_w);

                    addToken(GreaterThan, ">");
                    ungetc(c, file_ptr_r);
                }
                break;
            case COL:
                if (c == '=') {
                    fputs("Assign\t:=\n", file_ptr_w);
                    addToken(Assign, ":=");
                } else {
                    fputs("Colon\t:\n", file_ptr_w);
                    addToken(Colon, ":");
                    ungetc(c, file_ptr_r);
                }
                break;
            case NEQ:
                if (c == '=') {
                    fputs("NotEqual\t!=\n", file_ptr_w);
                    addToken(NotEqual, "!=");
                } else {
                    state = ERRORs;
                    addToken(ERROR, "Error.");
                }
                break;
            case MULT:
                if (c == '*') {
                    fputs("Raise\t**\n", file_ptr_w);
                    addToken(Raise, "**");
                } else {
                    fputs("Multiply\t*\n", file_ptr_w);
                    addToken(Multiply, "*");
                    ungetc(c, file_ptr_r);
                }
                break;
            default: // ERROR STATE
                state = ERRORs;
                break;
        }
    }
    fputs("EndOfFile\n", file_ptr_w);
}

// enum Token getToken() {
//     // enum Token out = tokenList[tokenIdx]; // out is set to tokenList at tokenptr
//     tokenIdx++; // increment token ptr
//     // return out;
// }
