#include <stdio.h>
#include "parser.h"
#include "scanner.h"
FILE *scanner_read, *scanner_write, *parser_write;
Token currentToken;

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

static void getTypeString() {
    
}

static void match(enum TokenType expectedType) {
    if (currentToken.type == expectedType) {
        advance();
    } else {
        fprintf(parser_write, "Parse Error on line %d: something expected.", currentToken.line);
    }
}

// non-terminal recursive subroutine functions
static void parsePrg() {
    // Blk EOF
    parseBlk();
    match(EndofFile);
}

static void parseBlk() {
    // Stm Blk, for {Identifier, PRINT, IF}
    if (currentToken.type == Identifier || currentToken.type == PRINT || currentToken.type == IF) {
        parseStm();
        parseBlk();
    }
}

static void parseStm() {
    switch (currentToken.type) {
        case Identifier:
            // Identifer := Exp;
            match(Identifier);
            match(Assign);
            parseExp();
            match(Semicolon);

            fputs("Assignment Statement Recognized", parser_write);
            break;
        case PRINT:
            // PRINT ( Arg Argfollow ) ;
            match(PRINT);
            match(LeftParen);
            parseArg();
            parseArgfollow();
            match(RightParen);
            match(Semicolon);

            fputs("Print Statement Recognized", parser_write);
            break;
        case IF:
            // IF Cnd : BLK Iffollow
            match(IF);

            fputs("If Statement Begins", parser_write);

            parseCnd();
            match(Colon);
            parseBlk();
            parseIffollow();

            fputs("If Statement Ends", parser_write);
            break;
        default:
            fputs("Invalid Statement", parser_write);
            break;
    }
}

static void parseArgfollow() {
    // Arg Argfollow
    parseArg();
    parseArgfollow();
}

static void parseArg() {
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

static void parseIffollow() {
    switch (currentToken.type) {
        case ENDIF:
            // ENDIF ;
            match(ENDIF);
            match(Colon);
            break;
        case ELSE:
            // ELSE Blk ENDIF ;
            match(ELSE);
            parseBlk();
            match(ENDIF);
            match(Colon);
            break;
        default:
            fputs("Incomplete if Statement", parser_write);
            break;
    }
}

static void parseExp() {
    // Trm Trmfollow
    parseTrm();
    parseTrmfollow();
}

static void parseTrmfollow() {
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

static void parseTrm() {
    // Fac Facfollow
    parseFac();
    parseFacfollow();
}

static void parseFacfollow() {
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

static void parseFac() {
    // Lit Litfollow
    parseLit();
    parseLitfollow();
}

static void parseLitfollow() {
    if (currentToken.type == Raise) {
        // ** Lit Litfollow
        match(Raise);
        parseLit();
        parseLitfollow();
    }
}

static void parseLit() {
    if (currentToken.type == Minus) {
        // - Val
        match(Minus);
        parseVal();
    } else {
        // Val
        parseVal();
    }
}

static void parseVal() {
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
        default:
            // ( Exp )
            match(LeftParen);
            parseExp();
            match(RightParen);
            break;
    }
}

static void parseCnd() {
    // Exp Rel Exp
    parseExp();
    parseRel();
    parseExp();
}

static void parseRel() {
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
            fputs("Missing relational operator", parser_write);
            break;
    }
}

// read ptr here should be the file of the output file for scanner
void parser(FILE* scan_read, FILE* scan_write, FILE* parse_write) {
    // test, scan parse scan parse
    scanner_read = scan_read;
    scanner_write = scan_write;
    parser_write = parse_write;

    advance();
    parsePrg();
}
