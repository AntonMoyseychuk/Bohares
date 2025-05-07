#include "pch.h"

#include "parser.h"
#include "utils/message/message.h"


#define BOH_THROW_PARSER_ERROR(LINE, COLUMN, FMT, ...)                                          \
{                                                                                               \
    fprintf_s(stderr, "[BOH PARSER ERROR] (%u, %u): ", (uint64_t)(LINE), (uint64_t)(COLUMN));   \
    BOH_THROW_ERROR_FMT(FMT, __VA_ARGS__);                                                      \
}


#define BOH_CHECK_PARSER_COND(COND, LINE, COLUMN, FMT, ...)     \
    if (!(COND)) {                                              \
        BOH_THROW_PARSER_ERROR(LINE, COLUMN, FMT, __VA_ARGS__); \
    }


static bohOperator parsTokenTypeToOperator(bohTokenType tokenType)
{
    switch (tokenType) {
        case BOH_TOKEN_TYPE_PLUS:       return BOH_OP_PLUS;
        case BOH_TOKEN_TYPE_MINUS:      return BOH_OP_MINUS;
        case BOH_TOKEN_TYPE_MULT:       return BOH_OP_MULT;
        case BOH_TOKEN_TYPE_DIV:        return BOH_OP_DIV;
        case BOH_TOKEN_TYPE_MOD:        return BOH_OP_MOD;
        case BOH_TOKEN_TYPE_XOR:        return BOH_OP_XOR;
        case BOH_TOKEN_TYPE_BITWISE_NOT: return BOH_OP_BITWISE_NOT;
        case BOH_TOKEN_TYPE_NOT:        return BOH_OP_NOT;
        case BOH_TOKEN_TYPE_GREATER:    return BOH_OP_GREATER;
        case BOH_TOKEN_TYPE_LESS:       return BOH_OP_LESS;
        case BOH_TOKEN_TYPE_NOT_EQUAL:  return BOH_OP_NOT_EQUAL;
        case BOH_TOKEN_TYPE_GEQUAL:     return BOH_OP_GEQUAL;
        case BOH_TOKEN_TYPE_LEQUAL:     return BOH_OP_LEQUAL;
        case BOH_TOKEN_TYPE_EQUAL:      return BOH_OP_EQUAL;
        case BOH_TOKEN_TYPE_RSHIFT:     return BOH_OP_RSHIFT;
        case BOH_TOKEN_TYPE_LSHIFT:     return BOH_OP_LSHIFT;
        default:                        return BOH_OP_UNKNOWN;
    }
}


static const bohToken* parsPeekCurrToken(const bohParser* pParser)
{
    assert(pParser);
    assert(pParser->currTokenIdx < bohDynArrayGetSize(pParser->pTokenStorage));

    return bohDynArrayAtConst(pParser->pTokenStorage, pParser->currTokenIdx);
}


static const bohToken* parsPeekPrevToken(const bohParser* pParser)
{
    assert(pParser);
    assert(pParser->currTokenIdx > 0);

    const size_t prevTokenIdx = pParser->currTokenIdx - 1;
    assert(prevTokenIdx < bohDynArrayGetSize(pParser->pTokenStorage));

    return bohDynArrayAtConst(pParser->pTokenStorage, prevTokenIdx);
}


static const bohToken* parsAdvanceToken(bohParser* pParser)
{
    assert(pParser);

    ++pParser->currTokenIdx;
    return parsPeekPrevToken(pParser);
}


static bool parsIsCurrTokenMatch(bohParser* pParser, bohTokenType type)
{
    assert(pParser);

    if (pParser->currTokenIdx >= bohDynArrayGetSize(pParser->pTokenStorage)) {
        return false;
    }

    if (parsPeekCurrToken(pParser)->type != type) {
        return false;
    }

    parsAdvanceToken(pParser);
    return true;
}


static bohAstNode* parsAddition(bohParser* pParser);


