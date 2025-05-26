#include "pch.h"

#include "parser.h"
#include "lexer/lexer.h"

#include "state.h"


#define BOH_CHECK_PARSER_COND(COND, LINE, COLUMN, FMT, ...)                 \
    if (!(COND)) {                                                          \
        char msg[1024] = {0};                                               \
        sprintf_s(msg, sizeof(msg) - 1, FMT, __VA_ARGS__);                  \
        bohStateEmplaceParserError(bohGlobalStateGet(), LINE, COLUMN, msg); \
    }


const char* bohParsOperatorToStr(bohOperator op)
{
    switch (op) {
        case BOH_OP_PLUS: return "+";
        case BOH_OP_PLUS_ASSIGN: return "+=";
        case BOH_OP_MINUS: return "-";
        case BOH_OP_MINUS_ASSIGN: return "-=";
        case BOH_OP_MULT: return "*";
        case BOH_OP_MULT_ASSIGN: return "*=";
        case BOH_OP_DIV: return "/";
        case BOH_OP_DIV_ASSIGN: return "/=";
        case BOH_OP_MOD: return "%";
        case BOH_OP_MOD_ASSIGN: return "%=";
        case BOH_OP_NOT: return "!";
        case BOH_OP_GREATER: return ">";
        case BOH_OP_LESS: return "<";
        case BOH_OP_NOT_EQUAL: return "!=";
        case BOH_OP_GEQUAL: return ">=";
        case BOH_OP_LEQUAL: return "<=";
        case BOH_OP_EQUAL: return "==";
        case BOH_OP_BITWISE_AND: return "&";
        case BOH_OP_BITWISE_AND_ASSIGN: return "&=";
        case BOH_OP_BITWISE_OR: return "|";
        case BOH_OP_BITWISE_OR_ASSIGN: return "|=";
        case BOH_OP_BITWISE_XOR: return "^";
        case BOH_OP_BITWISE_XOR_ASSIGN: return "^=";
        case BOH_OP_BITWISE_NOT: return "~";
        case BOH_OP_BITWISE_NOT_ASSIGN: return "~=";
        case BOH_OP_BITWISE_RSHIFT: return ">>";
        case BOH_OP_BITWISE_RSHIFT_ASSIGN: return ">>=";
        case BOH_OP_BITWISE_LSHIFT: return "<<";
        case BOH_OP_BITWISE_LSHIFT_ASSIGN: return "<<=";
        case BOH_OP_AND: return "and";
        case BOH_OP_OR: return "or";
        default:
            assert(false && "Invalid operator type");
            return NULL;
    }
}


bool bohParsIsBitwiseOperator(bohOperator op)
{
    return 
        op == BOH_OP_BITWISE_AND || 
        op == BOH_OP_BITWISE_AND_ASSIGN || 
        op == BOH_OP_BITWISE_OR || 
        op == BOH_OP_BITWISE_OR_ASSIGN || 
        op == BOH_OP_BITWISE_XOR || 
        op == BOH_OP_BITWISE_XOR_ASSIGN ||
        op == BOH_OP_BITWISE_NOT ||
        op == BOH_OP_BITWISE_NOT_ASSIGN ||
        op == BOH_OP_BITWISE_RSHIFT ||
        op == BOH_OP_BITWISE_RSHIFT_ASSIGN ||
        op == BOH_OP_BITWISE_LSHIFT ||
        op == BOH_OP_BITWISE_LSHIFT_ASSIGN;
}


