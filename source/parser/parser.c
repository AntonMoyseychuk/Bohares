#include "pch.h"

#include "parser.h"


static const bohToken* parsGetCurrToken(const bohParser* pParser)
{
    assert(pParser);
    return bohDynArrayAtConst(pParser->pTokenStorage, pParser->currTokenIdx);
}


// <primary> = <integer> | <float> | '(' <expr> ')'
// static bohGrouping parsGetPrimary(const bohParser* pParser)
// {
//     assert(pParser);
//
//     const bohToken* pCurrToken = parsGetCurrToken(pParser);
//
//     const bohTokenType tokenType = bohTokenGetType(pCurrToken);
//     const bohStringView lexeme = bohTokenGetLexeme(pCurrToken);
//
//     bohGrouping result;
//
//     switch (tokenType) {
//         case BOH_TOKEN_TYPE_INTEGER:
//             // result.number = bohNumberCreateI32(atoi(bohStringViewGetData(&lexeme)));
//             // result.type = BOH_GROUPING_TYPE_NUMBER;
//             break;
//         case BOH_TOKEN_TYPE_FLOAT:
//             // result.number = bohNumberCreateF32((float)atof(bohStringViewGetData(&lexeme)));
//             // result.type = BOH_GROUPING_TYPE_NUMBER;
//             break;
//         case BOH_TOKEN_TYPE_LPAREN:
//             // expression = bohGetExpression();
//             // assert(bohTokenGetType(parsGetCurrToken(pParser)) == BOH_TOKEN_TYPE_RPAREN);
//             // result.expression = expression;
//             // result.type = BOH_GROUPING_TYPE_EXPRESSION;
//             break;
//         default:
//             assert(false && "Invalid primary token type");
//             break;
//     }
//
//     return result;
// }


bohParser bohParserCreate(const bohTokenStorage* pTokenStorage)
{
    assert(pTokenStorage);

    bohParser parser;

    parser.pTokenStorage = pTokenStorage;
    parser.currTokenIdx = 0;

    return parser;
}


void bohParserDestroy(bohParser* pParser)
{
    assert(pParser);

    pParser->pTokenStorage = NULL;
    pParser->currTokenIdx = 0;
}


bohAST bohParserParse(bohParser* pParser)
{
    bohAST ast;

    return ast;
}


bohNumber bohNumberCreateI32(int32_t value)
{
    bohNumber number;

    bohNumberSetI32(&number, value);

    return number;
}


bohNumber bohNumberCreateF32(float value)
{
    bohNumber number;

    bohNumberSetF32(&number, value);

    return number;
}


bool bohNumberIsI32(const bohNumber* pNumber)
{
    assert(pNumber);
    return pNumber->type == BOH_NUMBER_TYPE_INTEGER;
}


bool bohNumberIsF32(const bohNumber* pNumber)
{
    assert(pNumber);
    return pNumber->type == BOH_NUMBER_TYPE_FLOAT;
}


int32_t bohNumberGetI32(const bohNumber* pNumber)
{
    assert(pNumber);
    assert(bohNumberIsI32(pNumber));
    
    return pNumber->i32;
}


float bohNumberGetF32(const bohNumber* pNumber)
{
    assert(pNumber);
    assert(bohNumberIsF32(pNumber));
    
    return pNumber->f32;
}


void bohNumberSetI32(bohNumber* pNumber, int32_t value)
{
    assert(pNumber);
    
    pNumber->type = BOH_NUMBER_TYPE_INTEGER;
    pNumber->i32 = value;
}


void bohNumberSetF32(bohNumber* pNumber, float value)
{
    assert(pNumber);
    
    pNumber->type = BOH_NUMBER_TYPE_FLOAT;
    pNumber->f32 = value;
}
