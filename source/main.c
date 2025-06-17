#include "pch.h"

#include "error.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

#include "core.h"


#define BOH_OUTPUT_COLOR_VALUE          BOH_OUTPUT_COLOR_WHITE
#define BOH_OUTPUT_COLOR_OPERATOR       BOH_OUTPUT_COLOR_GREEN
#define BOH_OUTPUT_COLOR_OPERATOR_EXPR  BOH_OUTPUT_COLOR_YELLOW
#define BOH_OUTPUT_COLOR_STMT           BOH_OUTPUT_COLOR_BLUE

#define BOH_OUTPUT_COLOR_ERROR          BOH_OUTPUT_COLOR_RED


static void PrintExpr(const bohExpr* pExpr, uint64_t offsetLen);

// Returns last printed stmt
static void PrintAstStmt(const bohStmt* pStmt, uint64_t offsetLen);


static void PrintOffset(FILE* pStream, uint64_t offsetLen)
{
    for (uint64_t i = 0; i < offsetLen; ++i) {
        fputc(' ', pStream);
    }

    if (offsetLen > 0) {
        fflush(pStream);
    }
}


static void PrintEscapedCString(FILE* pStream, const char* pString, size_t strLen)
{
    BOH_ASSERT(pStream);
    BOH_ASSERT(pString);

    for (size_t i = 0; i < strLen; ++i) {
        if (pString[i] == '\n') {
            fputc('\\', pStream);
            fputc('n', pStream);
        } else if (pString[i] == '\t') {
            fputc('\\', pStream);
            fputc('t', pStream);
        } else if (pString[i] == '\"') {
            fputc('\\', pStream);
            fputc('\"', pStream);
        } else if (pString[i] == '\'') {
            fputc('\\', pStream);
            fputc('\'', pStream);
        } else if (pString[i] == '\r') {
            fputc('\\', pStream);
            fputc('r', pStream);
        } else {
            fputc(pString[i], pStream);
        }
    }

    if (strLen > 0) {
        fflush(pStream);
    }
}


static void PrintToken(const bohToken* pToken)
{
    BOH_ASSERT(pToken);

    const bohStringView* pLexeme = bohTokenGetLexeme(pToken);

    fprintf_s(stdout, "(%s%s%s, ", BOH_OUTPUT_COLOR_YELLOW, bohTokenGetTypeStr(pToken), BOH_OUTPUT_COLOR_RESET);
    fprintf_s(stdout, BOH_OUTPUT_COLOR_GREEN);
    
    const bool isTokenString = bohTokenGetType(pToken) == BOH_TOKEN_TYPE_STRING;

    if (isTokenString) {
        fputc('\"', stdout);
    }
    
    PrintEscapedCString(stdout, bohStringViewGetData(pLexeme), bohStringViewGetSize(pLexeme));
    
    if (isTokenString) {
        fputc('\"', stdout);
    }

    fprintf_s(stdout, BOH_OUTPUT_COLOR_RESET);
    fprintf_s(stdout, ", %u, %u)\n", pToken->line, pToken->column);
}


static void PrintTokens(const bohTokenStorage* pTokens)
{
    BOH_ASSERT(pTokens);

    const size_t tokensCount = bohDynArrayGetSize(pTokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        PrintToken(BOH_DYN_ARRAY_AT_CONST(bohToken, pTokens, i));
    }
}


static void PrintValueExpr(const bohValueExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    switch (pExpr->type) {
        case BOH_VALUE_EXPR_TYPE_NUMBER:
            if (bohNumberIsI64(&pExpr->number)) {
                fprintf_s(stdout, "%sI64[%d]%s", BOH_OUTPUT_COLOR_VALUE, bohNumberGetI64(&pExpr->number), BOH_OUTPUT_COLOR_RESET);
            } else {
                fprintf_s(stdout, "%sF64[%f]%s", BOH_OUTPUT_COLOR_VALUE, bohNumberGetF64(&pExpr->number), BOH_OUTPUT_COLOR_RESET);
            }
            break;
        case BOH_VALUE_EXPR_TYPE_STRING:
            if (bohBoharesStringIsString(&pExpr->string)) {
                const bohString* pString = bohBoharesStringGetString(&pExpr->string);
                fprintf_s(stdout, "%sStr[\"", BOH_OUTPUT_COLOR_VALUE);
                PrintEscapedCString(stdout, bohStringGetCStr(pString), bohStringGetSize(pString));
                fprintf_s(stdout, "\"]%s", BOH_OUTPUT_COLOR_RESET);
            } else {
                const bohStringView* pStrView = bohBoharesStringGetStringView(&pExpr->string);

                fprintf_s(stdout, "%sStrView[\"", BOH_OUTPUT_COLOR_VALUE);
                PrintEscapedCString(stdout, bohStringViewGetData(pStrView), bohStringViewGetSize(pStrView));
                fprintf_s(stdout, "\"]%s", BOH_OUTPUT_COLOR_RESET);
            }
            break;
        default:
            BOH_ASSERT(false && "Invalid value expression type");
            break;
    }
}