static bohOperator parsTokenTypeToOperator(bohTokenType tokenType)
{
    switch (tokenType) {
        case BOH_TOKEN_TYPE_PLUS:                   return BOH_OP_PLUS;
        case BOH_TOKEN_TYPE_PLUS_ASSIGN:            return BOH_OP_PLUS_ASSIGN;
        case BOH_TOKEN_TYPE_MINUS:                  return BOH_OP_MINUS;
        case BOH_TOKEN_TYPE_MINUS_ASSIGN:           return BOH_OP_MINUS_ASSIGN;
        case BOH_TOKEN_TYPE_MULT:                   return BOH_OP_MULT;
        case BOH_TOKEN_TYPE_MULT_ASSIGN:            return BOH_OP_MULT_ASSIGN;
        case BOH_TOKEN_TYPE_DIV:                    return BOH_OP_DIV;
        case BOH_TOKEN_TYPE_DIV_ASSIGN:             return BOH_OP_DIV_ASSIGN;
        case BOH_TOKEN_TYPE_MOD:                    return BOH_OP_MOD;
        case BOH_TOKEN_TYPE_MOD_ASSIGN:             return BOH_OP_MOD_ASSIGN;
        case BOH_TOKEN_TYPE_NOT:                    return BOH_OP_NOT;
        case BOH_TOKEN_TYPE_GREATER:                return BOH_OP_GREATER;
        case BOH_TOKEN_TYPE_LESS:                   return BOH_OP_LESS;
        case BOH_TOKEN_TYPE_NOT_EQUAL:              return BOH_OP_NOT_EQUAL;
        case BOH_TOKEN_TYPE_GEQUAL:                 return BOH_OP_GEQUAL;
        case BOH_TOKEN_TYPE_LEQUAL:                 return BOH_OP_LEQUAL;
        case BOH_TOKEN_TYPE_EQUAL:                  return BOH_OP_EQUAL;
        case BOH_TOKEN_TYPE_BITWISE_AND:            return BOH_OP_BITWISE_AND;
        case BOH_TOKEN_TYPE_BITWISE_AND_ASSIGN:     return BOH_OP_BITWISE_AND_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_OR:             return BOH_OP_BITWISE_OR;
        case BOH_TOKEN_TYPE_BITWISE_OR_ASSIGN:      return BOH_OP_BITWISE_OR_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_XOR:            return BOH_OP_BITWISE_XOR;
        case BOH_TOKEN_TYPE_BITWISE_XOR_ASSIGN:     return BOH_OP_BITWISE_XOR_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_NOT:            return BOH_OP_BITWISE_NOT;
        case BOH_TOKEN_TYPE_BITWISE_NOT_ASSIGN:     return BOH_OP_BITWISE_NOT_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_RSHIFT:         return BOH_OP_BITWISE_RSHIFT;
        case BOH_TOKEN_TYPE_BITWISE_RSHIFT_ASSIGN:  return BOH_OP_BITWISE_RSHIFT_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_LSHIFT:         return BOH_OP_BITWISE_LSHIFT;
        case BOH_TOKEN_TYPE_BITWISE_LSHIFT_ASSIGN:  return BOH_OP_BITWISE_LSHIFT_ASSIGN;
        case BOH_TOKEN_TYPE_AND:                    return BOH_OP_AND;
        case BOH_TOKEN_TYPE_OR:                     return BOH_OP_OR;
        default:                                    return BOH_OP_UNKNOWN;
    }
}


static const bohToken* parsPeekCurrToken(const bohParser* pParser)
{
    assert(pParser);

    return pParser->currTokenIdx < bohDynArrayGetSize(pParser->pTokenStorage) ? 
        bohDynArrayAtConst(pParser->pTokenStorage, pParser->currTokenIdx) : NULL;
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


static bohAstNode* parsExpr(bohParser* pParser);


// <primary> = <integer> | <float> | <string> | '(' <expr> ')' 
static bohAstNode* parsPrimary(bohParser* pParser)
{
    assert(pParser);

    const bohToken* pCurrToken = parsPeekCurrToken(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_TRUE)) {
        return bohAstNodeCreateNumberI64(1, pCurrToken->line, pCurrToken->column);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FALSE)) {
        return bohAstNodeCreateNumberI64(0, pCurrToken->line, pCurrToken->column);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_INTEGER)) {
        const int64_t value = _atoi64(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        return bohAstNodeCreateNumberI64(value, pCurrToken->line, pCurrToken->column);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FLOAT)) {
        const double value = atof(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        return bohAstNodeCreateNumberF64(value, pCurrToken->line, pCurrToken->column);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_STRING)) {
        return bohAstNodeCreateStringViewStringView(parsPeekPrevToken(pParser)->lexeme, pCurrToken->line, pCurrToken->column);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LPAREN)) {
        const uint32_t line = parsPeekCurrToken(pParser)->line;
        const uint32_t column = parsPeekCurrToken(pParser)->column;

        bohAstNode* pExpr = parsExpr(pParser);
        
        if (!parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RPAREN)) {
            BOH_CHECK_PARSER_COND(false, line, column, "missed closing \')\'");
        }

        return pExpr;
    }
    
    return NULL;
}


