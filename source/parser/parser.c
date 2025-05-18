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
        default:
            assert(false && "Invalid operator type");
            return NULL;
    }
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
        default:                                    return BOH_OP_UNKNOWN;
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


// <primary> = <integer> | <float> | <bool> | <string> | '(' <expr> ')' 
static bohAstNode* parsPrimary(bohParser* pParser)
{
    assert(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_TRUE)) {
        return bohAstNodeCreateNumberI64(1);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FALSE)) {
        return bohAstNodeCreateNumberI64(0);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_INTEGER)) {
        const int64_t value = _atoi64(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        return bohAstNodeCreateNumberI64(value);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FLOAT)) {
        const double value = atof(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        return bohAstNodeCreateNumberF64(value);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_STRING)) {
        return bohAstNodeCreateStringStringViewPtr(&parsPeekPrevToken(pParser)->lexeme);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LPAREN)) {
        const uint32_t line = parsPeekCurrToken(pParser)->line;
        const uint32_t column = parsPeekCurrToken(pParser)->column;

        bohAstNode* pExpr = parsAddition(pParser);
        
        if (!parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RPAREN)) {
            BOH_CHECK_PARSER_COND(false, line, column, "missed closing \')\'");
        }

        return pExpr;
    }

    const bohToken* pCurrToken = parsPeekCurrToken(pParser);
    BOH_CHECK_PARSER_COND(false, pCurrToken->line, pCurrToken->column, "unknown primary token type: %.*s", 
        bohStringViewGetSize(&pCurrToken->lexeme), bohStringViewGetData(&pCurrToken->lexeme));
    
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

        return bohAstNodeCreateUnary(op, pOperand);
    }

    return parsPrimary(pParser);
}


// <multiplication> = <unary> (('*' | '/' | '%' | '&' | '|') <unary>)*
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
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_OR)
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


bohAstNode* bohAstNodeCreateNumberI64(int64_t value)
{
    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    memset(pNode, 0, sizeof(bohAstNode));
    bohAstNodeSetNumberI64(pNode, value);

    return pNode;
}


bohAstNode* bohAstNodeCreateNumberF64(double value)
{
    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    memset(pNode, 0, sizeof(bohAstNode));
    bohAstNodeSetNumberF64(pNode, value);

    return pNode;
}


bohAstNode* bohAstNodeCreateString(const char* pCStr)
{
    assert(pCStr);

    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    memset(pNode, 0, sizeof(bohAstNode));
    bohAstNodeSetStringCStr(pNode, pCStr);

    return pNode;
}


bohAstNode* bohAstNodeCreateStringStringView(bohStringView strView)
{
    return bohAstNodeCreateStringStringViewPtr(&strView);
}


bohAstNode* bohAstNodeCreateStringStringViewPtr(const bohStringView* pStrView)
{
    assert(pStrView);

    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    memset(pNode, 0, sizeof(bohAstNode));
    bohAstNodeSetStringStringViewPtr(pNode, pStrView);

    return pNode;
}


bohAstNode* bohAstNodeCreateUnary(bohOperator op, bohAstNode* pArg)
{
    assert(pArg);

    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    memset(pNode, 0, sizeof(bohAstNode));
    bohAstNodeSetUnary(pNode, op, pArg);

    return pNode;
}


bohAstNode* bohAstNodeCreateBinary(bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg)
{
    assert(pLeftArg);
    assert(pRightArg);

    bohAstNode* pNode = (bohAstNode*)malloc(sizeof(bohAstNode));
    assert(pNode);

    memset(pNode, 0, sizeof(bohAstNode));
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
    bohAST ast = { NULL };

    if (!bohDynArrayIsEmpty(pParser->pTokenStorage)) {
        ast.pRoot = parsExpr(pParser);
    }

    return ast;
}
