#pragma once

#include "utils/memory/arena_allocator.h"
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


typedef enum ValueExprType
{
    BOH_VALUE_EXPR_TYPE_NUMBER,
    BOH_VALUE_EXPR_TYPE_STRING
} bohValueExprType;


typedef struct ValueExpr
{
    bohValueExprType type;
    
    union 
    {
        bohNumber number;
        bohBoharesString string;
    };
} bohValueExpr;


void bohValueExprDestroy(bohValueExpr* pExpr);

// NOTE: *CreateInPlace functions don't call destroy function
void bohValueExprCreateInPlace(bohValueExpr* pExpr);
void bohValueExprCreateNumberInPlace(bohValueExpr* pExpr, bohNumber number);
void bohValueExprCreateNumberNumberPtrInPlace(bohValueExpr* pExpr, const bohNumber* pNumber);
void bohValueExprCreateStringStringPtrInPlace(bohValueExpr* pExpr, const bohBoharesString* pString);
void bohValueExprCreateStringStringMoveInPlace(bohValueExpr* pExpr, bohBoharesString* pString);

bool bohValueExprIsNumber(const bohValueExpr* pExpr);
bool bohValueExprIsNumberI64(const bohValueExpr* pExpr);
bool bohValueExprIsNumberF64(const bohValueExpr* pExpr);
bool bohValueExprIsString(const bohValueExpr* pExpr);

bohValueExprType bohValueExprGetType(const bohValueExpr* pExpr);
const bohNumber* bohValueExprGetNumber(const bohValueExpr* pExpr);
const bohBoharesString* bohValueExprGetString(const bohValueExpr* pExpr);

bohValueExpr* bohValueExprAssign(bohValueExpr* pDst, const bohValueExpr* pSrc);
bohValueExpr* bohValueExprMove(bohValueExpr* pDst, bohValueExpr* pSrc);


typedef struct UnaryExpr
{
    const bohExpr* pExpr;
    bohExprOperator op;
} bohUnaryExpr;


void bohUnaryExprDestroy(bohUnaryExpr* pExpr);

// NOTE: *CreateInPlace functions don't call destroy function
void bohUnaryExprCreateOpExprInPlace(bohUnaryExpr* pExpr, bohExprOperator op, const bohExpr* pArgExpr);

const bohExpr* bohUnaryExprGetExpr(const bohUnaryExpr* pExpr);
bohExprOperator bohUnaryExprGetOperator(const bohUnaryExpr* pExpr);

bohUnaryExpr* bohUnaryExprAssign(bohUnaryExpr* pDst, const bohUnaryExpr* pSrc);
bohUnaryExpr* bohUnaryExprMove(bohUnaryExpr* pDst, bohUnaryExpr* pSrc);


typedef struct BinaryExpr
{
    const bohExpr* pLeftExpr;
    const bohExpr* pRightExpr;
    bohExprOperator op;
} bohBinaryExpr;


void bohBinaryExprDestroy(bohBinaryExpr* pExpr);

// NOTE: *CreateInPlace functions don't call destroy function
void bohBinaryExprCreateOpExprInPlace(bohBinaryExpr* pExpr, bohExprOperator op, const bohExpr* pLeftExpr, const bohExpr* pRightExpr);

const bohExpr* bohBinaryExprGetLeftExpr(const bohBinaryExpr* pExpr);
const bohExpr* bohBinaryExprGetRightExpr(const bohBinaryExpr* pExpr);
bohExprOperator bohBinaryExprGetOperator(const bohBinaryExpr* pExpr);

bohBinaryExpr* bohBinaryExprAssign(bohBinaryExpr* pDst, const bohBinaryExpr* pSrc);
bohBinaryExpr* bohBinaryExprMove(bohBinaryExpr* pDst, bohBinaryExpr* pSrc);


typedef struct IdentifierExpr
{
    bohStringView name;
} bohIdentifierExpr;


void bohIdentifierExprDestroy(bohIdentifierExpr* pExpr);

// NOTE: *CreateInPlace functions don't call destroy function
void bohIdentifierExprCreateInPlace(bohIdentifierExpr* pExpr, const bohStringView* pName);

const bohStringView* bohIdentifierExprGetName(const bohIdentifierExpr* pExpr);

bohIdentifierExpr* bohIdentifierExprAssign(bohIdentifierExpr* pDst, const bohIdentifierExpr* pSrc);
bohIdentifierExpr* bohIdentifierExprMove(bohIdentifierExpr* pDst, bohIdentifierExpr* pSrc);


typedef enum ExprType
{
    BOH_EXPR_TYPE_VALUE,
    BOH_EXPR_TYPE_UNARY,
    BOH_EXPR_TYPE_BINARY,
    BOH_EXPR_TYPE_IDENTIFIER,
} bohExprType;


