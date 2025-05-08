#pragma once

#include <stdint.h>


typedef enum Operator
{
    BOH_OP_UNKNOWN,

    BOH_OP_PLUS,
    BOH_OP_MINUS,
    BOH_OP_MULT,
    BOH_OP_DIV,
    BOH_OP_MOD,
    BOH_OP_XOR,
    BOH_OP_BITWISE_NOT,
    BOH_OP_NOT,
    BOH_OP_GREATER,
    BOH_OP_LESS,
    BOH_OP_NOT_EQUAL,
    BOH_OP_GEQUAL,
    BOH_OP_LEQUAL,
    BOH_OP_EQUAL,
    BOH_OP_RSHIFT,
    BOH_OP_LSHIFT
} bohOperator;


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
