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
        
    }


}

enum Token gettoken() //only call this after finished scanning
{
    enum Token out = tokenList[tokenPtr]; //out is set to tokenList at tokenptr
    tokenPtr++; // increment token ptr
    return out;
}
