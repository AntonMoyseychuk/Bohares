#include "pch.h"

#include "lexer/lexer.h"
#include "utils/file/file.h"


int main(int argc, char* argv[])
{
#if defined(DEBUG_NO_ARGS)
    const char* str = "#= () {\n\r }[], .+-*/%^:;?~!><\n== >> << >= <= !=";
    bohLexer lexer = bohLexerCreate(str, strlen(str) + 1);

    bohTokenStorage tokens = bohLexerTokenize(&lexer);

    const size_t tokensCount = bohTokenStorageGetSize(&tokens);
    for (size_t i = 0; i < tokensCount; ++i) {
        const bohToken* pToken = bohTokenStorageAt(&tokens, i);
        assert(pToken);

        fprintf_s(stdout, "%s\n", bohTokenGetLexeme(pToken));
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
        assert(pToken);

        fprintf_s(stdout, "%s (line: %u, column: %u)\n", bohTokenGetLexeme(pToken), pToken->line, pToken->column);
    }

    bohFileContentFree(&fileContent);

    return EXIT_SUCCESS;
#endif
}
