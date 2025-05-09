#pragma once

#include <stdint.h>


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


bohNumber bohNumberCreateI64(int64_t value);
bohNumber bohNumberCreateF64(double value);

bool bohNumberIsI64(const bohNumber* pNumber);
bool bohNumberIsF64(const bohNumber* pNumber);

int64_t bohNumberGetI64(const bohNumber* pNumber);
double bohNumberGetF64(const bohNumber* pNumber);

void bohNumberSetI64(bohNumber* pNumber, int64_t value);
void bohNumberSetF64(bohNumber* pNumber, double value);

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

bohNumber  bohNumberGetBitwiseNegation(const bohNumber* pNumber);
bohNumber* bohNumberMakeBitwiseNegation(bohNumber* pNumber);

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

bohNumber  bohNumberXor(const bohNumber* pValue, const bohNumber* pBits);
bohNumber* bohNumberXorAssign(bohNumber* pDst, const bohNumber* pBits);

bohNumber  bohNumberLShift(const bohNumber* pValue, const bohNumber* pBits);
bohNumber* bohNumberLShiftAssign(bohNumber* pDst, const bohNumber* pBits);

bohNumber  bohNumberRShift(const bohNumber* pValue, const bohNumber* pBits);
bohNumber* bohNumberRShiftAssign(bohNumber* pDst, const bohNumber* pBits);


#define BOH_NUMBER_GET_UNDERLYING_VALUE(NUMBER) (bohNumberIsI64(&(NUMBER)) ? bohNumberGetI64(&(NUMBER)) : bohNumberGetF64(&(NUMBER)))
