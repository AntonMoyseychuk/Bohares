#pragma once

#include "types.h"


typedef enum ExprOperator
{
    BOH_OP_UNKNOWN,

    BOH_OP_PLUS,
    BOH_OP_PLUS_ASSIGN,
    BOH_OP_MINUS,
    BOH_OP_MINUS_ASSIGN,
    BOH_OP_MULT,
    BOH_OP_MULT_ASSIGN,
    BOH_OP_DIV,
    BOH_OP_DIV_ASSIGN,
    BOH_OP_MOD,
    BOH_OP_MOD_ASSIGN,
    BOH_OP_NOT,
    BOH_OP_GREATER,
    BOH_OP_LESS,
    BOH_OP_NOT_EQUAL,
    BOH_OP_GEQUAL,
    BOH_OP_LEQUAL,
    BOH_OP_EQUAL,

    BOH_OP_BITWISE_AND,
    BOH_OP_BITWISE_AND_ASSIGN,
    BOH_OP_BITWISE_OR,
    BOH_OP_BITWISE_OR_ASSIGN,
    BOH_OP_BITWISE_XOR,
    BOH_OP_BITWISE_XOR_ASSIGN,
    BOH_OP_BITWISE_NOT,
    BOH_OP_BITWISE_NOT_ASSIGN,
    BOH_OP_BITWISE_RSHIFT,
    BOH_OP_BITWISE_RSHIFT_ASSIGN,
    BOH_OP_BITWISE_LSHIFT,
    BOH_OP_BITWISE_LSHIFT_ASSIGN,

    BOH_OP_AND,
    BOH_OP_OR
} bohExprOperator;


const char* bohParsExprOperatorToStr(bohExprOperator op);
bool bohParsIsBitwiseExprOperator(bohExprOperator op);


typedef struct Expr bohExpr;
typedef struct Stmt bohStmt;
typedef struct AST bohAST;

typedef uint32_t bohExprIdx;
#define BOH_EXPR_IDX_INVALID 0xffffffff

typedef uint32_t bohStmtIdx;
#define BOH_STMT_IDX_INVALID 0xffffffff


typedef enum ValueExprType
{
    BOH_VALUE_EXPR_TYPE_NUMBER,
    BOH_VALUE_EXPR_TYPE_STRING
} bohValueExprType;


typedef struct ValueExpr
{
    union 
    {
        bohNumber number;
        bohBoharesString string;
    };

    bohValueExprType type;
} bohValueExpr;


void bohValueExprDestroy(bohValueExpr* pExpr);

bohValueExpr bohValueExprCreate(void);
bohValueExpr bohValueExprCreateNumber(bohNumber number);
bohValueExpr bohValueExprCreateNumberNumberPtr(const bohNumber* pNumber);
bohValueExpr bohValueExprCreateStringStringPtr(const bohBoharesString* pString);
bohValueExpr bohValueExprCreateStringStringMove(bohBoharesString* pString);

bool bohValueExprIsNumber(const bohValueExpr* pExpr);
bool bohValueExprIsNumberI64(const bohValueExpr* pExpr);
bool bohValueExprIsNumberF64(const bohValueExpr* pExpr);
bool bohValueExprIsString(const bohValueExpr* pExpr);

const bohNumber* bohValueExprGetNumber(const bohValueExpr* pExpr);
const bohBoharesString* bohValueExprGetString(const bohValueExpr* pExpr);

void bohValueExprSetNumber(bohValueExpr* pExpr, bohNumber number);
void bohValueExprSetNumberNumberPtr(bohValueExpr* pExpr, const bohNumber* pNumber);
void bohValueExprSetStringStringPtr(bohValueExpr* pExpr, const bohBoharesString* pString);
void bohValueExprSetStringStringMove(bohValueExpr* pExpr, bohBoharesString* pString);