// <unary> = ('+' | '-' | '~' | '!') <unary> | <primary>
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

        return bohAstNodeCreateUnary(op, pOperand, pOperand->line, pOperand->column);
    }

    return parsPrimary(pParser);
}


// <multiplication> = <unary> (('*' | '/' | '%' | '&' | '|' | '^') <unary>)*
static bohAstNode* parsMultiplication(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsUnary(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MULT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_DIV) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MOD) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_AND) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_XOR)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsUnary(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown term operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg, pOperatorToken->line, pOperatorToken->column);
    }

    return pExpr;
}


static bohAstNode* parsAddition(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsMultiplication(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PLUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_OR)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsMultiplication(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg, pOperatorToken->line, pOperatorToken->column);
    }

    return pExpr;
}


static bohAstNode* parsBitwiseShift(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsAddition(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_LSHIFT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_RSHIFT)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsAddition(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg, pOperatorToken->line, pOperatorToken->column);
    }

    return pExpr;
}


static bohAstNode* parsComparison(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsBitwiseShift(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_GREATER) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LESS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_GEQUAL) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LEQUAL)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsBitwiseShift(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg, pOperatorToken->line, pOperatorToken->column);
    }

    return pExpr;
}


static bohAstNode* parsEquality(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsComparison(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_NOT_EQUAL) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_EQUAL)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsComparison(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg, pOperatorToken->line, pOperatorToken->column);
    }

    return pExpr;
}


static bohAstNode* parsAnd(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsEquality(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_AND)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsEquality(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg, pOperatorToken->line, pOperatorToken->column);
    }

    return pExpr;
}


static bohAstNode* parsOr(bohParser* pParser)
{
    assert(pParser);

    bohAstNode* pExpr = parsAnd(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_OR)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohAstNode* pRightArg = parsAnd(pParser);

        const bohOperator op = parsTokenTypeToOperator(pOperatorToken->type);
        BOH_CHECK_PARSER_COND(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %s", bohStringViewGetData(&pOperatorToken->lexeme));

        pExpr = bohAstNodeCreateBinary(op, pExpr, pRightArg, pOperatorToken->line, pOperatorToken->column);
    }

    return pExpr;
}


static bohAstNode* parsExpr(bohParser* pParser)
{
    return parsOr(pParser);
}


void bohAstNodeDestroy(bohAstNode *pNode)
{
    if (!pNode) {
        return;
    }

    switch (pNode->type) {
        case BOH_AST_NODE_TYPE_NUMBER:
            bohNumberSetI64(&pNode->number, 0);
            break;
        case BOH_AST_NODE_TYPE_STRING:
            bohBoharesStringDestroy(&pNode->string);
            break;
        case BOH_AST_NODE_TYPE_UNARY:
            bohAstNodeFree(&pNode->unary.pNode);
            break;
        case BOH_AST_NODE_TYPE_BINARY:
            bohAstNodeFree(&pNode->binary.pLeftNode);
            bohAstNodeFree(&pNode->binary.pRightNode);
            break;
        default:
            assert(false && "Invalid AST node type");
            break;
    }

    const uint64_t line = pNode->line;
    const uint64_t column = pNode->column;

    memset(pNode, 0, sizeof(bohAstNode));

    pNode->line = line;
    pNode->column = column;
}


void bohAstNodeFree(bohAstNode** ppNode)
{
    if (!ppNode) {
        return;
    }

    bohAstNodeDestroy(*ppNode);
    free(*ppNode);

    ppNode = NULL;
}


bohAstNode* bohAstNodeCreate(uint64_t line, uint64_t column)
{
    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    memset(pNode, 0, sizeof(bohAstNode));
    pNode->line = line;
    pNode->column = column;

    return pNode;
}