static bohAstNode* parsPrimary(bohParser* pParser)
{
    assert(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_INTEGER)) {
        const int32_t value = atoi(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        return bohAstNodeCreateNumberI32(value);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FLOAT)) {
        const float value = atof(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        return bohAstNodeCreateNumberF32(value);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LPAREN)) {
        bohAstNode* pExpr = parsAddition(pParser);
        
        if (!parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RPAREN)) {
            const bohToken* pPrevToken = parsPeekPrevToken(pParser);
            BOH_CHECK_PARSER_COND(false, pPrevToken->line, pPrevToken->column, "\')\' expected");
        }

        return pExpr;
    }

    const bohToken* pCurrToken = parsPeekPrevToken(pParser);
    BOH_CHECK_PARSER_COND(false, pCurrToken->line, pCurrToken->column, "unknown primary token type: %s", bohTokenGetTypeStr(pCurrToken));
    
    return NULL;
}


static bohAstNode* parsUnary(bohParser* pParser)
{
    assert(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PLUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_NOT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_NOT)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pOperand = parsUnary(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown unary operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        return bohAstNodeCreateUnary(op, pOperand);
    }

    return parsPrimary(pParser);
}


static bohAstNode* parsMultiplication(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsUnary(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MULT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_DIV) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MOD) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_XOR)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsUnary(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown term operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg);
    }

    return pExpr;
}


static bohAstNode* parsAddition(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsMultiplication(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PLUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsMultiplication(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg);
    }

    return pExpr;
}


static bohAstNode* parsExpr(bohParser* pParser)
{
    return parsAddition(pParser);
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
    if (!pNode) {
        return;
    }

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


bohAstNode* bohAstNodeCreateUnary(bohOperator op, bohAstNode* pArg)
{
    assert(pArg);

    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    bohAstNodeSetUnary(pNode, op, pArg);

    return pNode;
}


bohAstNode* bohAstNodeCreateBinary(bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg)
{
    assert(pLeftArg);
    assert(pRightArg);

    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    bohAstNodeSetBinary(pNode, op, pLeftArg, pRightArg);

    return pNode;
}


bool bohAstNodeIsNumber(const bohAstNode* pNode)
{
    assert(pNode);
    return pNode->type == BOH_AST_NODE_TYPE_NUMBER;
}


bool bohAstNodeIsUnary(const bohAstNode *pNode)
{
    assert(pNode);
    return pNode->type == BOH_AST_NODE_TYPE_UNARY;
}


bool bohAstNodeIsBinary(const bohAstNode *pNode)
{
    assert(pNode);
    return pNode->type == BOH_AST_NODE_TYPE_BINARY;
}


const bohNumber* bohAstNodeGetNumber(const bohAstNode* pNode)
{
    assert(pNode);
    assert(bohAstNodeIsNumber(pNode));

    return &pNode->number;
}


const bohAstNodeUnary* bohAstNodeGetUnary(const bohAstNode* pNode)
{
    assert(pNode);
    assert(bohAstNodeIsUnary(pNode));

    return &pNode->unary;
}


const bohAstNodeBinary* bohAstNodeGetBinary(const bohAstNode* pNode)
{
    assert(pNode);
    assert(bohAstNodeIsBinary(pNode));

    return &pNode->binary;
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


bohAstNode* bohAstNodeSetUnary(bohAstNode* pNode, bohOperator op, bohAstNode* pArg)
{
    assert(pNode);
    assert(pArg);
    
    pNode->type = BOH_AST_NODE_TYPE_UNARY;
    pNode->unary.op = op;
    pNode->unary.pNode = pArg;

    return pNode;
}


bohAstNode* bohAstNodeSetBinary(bohAstNode* pNode, bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg)
{
    assert(pNode);
    assert(pLeftArg);
    assert(pRightArg);
    
    pNode->type = BOH_AST_NODE_TYPE_BINARY;
    pNode->binary.op = op;
    pNode->binary.pLeftNode = pLeftArg;
    pNode->binary.pRightNode = pRightArg;

    return pNode;
}


void bohAstDestroy(bohAST* pAST)
{
    assert(pAST);
    BOH_AST_NODE_DESTROY(pAST->pRoot);
}


bool bohAstIsEmpty(const bohAST* pAST)
{
    assert(pAST);
    return pAST->pRoot == NULL;
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
    bohAST ast = { NULL };

    if (!bohDynArrayIsEmpty(pParser->pTokenStorage)) {
        ast.pRoot = parsExpr(pParser);
    }

    return ast;
}
