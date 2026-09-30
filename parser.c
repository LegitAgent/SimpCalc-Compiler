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

static void match(enum TokenType expectedType) {
    if (hasError) return;   

    if (currentToken.type == expectedType) {
        advance();
    } else {
        fprintf(parser_write, "Parse Error on line %d: %s expected.\n", currentToken.line, tokenTypetoString(expectedType));
        hasError = true;
    }
}

// non-terminal recursive subroutine functions
static void parsePrg() {
    if (hasError) return;
    // Blk EOF
    parseBlk();
    if (hasError) return;
    if (currentToken.type == EndofFile) {
        return;
    }
    fprintf(parser_write, "Parse Error on line %d: EndofFile expected.\n", currentToken.line);
    hasError = true;
}

static void parseBlk() {
    if (hasError) return;
    // Stm Blk, for {Identifier, PRINT, IF}
    if (currentToken.type == Identifier || currentToken.type == PRINT || currentToken.type == IF) {
        parseStm();
        parseBlk();
    }
}

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

static void parseArgfollow() {
    if (hasError) return;
    // , Arg Argfollow
    if (currentToken.type == Comma) {
        match(Comma);
        parseArg();
        parseArgfollow();
    }
}

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

static void parseExp() {
    if (hasError) return;
    // Trm Trmfollow
    parseTrm();
    parseTrmfollow();
}

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

static void parseTrm() {
    if (hasError) return;
    // Fac Facfollow
    parseFac();
    parseFacfollow();
}

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

static void parseFac() {
    if (hasError) return;
    // Lit Litfollow
    parseLit();
    parseLitfollow();
}

static void parseLitfollow() {
    if (hasError) return;
    if (currentToken.type == Raise) {
        // ** Lit Litfollow
        match(Raise);
        parseLit();
        parseLitfollow();
    }
}

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

static void parseCnd() {
    if (hasError) return;
    // Exp Rel Exp
    parseExp();
    parseRel();
    parseExp();
}

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

    resetScanner();
    hasError = false;

    advance();
    parsePrg();
    return !hasError;
}
