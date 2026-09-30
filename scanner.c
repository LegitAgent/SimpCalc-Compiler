#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"

// DFA states
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
};

// character buffer for stmt (string)
static char buffer[4096]; // so max stmt len would be 4095, + 1 for null terminator
static int bufferIdx;
static enum State state = A; // DEFAULT STATE
static int lineCount = 1;

// sets statement buffer and state to default values
static void setDefaultStates(void) {
    bufferIdx = 0; 
    state = A;
}

// resets scanner vars
void resetScanner(void) {
    setDefaultStates();
    lineCount = 1;
}

// gets tokentype and converts it to string
const char *tokenTypetoString(enum TokenType type) {
    switch (type) {
        case Identifier:   return "Identifier";
        case Number:       return "Number";
        case String:       return "String";
        case Assign:       return "Assign";
        case Semicolon:    return "Semicolon";
        case Colon:        return "Colon";
        case Comma:        return "Comma";
        case LeftParen:    return "LeftParen";
        case RightParen:   return "RightParen";
        case Plus:         return "Plus";
        case Minus:        return "Minus";
        case Multiply:     return "Multiply";
        case Divide:       return "Divide";
        case Raise:        return "Raise";
        case LessThan:     return "LessThan";
        case Equal:        return "Equal";
        case GreaterThan:  return "GreaterThan";
        case LTEqual:      return "LTEqual";
        case GTEqual:      return "GTEqual";
        case NotEqual:     return "NotEqual";
        case ERROR:        return "ERROR";
        case EndofFile:    return "EndofFile";

        // Unique Identifier Tokens
        case PRINT:        return "PRINT";
        case IF:           return "IF";
        case ELSE:         return "ELSE";
        case ENDIF:        return "ENDIF";
        case SQRT:         return "SQRT";
        case AND:          return "AND";
        case OR:           return "OR";
        case NOT:          return "NOT";

        default:           return "Unknown";
    }
}

// makes a token according to the type and lexeme contents
static Token makeToken(enum TokenType type, const char *lexeme) {
    Token token; 

    // checks first if the maximum lexeme length is exceeded
    if (strlen(lexeme) >= sizeof(token.lexeme)) { 
        type = ERROR; // if lexeme is too long, make it ERROR
        lexeme = "Exceeded lexeme length (4095 characters).";
    }

    token.type = type;
    strcpy(token.lexeme, lexeme); // copies the lexeme string to the lexeme token array
    token.line = lineCount;
    setDefaultStates();
    return token;
}

// appends a character while checking for buffer overflow
static bool appendCharacter(int c, Token *outToken) {
    // to make space for the character
    if (bufferIdx >= (int)sizeof(buffer) - 1) {
        *outToken = makeToken(ERROR, "Exceeded statement length.");
        return false;
    }
    buffer[bufferIdx++] = (char)c;
    return true;
}

// writes the error overflow message, in case stmt len goes over 4095
static Token writeOverflow(FILE *output, Token overflow) {
    fprintf(output, "ERROR\t%s\n", overflow.lexeme);
    return overflow;
}

// creates a token and writes its formatted output
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

// null-terminates the buffer before creating a token from it
static Token writeScanLine(FILE *output, enum TokenType type, const char *name) {
    buffer[bufferIdx] = '\0';
    return writeToken(output, type, name, buffer);
}

// checks a specific key word in the reserved key word table
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

// checks whether the buffered identifier is a keyword
static Token writeIdentifier(FILE *output) {
    enum TokenType type = Identifier;
    const char *name = "Identifier";
    checkKeyword(&type, &name);
    return writeScanLine(output, type, name);
}

// gets a specific token from the input file
Token gettoken(FILE *input, FILE *output) {
    int c; // cur char
    Token overflow;
    // https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    // iterate over file character by character
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
                } else if (c == '=') {
                    return writeToken(output, Equal, "Equal", "=");
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
                } else if (isspace(c)) {
                    if (c == '\n') lineCount++;
                    // https://www.geeksforgeeks.org/c/isspace-in-c/
                    // whitespace separates tokens, stuff like tabs, new lines as well
                } else {
                    return writeToken(output, ERROR, "ERROR", "Invalid character.");
                }
                break;
            case D:
                // continue reading digits, or transition to decimal/exponent form
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
                // decimal point must be followed by at least one digit
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
                // exponent must be followed by a sign or digit
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
                // exponent sign must be followed by a digit
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
                    // reset state and write to buffer the token LTEqual
                    return writeToken(output, LTEqual, "LTEqual", "<=");
                }
                
                // do push back
                // reset state and write to buffer token LessThan
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
                    setDefaultStates();
                    lineCount++;
                }
                break;
            case DIV:
                if (c == '/') {
                    state = COM;
                } else {
                    // do push back
                    // reset state and write to output the token Divide
                    ungetc(c, input);
                    return writeToken(output, Divide, "Divide", "/");
                }
                break;
            case GT:
                if (c == '=') {
                    // reset state and write to output the token GTEqual
                    return writeToken(output, GTEqual, "GTEqual", ">=");
                }
                // do push back
                // reset state and write to output the token GreaterThan
                ungetc(c, input);
                return writeToken(output, GreaterThan, "GreaterThan", ">");
            case COL:
                if (c == '=') {
                    // reset state and write to output the token Assign
                    return writeToken(output, Assign, "Assign", ":=");
                }
                // do push back
                // reset state and write to output the token Colon
                ungetc(c, input);
                return writeToken(output, Colon, "Colon", ":");
            case NEQ:
                if (c == '=') {
                    // reset state and write to output the token NotEqual
                    return writeToken(output, NotEqual, "NotEqual", "!=");
                }
                // reset state and output error
                return writeToken(output, ERROR, "ERROR", "Should be followed by an =.");
            case MULT:
                if (c == '*') {
                    // reset state and write to output the token Raise
                    return writeToken(output, Raise, "Raise", "**");
                }
                // do push back
                // reset state and write to output the token Multiply
                ungetc(c, input);
                return writeToken(output, Multiply, "Multiply", "*");
        }
    }
    // finish tokens that have no trailing delimiter, i.e. EOF while incomplete
    switch (state) {
        case D:
            return writeToken(output, Number, "Number", buffer);
        case F:
            return writeToken(output, Number, "Number", buffer);
        case EXP:
            return writeToken(output, Number, "Number", buffer);
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
            return writeToken(output, ERROR, "ERROR", "Incomplete token at end of file.");
        case E:
            return writeToken(output, ERROR, "ERROR", "Incomplete token at end of file.");
        case SIGN:
            return writeToken(output, ERROR, "ERROR", "Incomplete token at end of file.");
        case STR:
            return writeToken(output, ERROR, "ERROR", "Incomplete token at end of file.");
        case NEQ:
            return writeToken(output, ERROR, "ERROR", "Incomplete token at end of file.");
        case A:
        case COM:
            return writeToken(output, EndofFile, "EndofFile", "");
    }
    return writeToken(output, ERROR, "ERROR", "Unknown scanner state.");
}
