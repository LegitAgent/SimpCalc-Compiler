#include <stdio.h>
#include <string.h>
#include <ctype.h>
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

Token tokenList[10000];
int tokenIdx = 0;
static int tokenReadIdx = 0; // for tracking which token to read

// character buffer for stmt (string)
char buffer[4096]; // so max stmt len would be 4095, + 1 for null terminator
int bufferIdx = 0;
enum State state = A; // DEFAULT STATE

void setDefaultStates() {
    bufferIdx = 0;
    state = A;
}

// adds a specific token to the token list, with a token type and token contents
void addToken(enum TokenType type, const char *lexeme) {
    const int tokenCapacity = (int)(sizeof(tokenList) / sizeof(tokenList[0]));
    if (state == ERRORs) return;

    // keep one slot available to report overflow without writing out of bounds
    if (tokenIdx >= tokenCapacity - 1) {
        type = ERROR;
        lexeme = "Exceeded token limit.";
    } else if (strlen(lexeme) >= sizeof(tokenList[0].lexeme)) {
        type = ERROR;
        lexeme = "Exceeded lexeme length (255 characters).";
    }

    tokenList[tokenIdx].type = type;
    strcpy(tokenList[tokenIdx].lexeme, lexeme);
    tokenIdx++;

    setDefaultStates();
    if (type == ERROR) state = ERRORs;
}

static void appendCharacter(int c) {
    if (bufferIdx >= (int)sizeof(buffer) - 1) {
        addToken(ERROR, "Exceeded statement length.");
        return;
    }
    buffer[bufferIdx++] = (char)c;
}

void finishScanLine(FILE* file_ptr_w, enum TokenType type, const char *scanType) {
    buffer[bufferIdx] = '\0';
    addToken(type, buffer);
    if (state != ERRORs) fprintf(file_ptr_w, "%s\t%s\n", scanType, buffer);
}

