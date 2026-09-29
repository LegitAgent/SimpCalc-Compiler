#include "scanner.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
    MULT
};

static char buffer[4096];
static int bufferIdx;
static enum State state = A;

static void setDefaultStates(void) {
    bufferIdx = 0;
    state = A;
}

static Token makeToken(enum TokenType type, const char *lexeme) {
    Token token;
    if (strlen(lexeme) >= sizeof(token.lexeme)) {
        type = ERROR;
        lexeme = "Exceeded lexeme length (4095 characters).";
    }
    token.type = type;
    strcpy(token.lexeme, lexeme);
    setDefaultStates();
    return token;
}

static bool appendCharacter(int c, Token *outToken) {
    if (bufferIdx >= (int)sizeof(buffer) - 1) {
        *outToken = makeToken(ERROR, "Exceeded statement length.");
        return false;
    }
    buffer[bufferIdx++] = (char)c;
    return true;
}

static Token writeOverflow(FILE *output, Token overflow) {
    fprintf(output, "ERROR\t%s\n", overflow.lexeme);
    return overflow;
}

static Token writeToken(FILE *output, enum TokenType type, const char *name, const char *lexeme) {
    Token token = makeToken(type, lexeme);
    if (token.type == ERROR) {
        fprintf(output, "ERROR\t%s\n", token.lexeme);
    } else if (token.type == EndofFile) {
        fputs("EndofFile\n", output);
    } else {
        fprintf(output, "%s\t%s\n", name, token.lexeme);
    }
    return token;
}

static Token writeScanLine(FILE *output, enum TokenType type, const char *name) {
    buffer[bufferIdx] = '\0';
    return writeToken(output, type, name, buffer);
}

static void checkKeyword(enum TokenType *type, const char **name) {
    buffer[bufferIdx] = '\0';
    if (strcmp(buffer, "PRINT") == 0) {
        *type = PRINT;
        *name = "Print";
    } else if (strcmp(buffer, "IF") == 0) {
        *type = IF;
        *name = "If";
    } else if (strcmp(buffer, "ELSE") == 0) {
        *type = ELSE;
        *name = "Else";
    } else if (strcmp(buffer, "ENDIF") == 0) {
        *type = ENDIF;
        *name = "Endif";
    } else if (strcmp(buffer, "SQRT") == 0) {
        *type = SQRT;
        *name = "Sqrt";
    } else if (strcmp(buffer, "AND") == 0) {
        *type = AND;
        *name = "And";
    } else if (strcmp(buffer, "OR") == 0) {
        *type = OR;
        *name = "Or";
    } else if (strcmp(buffer, "NOT") == 0) {
        *type = NOT;
        *name = "Not";
    }
}

static Token writeIdentifier(FILE *output) {
    enum TokenType type = Identifier;
    const char *name = "Identifier";
    checkKeyword(&type, &name);
    return writeScanLine(output, type, name);
}