bohValueExpr* bohValueExprAssign(bohValueExpr* pDst, const bohValueExpr* pSrc);
bohValueExpr* bohValueExprMove(bohValueExpr* pDst, bohValueExpr* pSrc);


typedef struct UnaryExpr
{
    bohExprIdx exprIdx;
    bohExprOperator op;
} bohUnaryExpr;


void bohUnaryExprDestroy(bohUnaryExpr* pExpr);

bohUnaryExpr bohUnaryExprCreateOpExpr(bohExprOperator op, bohExprIdx exprIdx);

bohExprIdx bohUnaryExprGetExprIdx(const bohUnaryExpr* pExpr);
bohExprOperator bohUnaryExprGetOp(const bohUnaryExpr* pExpr);

bohUnaryExpr* bohUnaryExprAssign(bohUnaryExpr* pDst, const bohUnaryExpr* pSrc);
bohUnaryExpr* bohUnaryExprMove(bohUnaryExpr* pDst, bohUnaryExpr* pSrc);


typedef struct BinaryExpr
{
    bohExprIdx leftExprIdx;
    bohExprIdx rightExprIdx;
    bohExprOperator op;
} bohBinaryExpr;


void bohBinaryExprDestroy(bohBinaryExpr* pExpr);

bohBinaryExpr bohBinaryExprCreateOpExpr(bohExprOperator op, bohExprIdx leftExprIdx, bohExprIdx rightExprIdx);

bohExprIdx bohBinaryExprGetLeftExprIdx(const bohBinaryExpr* pExpr);
bohExprIdx bohBinaryExprGetRightExprIdx(const bohBinaryExpr* pExpr);
bohExprOperator bohBinaryExprGetOp(const bohBinaryExpr* pExpr);

bohBinaryExpr* bohBinaryExprAssign(bohBinaryExpr* pDst, const bohBinaryExpr* pSrc);
bohBinaryExpr* bohBinaryExprMove(bohBinaryExpr* pDst, bohBinaryExpr* pSrc);


typedef enum ExprType
{
    BOH_EXPR_TYPE_VALUE,
    BOH_EXPR_TYPE_UNARY,
    BOH_EXPR_TYPE_BINARY
} bohExprType;


typedef struct Expr
{
    union
    {
        bohValueExpr valueExpr;
        bohUnaryExpr unaryExpr;
        bohBinaryExpr binaryExpr;
    };
    
    bohExprType type;
} bohExpr;


void bohExprDestroy(bohExpr* pExpr);

bohExpr bohExprCreate(void);

bohExpr bohExprCreateNumberValueExpr(bohNumber number);
bohExpr bohExprCreateNumberValueExprPtr(const bohNumber* pNumber);
bohExpr bohExprCreateStringValueExpr(const bohBoharesString* pString);
bohExpr bohExprCreateStringValueExprMove(bohBoharesString* pString);

bohExpr bohExprCreateUnaryExpr(bohExprOperator op, bohExprIdx exprIdx);
bohExpr bohExprCreateBinaryExpr(bohExprOperator op, bohExprIdx leftExprIdx, bohExprIdx rightExprIdx);

bool bohExprIsValueExpr(const bohExpr* pExpr);
bool bohExprIsUnaryExpr(const bohExpr* pExpr);
bool bohExprIsBinaryExpr(const bohExpr* pExpr);

const bohValueExpr* bohExprGetValueExpr(const bohExpr* pExpr);
const bohUnaryExpr* bohExprGetUnaryExpr(const bohExpr* pExpr);
const bohBinaryExpr* bohExprGetBinaryExpr(const bohExpr* pExpr);

bohExpr* bohExprAssign(bohExpr* pDst, const bohExpr* pSrc);
bohExpr* bohExprMove(bohExpr* pDst, bohExpr* pSrc);


typedef struct RawExprStmt
{
    bohExprIdx exprIdx;
} bohRawExprStmt;


void bohRawExprStmtDestroy(bohRawExprStmt* pStmt);