static void PrintUnaryExpr(const bohUnaryExpr* pUnaryExpr, uint64_t offsetLen)
{
    BOH_ASSERT(pUnaryExpr);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;
    const bohExpr* pOperandExpr = pUnaryExpr->pExpr;

    const bool isOperandValueExpr = bohExprIsValueExpr(pOperandExpr);
    const bool isOperandNumber = isOperandValueExpr && bohValueExprIsNumber(bohExprGetValueExpr(pOperandExpr));

    fprintf_s(stdout, "%sUnOp%s(", BOH_OUTPUT_COLOR_OPERATOR_EXPR, BOH_OUTPUT_COLOR_RESET);
    fprintf_s(stdout, "%s%s%s", BOH_OUTPUT_COLOR_OPERATOR, bohParsExprOperatorToStr(pUnaryExpr->op), BOH_OUTPUT_COLOR_RESET);
    fputs(isOperandNumber ? ", " : ",\n", stdout);

    if (!isOperandNumber) {
        PrintOffset(stdout, nextlevelOffsetLen);
    }

    PrintExpr(pOperandExpr, nextlevelOffsetLen);
            
    if (!isOperandNumber) {
        fputc('\n', stdout);
        PrintOffset(stdout, offsetLen);
    }
    fputc(')', stdout);
}


static void PrintBinaryExpr(const bohBinaryExpr* pBinaryExpr, uint64_t offsetLen)
{
    BOH_ASSERT(pBinaryExpr);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;

    const bohExpr* pLeftExpr = pBinaryExpr->pLeftExpr;
    const bool isLeftOperandValueExpr = bohExprIsValueExpr(pLeftExpr);
    const bool isLeftOperandNumber = isLeftOperandValueExpr && bohValueExprIsNumber(bohExprGetValueExpr(pLeftExpr));

    const bohExpr* pRightExpr = pBinaryExpr->pRightExpr;
    const bool isRightOperandValueExpr = bohExprIsValueExpr(pRightExpr);
    const bool isRightOperandNumber = isRightOperandValueExpr && bohValueExprIsNumber(bohExprGetValueExpr(pRightExpr));

    const bool areLeftAndRightNodesNumbers = isLeftOperandNumber && isRightOperandNumber;
    
    fprintf_s(stdout, "%sBinOp%s(", BOH_OUTPUT_COLOR_OPERATOR_EXPR, BOH_OUTPUT_COLOR_RESET);
    fprintf_s(stdout, "%s%s%s", BOH_OUTPUT_COLOR_OPERATOR, bohParsExprOperatorToStr(pBinaryExpr->op), BOH_OUTPUT_COLOR_RESET);
    fputs(areLeftAndRightNodesNumbers ? ", " : ",\n", stdout);

    if (!areLeftAndRightNodesNumbers) {
        PrintOffset(stdout, nextlevelOffsetLen);
    }

    PrintExpr(pLeftExpr, nextlevelOffsetLen);
            
    fputs(areLeftAndRightNodesNumbers ? ", " : ",\n", stdout);

    if (!areLeftAndRightNodesNumbers) {
        PrintOffset(stdout, nextlevelOffsetLen);
    }

    PrintExpr(pRightExpr, nextlevelOffsetLen);
            
    if (!areLeftAndRightNodesNumbers) {
        fputc('\n', stdout);
        PrintOffset(stdout, offsetLen);
    }
    fputc(')', stdout);
}


