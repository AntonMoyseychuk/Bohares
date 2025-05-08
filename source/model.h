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

bohNumber  bohNumberGetOpposite(const bohNumber* pNumber);
bohNumber* bohNumberMakeOpposite(bohNumber* pNumber);

bohNumber  bohNumberGetInverted(const bohNumber* pNumber);  // operator: !number
bohNumber* bohNumberMakeInverted(bohNumber* pNumber);

bohNumber  bohNumberAdd(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberAddAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberSub(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberSubAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberMult(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberMultAssign(bohNumber* pDst, const bohNumber* pValue);

bohNumber  bohNumberDiv(const bohNumber* pLeft, const bohNumber* pRight);
bohNumber* bohNumberDivAssign(bohNumber* pDst, const bohNumber* pValue);