Token gettoken(FILE *input, FILE *output) {
    int c;
    Token overflow;
    while ((c = fgetc(input)) != EOF) {
        switch (state) {
            case A:
                if (c == '<') {
                    state = LT;
                } else if (c == '>') {
                    state = GT;
                } else if (c == '/') {
                    state = DIV;
                } else if (c == ':') {
                    state = COL;
                } else if (c == '!') {
                    state = NEQ;
                } else if (c == '*') {
                    state = MULT;
                } else if (c == '(') {
                    return writeToken(output, LeftParen, "LeftParen", "(");
                } else if (c == ')') {
                    return writeToken(output, RightParen, "RightParen", ")");
                } else if (c == '-') {
                    return writeToken(output, Minus, "Minus", "-");
                } else if (c == '+') {
                    return writeToken(output, Plus, "Plus", "+");
                } else if (c == ';') {
                    return writeToken(output, Semicolon, "Semicolon", ";");
                } else if (c == ',') {
                    return writeToken(output, Comma, "Comma", ",");
                } else if (c == '"') {
                    state = STR;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else if (c >= '0' && c <= '9') {
                    state = D;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                    state = I;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else if (!isspace(c)) {
                    return writeToken(output, ERROR, "ERROR", "Invalid character.");
                }
                break;
            case D:
                if (c >= '0' && c <= '9') {
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else if (c == '.') {
                    state = DOT;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else if (c == 'e' || c == 'E') {
                    state = E;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else {
                    ungetc(c, input);
                    return writeScanLine(output, Number, "Number");
                }
                break;
            case DOT:
                if (c >= '0' && c <= '9') {
                    state = F;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else {
                    return writeToken(output, ERROR, "ERROR", "Should be followed by digits.");
                }
                break;
            case F:
                if (c >= '0' && c <= '9') {
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else if (c == 'e' || c == 'E') {
                    state = E;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else {
                    ungetc(c, input);
                    return writeScanLine(output, Number, "Number");
                }
                break;
            case E:
                if (c == '+' || c == '-') {
                    state = SIGN;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else if (c >= '0' && c <= '9') {
                    state = EXP;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else {
                    return writeToken(output, ERROR, "ERROR",
                                      "Should be followed by a sign or digit.");
                }
                break;
            case EXP:
                if (c >= '0' && c <= '9') {
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else {
                    ungetc(c, input);
                    return writeScanLine(output, Number, "Number");
                }
                break;
            case SIGN:
                if (c >= '0' && c <= '9') {
                    state = EXP;
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else {
                    return writeToken(output, ERROR, "ERROR", "Should be followed by digits.");
                }
                break;
            case STR:
                if (c == '"') {
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                    return writeScanLine(output, String, "String");
                } else if (c == '\n') {
                    return writeToken(output, ERROR, "ERROR", "Should end with a quotation mark.");
                } else if (!appendCharacter(c, &overflow)) {
                    return writeOverflow(output, overflow);
                }
                break;
            case LT:
                if (c == '=') {
                    return writeToken(output, LTEqual, "LTEqual", "<=");
                }
                ungetc(c, input);
                return writeToken(output, LessThan, "LessThan", "<");
            case I:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                    c == '_') {
                    if (!appendCharacter(c, &overflow)) {
                        return writeOverflow(output, overflow);
                    }
                } else {
                    ungetc(c, input);
                    return writeIdentifier(output);
                }
                break;
            case COM:
                if (c == '\n') {
                    state = A;
                }
                break;
            case DIV:
                if (c == '/') {
                    state = COM;
                } else {
                    ungetc(c, input);
                    return writeToken(output, Divide, "Divide", "/");
                }
                break;
            case GT:
                if (c == '=') {
                    return writeToken(output, GTEqual, "GTEqual", ">=");
                }
                ungetc(c, input);
                return writeToken(output, GreaterThan, "GreaterThan", ">");
            case COL:
                if (c == '=') {
                    return writeToken(output, Assign, "Assign", ":=");
                }
                ungetc(c, input);
                return writeToken(output, Colon, "Colon", ":");
            case NEQ:
                if (c == '=') {
                    return writeToken(output, NotEqual, "NotEqual", "!=");
                }
                return writeToken(output, ERROR, "ERROR", "Should be followed by an =.");
            case MULT:
                if (c == '*') {
                    return writeToken(output, Raise, "Raise", "**");
                }
                ungetc(c, input);
                return writeToken(output, Multiply, "Multiply", "*");
        }
    }
    switch (state) {
        case D:
        case F:
        case EXP:
            return writeScanLine(output, Number, "Number");
        case I:
            return writeIdentifier(output);
        case LT:
            return writeToken(output, LessThan, "LessThan", "<");
        case GT:
            return writeToken(output, GreaterThan, "GreaterThan", ">");
        case DIV:
            return writeToken(output, Divide, "Divide", "/");
        case COL:
            return writeToken(output, Colon, "Colon", ":");
        case MULT:
            return writeToken(output, Multiply, "Multiply", "*");
        case DOT:
        case E:
        case SIGN:
        case STR:
        case NEQ:
            return writeToken(output, ERROR, "ERROR", "Incomplete token at end of file.");
        case A:
        case COM:
            return writeToken(output, EndofFile, "EndofFile", "");
    }
    return writeToken(output, ERROR, "ERROR", "Unknown scanner state.");
}
