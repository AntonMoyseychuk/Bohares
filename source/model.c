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


bohNumber bohNumberGetOpposite(const bohNumber* pNumber)
{
    assert(pNumber);
    
    if (bohNumberIsF64(pNumber)) {
        return bohNumberCreateF64(-bohNumberGetF64(pNumber));
    } else {
        return bohNumberCreateI64(-bohNumberGetI64(pNumber));
    }
}


bohNumber* bohNumberMakeOpposite(bohNumber* pNumber)
{
    *pNumber = bohNumberGetOpposite(pNumber);
    return pNumber;
}


bohNumber bohNumberGetInverted(const bohNumber* pNumber)
{
    assert(pNumber);
    
    if (bohNumberIsF64(pNumber)) {
        return bohNumberCreateF64(!bohNumberGetF64(pNumber));
    } else {
        return bohNumberCreateI64(!bohNumberGetI64(pNumber));
    }
}


bohNumber* bohNumberMakeInverted(bohNumber* pNumber)
{
    *pNumber = bohNumberGetInverted(pNumber);
    return pNumber;
}


bohNumber bohNumberAdd(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) + (bohNumberIsF64(pRight) ? bohNumberGetF64(pRight) : bohNumberGetI64(pRight)));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64((bohNumberIsF64(pLeft) ? bohNumberGetF64(pLeft) : bohNumberGetI64(pLeft)) + bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) + bohNumberGetI64(pRight));
}


bohNumber* bohNumberAddAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberAdd(pDst, pValue);
    return pDst;
}


bohNumber bohNumberSub(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) - (bohNumberIsF64(pRight) ? bohNumberGetF64(pRight) : bohNumberGetI64(pRight)));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64((bohNumberIsF64(pLeft) ? bohNumberGetF64(pLeft) : bohNumberGetI64(pLeft)) - bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) - bohNumberGetI64(pRight));
}


bohNumber* bohNumberSubAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberSub(pDst, pValue);
    return pDst;
}


bohNumber bohNumberMult(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) * (bohNumberIsF64(pRight) ? bohNumberGetF64(pRight) : bohNumberGetI64(pRight)));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64((bohNumberIsF64(pLeft) ? bohNumberGetF64(pLeft) : bohNumberGetI64(pLeft)) * bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) * bohNumberGetI64(pRight));
}


bohNumber* bohNumberMultAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberMult(pDst, pValue);
    return pDst;
}


bohNumber bohNumberDiv(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) / (bohNumberIsF64(pRight) ? bohNumberGetF64(pRight) : bohNumberGetI64(pRight)));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64((bohNumberIsF64(pLeft) ? bohNumberGetF64(pLeft) : bohNumberGetI64(pLeft)) / bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) / bohNumberGetI64(pRight));
}


bohNumber* bohNumberDivAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberDiv(pDst, pValue);
    return pDst;
}
