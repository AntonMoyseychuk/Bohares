#include "pch.h"

#include "model.h"


bohNumber bohNumberCreateI64(int64_t value)
{
    bohNumber number;
    bohNumberSetI64(&number, value);

    return number;
}


bohNumber bohNumberCreateF64(double value)
{
    bohNumber number;
    bohNumberSetF64(&number, value);

    return number;
}


bool bohNumberIsI64(const bohNumber* pNumber)
{
    assert(pNumber);
    return pNumber->type == BOH_NUMBER_TYPE_INTEGER;
}


bool bohNumberIsF64(const bohNumber* pNumber)
{
    assert(pNumber);
    return pNumber->type == BOH_NUMBER_TYPE_FLOAT;
}


int64_t bohNumberGetI64(const bohNumber* pNumber)
{
    assert(pNumber);
    assert(bohNumberIsI64(pNumber));
    
    return pNumber->i64;
}


double bohNumberGetF64(const bohNumber* pNumber)
{
    assert(pNumber);
    assert(bohNumberIsF64(pNumber));
    
    return pNumber->f64;
}


void bohNumberSetI64(bohNumber* pNumber, int64_t value)
{
    assert(pNumber);
    
    pNumber->type = BOH_NUMBER_TYPE_INTEGER;
    pNumber->i64 = value;
}


void bohNumberSetF64(bohNumber* pNumber, double value)
{
    assert(pNumber);
    
    pNumber->type = BOH_NUMBER_TYPE_FLOAT;
    pNumber->f64 = value;
}
