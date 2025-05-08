#include "pch.h"

#include "state.h"

#include "lexer/lexer.h"
#include "parser/parser.h"


static const char* OperatorToStr(bohOperator op)
{
    switch (op) {
        case BOH_OP_PLUS: return "+";
        case BOH_OP_MINUS: return "-";
        case BOH_OP_MULT: return "*";
        case BOH_OP_DIV: return "/";
        case BOH_OP_MOD: return "%";
        case BOH_OP_XOR: return "^";
        case BOH_OP_BITWISE_NOT: return "~";
        case BOH_OP_NOT: return "!";
        case BOH_OP_GREATER: return ">";
        case BOH_OP_LESS: return "<";
        case BOH_OP_NOT_EQUAL: return "!=";
        case BOH_OP_GEQUAL: return ">=";
        case BOH_OP_LEQUAL: return "<=";
        case BOH_OP_EQUAL: return "==";
        case BOH_OP_RSHIFT: return ">>";
        case BOH_OP_LSHIFT: return "<<";
        default:
            assert(false && "Error: Failed to convert operator to string");
            return NULL;
    }
}


static void PrintLexerError(const bohLexerError* pError)
{
    assert(pError);

    fprintf_s(stderr, "[LEXER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(&pError->filepath), bohStringViewGetData(&pError->filepath), pError->line, pError->column);
    
    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, bohStringGetCStr(&pError->message));
    fputc('\n', stderr);
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

    fprintf_s(stderr, "[PARSER ERROR]: %.*s (%u, %u): ", 
        bohStringViewGetSize(&pError->filepath), bohStringViewGetData(&pError->filepath), pError->line, pError->column);
    
    bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, bohStringGetCStr(&pError->message));
    fputc('\n', stderr);
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
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "Int32[%d]", bohNumberGetI64(&pNode->number));
            } else {
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "Float[%f]", bohNumberGetF64(&pNode->number));
            }
        }
            break;
        case BOH_AST_NODE_TYPE_UNARY:
        {
            const bohAstNodeUnary* pUnary = bohAstNodeGetUnary(pNode);

            const uint64_t nextlevelOffsetLen = offsetLen + 4;
            const bool isOperandNumber = bohAstNodeIsNumber(pUnary->pNode);

            fputs("UnOp(", stdout);
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%s", OperatorToStr(pUnary->op));
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
        }
            break;
        case BOH_AST_NODE_TYPE_BINARY:
        {
            const bohAstNodeBinary* pBinary = bohAstNodeGetBinary(pNode);

            const uint64_t nextlevelOffsetLen = offsetLen + 4;
            const bool areLeftAndRightNodesNumbers = bohAstNodeIsNumber(pBinary->pLeftNode) && bohAstNodeIsNumber(pBinary->pRightNode);
            
            fputs("BinOp(", stdout);
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%s", OperatorToStr(pBinary->op));
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
        }
            break;
        default:
            assert(false && "Invalid AST node type");
            break;
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

    bohLexer lexer = bohLexerCreate(pSourceCode, sourceCodeSize);
    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    if (bohStateHasLexerErrors(pState)) {
        PrintLexerErrors(pState);        
        exit(-1);
    }

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "LEXER TOKENS:");
    fputc('\n', stdout);

    const size_t tokensCount = bohDynArrayGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohDynArrayAt(&tokens, i);

        const bohStringView lexeme = bohTokenGetLexeme(pToken);
        const char* pLexemeStr = bohStringViewGetData(&lexeme);
        const size_t lexemeSize = bohStringViewGetSize(&lexeme);

        fputs("(", stdout);
        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "%s", bohTokenGetTypeStr(pToken));
        fputs(", ", stdout);
        bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%.*s", lexemeSize, pLexemeStr);
        fprintf_s(stdout, ", %u, %u)\n", pToken->line, pToken->column);
    }

    bohParser parser = bohParserCreate(&tokens);
    bohAST ast = bohParserParse(&parser);

    if (bohStateHasParserErrors(pState)) {
        PrintParserErrors(pState);        
        exit(-2);
    }

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "\nAST:");
    fputc('\n', stdout);
    PrintAstNode(ast.pRoot, 0);

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "\n\nINTERPRETER:");
    fputc('\n', stdout);

    bohAstDestroy(&ast);
    bohParserDestroy(&parser);
    bohLexerDestroy(&lexer);

    bohGlobalStateDestroy();

    return EXIT_SUCCESS;
}