void scanner(FILE* file_ptr_r, FILE* file_ptr_w) {

    int c; // cur char
    tokenIdx = 0;
    tokenReadIdx = 0;
    setDefaultStates();
    // https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    // iterate over file character by character

    while (state != ERRORs && (c = fgetc(file_ptr_r)) != EOF) {
        switch(state) {
            case A:
                if (c == '<') state = LT;
                else if (c == '>') state = GT;
                else if (c == '/') state = DIV;
                else if (c == ':') state = COL;
                else if (c == '!') state = NEQ;
                else if (c == '*') state = MULT;
                else if (c == '(') {
                    addToken(LeftParen, "(");
                    if (state != ERRORs) fputs("LeftParen\t(\n", file_ptr_w);
                } else if (c == ')') {
                    addToken(RightParen, ")");
                    if (state != ERRORs) fputs("RightParen\t)\n", file_ptr_w);
                } else if (c == '-') {
                    addToken(Minus, "-");
                    if (state != ERRORs) fputs("Minus\t-\n", file_ptr_w);
                } else if (c == '+') {
                    addToken(Plus, "+");
                    if (state != ERRORs) fputs("Plus\t+\n", file_ptr_w);
                } else if (c == ';') {
                    addToken(Semicolon, ";");
                    if (state != ERRORs) fputs("Semicolon\t;\n", file_ptr_w);
                } else if (c == ',') {
                    addToken(Comma, ",");
                    if (state != ERRORs) fputs("Comma\t,\n", file_ptr_w);
                } else if (c == '"') {
                    state = STR;
                    appendCharacter(c);
                } else if (c >= '0' && c <= '9') {
                    state = D;
                    appendCharacter(c);
                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                    state = I;
                    appendCharacter(c);
                } else if (isspace(c)) {
                    // https://www.geeksforgeeks.org/c/isspace-in-c/
                    // whitespace separates tokens, stuff like tabs, new lines as well
                } else {
                    addToken(ERROR, "Invalid character.");
                }
                break;
            case D:
                if (c >= '0' && c <= '9') {
                    state = D;

                    appendCharacter(c);
                } else if (c == '.') {
                    state = DOT;

                    appendCharacter(c);
                } else if (c == 'e' || c == 'E') {
                    state = E;

                    appendCharacter(c);
                } else {
                    finishScanLine(file_ptr_w, Number, "Number");

                    ungetc(c, file_ptr_r);
                }
                break;
            case DOT:
                if (c >= '0' && c <= '9') {
                    state = F;
                    appendCharacter(c);
                } else {
                    addToken(ERROR, "Should be followed by digits.");
                }
                break;
            case F:
                if (c >= '0' && c <= '9') {
                    state = F;
                    appendCharacter(c);
                } else if (c == 'e' || c == 'E') {
                    state = E;
                    appendCharacter(c);
                } else {
                    finishScanLine(file_ptr_w, Number, "Number");

                    ungetc(c, file_ptr_r);
                }
                break;
            case E:
                if (c == '+' || c == '-') {
                    state = SIGN;
                    appendCharacter(c);
                } else if (c >= '0' && c <= '9') {
                    state = EXP;
                    appendCharacter(c);
                } else {
                    addToken(ERROR, "Should followed by a + or -, or digits.");
                }
                break;
            case EXP:
                if (c >= '0' && c <= '9') {
                    state = EXP;
                    appendCharacter(c);
                } else {
                    finishScanLine(file_ptr_w, Number, "Number");

                    ungetc(c, file_ptr_r);
                }
                break;
            case SIGN:
                if (c >= '0' && c <= '9') {
                    state = EXP;
                    appendCharacter(c);
                } else {
                    addToken(ERROR, "Should be followed by digits.");
                }
                break;
            case STR:
                if (c == '"') {
                    appendCharacter(c);
                    if (state == ERRORs) break;
                    finishScanLine(file_ptr_w, String, "String");
                } else if (c == '\n') {
                    addToken(ERROR, "Should end with a quoatation mark.");
                } else {
                    state = STR;
                    appendCharacter(c);
                }
                break;
            case LT:
                // this fails in the case of <=h, should be not valid, but with this, it will still recognize LTE
                if (c == '=') {
                    // reset state and push to tokenList the token LTEqual
                    addToken(LTEqual, "<=");
                    if (state != ERRORs) fputs("LTEqual\t<=\n", file_ptr_w);
                } else {
                    // reset state and push to tokenList the token LessThan
                    // do push back
                    addToken(LessThan, "<");
                    if (state != ERRORs) fputs("LessThan\t<\n", file_ptr_w);
                    ungetc(c, file_ptr_r);
                }
                break;
            case I:
                if ((c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_') {
                    state = I;
                    appendCharacter(c);
                } else {
                    enum TokenType type = Identifier;
                    char *scanType = "Identifier";
                    if (strcmp(buffer, "PRINT") == 0) {
                        type = PRINT;
                        scanType = "Print";
                    } else if (strcmp(buffer, "IF") == 0) {
                        type = IF;
                        scanType = "If";
                    } else if (strcmp(buffer, "ELSE") == 0) {
                        type = ELSE;
                        scanType = "Else";
                    } else if (strcmp(buffer, "ENDIF") == 0) {
                        type = ENDIF;
                        scanType = "Endif";
                    } else if (strcmp(buffer, "SQRT") == 0) {
                        type = SQRT;
                        scanType = "Sqrt";
                    } else if (strcmp(buffer, "AND") == 0) {
                        type = AND;
                        scanType = "And";
                    } else if (strcmp(buffer, "OR") == 0) {
                        type = OR;
                        scanType = "Or";
                    } else if (strcmp(buffer, "NOT") == 0) {
                        type = NOT;
                        scanType = "Not";
                    }
                    finishScanLine(file_ptr_w, type, scanType);

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
                    addToken(Divide, "/");
                    if (state != ERRORs) fputs("Divide\t/\n", file_ptr_w);
                    ungetc(c, file_ptr_r);
                }
                break;
            case GT:
                if (c == '=') {
                    // reset state and push to tokenList the token GTEqual
                    addToken(GTEqual, ">=");
                    if (state != ERRORs) fputs("GTEqual\t>=\n", file_ptr_w);
                } else {
                    addToken(GreaterThan, ">");
                    if (state != ERRORs) fputs("GreaterThan\t>\n", file_ptr_w);
                    ungetc(c, file_ptr_r);
                }
                break;
            case COL:
                if (c == '=') {
                    addToken(Assign, ":=");
                    if (state != ERRORs) fputs("Assign\t:=\n", file_ptr_w);
                } else {
                    addToken(Colon, ":");
                    if (state != ERRORs) fputs("Colon\t:\n", file_ptr_w);
                    ungetc(c, file_ptr_r);
                }
                break;
            case NEQ:
                if (c == '=') {
                    addToken(NotEqual, "!=");
                    if (state != ERRORs) fputs("NotEqual\t!=\n", file_ptr_w);
                } else {
                    addToken(ERROR, "Should be followed by an =.");
                }
                break;
            case MULT:
                if (c == '*') {
                    addToken(Raise, "**");
                    if (state != ERRORs) fputs("Raise\t**\n", file_ptr_w);
                } else {
                    addToken(Multiply, "*");
                    if (state != ERRORs) fputs("Multiply\t*\n", file_ptr_w);
                    ungetc(c, file_ptr_r);
                }
                break;
            default: // ERROR STATE
                addToken(ERROR, "Unknown case, error.");
                break;
        }
    }

    // finish tokens that have no trailing delimiter, i.e. EOF while incomplete
    switch (state) {
        case D:
        case F:
        case EXP:
            finishScanLine(file_ptr_w, Number, "Number");
            break;
        case I:
            finishScanLine(file_ptr_w, Identifier, "Identifier");
            break;
        case LT:
            addToken(LessThan, "<");
            if (state != ERRORs) fputs("LessThan\t<\n", file_ptr_w);
            break;
        case GT:
            addToken(GreaterThan, ">");
            if (state != ERRORs) fputs("GreaterThan\t>\n", file_ptr_w);
            break;
        case DIV:
            addToken(Divide, "/");
            if (state != ERRORs) fputs("Divide\t/\n", file_ptr_w);
            break;
        case COL:
            addToken(Colon, ":");
            if (state != ERRORs) fputs("Colon\t:\n", file_ptr_w);
            break;
        case MULT:
            addToken(Multiply, "*");
            if (state != ERRORs) fputs("Multiply\t*\n", file_ptr_w);
            break;
        case DOT:
        case E:
        case SIGN:
        case STR:
        case NEQ:
            addToken(ERROR, "Incomplete token at end of file.");
            break;
        default:
            break;
    }
    if (state == ERRORs) {
        fprintf(file_ptr_w, "ERROR\t%s\n", tokenList[tokenIdx - 1].lexeme);
    }
    fputs("EndOfFile\n", file_ptr_w);
}

// gets a token using a static token reading variable
Token getToken() {
    if (tokenReadIdx >= tokenIdx) {
        Token eof = {EndofFile, "End of File."};
        return eof;
    }

    return tokenList[tokenReadIdx++];
}
