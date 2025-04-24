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

static bohAstNode* parsPrimary(bohParser* pParser)
{

}


static bohAstNode* parsUnary(bohParser* pParser)
{

}


static bohAstNode* parsFactor(bohParser* pParser)
{

}


static bohAstNode* parsTerm(bohParser* pParser)
{

}


static bohAstNode* parsExpr(bohParser* pParser)
{
    
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


void bohAstNodeDestroy(bohAstNode* pNode)
{
    assert(pNode);

    switch (pNode->type) {
        case BOH_AST_NODE_TYPE_NUMBER:
            break;
        case BOH_AST_NODE_TYPE_UNARY:
            BOH_AST_NODE_DESTROY(pNode->unary.pNode);
            break;
        case BOH_AST_NODE_TYPE_BINARY:
            BOH_AST_NODE_DESTROY(pNode->binary.pLeftNode);
            BOH_AST_NODE_DESTROY(pNode->binary.pRightNode);
            break;
        default:
            assert(false && "Invalid AST node type");
            break;
    }

    free(pNode);
}


bohAstNode* bohAstNodeCreateNumberI32(int32_t value)
{
    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    bohAstNodeSetNumberI32(pNode, value);

    return pNode;
}


bohAstNode* bohAstNodeCreateNumberF32(float value)
{
    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    bohAstNodeSetNumberF32(pNode, value);

    return pNode;
}


bool bohAstNodeIsNumber(const bohAstNode* pNode)
{
    assert(pNode);
    return pNode->type == BOH_AST_NODE_TYPE_NUMBER;
}


const bohNumber* bohAstNodeGetNumber(const bohAstNode* pNode)
{
    assert(pNode);
    assert(bohAstNodeIsNumber(pNode));

    return &pNode->number;
}


void bohAstNodeSetNumberI32(bohAstNode* pNode, int32_t value)
{
    assert(pNode);

    pNode->type = BOH_AST_NODE_TYPE_NUMBER;
    pNode->number = bohNumberCreateI32(value);
}


void bohAstNodeSetNumberF32(bohAstNode* pNode, float value)
{
    assert(pNode);
    
    pNode->type = BOH_AST_NODE_TYPE_NUMBER;
    pNode->number = bohNumberCreateF32(value);
}


void bohAstDestroy(bohAST* pAST)
{
    assert(pAST);

    BOH_AST_NODE_DESTROY(pAST->pRoot);
}


bohParser bohParserCreate(const bohTokenStorage *pTokenStorage)
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

    ast.pRoot = parsExpr(pParser);

    return ast;
}
