#include "pch.h"

#include "state.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"


static void PrintExpr(const bohAST* pAst, bohExprIdx exprIdx, uint64_t offsetLen);


#define BOH_OUTPUT_COLOR_VALUE          BOH_OUTPUT_COLOR_WHITE
#define BOH_OUTPUT_COLOR_OPERATOR       BOH_OUTPUT_COLOR_GREEN
#define BOH_OUTPUT_COLOR_OPERATOR_EXPR  BOH_OUTPUT_COLOR_YELLOW

#define BOH_OUTPUT_COLOR_ERROR          BOH_OUTPUT_COLOR_RED


static void PrintLexerError(const bohLexerError* pError)
{
    BOH_ASSERT(pError);

    const bohStringView* pFilepath = &pError->filepath;

    fprintf_s(stderr, "[LEXER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(pFilepath), bohStringViewGetData(pFilepath), pError->line, pError->column);
    
    fprintf_s(stderr, "%s%s%s\n", BOH_OUTPUT_COLOR_ERROR, bohStringGetCStr(&pError->message), BOH_OUTPUT_COLOR_RESET);
}


static void PrintLexerErrors(const bohState* pState)
{
    BOH_ASSERT(pState);

    const size_t lexerErrorsCount = bohStateGetLexerErrorsCount(pState);
        
    for (size_t i = 0; i < lexerErrorsCount; ++i) {
        const bohLexerError* pError = bohStateLexerErrorAt(pState, i);
        PrintLexerError(pError);
    }
}


static void PrintParserError(const bohParserError* pError)
{
    BOH_ASSERT(pError);

    const bohStringView* pFilepath = &pError->filepath;

    fprintf_s(stderr, "[PARSER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(pFilepath), bohStringViewGetData(pFilepath), pError->line, pError->column);
    
    fprintf_s(stderr, "%s%s%s\n", BOH_OUTPUT_COLOR_ERROR, bohStringGetCStr(&pError->message), BOH_OUTPUT_COLOR_RESET);
}


static void PrintParserErrors(const bohState* pState)
{
    BOH_ASSERT(pState);

    const size_t parserErrorsCount = bohStateGetParserErrorsCount(pState);
        
    for (size_t i = 0; i < parserErrorsCount; ++i) {
        const bohParserError* pError = bohStateParserErrorAt(pState, i);
        PrintParserError(pError);
    }
}


static void PrintInterpreterError(const bohInterpreterError* pError)
{
    BOH_ASSERT(pError);

    const bohStringView* pFilepath = &pError->filepath;

    fprintf_s(stderr, "[INTERPRETER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(pFilepath), bohStringViewGetData(pFilepath), pError->line, pError->column);
    
    fprintf_s(stderr, "%s%s%s\n", BOH_OUTPUT_COLOR_ERROR, bohStringGetCStr(&pError->message), BOH_OUTPUT_COLOR_RESET);
}


static void PrintInterpreterErrors(const bohState* pState)
{
    BOH_ASSERT(pState);

    const size_t interpErrorsCount = bohStateGetInterpreterErrorsCount(pState);
        
    for (size_t i = 0; i < interpErrorsCount; ++i) {
        const bohInterpreterError* pError = bohStateInterpreterErrorAt(pState, i);
        PrintInterpreterError(pError);
    }
}


static void PrintOffset(FILE* pStream, uint64_t length)
{
    for (uint64_t i = 0; i < length; ++i) {
        fputc(' ', pStream);
    }
}


static void PrintValueExpr(const bohAST* pAst, bohExprIdx exprIdx, uint64_t offsetLen)
{
    BOH_ASSERT(pAst);

    const bohValueExpr* pExpr = bohExprGetValueExpr(bohAstGetExprByIdx(pAst, exprIdx));

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
                fprintf_s(stdout, "%sStr[\"%s\"]%s", BOH_OUTPUT_COLOR_VALUE, bohStringGetCStr(pString), BOH_OUTPUT_COLOR_RESET);
            } else {
                const bohStringView* pStrView = bohBoharesStringGetStringView(&pExpr->string);
                fprintf_s(stdout, "%sStrView[\"%.*s\"]%s", 
                    BOH_OUTPUT_COLOR_VALUE, bohStringViewGetSize(pStrView), bohStringViewGetData(pStrView), BOH_OUTPUT_COLOR_RESET);
            }
            break;
        default:
            BOH_ASSERT(false && "Invalid value expression type");
            break;
    }
}


static void PrintUnaryExpr(const bohAST* pAst, bohExprIdx exprIdx, uint64_t offsetLen)
{
    BOH_ASSERT(pAst);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;
    
    const bohExpr* pExpr = bohAstGetExprByIdx(pAst, exprIdx);
    const bohUnaryExpr* pUnaryExpr = bohExprGetUnaryExpr(pExpr);
    const bohExpr* pOperandExpr = bohAstGetExprByIdx(pAst, pUnaryExpr->exprIdx);

    const bool isOperandValueExpr = bohExprIsValueExpr(pOperandExpr);
    const bool isOperandNumber = isOperandValueExpr && bohValueExprIsNumber(bohExprGetValueExpr(pOperandExpr));

    fprintf_s(stdout, "%sUnOp%s(", BOH_OUTPUT_COLOR_OPERATOR_EXPR, BOH_OUTPUT_COLOR_RESET);
    fprintf_s(stdout, "%s%s%s", BOH_OUTPUT_COLOR_OPERATOR, bohParsExprOperatorToStr(pUnaryExpr->op), BOH_OUTPUT_COLOR_RESET);
    fputs(isOperandNumber ? ", " : ",\n", stdout);

    if (!isOperandNumber) {
        PrintOffset(stdout, nextlevelOffsetLen);
    }

    PrintExpr(pAst, pOperandExpr->selfIdx, nextlevelOffsetLen);
            
    if (!isOperandNumber) {
        fputc('\n', stdout);
        PrintOffset(stdout, offsetLen);
    }
    fputc(')', stdout);
}


static void PrintBinaryExpr(const bohAST* pAst, bohExprIdx exprIdx, uint64_t offsetLen)
{
    BOH_ASSERT(pAst);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;
    
    const bohExpr* pExpr = bohAstGetExprByIdx(pAst, exprIdx);
    const bohBinaryExpr* pBinaryExpr = bohExprGetBinaryExpr(pExpr);

    const bohExpr* pLeftExpr = bohAstGetExprByIdx(pAst, pBinaryExpr->leftExprIdx);
    const bool isLeftOperandValueExpr = bohExprIsValueExpr(pLeftExpr);
    const bool isLeftOperandNumber = isLeftOperandValueExpr && bohValueExprIsNumber(bohExprGetValueExpr(pLeftExpr));

    const bohExpr* pRightExpr = bohAstGetExprByIdx(pAst, pBinaryExpr->rightExprIdx);
    const bool isRightOperandValueExpr = bohExprIsValueExpr(pRightExpr);
    const bool isRightOperandNumber = isRightOperandValueExpr && bohValueExprIsNumber(bohExprGetValueExpr(pRightExpr));

    const bool areLeftAndRightNodesNumbers = isLeftOperandNumber && isRightOperandNumber;
    
    fprintf_s(stdout, "%sBinOp%s(", BOH_OUTPUT_COLOR_OPERATOR_EXPR, BOH_OUTPUT_COLOR_RESET);
    fprintf_s(stdout, "%s%s%s", BOH_OUTPUT_COLOR_OPERATOR, bohParsExprOperatorToStr(pBinaryExpr->op), BOH_OUTPUT_COLOR_RESET);
    fputs(areLeftAndRightNodesNumbers ? ", " : ",\n", stdout);

    if (!areLeftAndRightNodesNumbers) {
        PrintOffset(stdout, nextlevelOffsetLen);
    }

    PrintExpr(pAst, pLeftExpr->selfIdx, nextlevelOffsetLen);
            
    fputs(areLeftAndRightNodesNumbers ? ", " : ",\n", stdout);

    if (!areLeftAndRightNodesNumbers) {
        PrintOffset(stdout, nextlevelOffsetLen);
    }

    PrintExpr(pAst, pRightExpr->selfIdx, nextlevelOffsetLen);
            
    if (!areLeftAndRightNodesNumbers) {
        fputc('\n', stdout);
        PrintOffset(stdout, offsetLen);
    }
    fputc(')', stdout);
}


static void PrintExpr(const bohAST* pAst, bohExprIdx exprIdx, uint64_t offsetLen)
{
    BOH_ASSERT(pAst);

    const bohExpr* pExpr = bohAstGetExprByIdx(pAst, exprIdx);

    switch (pExpr->type) {
        case BOH_EXPR_TYPE_VALUE:
            PrintValueExpr(pAst, pExpr->selfIdx, offsetLen);
            break;
        case BOH_EXPR_TYPE_UNARY:
            PrintUnaryExpr(pAst, pExpr->selfIdx, offsetLen);
            break;
        case BOH_EXPR_TYPE_BINARY:
            PrintBinaryExpr(pAst, pExpr->selfIdx, offsetLen);
            break;
        default:
            BOH_ASSERT(false && "Invalid AST node type");
            break;
    }
}

static void PrintAstRawExprStmt(const bohAST* pAst, bohStmtIdx rawExprStmtIdx, uint64_t offsetLen)
{
    BOH_ASSERT(pAst);

    const uint64_t nextlevelOffsetLen = offsetLen + 4;

    const bohStmt* pStmt = bohAstGetStmtByIdx(pAst, rawExprStmtIdx);
    BOH_ASSERT(pStmt);

    const bohRawExprStmt* pRawExprStmt = bohStmtGetRawExpr(pStmt);

    fputs("RawExprStmt(\n", stdout);
    PrintOffset(stdout, nextlevelOffsetLen);

    PrintExpr(pAst, pRawExprStmt->exprIdx, nextlevelOffsetLen);

    fputc('\n', stdout);
    PrintOffset(stdout, offsetLen);
    fputc(')', stdout);
}


static void PrintAstStmt(const bohAST* pAst, bohStmtIdx stmtIdx, uint64_t offsetLen)
{
    BOH_ASSERT(pAst);

    const bohStmt* pStmt = bohAstGetStmtByIdx(pAst, stmtIdx);
    BOH_ASSERT(pStmt);
    
    switch (pStmt->type) {
        // case BOH_STMT_TYPE_EMPTY:
        //     break;
        case BOH_STMT_TYPE_RAW_EXPR:
            PrintAstRawExprStmt(pAst, pStmt->selfIdx, offsetLen);
            break;
        // case BOH_STMT_TYPE_PRINT:
        //     break;
        default:
            BOH_ASSERT(false && "Invalid statement type");
            break;
    }
}


static void PrintAst(const bohAST* pAst)
{
    BOH_ASSERT(pAst);
    
    const bohStmtStorage* pStmts = bohAstGetStmtsConst(pAst);
    const size_t stmtCount = bohDynArrayGetSize(pStmts);

    uint64_t offsetLen = 0;

    for (size_t stmtIdx = 0; stmtIdx < stmtCount; ++stmtIdx) {
        PrintAstStmt(pAst, stmtIdx, offsetLen);
    }
}


int main(int argc, char* argv[])
{
    bohGlobalStateInit();
    bohState* pState = bohGlobalStateGet();

#define DEBUG_NO_ARGS
#ifndef DEBUG_NO_ARGS
    if (argc != 2) {
        bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "Invalid command line arguments count: %d\n", argc);
        return EXIT_FAILURE;
    }

    const char* pFilePath = argv[1];
#else
    (void)argc;
    (void)argv;

    const char* pFilePath = "../test/test.boh";
#endif

    bohStateSetCurrProcessingFile(pState, bohStringViewCreateCStr(pFilePath));

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

    if (bohStateHasLexerErrors(pState)) {
        PrintLexerErrors(pState);        
        exit(-1);
    }

    fprintf_s(stdout, "\n%sLEXER TOKENS:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);

    const bohTokenStorage* pTokens = bohLexerGetTokens(&lexer);

    const size_t tokensCount = bohDynArrayGetSize(pTokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohDynArrayAtConst(pTokens, i);

        const bohStringView* pLexeme = bohTokenGetLexeme(pToken);

        fprintf_s(stdout, "(%s%s%s, ", BOH_OUTPUT_COLOR_YELLOW, bohTokenGetTypeStr(pToken), BOH_OUTPUT_COLOR_RESET);
        fprintf_s(stdout, "%s%.*s%s", BOH_OUTPUT_COLOR_GREEN, bohStringViewGetSize(pLexeme), bohStringViewGetData(pLexeme), BOH_OUTPUT_COLOR_RESET);
        fprintf_s(stdout, ", %u, %u)\n", pToken->line, pToken->column);
    }

    bohParser parser = bohParserCreate(pTokens);
    bohParserParse(&parser);

    if (bohStateHasParserErrors(pState)) {
        PrintParserErrors(pState);        
        exit(-2);
    }

    fprintf_s(stdout, "%s\nAST:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);
    PrintAst(bohParserGetAST(&parser));

#if 0

    bohInterpreter interp = bohInterpCreate(&ast);

    fprintf_s(stdout, "\n\n%sINTERPRETER:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);
    bohInterpResult interpResult = bohInterpInterpret(&interp);

    if (bohStateHasInterpreterErrors(pState)) {
        PrintInterpreterErrors(pState);        
        exit(-3);
    }

    if (bohInterpResultIsNumber(&interpResult)) {
        const bohNumber* pNumber = bohInterpResultGetNumber(&interpResult);

        if (bohNumberIsI64(pNumber)) {
            fprintf_s(stdout, "%sresult: %d%s\n", BOH_OUTPUT_COLOR_YELLOW, bohNumberGetI64(pNumber), BOH_OUTPUT_COLOR_RESET);
        } else {
            fprintf_s(stdout, "%sresult: %f%s\n", BOH_OUTPUT_COLOR_YELLOW, bohNumberGetF64(pNumber), BOH_OUTPUT_COLOR_RESET);
        }
    } else if (bohInterpResultIsString(&interpResult)) {
        const bohBoharesString* pString = bohInterpResultGetString(&interpResult);

        if (bohBoharesStringIsStringView(pString)) {
            const bohStringView* pStrView = bohBoharesStringGetStringView(pString);
            fprintf_s(stdout, "%sresult: %.*s%s\n", 
                BOH_OUTPUT_COLOR_YELLOW, bohStringViewGetSize(pStrView), bohStringViewGetData(pStrView), BOH_OUTPUT_COLOR_RESET);
        } else {
            const bohString* pStrStr = bohBoharesStringGetString(pString);
            fprintf_s(stdout, "%sresult: %s%s\n", BOH_OUTPUT_COLOR_YELLOW, bohStringGetCStr(pStrStr), BOH_OUTPUT_COLOR_RESET);
        }
    }

    bohInterpreterDestroy(&interp);
#endif

    bohParserDestroy(&parser);
    bohLexerDestroy(&lexer);

    bohGlobalStateDestroy();

    return EXIT_SUCCESS;
}
