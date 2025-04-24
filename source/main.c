#include "pch.h"

#include "lexer/lexer.h"
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


int main(int argc, char* argv[])
{
// #define DEBUG_NO_ARGS
#ifdef DEBUG_NO_ARGS
    const char* str = "if x >= 0 then print(\"x is positive\") else print(\"x is negative\") end";

    bohLexer lexer = bohLexerCreate(str, strlen(str) + 1);

    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    const size_t tokensCount = bohDynArrayGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohDynArrayAt(&tokens, i);

        const bohStringView lexeme = bohTokenGetLexeme(pToken);
        const char* pLexemeStr = bohStringViewGetData(&lexeme);
        const size_t lexemeSize = bohStringViewGetSize(&lexeme);

        fprintf_s(stdout, "(" BOH_MAKE_COLORED_TEXT("%s", BOH_OUTPUT_COLOR_YELLOW_ASCII_CODE) ", " BOH_MAKE_COLORED_TEXT("%.*s", BOH_OUTPUT_COLOR_GREEN_ASCII_CODE) ", %u, %u)\n", 
            bohTokenGetTypeStr(pToken), lexemeSize, pLexemeStr, pToken->line, pToken->column);
    }

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

    bohFileContentFree(&fileContent);

    return EXIT_SUCCESS;
#endif
}
