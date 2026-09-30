#include <stdio.h>
#include <stdbool.h>
#include "parser.h"
#include "scanner.h"

FILE *scanner_read, *scanner_write, *parser_write;
Token currentToken;
static bool hasError = false;

// static forward declaration, so can resolve function hierarchy
static void parsePrg();
static void parseBlk();
static void parseStm();
static void parseArgfollow();
static void parseArg();
static void parseIffollow();
static void parseExp();
static void parseTrmfollow();
static void parseTrm();
static void parseFacfollow();
static void parseFac();
static void parseLitfollow();
static void parseLit();
static void parseVal();
static void parseCnd();
static void parseRel();

// advance the scanner and replace currentToken with the next token
static void advance() {
    currentToken = gettoken(scanner_read, scanner_write);
}

// checks if current token matches the expected token, raises an error if not
static void match(enum TokenType expectedType) {
    if (hasError) return;   

    if (currentToken.type == expectedType) {
        advance();
    } else {
        fprintf(parser_write, "Parse Error on line %d: %s Expected.\n", currentToken.line, tokenTypetoString(expectedType));
        hasError = true;
    }
}

// non-terminal recursive subroutine functions

// Checks if current token matches Prg grammar, raises an error if not
static void parsePrg() {
    if (hasError) return;
    // Blk EOF
    parseBlk();
    if (hasError) return;
    if (currentToken.type == EndofFile) { // EOF Check
        return;
    }
    fprintf(parser_write, "Parse Error on line %d: EndofFile expected.\n", currentToken.line);
    hasError = true;
}

// Checks if current token matches Blk grammar, raises an error if not
static void parseBlk() {
    if (hasError) return;
    // Stm Blk, for {Identifier, PRINT, IF}
    if (currentToken.type == Identifier || currentToken.type == PRINT || currentToken.type == IF) {
        parseStm();
        parseBlk();
    }
}

// Checks if current token matches Statement grammar, raises an error if not
static void parseStm() {
    if (hasError) return;
    switch (currentToken.type) {
        case Identifier:
            // Identifer := Exp ;
            match(Identifier);
            match(Assign);
            parseExp();
            match(Semicolon);

            if (!hasError) fputs("Assignment Statement Recognized\n", parser_write);
            break;
        case PRINT:
            // PRINT ( Arg Argfollow ) ;
            match(PRINT);
            match(LeftParen);
            parseArg();
            parseArgfollow();
            match(RightParen);
            match(Semicolon);

            if (!hasError) fputs("Print Statement Recognized\n", parser_write);
            break;
        case IF:
            // IF Cnd : BLK Iffollow
            match(IF);

            fputs("If Statement Begins\n", parser_write);

            parseCnd();
            match(Colon);
            parseBlk();
            parseIffollow();

            if (!hasError) fputs("If Statement Ends\n", parser_write);
            break;
        default:
            fputs("Invalid Statement\n", parser_write);
            hasError = true;
            break;
    }
}

// Checks if current token matches Argfollow grammar, raises an error if not
static void parseArgfollow() {
    if (hasError) return;
    // , Arg Argfollow
    if (currentToken.type == Comma) {
        match(Comma);
        parseArg();
        parseArgfollow();
    }
}

// Checks if current token matches Arg grammar, raises an error if not
static void parseArg() {
    if (hasError) return;
    switch (currentToken.type) {
        case String:
            // String
            match(String);
            break;
        default:
            // Exp
            parseExp();
            break;
    }
}

// Checks if current token matches Iffollow grammar, raises an error if not
static void parseIffollow() {
    if (hasError) return;
    switch (currentToken.type) {
        case ENDIF:
            // ENDIF ;
            match(ENDIF);
            match(Semicolon);
            break;
        case ELSE:
            // ELSE Blk ENDIF ;
            match(ELSE);
            parseBlk();
            match(ENDIF);
            match(Semicolon);
            break;
        default:
            fputs("Incomplete if Statement\n", parser_write);
            hasError = true;
            break;
    }
}

