#include "pch.h"

#include "state.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"


static void PrintLexerError(const bohLexerError* pError)
{
    assert(pError);

    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_WHITE, "[LEXER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(&pError->filepath), bohStringViewGetData(&pError->filepath), pError->line, pError->column);
    
    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "%s\n", bohStringGetCStr(&pError->message));
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

    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_WHITE, "[PARSER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(&pError->filepath), bohStringViewGetData(&pError->filepath), pError->line, pError->column);
    
    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "%s\n", bohStringGetCStr(&pError->message));
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

    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_WHITE, "[INTERPRETER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(&pError->filepath), bohStringViewGetData(&pError->filepath), pError->line, pError->column);
    
    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "%s\n", bohStringGetCStr(&pError->message));
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
        bohColorPrintf(pStream, BOH_OUTPUT_COLOR_WHITE, "%s", " ");
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
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "I64[%d]", bohNumberGetI64(&pNode->number));
            } else {
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "F64[%f]", bohNumberGetF64(&pNode->number));
            }
            break;
        }
        case BOH_AST_NODE_TYPE_STRING:
        {
            if (bohBoharesStringIsString(&pNode->string)) {
                const bohString* pString = bohBoharesStringGetString(&pNode->string);
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "Str[\"%s\"]", bohStringGetCStr(pString));
            } else {
                const bohStringView* pStrView = bohBoharesStringGetStringView(&pNode->string);
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "StrView[\"%.*s\"]", bohStringViewGetSize(pStrView), bohStringViewGetData(pStrView));
            }
            break;
        }
        case BOH_AST_NODE_TYPE_UNARY:
        {
            const bohAstNodeUnary* pUnary = bohAstNodeGetUnary(pNode);

            const uint64_t nextlevelOffsetLen = offsetLen + 4;
            const bool isOperandNumber = bohAstNodeIsNumber(pUnary->pNode);

            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, "UnOp(");
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%s", bohParsOperatorToStr(pUnary->op));
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, isOperandNumber ? ", " : ",\n");

            if (!isOperandNumber) {
                PrintOffset(stdout, nextlevelOffsetLen);
            }

            PrintAstNode(pUnary->pNode, nextlevelOffsetLen);
            
            if (!isOperandNumber) {
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, "\n");
                PrintOffset(stdout, offsetLen);
            }
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, ")");

            break;
        }
        case BOH_AST_NODE_TYPE_BINARY:
        {
            const bohAstNodeBinary* pBinary = bohAstNodeGetBinary(pNode);

            const uint64_t nextlevelOffsetLen = offsetLen + 4;
            const bool areLeftAndRightNodesNumbers = bohAstNodeIsNumber(pBinary->pLeftNode) && bohAstNodeIsNumber(pBinary->pRightNode);
            
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, "BinOp(");
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%s", bohParsOperatorToStr(pBinary->op));
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, areLeftAndRightNodesNumbers ? ", " : ",\n");

            if (!areLeftAndRightNodesNumbers) {
                PrintOffset(stdout, nextlevelOffsetLen);
            }

            PrintAstNode(pBinary->pLeftNode, nextlevelOffsetLen);
            
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, areLeftAndRightNodesNumbers ? ", " : ",\n");

            if (!areLeftAndRightNodesNumbers) {
                PrintOffset(stdout, nextlevelOffsetLen);
            }

            PrintAstNode(pBinary->pRightNode, nextlevelOffsetLen);
            
            if (!areLeftAndRightNodesNumbers) {
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, "\n");
                PrintOffset(stdout, offsetLen);
            }
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, ")");
            
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
            bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "Filepath is NULL\n");
            return EXIT_FAILURE;
        case BOH_FILE_CONTENT_ERROR_OPEN_FAILED:
            bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "Failed to open file: %s\n", pFilePath);
            return EXIT_FAILURE;
        default:
            break;
    }

    const char* pSourceCode = (const char*)fileContent.pData;
    const size_t sourceCodeSize = fileContent.dataSize;

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "SOURCE:\n");
    if (sourceCodeSize > 0) {
        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, "%.*s\n", sourceCodeSize, pSourceCode);
    }

    bohLexer lexer = bohLexerCreate(pSourceCode, sourceCodeSize);
    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    if (bohStateHasLexerErrors(pState)) {
        PrintLexerErrors(pState);        
        exit(-1);
    }

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "\nLEXER TOKENS:\n");

    const size_t tokensCount = bohDynArrayGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohDynArrayAt(&tokens, i);

        const bohStringView* pLexeme = bohTokenGetLexeme(pToken);

        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, "(");
        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "%s", bohTokenGetTypeStr(pToken));
        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, ", ");
        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%.*s", bohStringViewGetSize(pLexeme), bohStringViewGetData(pLexeme));
        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_WHITE, ", %u, %u)\n", pToken->line, pToken->column);
    }

    bohParser parser = bohParserCreate(&tokens);
    bohAST ast = bohParserParse(&parser);

    if (bohStateHasParserErrors(pState)) {
        PrintParserErrors(pState);        
        exit(-2);
    }

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "\nAST:\n");
    PrintAst(&ast);

    bohInterpreter interp = bohInterpCreate(&ast);

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "\n\nINTERPRETER:\n");
    bohInterpResult interpResult = bohInterpInterpret(&interp);

    if (bohStateHasInterpreterErrors(pState)) {
        PrintInterpreterErrors(pState);        
        exit(-3);
    }

    if (bohInterpResultIsNumber(&interpResult)) {
        const bohNumber* pNumber = bohInterpResultGetNumber(&interpResult);

        if (bohNumberIsI64(pNumber)) {
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "result: %d\n", bohNumberGetI64(pNumber));
        } else {
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "result: %f\n", bohNumberGetF64(pNumber));
        }
    } else if (bohInterpResultIsString(&interpResult)) {
        const bohBoharesString* pString = bohInterpResultGetString(&interpResult);

        if (bohBoharesStringIsStringView(pString)) {
            const bohStringView* pStrView = bohBoharesStringGetStringView(pString);
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "result: %.*s\n", bohStringViewGetSize(pStrView), bohStringViewGetData(pStrView));
        } else {
            const bohString* pStrStr = bohBoharesStringGetString(pString);
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "result: %s\n", bohStringGetCStr(pStrStr));
        }
    }

    bohAstDestroy(&ast);
    bohParserDestroy(&parser);
    bohLexerDestroy(&lexer);

    bohGlobalStateDestroy();

    return EXIT_SUCCESS;
}
