#include "pch.h"

#include "lexer/lexer.h"
#include "utils/file/file.h"


int main(int argc, char* argv[])
{
// #define DEBUG_NO_ARGS
#ifdef DEBUG_NO_ARGS
    const char* str = "\"Hellow World!\" 123 + 33.33";
    bohLexer lexer = bohLexerCreate(str, strlen(str) + 1);

    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    const size_t tokensCount = bohTokenStorageGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohTokenStorageAt(&tokens, i);

        const bohString* pLexeme = bohTokenGetLexeme(pToken);
        const char* pLexemeStr = bohStringGetDataConst(pLexeme);

        fprintf_s(stdout, "token: %s\t(line: %u, column: %u)\n", pLexemeStr, pToken->line, pToken->column);
    }

    return EXIT_SUCCESS;
#else
    if (argc != 2) {
        fprintf_s(stderr, "Invalid command line arguments count: %d\n", argc);
        return EXIT_FAILURE;
    }

    bohFileContent fileContent = bohReadTextFile(argv[1]);
    if (bohFileContentIsError(&fileContent)) {
        fprintf_s(stderr, "%s\n", fileContent.pErrorMsg);
        return EXIT_FAILURE;
    }

    bohLexer lexer = bohLexerCreate(fileContent.pData, fileContent.dataSize);
    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    const size_t tokensCount = bohTokenStorageGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohTokenStorageAt(&tokens, i);

        const bohString* pLexeme = bohTokenGetLexeme(pToken);
        const char* pLexemeStr = bohStringGetDataConst(pLexeme);

        fprintf_s(stdout, "token: %s\t(line: %u, column: %u)\n", pLexemeStr, pToken->line, pToken->column);
    }

    bohFileContentFree(&fileContent);

    return EXIT_SUCCESS;
#endif
}
