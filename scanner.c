#include <stdio.h>
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

enum Token tokenList[1000];
int tokenIdx = 0;
// when a thing is scanned, put into tokenList the enum Token.
// i.e. "Chudhalla" gets scanned and recognized as a valid Identifier, we put the Identifier enum into tokenList

void scanner(FILE* file_ptr_r, FILE* file_ptr_w) {
    enum State state = A;
    int c; // cur char
    // character buffer for stmt (string)
    char buffer[256]; // so max stmt len would be 255, + 1 for null terminator
    int buffer_idx = 0;

    // https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    // iterate over file character by character
    while ((c = fgetc(file_ptr_r)) != EOF) {
        if (state == ERRORs) {
            printf("Error.");
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
                else if (c == '"') state = STR;
                else if (c >= '0' && c <= '9') state = D;
                else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                    state = I;
                    buffer[buffer_idx++] = c;
                }
                break;
            case D:
                if (c >= '0' && c <= '9')
                    state = D;
                else if (c == '.')
                    state = DOT;
                else if (c == 'e' || c == 'E')
                    state = E;
                else {
                    state = A;
                    tokenList[tokenIdx] = Number;
                    tokenIdx++;
                    ungetc(c, file_ptr_r);
                }
                break;
            case DOT:
                if (c >= '0' && c <= '9')
                    state = F;
                else {
                    state = ERRORs;
                    tokenList[tokenIdx] = ERROR;
                    tokenIdx++;;
                }
                break;
            case F:
                if (c >= '0' && c <= '9')
                    state = F;
                else if (c == 'e' || c == 'E')
                    state = E;
                else {
                    state = A;
                    tokenList[tokenIdx] = Number;
                    tokenIdx++;
                    ungetc(c, file_ptr_r);
                }
                break;
            case E:
                if (c == '+' || c == '-')
                    state = SIGN;
                else if (c >= '0' && c <= '9')
                    state = EXP;
                else {
                    state = ERRORs;
                    tokenList[tokenIdx] = ERROR;
                    tokenIdx++;
                }
                break;
            case EXP:
                if (c >= '0' && c <= '9')
                    state = EXP;
                else {
                    state = A;
                    tokenList[tokenIdx] = Number;
                    tokenIdx++;
                    ungetc(c, file_ptr_r);
                }
                break;
            case SIGN:
                if (c >= '0' && c <= '9') 
                    state = EXP;
                else {
                    state = ERRORs;
                    tokenList[tokenIdx] = ERROR;
                    tokenIdx++;
                }
                break;
            case STR:
                if (c == '"') {
                    state = A;
                    tokenList[tokenIdx] = String;
                    tokenIdx++;
                }
                else if (c == '\n') {
                    state = ERRORs;
                    tokenList[tokenIdx] = ERROR;
                }
                else {
                    state = STR;
                }
                break;
            case LT:
                // this fails in the case of <=h, should be not valid, but with this, it will still recognize LTE
                if (c == '=') {
                    // reset state and push to tokenList the token LTEqual
                    fputs("LessThan\t<=\n", file_ptr_w);
                    state = A;
                    tokenList[tokenIdx] = LTEqual;
                    tokenIdx++;
                } else {
                    // reset state and push to tokenList the token LessThan
                    // do push back
                    fputs("LessThan\t<\n", file_ptr_w);
                    state = A;
                    tokenList[tokenIdx] = LessThan;
                    tokenIdx++;
                    ungetc(c, file_ptr_r);
                }
                break;
            case I:
                if ((c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_') {
                    state = I;
                    buffer[buffer_idx++] = c;
                }
                else {
                    state = A;
                    // text put
                    buffer[buffer_idx] = '\0';
                    fprintf(file_ptr_w, "Identifier\t%s\n", buffer);
                    buffer_idx = 0;
                    
                    // parse token
                    tokenList[tokenIdx++] = Identifier;

                    // pushback
                    ungetc(c, file_ptr_r);
                }
                break;
            case COM:
                if (c == '\n')
                    state = A; // reset state if newline
                break;
            case DIV:
                if (c == '/')
                    // set state to COM
                    state = COM;
                else {
                    // reset state and push to tokenList the token Divide
                    // do push back
                    state = A;
                    tokenList[tokenIdx] = Divide;
                    tokenIdx++;
                    ungetc(c, file_ptr_r);
                }
                break;
            case GT:
                if (c == '=') {
                    // reset state and push to tokenList the token GTEqual
                    state = A;
                    tokenList[tokenIdx] = GTEqual;
                    tokenIdx++;
                }
                else {
                    // reset state and push to tokenList the token GreaterThan
                    // do push back
                    state = A;
                    tokenList[tokenIdx] = GreaterThan;
                    tokenIdx++;
                    ungetc(c, file_ptr_r);
                }
                break;
            case COL:
                if (c == '=') {
                    // reset state and push to tokenList the token Assign
                    state = A;
                    tokenList[tokenIdx] = Assign;
                    tokenIdx++;
                }
                else {
                    // reset state and push to tokenList the token Colon
                    // do push back
                    state = A;
                    tokenList[tokenIdx] = Colon;
                    ungetc(c, file_ptr_r);
                }
                break;
            case NEQ:
                if (c == '=') {
                    // reset state and push to tokenList the token NotEqual
                    state = A;
                    tokenList[tokenIdx] = NotEqual;
                    tokenIdx++;
                }
                else {
                    // set to error state push to tokenList the token ERROR
                    state = ERRORs;
                    tokenList[tokenIdx] = ERROR;
                    tokenIdx++;
                }
                break;
            case MULT:
                if (c == '*') {
                    // reset state and push to tokenList the token Raise
                    state = A;
                    tokenList[tokenIdx] = Raise;
                    tokenIdx++;
                }
                else {
                    // reset state and push to tokenList the token Multiply
                    // do pushback
                    state = A;
                    tokenList[tokenIdx] = Multiply;
                    tokenIdx++;
                    ungetc(c, file_ptr_r);
                }
                break;
            default: // ERROR STATE
                state = ERRORs;
                break;
        }
    }


}

enum Token gettoken() //only call this after finished scanning
{
    enum Token out = tokenList[tokenIdx]; // out is set to tokenList at tokenptr
    tokenIdx++; // increment token ptr
    return out;
}