bohAstNode* bohAstNodeCreateNumberI64(int64_t value, uint64_t line, uint64_t column)
{
    bohAstNode* pNode = bohAstNodeCreate(line, column);
    bohAstNodeSetNumberI64(pNode, value);

    return pNode;
}


bohAstNode* bohAstNodeCreateNumberF64(double value, uint64_t line, uint64_t column)
{
    bohAstNode* pNode = bohAstNodeCreate(line, column);
    bohAstNodeSetNumberF64(pNode, value);

    return pNode;
}


bohAstNode* bohAstNodeCreateString(const char* pCStr, uint64_t line, uint64_t column)
{
    assert(pCStr);

    bohAstNode* pNode = bohAstNodeCreate(line, column);
    bohAstNodeSetStringCStr(pNode, pCStr);

    return pNode;
}


bohAstNode* bohAstNodeCreateStringStringView(bohStringView strView, uint64_t line, uint64_t column)
{
    return bohAstNodeCreateStringStringViewPtr(&strView, line, column);
}


bohAstNode* bohAstNodeCreateStringStringViewPtr(const bohStringView* pStrView, uint64_t line, uint64_t column)
{
    assert(pStrView);

    bohAstNode* pNode = bohAstNodeCreate(line, column);
    bohAstNodeSetStringStringViewPtr(pNode, pStrView);

    return pNode;
}


bohAstNode* bohAstNodeCreateStringViewStringView(bohStringView strView, uint64_t line, uint64_t column)
{
    return bohAstNodeCreateStringViewStringViewPtr(&strView, line, column);
}


bohAstNode* bohAstNodeCreateStringViewStringViewPtr(const bohStringView* pStrView, uint64_t line, uint64_t column)
{
    assert(pStrView);

    bohAstNode* pNode = bohAstNodeCreate(line, column);
    bohAstNodeSetStringViewStringViewPtr(pNode, pStrView);

    return pNode;
}


bohAstNode* bohAstNodeCreateUnary(bohOperator op, bohAstNode* pArg, uint64_t line, uint64_t column)
{
    assert(pArg);

    bohAstNode* pNode = bohAstNodeCreate(line, column);
    bohAstNodeSetUnary(pNode, op, pArg);

    return pNode;
}


bohAstNode* bohAstNodeCreateBinary(bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg, uint64_t line, uint64_t column)
{
    assert(pLeftArg);
    assert(pRightArg);

    bohAstNode* pNode = bohAstNodeCreate(line, column);
    bohAstNodeSetBinary(pNode, op, pLeftArg, pRightArg);

    return pNode;
}


bool bohAstNodeIsNumber(const bohAstNode* pNode)
{
    assert(pNode);
    return pNode->type == BOH_AST_NODE_TYPE_NUMBER;
}


bool bohAstNodeIsNumberI64(const bohAstNode* pNode)
{
    assert(pNode);
    return bohAstNodeIsNumber(pNode) && bohNumberIsI64(&pNode->number);
}


bool bohAstNodeIsNumberF64(const bohAstNode *pNode)
{
    assert(pNode);
    return bohAstNodeIsNumber(pNode) && bohNumberIsF64(&pNode->number);
}


bool bohAstNodeIsString(const bohAstNode* pNode)
{
    assert(pNode);
    return pNode->type == BOH_AST_NODE_TYPE_STRING;
}


bool bohAstNodeIsUnary(const bohAstNode* pNode)
{
    assert(pNode);
    return pNode->type == BOH_AST_NODE_TYPE_UNARY;
}


bool bohAstNodeIsBinary(const bohAstNode* pNode)
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


int64_t bohAstNodeGetNumberI64(const bohAstNode* pNode)
{
    assert(pNode);
    assert(bohAstNodeIsNumberI64(pNode));

    return bohNumberGetI64(&pNode->number);
}


double bohAstNodeGetNumberF64(const bohAstNode *pNode)
{
    assert(pNode);
    assert(bohAstNodeIsNumberF64(pNode));

    return bohNumberGetF64(&pNode->number);
}