typedef struct Expr
{
    bohExprType type;

    union
    {
        bohValueExpr valueExpr;
        bohUnaryExpr unaryExpr;
        bohBinaryExpr binaryExpr;
        bohIdentifierExpr identifierExpr;
    };

    bohLineNmb line;
    bohColumnNmb column;
} bohExpr;


void bohExprDestroy(bohExpr* pExpr);

// NOTE: *CreateInPlace functions don't call destroy function
void bohExprCreateNumberValueExprInPlace(bohExpr* pExpr, bohNumber number, bohLineNmb line, bohColumnNmb column);
void bohExprCreateNumberValueExprPtrInPlace(bohExpr* pExpr, const bohNumber* pNumber, bohLineNmb line, bohColumnNmb column);
void bohExprCreateStringValueExprInPlace(bohExpr* pExpr, const bohBoharesString* pString, bohLineNmb line, bohColumnNmb column);
void bohExprCreateStringValueExprMoveInPlace(bohExpr* pExpr, bohBoharesString* pString, bohLineNmb line, bohColumnNmb column);
void bohExprCreateUnaryExprInPlace(bohExpr* pExpr, bohExprOperator op, bohExpr* pArgExpr, bohLineNmb line, bohColumnNmb column);
void bohExprCreateBinaryExprInPlace(bohExpr* pExpr, bohExprOperator op, bohExpr* pLeftArgExpr, bohExpr* pRightArgExpr, bohLineNmb line, bohColumnNmb column);
void bohExprCreateIdentifierExprInPlace(bohExpr* pExpr, const bohStringView* pName, bohLineNmb line, bohColumnNmb column);

bool bohExprIsValueExpr(const bohExpr* pExpr);
bool bohExprIsUnaryExpr(const bohExpr* pExpr);
bool bohExprIsBinaryExpr(const bohExpr* pExpr);
bool bohExprIsIdentifierExpr(const bohExpr* pExpr);

const bohValueExpr* bohExprGetValueExpr(const bohExpr* pExpr);
const bohUnaryExpr* bohExprGetUnaryExpr(const bohExpr* pExpr);
const bohBinaryExpr* bohExprGetBinaryExpr(const bohExpr* pExpr);
const bohIdentifierExpr* bohExprGetIdentifierExpr(const bohExpr* pExpr);

bohExprType bohExprGetType(const bohExpr* pExpr);
bohLineNmb bohExprGetLine(const bohExpr* pExpr);
bohColumnNmb bohExprGetColumn(const bohExpr* pExpr);

bohExpr* bohExprAssign(bohExpr* pDst, const bohExpr* pSrc);
bohExpr* bohExprMove(bohExpr* pDst, bohExpr* pSrc);


typedef struct PrintStmt
{
    const bohExpr* pArgExpr;
} bohPrintStmt;


void bohPrintStmtDestroy(bohPrintStmt* pStmt);

// NOTE: *CreateInPlace functions don't call destroy function
void bohPrintStmtCreateInPlace(bohPrintStmt* pStmt, const bohExpr* pArgExpr);

const bohExpr* bohPrintStmtGetArgExpr(const bohPrintStmt* pStmt);

bohPrintStmt* bohPrintStmtAssign(bohPrintStmt* pDst, const bohPrintStmt* pSrc);
bohPrintStmt* bohPrintStmtMove(bohPrintStmt* pDst, bohPrintStmt* pSrc);


typedef struct IfStmt
{
    const bohExpr* pCondExpr;
    bohDynArray thenStmtPtrs;
    bohDynArray elseStmtPtrs;
} bohIfStmt;


void bohIfStmtDestroy(bohIfStmt* pStmt);

// NOTE: *CreateInPlace functions don't call destroy function
void bohIfStmtCreateInPlace(bohIfStmt* pStmt, const bohExpr* pCondExpr, bohDynArray* pThenStmtPtrs, bohDynArray* pElseStmtPtrs);

const bohExpr* bohIfStmtGetCondExpr(const bohIfStmt* pStmt);
const bohDynArray* bohIfStmtGetThenStmts(const bohIfStmt* pStmt);
const bohDynArray* bohIfStmtGetElseStmts(const bohIfStmt* pStmt);

size_t bohIfStmtGetThenStmtsCount(const bohIfStmt* pStmt);
size_t bohIfStmtGetElseStmtsCount(const bohIfStmt* pStmt);

const bohStmt* bohIfStmtGetThenStmtAt(const bohIfStmt* pStmt, size_t index);
const bohStmt* bohIfStmtGetElseStmtAt(const bohIfStmt* pStmt, size_t index);

bohIfStmt* bohIfStmtAssign(bohIfStmt* pDst, const bohIfStmt* pSrc);
bohIfStmt* bohIfStmtMove(bohIfStmt* pDst, bohIfStmt* pSrc);


typedef struct VarDeclStmt
{
    bohStringView name;
} bohVarDeclStmt;


void bohVarDeclStmtDestroy(bohVarDeclStmt* pStmt);