bohRawExprStmt bohRawExprStmtCreateExprIdx(bohExprIdx exprIdx);

bohExprIdx bohRawExprStmtGetExprIdx(const bohRawExprStmt* pStmt);

bohRawExprStmt* bohRawExprStmtAssign(bohRawExprStmt* pDst, const bohRawExprStmt* pSrc);
bohRawExprStmt* bohRawExprStmtMove(bohRawExprStmt* pDst, bohRawExprStmt* pSrc);


typedef struct PrintStmt
{
    bohStmtIdx stmtIdx;
} bohPrintStmt;


void bohPrintStmtDestroy(bohPrintStmt* pStmt);

bohPrintStmt bohPrintStmtCreateStmtIdx(bohStmtIdx stmtIdx);

bohStmtIdx bohPrintStmtGetStmtIdx(const bohPrintStmt* pStmt);

bohPrintStmt* bohPrintStmtAssign(bohPrintStmt* pDst, const bohPrintStmt* pSrc);
bohPrintStmt* bohPrintStmtMove(bohPrintStmt* pDst, bohPrintStmt* pSrc);


typedef enum StmtType
{
    BOH_STMT_TYPE_EMPTY,
    BOH_STMT_TYPE_RAW_EXPR,
    BOH_STMT_TYPE_PRINT,
} bohStmtType;


typedef struct Stmt
{
    union
    {
        bohRawExprStmt rawExpr;
        bohPrintStmt printStmt;
    };
    
    bohStmtType type;
} bohStmt;


void bohStmtDestroy(bohStmt* pStmt);

bohStmt bohStmtCreate(void);
bohStmt bohStmtCreateRawExpr(bohExprIdx exprIdx);
bohStmt bohStmtCreatePrint(bohStmtIdx stmtIdx);

bohStmtType bohStmtGetType(const bohStmt* pStmt);

bool bohStmtIsEmpty(const bohStmt* pStmt);
bool bohStmtIsRawExpr(const bohStmt* pStmt);
bool bohStmtIsPrint(const bohStmt* pStmt);

const bohRawExprStmt* bohStmtGetRawExpr(const bohStmt* pStmt);
const bohPrintStmt* bohStmtGetPrint(const bohStmt* pStmt);

bohStmt* bohStmtAssign(bohStmt* pDst, const bohStmt* pSrc);
bohStmt* bohStmtMove(bohStmt* pDst, bohStmt* pSrc);


typedef bohDynArray bohStmtStorage;
typedef bohDynArray bohExprStorage;

typedef struct AST
{
    bohStmtStorage stmts;
    bohExprStorage exprs;
} bohAST;


void bohAstDestroy(bohAST* pAST);


typedef struct StmtStorageCreateInfo
{
    bohDynArrElemDefConstr pStmtDefConstr; 
    bohDynArrElemDestr pStmtDestr;
    bohDynArrElemCopyFunc pStmtCopyFunc;
} bohStmtStorageCreateInfo;


typedef struct ExprStorageCreateInfo
{
    bohDynArrElemDefConstr pExprDefConstr; 
    bohDynArrElemDestr pExprDestr;
    bohDynArrElemCopyFunc pExprCopyFunc;
} bohExprStorageCreateInfo;


bohAST bohASTCreate(
    const bohStmtStorageCreateInfo* pStmtStorageCreateInfo, 
    const bohExprStorageCreateInfo* pExprStorageCreateInfo
);

bool bohAstIsEmpty(const bohAST* pAST);


typedef bohDynArray bohTokenStorage;

typedef struct Parser
{
    const bohTokenStorage* pTokenStorage;
    size_t currTokenIdx;

    bohAST ast;
} bohParser;


bohParser bohParserCreate(const bohTokenStorage* pTokenStorage);
void bohParserDestroy(bohParser* pParser);

const bohAST* bohParserGetAST(const bohParser* pParser);

void bohParserParse(bohParser* pParser);