static void PrintExpr(const bohExpr* pExpr, uint64_t offsetLen)
{
    BOH_ASSERT(pExpr);

    switch (pExpr->type) {
        case BOH_EXPR_TYPE_VALUE:
            PrintValueExpr(bohExprGetValueExpr(pExpr));
            break;
        case BOH_EXPR_TYPE_UNARY:
            PrintUnaryExpr(bohExprGetUnaryExpr(pExpr), offsetLen);
            break;
        case BOH_EXPR_TYPE_BINARY:
            PrintBinaryExpr(bohExprGetBinaryExpr(pExpr), offsetLen);
            break;
        default:
            BOH_ASSERT(false && "Invalid AST node type");
            break;
    }
}


static void PrintAstRawExprStmt(const bohRawExprStmt* pRawExprStmt, uint64_t offsetLen)
{
    BOH_ASSERT(pRawExprStmt);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;

    fprintf_s(stdout, "%sRawExprStmt%s(\n", BOH_OUTPUT_COLOR_STMT, BOH_OUTPUT_COLOR_RESET);
    PrintOffset(stdout, nextlevelOffsetLen);

    PrintExpr(pRawExprStmt->pExpr, nextlevelOffsetLen);

    fputc('\n', stdout);
    PrintOffset(stdout, offsetLen);
    fputc(')', stdout);
}


static void PrintPrintStmt(const bohPrintStmt* pPrintStmt, uint64_t offsetLen)
{
    BOH_ASSERT(pPrintStmt);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;

    fprintf_s(stdout, "%sPrintStmt%s(\n", BOH_OUTPUT_COLOR_STMT, BOH_OUTPUT_COLOR_RESET);
    PrintOffset(stdout, nextlevelOffsetLen);

    PrintAstStmt(pPrintStmt->pArgStmt, nextlevelOffsetLen);

    fputc('\n', stdout);
    PrintOffset(stdout, offsetLen);
    fputc(')', stdout);
}


static void PrintIfStmt(const bohIfStmt* pIfStmt, uint64_t offsetLen)
{
    BOH_ASSERT(pIfStmt);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;

    fprintf_s(stdout, "%sIfStmt%s(\n", BOH_OUTPUT_COLOR_STMT, BOH_OUTPUT_COLOR_RESET);
    PrintOffset(stdout, nextlevelOffsetLen);
    
    const uint64_t nextlevelOffsetLen2 = nextlevelOffsetLen + 4;

    fprintf_s(stdout, "%scondition:%s\n", BOH_OUTPUT_COLOR_YELLOW, BOH_OUTPUT_COLOR_RESET);
    PrintOffset(stdout, nextlevelOffsetLen2);

    PrintAstStmt(pIfStmt->pCondStmt, nextlevelOffsetLen2);

    fputc('\n', stdout);
    PrintOffset(stdout, nextlevelOffsetLen);

    fprintf_s(stdout, "%sstatements block:%s\n", BOH_OUTPUT_COLOR_YELLOW, BOH_OUTPUT_COLOR_RESET);
    PrintOffset(stdout, nextlevelOffsetLen2);

    const size_t thenStmtCount = bohIfStmtGetThenStmtsCount(pIfStmt);
    for (size_t i = 0; i < thenStmtCount; ++i) {
        const bohStmt* pStmt = bohIfStmtGetThenStmtAt(pIfStmt, i);
        PrintAstStmt(pStmt, nextlevelOffsetLen2);

        if (i + 1 < thenStmtCount) {
            fputc('\n', stdout);
            PrintOffset(stdout, nextlevelOffsetLen2);
        }
    }

    fputc('\n', stdout);
    PrintOffset(stdout, offsetLen);
    fputc(')', stdout);

    if (bohIfStmtGetElseStmtsCount(pIfStmt) > 0) {
        fputc('\n', stdout);
        PrintOffset(stdout, offsetLen);
        
        fprintf_s(stdout, "%sElseStmt%s(\n", BOH_OUTPUT_COLOR_STMT, BOH_OUTPUT_COLOR_RESET);
        PrintOffset(stdout, nextlevelOffsetLen);

        fprintf_s(stdout, "%sstatements block:%s\n", BOH_OUTPUT_COLOR_YELLOW, BOH_OUTPUT_COLOR_RESET);
        PrintOffset(stdout, nextlevelOffsetLen2);

        const size_t elseStmtCount = bohIfStmtGetElseStmtsCount(pIfStmt);
        for (size_t i = 0; i < elseStmtCount; ++i) {
            const bohStmt* pStmt = bohIfStmtGetElseStmtAt(pIfStmt, i);
            PrintAstStmt(pStmt, nextlevelOffsetLen2);

            if (i + 1 < elseStmtCount) {
                fputc('\n', stdout);
                PrintOffset(stdout, nextlevelOffsetLen2);
            }
        }

        fputc('\n', stdout);
        PrintOffset(stdout, offsetLen);
        fputc(')', stdout);
    }
}


