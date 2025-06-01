#pragma once

#include "utils/ds/string.h"
#include "utils/ds/string_view.h"

#include <stdint.h>


typedef enum StringType
{
    BOH_STRING_TYPE_VIEW,  // String literals (example: "Hello World")
    BOH_STRING_TYPE_STRING // (example: var = "Hello World", var2 = var)
} bohStringType;


typedef struct BoharesString
{
    bohStringType type;

    union {
        bohStringView view;
        bohString string;
    };
} bohBoharesString;


bohBoharesString bohBoharesStringCreateStringView(void);
bohBoharesString bohBoharesStringCreateStringViewStringView(bohStringView strView);
bohBoharesString bohBoharesStringCreateStringViewStringViewPtr(const bohStringView* pStrView);

bohBoharesString bohBoharesStringCreateString(void);
bohBoharesString bohBoharesStringCreateStringString(const bohString* pString);
bohBoharesString bohBoharesStringCreateStringStringView(bohStringView strView);
bohBoharesString bohBoharesStringCreateStringStringViewPtr(const bohStringView* pStrView);

void bohBoharesStringDestroy(bohBoharesString* pString);

bohBoharesString* bohBoharesStringAssign(bohBoharesString* pDst, const bohBoharesString* pSrc);
bohBoharesString* bohBoharesStringStringAssignStringView(bohBoharesString* pDst, bohStringView src);
bohBoharesString* bohBoharesStringStringAssignStringViewPtr(bohBoharesString* pDst, const bohStringView* pSrc);
bohBoharesString* bohBoharesStringStringAssignString(bohBoharesString* pDst, const bohString* pSrc);
bohBoharesString* bohBoharesStringStringViewAssignStringView(bohBoharesString* pDst, bohStringView src);
bohBoharesString* bohBoharesStringStringViewAssignStringViewPtr(bohBoharesString* pDst, const bohStringView* pSrc);
bohBoharesString* bohBoharesStringStringViewAssignString(bohBoharesString* pDst, const bohString* pSrc);

bohBoharesString* bohBoharesStringMove(bohBoharesString* pDst, bohBoharesString* pSrc);

bool bohBoharesStringIsStringView(const bohBoharesString* pString);
bool bohBoharesStringIsString(const bohBoharesString* pString);

const bohStringView* bohBoharesStringGetStringView(const bohBoharesString* pString);
const bohString* bohBoharesStringGetString(const bohBoharesString* pString);

const char* bohBoharesStringGetData(const bohBoharesString* pString);
size_t bohBoharesStringGetSize(const bohBoharesString* pString);

char bohBoharesStringAt(const bohBoharesString* pString, size_t index);

bool bohBoharesStringIsEmpty(const bohBoharesString* pString);

int32_t bohBoharesStringCmp(const bohBoharesString* pLeft, const bohBoharesString* pRight);

bool bohBoharesStringEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight);
bool bohBoharesStringNotEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight);
bool bohBoharesStringLess(const bohBoharesString* pLeft, const bohBoharesString* pRight);
bool bohBoharesStringLessEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight);
bool bohBoharesStringGreater(const bohBoharesString* pLeft, const bohBoharesString* pRight);
bool bohBoharesStringGreaterEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight);

bohBoharesString bohBoharesStringAdd(const bohBoharesString* pLeft, const bohBoharesString* pRight);


typedef struct Number bohNumber;


typedef enum NumberType
{
    BOH_NUMBER_TYPE_INTEGER,
    BOH_NUMBER_TYPE_FLOAT
} bohNumberType;


typedef struct Number
{
    bohNumberType type;

    union {
        int64_t i64;
        double  f64;
    };
} bohNumber;


bohNumber bohNumberCreate(void);

bohNumber bohNumberCreateI64(int64_t value);
bohNumber bohNumberCreateF64(double value);

bool bohNumberIsI64(const bohNumber* pNumber);
bool bohNumberIsF64(const bohNumber* pNumber);

bool bohNumberIsIntegral(const bohNumber* pNumber);
bool bohNumberIsFloatingPoint(const bohNumber* pNumber);

int64_t bohNumberGetI64(const bohNumber* pNumber);
double bohNumberGetF64(const bohNumber* pNumber);

void bohNumberSetI64(bohNumber* pNumber, int64_t value);
void bohNumberSetF64(bohNumber* pNumber, double value);

bohNumber* bohNumberAssign(bohNumber* pDst, const bohNumber* pNumber);

bool bohNumberEqual(const bohNumber* pLeft, const bohNumber* pRight);
bool bohNumberNotEqual(const bohNumber* pLeft, const bohNumber* pRight);

bool bohNumberLess(const bohNumber* pLeft, const bohNumber* pRight);
bool bohNumberGreater(const bohNumber* pLeft, const bohNumber* pRight);

bool bohNumberLessEqual(const bohNumber* pLeft, const bohNumber* pRight);
bool bohNumberGreaterEqual(const bohNumber* pLeft, const bohNumber* pRight);

bool bohNumberIsZero(const bohNumber* pNumber);

bohNumber  bohNumberGetOpposite(const bohNumber* pNumber);
bohNumber* bohNumberMakeOpposite(bohNumber* pNumber);

bohNumber  bohNumberGetNegation(const bohNumber* pNumber);
bohNumber* bohNumberMakeNegation(bohNumber* pNumber);

bohNumber  bohNumberAdd(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberAddAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberSub(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberSubAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberMult(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberMultAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberDiv(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberDivAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberMod(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberModAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberGetBitwiseNegation(const bohNumber* pNumber);
bohNumber* bohNumberMakeBitwiseNegation(bohNumber* pNumber);

bohNumber  bohNumberBitwiseAnd(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberBitwiseAndAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberBitwiseOr(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberBitwiseOrAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberBitwiseXor(const bohNumber* pValue, const bohNumber* pBits);
bohNumber* bohNumberBitwiseXorAssign(bohNumber* pDst, const bohNumber* pBits);

bohNumber  bohNumberBitwiseLShift(const bohNumber* pValue, const bohNumber* pBits);
bohNumber* bohNumberBitwiseLShiftAssign(bohNumber* pDst, const bohNumber* pBits);

bohNumber  bohNumberBitwiseRShift(const bohNumber* pValue, const bohNumber* pBits);
bohNumber* bohNumberBitwiseRShiftAssign(bohNumber* pDst, const bohNumber* pBits);

bohString bohNumberToString(const bohNumber* pNumber);

bohNumber* bohNumberMove(bohNumber* pDst, bohNumber* pSrc);


#define BOH_NUMBER_GET_UNDERLYING_VALUE(NUMBER_PTR) (bohNumberIsI64(NUMBER_PTR) ? (NUMBER_PTR)->i64 : (NUMBER_PTR)->f64)