const bohBoharesString* bohAstNodeGetString(const bohAstNode* pNode)
{
    assert(pNode);
    assert(bohAstNodeIsString(pNode));

    return &pNode->string;
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


bohAstNode* bohAstNodeSetNumberI64(bohAstNode* pNode, int64_t value)
{
    assert(pNode);

    bohAstNodeDestroy(pNode);

    pNode->type = BOH_AST_NODE_TYPE_NUMBER;
    pNode->number = bohNumberCreateI64(value);

    return pNode;
}


bohAstNode* bohAstNodeSetNumberF64(bohAstNode* pNode, double value)
{
    assert(pNode);

    bohAstNodeDestroy(pNode);
    
    pNode->type = BOH_AST_NODE_TYPE_NUMBER;
    pNode->number = bohNumberCreateF64(value);

    return pNode;
}


bohAstNode* bohAstNodeSetStringCStr(bohAstNode* pNode, const char* pCStr)
{
    assert(pNode);
    assert(pCStr);

    bohAstNodeDestroy(pNode);
    
    pNode->type = BOH_AST_NODE_TYPE_STRING;
    pNode->string = bohBoharesStringCreateStringStringView(bohStringViewCreateCStr(pCStr));

    return pNode;
}


bohAstNode* bohAstNodeSetStringString(bohAstNode* pNode, const bohString* pString)
{
    return bohAstNodeSetStringCStr(pNode, bohStringGetCStr(pString));
}


bohAstNode* bohAstNodeSetStringStringViewPtr(bohAstNode* pNode, const bohStringView* pStrView)
{
    assert(pNode);
    assert(pStrView);

    bohAstNodeDestroy(pNode);
    
    pNode->type = BOH_AST_NODE_TYPE_STRING;
    pNode->string = bohBoharesStringCreateStringStringViewPtr(pStrView);

    return pNode;
}


bohAstNode* bohAstNodeSetStringViewStringView(bohAstNode* pNode, bohStringView strView)
{
    return bohAstNodeSetStringViewStringViewPtr(pNode, &strView);
}


bohAstNode* bohAstNodeSetStringViewStringViewPtr(bohAstNode* pNode, const bohStringView* pStrView)
{
    assert(pNode);
    assert(pStrView);

    bohAstNodeDestroy(pNode);
    
    pNode->type = BOH_AST_NODE_TYPE_STRING;
    pNode->string = bohBoharesStringCreateStringViewStringViewPtr(pStrView);

    return pNode;
}


bohAstNode* bohAstNodeSetUnary(bohAstNode* pNode, bohOperator op, bohAstNode* pArg)
{
    assert(pNode);
    assert(pArg);

    bohAstNodeDestroy(pNode);
    
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

    bohAstNodeDestroy(pNode);
    
    pNode->type = BOH_AST_NODE_TYPE_BINARY;
    pNode->binary.op = op;
    pNode->binary.pLeftNode = pLeftArg;
    pNode->binary.pRightNode = pRightArg;

    return pNode;
}


uint64_t bohAstNodeGetLine(const bohAstNode* pNode)
{
    assert(pNode);
    return pNode->line;
}


uint64_t bohAstNodeGetColumn(const bohAstNode* pNode)
{
    assert(pNode);
    return pNode->column;
}


const char* bohAstNodeTypeToStr(const bohAstNode* pNode)
{
    assert(pNode);

    switch (pNode->type) {
        case BOH_AST_NODE_TYPE_NUMBER: return "NUMBER NODE";
        case BOH_AST_NODE_TYPE_STRING: return "STRING NODE";
        case BOH_AST_NODE_TYPE_UNARY:  return "UNARY NODE";
        case BOH_AST_NODE_TYPE_BINARY: return "BINARY NODE";
        default: return "UNKNOWN AST NODE TYPE";
    }
}


void bohAstDestroy(bohAST* pAST)
{
    assert(pAST);
    bohAstNodeFree(&pAST->pRoot);
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
    bohAST ast;
    ast.pRoot = parsExpr(pParser);

    return ast;
}