// Returns last printed stmt
static void PrintAstStmt(const bohStmt* pStmt, uint64_t offsetLen)
{
    BOH_ASSERT(pStmt);
    
    switch (pStmt->type) {
        case BOH_STMT_TYPE_RAW_EXPR:
            PrintAstRawExprStmt(bohStmtGetRawExpr(pStmt), offsetLen);
            break;
        case BOH_STMT_TYPE_PRINT:
            PrintPrintStmt(bohStmtGetPrint(pStmt), offsetLen);
            break;
        case BOH_STMT_TYPE_IF:
            PrintIfStmt(bohStmtGetIf(pStmt), offsetLen);
            break;
        default:
            BOH_ASSERT(false && "Invalid statement type");
            break;
    }
}


static void PrintAst(const bohAST* pAst)
{
    BOH_ASSERT(pAst);

    const size_t stmtCount = bohAstGetStmtCount(pAst);
    uint64_t offsetLen = 0;

    for (size_t i = 0; i < stmtCount; ++i) {
        const bohStmt* pStmt = bohAstGetStmtByIdx(pAst, i);
        PrintAstStmt(pStmt, offsetLen);
        fputc('\n', stdout);
    }
}


int main(int argc, char* argv[])
{
#define DEBUG_NO_ARGS
#ifndef DEBUG_NO_ARGS
    if (argc != 2) {
        fprintf_s(stderr, BOH_OUTPUT_COLOR_RED, "Invalid command line arguments count: %d\n", argc);
        return EXIT_FAILURE;
    }

    const char* pFilePath = argv[1];
#else
    (void)argc;
    (void)argv;

    const char* pFilePath = "../test/test.boh";
#endif

    bohErrorsStateInit();
    bohErrorsStateSetCurrProcessingFile(bohErrorsStateGet(), bohStringViewCreateConstCStr(pFilePath));

    bohFileContent fileContent = bohReadTextFile(pFilePath);
    switch (bohFileContentGetErrorCode(&fileContent)) {
        case BOH_FILE_CONTENT_ERROR_NULL_FILEPATH:
            fprintf_s(stderr, "%sFilepath is NULL%s\n", BOH_OUTPUT_COLOR_RED, BOH_OUTPUT_COLOR_RESET);
            return EXIT_FAILURE;
        case BOH_FILE_CONTENT_ERROR_OPEN_FAILED:
            fprintf_s(stderr, "%sFailed to open file: %s%s\n", BOH_OUTPUT_COLOR_RED, pFilePath, BOH_OUTPUT_COLOR_RESET);
            return EXIT_FAILURE;
        default:
            break;
    }

    const char* pSourceCode = (const char*)fileContent.pData;
    const size_t sourceCodeSize = fileContent.dataSize;

    fprintf_s(stdout, "%sSOURCE:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);
    if (sourceCodeSize > 0) {
        fprintf_s(stdout, "%.*s\n", sourceCodeSize, pSourceCode);
    }

    bohLexer lexer = bohLexerCreate(pSourceCode, sourceCodeSize);
    bohLexerTokenize(&lexer);

    const bohTokenStorage* pTokens = bohLexerGetTokens(&lexer);

    fprintf_s(stdout, "\n%sLEXER TOKENS:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);
    PrintTokens(pTokens);

    if (bohErrorsStateHasLexerErrorGlobal()) {
        exit(-1);
    }

    bohParser parser = bohParserCreate(pTokens);
    bohParserParse(&parser);

    fprintf_s(stdout, "%s\nAST:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);
    PrintAst(bohParserGetAST(&parser));

    if (bohErrorsStateHasParserErrorGlobal()) {
        exit(-2);
    }

    bohInterpreter interp = bohInterpCreate(bohParserGetAST(&parser));

    fprintf_s(stdout, "\n\n%sINTERPRETER:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);
    bohInterpInterpret(&interp);

    if (bohErrorsStateHasInterpreterErrorGlobal()) {   
        exit(-3);
    }

    bohInterpDestroy(&interp);
    bohParserDestroy(&parser);
    bohLexerDestroy(&lexer);

    bohErrorsStateDestroy();

    return EXIT_SUCCESS;
}
