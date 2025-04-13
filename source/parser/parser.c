#include "pch.h"

#include "parser.h"
#include "lexer/lexer.h"


bohParser bohParserCreate(const bohTokenStorage* pTokenStorage)
{
    assert(pTokenStorage);

    bohParser parser;

    parser.pTokenStorage = pTokenStorage;
    parser.currToken = 0;

    return parser;
}


void bohParserDestroy(bohParser* pParser)
{
    assert(pParser);

    pParser->pTokenStorage = NULL;
    pParser->currToken = 0;
}


bohAST bohParserParse(bohParser* pParser)
{
    bohAST ast;

    return ast;
}