// NOTE: *CreateInPlace functions don't call destroy function
void bohVarDeclStmtCreateInPlace(bohVarDeclStmt* pStmt, const bohStringView* pName);

const bohStringView* bohVarDeclStmtGetName(const bohVarDeclStmt* pStmt);

bohVarDeclStmt* bohVarDeclStmtAssign(bohVarDeclStmt* pDst, const bohVarDeclStmt* pSrc);
bohVarDeclStmt* bohVarDeclStmtMove(bohVarDeclStmt* pDst, bohVarDeclStmt* pSrc);


typedef struct AssignmentStmt
{
    const bohExpr* pLeft;
    const bohExpr* pRight;
} bohAssignmentStmt;


void bohAssignmentStmtDestroy(bohAssignmentStmt* pStmt);

// NOTE: *CreateInPlace functions don't call destroy function
void bohAssignmentStmtCreateInPlace(bohAssignmentStmt* pStmt, const bohExpr* pLeft, const bohExpr* pRight);

const bohExpr* bohAssignmentStmtGetLeftExpr(const bohAssignmentStmt* pStmt);
const bohExpr* bohAssignmentStmtGetRightExpr(const bohAssignmentStmt* pStmt);

bohAssignmentStmt* bohAssignmentStmtAssign(bohAssignmentStmt* pDst, const bohAssignmentStmt* pSrc);
bohAssignmentStmt* bohAssignmentStmtMove(bohAssignmentStmt* pDst, bohAssignmentStmt* pSrc);


typedef enum StmtType
{
    BOH_STMT_TYPE_EMPTY,
    BOH_STMT_TYPE_PRINT,
    BOH_STMT_TYPE_IF,
    BOH_STMT_TYPE_VAR_DECL,
    BOH_STMT_TYPE_ASSIGNMENT,
} bohStmtType;


typedef struct Stmt
{
    bohStmtType type;

    union
    {
        bohIfStmt ifStmt;
        bohPrintStmt printStmt;
        bohVarDeclStmt varDeclStmt;
        bohAssignmentStmt assignStmt;
    };
    
    bohLineNmb line;
    bohColumnNmb column;
} bohStmt;


void bohStmtDestroy(bohStmt* pStmt);

// NOTE: *CreateInPlace functions don't call destroy function
void bohStmtCreatePrintInPlace(bohStmt* pStmt, const bohExpr* pArgExpr, bohLineNmb line, bohColumnNmb column);
void bohStmtCreateIfInPlace(bohStmt* pStmt, const bohExpr* pCondExpr, bohDynArray* pThenStmtPtrs, bohDynArray* pElseStmtPtrs, bohLineNmb line, bohColumnNmb column);
void bohStmtCreateAssignInPlace(bohStmt* pStmt, const bohExpr* pLeft, const bohExpr* pRight, bohLineNmb line, bohColumnNmb column);
void bohStmtCreateVarDeclInPlace(bohStmt* pStmt, const bohStringView* pName, bohLineNmb line, bohColumnNmb column);

bool bohStmtIsEmpty(const bohStmt* pStmt);
bool bohStmtIsPrint(const bohStmt* pStmt);
bool bohStmtIsIf(const bohStmt* pStmt);
bool bohStmtIsAssignment(const bohStmt* pStmt);
bool bohStmtIsVarDecl(const bohStmt* pStmt);

const bohPrintStmt* bohStmtGetPrint(const bohStmt* pStmt);
const bohIfStmt* bohStmtGetIf(const bohStmt* pStmt);
const bohAssignmentStmt* bohStmtGetAssignment(const bohStmt* pStmt);
const bohVarDeclStmt* bohStmtGetVarDecl(const bohStmt* pStmt);

bohStmt* bohStmtAssign(bohStmt* pDst, const bohStmt* pSrc);
bohStmt* bohStmtMove(bohStmt* pDst, bohStmt* pSrc);

bohStmtType bohStmtGetType(const bohStmt* pStmt);
bohLineNmb bohStmtGetLine(const bohStmt* pStmt);
bohColumnNmb bohStmtGetColumn(const bohStmt* pStmt);


typedef struct AST
{
    bohDynArray stmtPtrsStorage;

    bohArenaAllocator stmtMemArena;
    bohArenaAllocator epxrMemArena;
} bohAST;


void bohAstDestroy(bohAST* pAST);

bohAST bohAstCreate(void);

bohExpr* bohAstAllocateExpr(bohAST* pAst);
bohStmt* bohAstAllocateStmt(bohAST* pAst);

bohStmt** bohAstPushStmtPtr(bohAST* pAst, bohStmt* pStmt);

const bohStmt* bohAstGetStmtByIdx(const bohAST* pAst, size_t index);

size_t bohAstGetStmtCount(const bohAST* pAst);
size_t bohAstGetMemorySize(const bohAST* pAst);


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
