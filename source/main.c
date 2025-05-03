#include "pch.h"

#include "lexer/lexer.h"
#include "parser/parser.h"

#include "utils/file/file.h"
#include "utils/message/message.h"


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
            if (bohNumberIsI32(&pNode->number)) {
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "Int32[%d]", bohNumberGetI32(&pNode->number));
            } else {
                bohColorPrintf(stdout, BOH_OUTPUT_COLOR_YELLOW, "Float[%f]", bohNumberGetF32(&pNode->number));
            }
        }
            break;
        case BOH_AST_NODE_TYPE_UNARY:
        {
            const bohAstNodeUnary* pUnary = bohAstNodeGetUnary(pNode);

            fputs("UnOp(", stdout);
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%s", OperatorToStr(pUnary->op));
            fputs(", ", stdout);

            PrintAstNode(pUnary->pNode, offsetLen + sizeof("UnOp(") - 1);
            
            fputc(')', stdout);
        }
            break;
        case BOH_AST_NODE_TYPE_BINARY:
        {
            const bohAstNodeBinary* pBinary = bohAstNodeGetBinary(pNode);
            
            fputs("BinOp(", stdout);
            bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "%s", OperatorToStr(pBinary->op));
            fputs(",\n", stdout);

            const uint64_t nextlevelOffsetLen = offsetLen + sizeof("BinOp(") - 1;

            PrintOffset(stdout, nextlevelOffsetLen);
            PrintAstNode(pBinary->pLeftNode, nextlevelOffsetLen);
            
            fputs(",\n", stdout);

            PrintOffset(stdout, nextlevelOffsetLen);
            PrintAstNode(pBinary->pRightNode, nextlevelOffsetLen);
            
            fputc('\n', stdout);
            PrintOffset(stdout, offsetLen);
            fputs(")", stdout);
        }
            break;
        default:
            assert(false && "Invalid AST node type");
            break;
    }
}


int main(int argc, char* argv[])
{
#define DEBUG_NO_ARGS
#ifndef DEBUG_NO_ARGS
    if (argc != 2) {
        bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "Invalid command line arguments count: %d\n", argc);
        return EXIT_FAILURE;
    }

    bohFileContent fileContent = bohReadTextFile(argv[1]);
    switch (bohFileContentGetErrorCode(&fileContent)) {
        case BOH_FILE_CONTENT_ERROR_NULL_FILEPATH:
            bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "Filepath is NULL\n");
            return EXIT_FAILURE;
        case BOH_FILE_CONTENT_ERROR_OPEN_FAILED:
            bohColorPrintf(stderr, BOH_OUTPUT_COLOR_RED, "Failed to open file: %s\n", argv[1]);
            return EXIT_FAILURE;
        default:
            break;
    }

    const char* pSourceCode = (const char*)fileContent.pData;
    const size_t sourceCodeSize = fileContent.dataSize;
#else
    //     const char* str =
    // "# Initialize constants\n"

    // "M3D_PI = 3.141592\n"
    // "M3D_G = 9.81\n"


    // "# Initialize vars\n"

    // "x = 8\n"


    // "if x >= 0 then\n"
    // "    print(\"x is positive\n\")\n"
    // "else\n"
    // "    print(\"x is negative\n\")\n"
    // "end\n";

    const char* pSourceCode = "42 * 2 + (47 * +-33.301) % (~33 ^ 77)";
    const size_t sourceCodeSize = strlen(pSourceCode) + 1;
#endif

    bohLexer lexer = bohLexerCreate(pSourceCode, sourceCodeSize);
    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "LEXER TOKENS: \n");

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

    bohColorPrintf(stdout, BOH_OUTPUT_COLOR_GREEN, "\nAST: \n");
    PrintAstNode(ast.pRoot, 0);

    bohAstDestroy(&ast);
    bohParserDestroy(&parser);
    bohLexerDestroy(&lexer);

    return EXIT_SUCCESS;
}
