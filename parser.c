#include <stdio.h>
#include "parser.h"
#include "scanner.h"

// read ptr here should be the file of the output file for scanner
void parser(FILE* scanner_read, FILE* scanner_write, FILE* parser_write) {
    // test, scan parse scan parse
    Token currentToken = gettoken(scanner_read, scanner_write);
    // while (currentToken.type != EndofFile) {
    //     currentToken = gettoken(scanner_read, scanner_write);
    // }
}