// Checks if current token matches Exp grammar, raises an error if not
static void parseExp() {
    if (hasError) return;
    // Trm Trmfollow
    parseTrm();
    parseTrmfollow();
}

// Checks if current token matches Trmfollow grammar, raises an error if not
static void parseTrmfollow() {
    if (hasError) return;
    switch (currentToken.type) {
        case Plus:
            // + Trm Trmfollow
            match(Plus);
            parseTrm();
            parseTrmfollow();
            break;
        case Minus:
            // - Trm Trmfollow
            match(Minus);
            parseTrm();
            parseTrmfollow();
            break;
        default:
            break;
    }
}

// Checks if current token matches Trm grammar, raises an error if not
static void parseTrm() {
    if (hasError) return;
    // Fac Facfollow
    parseFac();
    parseFacfollow();
}

// Checks if current token matches Facfollow grammar, raises an error if not
static void parseFacfollow() {
    if (hasError) return;
    switch (currentToken.type) {
        case Multiply:
            // * Fac Facfollow
            match(Multiply);
            parseFac();
            parseFacfollow();
            break;
        case Divide:
            // / Fac Facfollow
            match(Divide);
            parseFac();
            parseFacfollow();
            break;
        default:
            break;
    }
}

// Checks if current token matches Fac grammar, raises an error if not
static void parseFac() {
    if (hasError) return;
    // Lit Litfollow
    parseLit();
    parseLitfollow();
}

// Checks if current token matches Litfollow grammar, raises an error if not
static void parseLitfollow() {
    if (hasError) return;
    if (currentToken.type == Raise) {
        // ** Lit Litfollow
        match(Raise);
        parseLit();
        parseLitfollow();
    }
}

// Checks if current token matches Lit grammar, raises an error if not
static void parseLit() {
    if (hasError) return;
    if (currentToken.type == Minus) {
        // - Val
        match(Minus);
        parseVal();
    } else {
        // Val
        parseVal();
    }
}

// Checks if current token matches Val grammar, raises an error if not
static void parseVal() {
    if (hasError) return;
    switch (currentToken.type) {
        case Identifier:
            // Identifier
            match(Identifier);
            break;
        case Number:
            // Number
            match(Number);
            break;
        case SQRT:
            // SQRT ( Exp )
            match(SQRT);
            match(LeftParen);
            parseExp();
            match(RightParen);
            break;
        case LeftParen:
            // ( Exp )
            match(LeftParen);
            parseExp();
            match(RightParen);
            break;
        default:
            fprintf(parser_write, "Parse Error on line %d: value expected.\n", currentToken.line);
            hasError = true;
            break;
    }
}

// Checks if current token matches Cnd grammar, raises an error if not
static void parseCnd() {
    if (hasError) return;
    // Exp Rel Exp
    parseExp();
    parseRel();
    parseExp();
}

// Checks if current token matches Rel grammar, raises an error if not
static void parseRel() {
    if (hasError) return;
    switch (currentToken.type) {
        case LessThan:
            // <
            match(LessThan);
            break;
        case Equal:
            // =
            match(Equal);
            break;
        case GreaterThan:
            // >
            match(GreaterThan);
            break;
        case LTEqual:
            // <=
            match(LTEqual);
            break;
        case NotEqual:
            // !=
            match(NotEqual);
            break;
        case GTEqual:
            // >=
            match(GTEqual);
            break;
        default:
            fputs("Missing relational operator\n", parser_write);
            hasError = true;
            break;
    }
}

// read ptr here should be the file of the output file for scanner
bool parser(FILE* scan_read, FILE* scan_write, FILE* parse_write) {
    scanner_read = scan_read;
    scanner_write = scan_write;
    parser_write = parse_write;

    resetScanner(); // so line and init vars get reset after every file
    hasError = false;

    advance();
    parsePrg();
    return !hasError;
}
