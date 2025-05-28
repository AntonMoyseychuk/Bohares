#include "pch.h"

#include "state.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"


static void PrintLexerError(const bohLexerError* pError)
{
    assert(pError);

    const bohStringView* pFilepath = &pError->filepath;

    fprintf_s(stderr, "[LEXER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(pFilepath), bohStringViewGetData(pFilepath), pError->line, pError->column);
    
    fprintf_s(stderr, "%s%s%s\n", BOH_OUTPUT_COLOR_RED, bohStringGetCStr(&pError->message), BOH_OUTPUT_COLOR_RESET);
}


static void PrintLexerErrors(const bohState* pState)
{
    assert(pState);

    const size_t lexerErrorsCount = bohStateGetLexerErrorsCount(pState);
        
    for (size_t i = 0; i < lexerErrorsCount; ++i) {
        const bohLexerError* pError = bohStateLexerErrorAt(pState, i);
        PrintLexerError(pError);
    }
}


static void PrintParserError(const bohParserError* pError)
{
    assert(pError);

    const bohStringView* pFilepath = &pError->filepath;

    fprintf_s(stderr, "[PARSER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(pFilepath), bohStringViewGetData(pFilepath), pError->line, pError->column);
    
    fprintf_s(stderr, "%s%s%s\n", BOH_OUTPUT_COLOR_RED, bohStringGetCStr(&pError->message), BOH_OUTPUT_COLOR_RESET);
}


static void PrintParserErrors(const bohState* pState)
{
    assert(pState);

    const size_t parserErrorsCount = bohStateGetParserErrorsCount(pState);
        
    for (size_t i = 0; i < parserErrorsCount; ++i) {
        const bohParserError* pError = bohStateParserErrorAt(pState, i);
        PrintParserError(pError);
    }
}


static void PrintInterpreterError(const bohInterpreterError* pError)
{
    assert(pError);

    const bohStringView* pFilepath = &pError->filepath;

    fprintf_s(stderr, "[INTERPRETER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(pFilepath), bohStringViewGetData(pFilepath), pError->line, pError->column);
    
    fprintf_s(stderr, "%s%s%s\n", BOH_OUTPUT_COLOR_RED, bohStringGetCStr(&pError->message), BOH_OUTPUT_COLOR_RESET);
}


static void PrintInterpreterErrors(const bohState* pState)
{
    assert(pState);

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


static void PrintAstNode(const bohAstNode* pNode, uint64_t offsetLen)
{
    if (!pNode) {
        return;
    }
    
    switch (pNode->type) {
        case BOH_AST_NODE_TYPE_NUMBER:
        {
            if (bohNumberIsI64(&pNode->number)) {
                fprintf_s(stdout, "%sI64[%d]%s", BOH_OUTPUT_COLOR_YELLOW, bohNumberGetI64(&pNode->number), BOH_OUTPUT_COLOR_RESET);
            } else {
                fprintf_s(stdout, "%sF64[%f]%s", BOH_OUTPUT_COLOR_YELLOW, bohNumberGetF64(&pNode->number), BOH_OUTPUT_COLOR_RESET);
            }
            break;
        }
        case BOH_AST_NODE_TYPE_STRING:
        {
            if (bohBoharesStringIsString(&pNode->string)) {
                const bohString* pString = bohBoharesStringGetString(&pNode->string);
                fprintf_s(stdout, "%sStr[\"%s\"]%s", BOH_OUTPUT_COLOR_YELLOW, bohStringGetCStr(pString), BOH_OUTPUT_COLOR_RESET);
            } else {
                const bohStringView* pStrView = bohBoharesStringGetStringView(&pNode->string);
                fprintf_s(stdout, "%sStrView[\"%.*s\"]%s", 
                    BOH_OUTPUT_COLOR_YELLOW, bohStringViewGetSize(pStrView), bohStringViewGetData(pStrView), BOH_OUTPUT_COLOR_RESET);
            }
            break;
        }
        case BOH_AST_NODE_TYPE_UNARY:
        {
            const bohAstNodeUnary* pUnary = bohAstNodeGetUnary(pNode);

            const uint64_t nextlevelOffsetLen = offsetLen + 4;
            const bool isOperandNumber = bohAstNodeIsNumber(pUnary->pNode);

            fputs("UnOp(", stdout);
            fprintf_s(stdout, "%s%s%s", BOH_OUTPUT_COLOR_GREEN, bohParsExprOperatorToStr(pUnary->op), BOH_OUTPUT_COLOR_RESET);
            fputs(isOperandNumber ? ", " : ",\n", stdout);

            if (!isOperandNumber) {
                PrintOffset(stdout, nextlevelOffsetLen);
            }

            PrintAstNode(pUnary->pNode, nextlevelOffsetLen);
            
            if (!isOperandNumber) {
                fputc('\n', stdout);
                PrintOffset(stdout, offsetLen);
            }
            fputc(')', stdout);

            break;
        }
        case BOH_AST_NODE_TYPE_BINARY:
        {
            const bohAstNodeBinary* pBinary = bohAstNodeGetBinary(pNode);

            const uint64_t nextlevelOffsetLen = offsetLen + 4;
            const bool areLeftAndRightNodesNumbers = bohAstNodeIsNumber(pBinary->pLeftNode) && bohAstNodeIsNumber(pBinary->pRightNode);
            
            fputs("BinOp(", stdout);
            fprintf_s(stdout, "%s%s%s", BOH_OUTPUT_COLOR_GREEN, bohParsExprOperatorToStr(pBinary->op), BOH_OUTPUT_COLOR_RESET);
            fputs(areLeftAndRightNodesNumbers ? ", " : ",\n", stdout);

            if (!areLeftAndRightNodesNumbers) {
                PrintOffset(stdout, nextlevelOffsetLen);
            }

            PrintAstNode(pBinary->pLeftNode, nextlevelOffsetLen);
            
            fputs(areLeftAndRightNodesNumbers ? ", " : ",\n", stdout);

            if (!areLeftAndRightNodesNumbers) {
                PrintOffset(stdout, nextlevelOffsetLen);
            }

            PrintAstNode(pBinary->pRightNode, nextlevelOffsetLen);
            
            if (!areLeftAndRightNodesNumbers) {
                fputc('\n', stdout);
                PrintOffset(stdout, offsetLen);
            }
            fputc(')', stdout);
            
            break;
        }
        default:
            assert(false && "Invalid AST node type");
            break;
    }
}


static void PrintAst(const bohAST* pAST)
{
    assert(pAST);
    PrintAstNode(pAST->pRoot, 0);
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
    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    if (bohStateHasLexerErrors(pState)) {
        PrintLexerErrors(pState);        
        exit(-1);
    }

    fprintf_s(stdout, "\n%sLEXER TOKENS:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);

    const size_t tokensCount = bohDynArrayGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohDynArrayAt(&tokens, i);

        const bohStringView* pLexeme = bohTokenGetLexeme(pToken);

        fprintf_s(stdout, "(%s%s%s, ", BOH_OUTPUT_COLOR_YELLOW, bohTokenGetTypeStr(pToken), BOH_OUTPUT_COLOR_RESET);
        fprintf_s(stdout, "%s%.*s%s", BOH_OUTPUT_COLOR_GREEN, bohStringViewGetSize(pLexeme), bohStringViewGetData(pLexeme), BOH_OUTPUT_COLOR_RESET);
        fprintf_s(stdout, ", %u, %u)\n", pToken->line, pToken->column);
    }

    bohParser parser = bohParserCreate(&tokens);
    bohAST ast = bohParserParse(&parser);

    if (bohStateHasParserErrors(pState)) {
        PrintParserErrors(pState);        
        exit(-2);
    }

    fprintf_s(stdout, "%s\nAST:%s\n", BOH_OUTPUT_COLOR_GREEN, BOH_OUTPUT_COLOR_RESET);
    PrintAst(&ast);

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

    bohAstDestroy(&ast);
    bohParserDestroy(&parser);
    bohLexerDestroy(&lexer);

    bohGlobalStateDestroy();

    return EXIT_SUCCESS;
}
