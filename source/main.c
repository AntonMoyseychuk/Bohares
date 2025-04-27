#include "pch.h"

#include "lexer/lexer.h"
#include "parser/parser.h"

#include "utils/file/file.h"


#define BOH_OUTPUT_COLOR_RESET_ASCII_CODE      "\033[0m"
#define BOH_OUTPUT_COLOR_BLACK_ASCII_CODE      "\033[30m"
#define BOH_OUTPUT_COLOR_RED_ASCII_CODE        "\033[31m"
#define BOH_OUTPUT_COLOR_GREEN_ASCII_CODE      "\033[32m"
#define BOH_OUTPUT_COLOR_YELLOW_ASCII_CODE     "\033[33m"
#define BOH_OUTPUT_COLOR_BLUE_ASCII_CODE       "\033[34m"
#define BOH_OUTPUT_COLOR_MAGENTA_ASCII_CODE    "\033[35m"
#define BOH_OUTPUT_COLOR_CYAN_ASCII_CODE       "\033[36m"
#define BOH_OUTPUT_COLOR_WHITE_ASCII_CODE      "\033[37m"

#define BOH_MAKE_COLORED_TEXT(text, color) color text BOH_OUTPUT_COLOR_RESET_ASCII_CODE


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


static void PrintAstNode(const bohAstNode* pNode)
{
    assert(pNode);
    
    switch (pNode->type) {
        case BOH_AST_NODE_TYPE_NUMBER:
            if (bohNumberIsI32(&pNode->number)) {
                fprintf_s(stdout, BOH_MAKE_COLORED_TEXT("%d", BOH_OUTPUT_COLOR_YELLOW_ASCII_CODE), bohNumberGetI32(&pNode->number));
            } else {
                fprintf_s(stdout, BOH_MAKE_COLORED_TEXT("%f", BOH_OUTPUT_COLOR_YELLOW_ASCII_CODE), bohNumberGetF32(&pNode->number));
            }
            break;
        case BOH_AST_NODE_TYPE_UNARY:
        {
            const bohAstNodeUnary* pUnary = bohAstNodeGetUnary(pNode);

            fprintf_s(stdout, "UnOp(" BOH_MAKE_COLORED_TEXT("%s", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) ", ", OperatorToStr(pUnary->op));
            PrintAstNode(pUnary->pNode);
            fputc(')', stdout);
        }
            break;
        case BOH_AST_NODE_TYPE_BINARY:
        {
            const bohAstNodeBinary* pBinary = bohAstNodeGetBinary(pNode);
            
            fprintf_s(stdout, "BinOp(" BOH_MAKE_COLORED_TEXT("%s", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) ", ", OperatorToStr(pBinary->op));
            PrintAstNode(pBinary->pLeftNode);
            fputs(", ", stdout);
            PrintAstNode(pBinary->pRightNode);
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
#define DEBUG_NO_ARGS
#ifdef DEBUG_NO_ARGS
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

    const char* str = "2 + 42 * 2 + (47 * -21) % 33";

    bohLexer lexer = bohLexerCreate(str, strlen(str) + 1);

    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    fprintf_s(stdout, BOH_MAKE_COLORED_TEXT("LEXER TOKENS:", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) "\n");

    const size_t tokensCount = bohDynArrayGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohDynArrayAt(&tokens, i);

        const bohStringView lexeme = bohTokenGetLexeme(pToken);
        const char* pLexemeStr = bohStringViewGetData(&lexeme);
        const size_t lexemeSize = bohStringViewGetSize(&lexeme);

        fprintf_s(stdout, "(" BOH_MAKE_COLORED_TEXT("%s", BOH_OUTPUT_COLOR_YELLOW_ASCII_CODE) ", " BOH_MAKE_COLORED_TEXT("%.*s", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) ", %u, %u)\n", 
            bohTokenGetTypeStr(pToken), lexemeSize, pLexemeStr, pToken->line, pToken->column);
    }

    bohParser parser = bohParserCreate(&tokens);
    bohAST ast = bohParserParse(&parser);

    fprintf_s(stdout, "\n" BOH_MAKE_COLORED_TEXT("AST:", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) "\n");
    PrintAstNode(ast.pRoot);
    fputc('\n', stdout);

    bohAstDestroy(&ast);
    bohParserDestroy(&parser);
    bohLexerDestroy(&lexer);

    return EXIT_SUCCESS;
#else
    if (argc != 2) {
        fprintf_s(stderr, BOH_MAKE_COLORED_TEXT("Invalid command line arguments count: %d\n", BOH_OUTPUT_COLOR_RED_ASCII_CODE), argc);
        return EXIT_FAILURE;
    }

    bohFileContent fileContent = bohReadTextFile(argv[1]);
    switch (bohFileContentGetErrorCode(&fileContent)) {
        case BOH_FILE_CONTENT_ERROR_NULL_FILEPATH:
            fprintf_s(stderr, BOH_MAKE_COLORED_TEXT("Filepath is NULL\n", BOH_OUTPUT_COLOR_RED_ASCII_CODE));
            return EXIT_FAILURE;
        case BOH_FILE_CONTENT_ERROR_OPEN_FAILED:
            fprintf_s(stderr, BOH_MAKE_COLORED_TEXT("Failed to open file: %s\n", BOH_OUTPUT_COLOR_RED_ASCII_CODE), argv[1]);
            return EXIT_FAILURE;
        default:
            break;
    }

    bohLexer lexer = bohLexerCreate(fileContent.pData, fileContent.dataSize);
    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    fprintf_s(stdout, BOH_MAKE_COLORED_TEXT("LEXER TOKENS:", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) "\n");
    
    const size_t tokensCount = bohDynArrayGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohDynArrayAt(&tokens, i);

        const bohStringView lexeme = bohTokenGetLexeme(pToken);
        const char* pLexemeStr = bohStringViewGetData(&lexeme);
        const size_t lexemeSize = bohStringViewGetSize(&lexeme);

        fprintf_s(stdout, "(" BOH_MAKE_COLORED_TEXT("%s", BOH_OUTPUT_COLOR_YELLOW_ASCII_CODE) ", " BOH_MAKE_COLORED_TEXT("%.*s", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) ", %u, %u)\n", 
            bohTokenGetTypeStr(pToken), lexemeSize, pLexemeStr, pToken->line, pToken->column);
    }

    bohLexerDestroy(&lexer);
    bohFileContentFree(&fileContent);

    return EXIT_SUCCESS;
#endif
}
