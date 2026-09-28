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
};

enum Token tokenList[1000];
int tokenPtr = 0;
// when a thing is scanned, put into tokenList the enum Token.
// i.e. "Chudhalla" gets scanned and recognized as a valid Identifier, we put the Identifier enum into tokenList

void scanner(FILE* file_ptr_r, FILE* file_ptr_w) {
    enum State state = A;
    int c; // cur char
    // character buffer for digits (string)
    char buffer[256]; // so max digit len would be 255, + 1 for null terminator
    int buffer_idx = 0;

    // https://stackoverflow.com/questions/4823177/reading-a-file-character-by-character-in-c
    // iterate over file character by character
    while ((c = fgetc(file_ptr_r)) != EOF) {
        switch(state)
        {
            case A:
                if (c == '<')
                    state = LT;
                else if (c == '>')
                    state = GT;
                else if (c == '/')
                    state = DIV;
                else if (c == ':')
                    state = COL;
                else if (c == '!')
                    state = NEQ;
                else if (c == '*')
                    state = MULT;
                break;
            case D:
                break;
            case DOT:
                break;
            case F:
                break;
            case E:
                break;
            case EXP:
                break;
            case SIGN:
                break;
            case STR:
                break;
            case LT:
                if (c == '=') {
                    // reset state and push to tokenList the token LTEqual
                    state = A;
                    tokenList[tokenPtr] = LTEqual;
                    tokenPtr++;
                }
                else{
                    // reset state and push to tokenList the token LessThan
                    // do push back
                    state = A;
                    tokenList[tokenPtr] = LessThan;
                    tokenPtr++;
                    //[TODO: PUSHBACK IDK HOW YOU IMPLEMENTED THIS THING ALBA]
                }
                break;
            case I:
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
                    tokenList[tokenPtr] = Divide;
                    tokenPtr++;
                    //[TODO: PUSHBACK idk how you implemented this thing alba]
                }
                break;
            case GT:
                if (c == '=') {
                    // reset state and push to tokenList the token GTEqual
                    state = A;
                    tokenList[tokenPtr] = GTEqual;
                    tokenPtr++;
                }
                else {
                    // reset state and push to tokenList the token GreaterThan
                    // do push back
                    state = A;
                    tokenList[tokenPtr] = GreaterThan;
                    tokenPtr++;
                    //[TODO: PUSHBACK idk how you implemented this thing alba]
                }
                break;
            case COL:
                if (c == '=') {
                    // reset state and push to tokenList the token Assign
                    state = A;
                    tokenList[tokenPtr] = Assign;
                    tokenPtr++;
                }
                else {
                    // reset state and push to tokenList the token Colon
                    // do push back
                    state = A;
                    tokenList[tokenPtr] = Colon;
                    //[TODO: PUSHBACK idk how you implemented this thing alba]
                }
                break;
            case NEQ:
                if (c == '=') {
                    // reset state and push to tokenList the token NotEqual
                    state = A;
                    tokenList[tokenPtr] = NotEqual;
                    tokenPtr++;
                }
                else {
                    // set to error state push to tokenList the token ERROR
                    state = ERROR;
                    tokenList[tokenPtr] = ERROR;
                    tokenPtr++;
                }
                break;
            case MULT:
                if (c == '*') {
                    // reset state and push to tokenList the token Raise
                    state = A;
                    tokenList[tokenPtr] = Raise;
                    tokenPtr++;
                }
                else {
                    // reset state and push to tokenList the token Multiply
                    // do pushback
                    state = A;
                    tokenList[tokenPtr] = Multiply;
                    tokenPtr++;
                    //[TODO: PUSHBACK idk how you implemented this thing alba]
                }
                break;
            default: // ERROR STATE
                break;
        }
    }


}

enum Token gettoken() //only call this after finished scanning
{
    enum Token out = tokenList[tokenPtr]; //out is set to tokenList at tokenptr
    tokenPtr++; // increment token ptr
    return out;
}
