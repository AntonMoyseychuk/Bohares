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


bool bohNumberEqual(const bohNumber* pLeft, const bohNumber* pRight, double precision)
{
    assert(pLeft);
    assert(pRight);

    if (bohNumberIsI64(pLeft) && bohNumberIsI64(pRight)) {
        return bohNumberGetI64(pLeft) == bohNumberGetI64(pRight);
    }
    
    return fabs(BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft) - BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight)) < precision;
}


bool bohNumberNotEqual(const bohNumber* pLeft, const bohNumber* pRight, double precision)
{
    return !bohNumberEqual(pLeft, pRight, precision);
}


bool bohNumberLess(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);
    
    return BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft) < BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight);
}


bool bohNumberGreater(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);
    
    return BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft) > BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight);
}


bool bohNumberLessEqual(const bohNumber* pLeft, const bohNumber* pRight)
{
    return !bohNumberGreater(pLeft, pRight);
}


bool bohNumberGreaterEqual(const bohNumber* pLeft, const bohNumber* pRight)
{
    return !bohNumberLess(pLeft, pRight);
}


bool bohNumberIsZero(const bohNumber* pNumber)
{
    assert(pNumber);
    
    if (bohNumberIsF64(pNumber)) {
        const bohNumber zeroF64 = bohNumberCreateF64(0.0);
        return bohNumberEqual(pNumber, &zeroF64, __DBL_EPSILON__);
    } else {
        const bohNumber zeroI64 = bohNumberCreateI64(0);
        return bohNumberEqual(pNumber, &zeroI64, 0.0);
    }
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


bohNumber bohNumberGetNegation(const bohNumber* pNumber)
{
    assert(pNumber);
    
    if (bohNumberIsF64(pNumber)) {
        return bohNumberCreateF64(!bohNumberGetF64(pNumber));
    } else {
        return bohNumberCreateI64(!bohNumberGetI64(pNumber));
    }
}


bohNumber* bohNumberMakeNegation(bohNumber* pNumber)
{
    *pNumber = bohNumberGetNegation(pNumber);
    return pNumber;
}


bohNumber bohNumberGetBitwiseNegation(const bohNumber* pNumber)
{
    assert(pNumber);
    assert(bohNumberIsI64(pNumber));

    return bohNumberCreateI64(~bohNumberGetI64(pNumber));
}


bohNumber* bohNumberMakeBitwiseNegation(bohNumber* pNumber)
{
    *pNumber = bohNumberGetBitwiseNegation(pNumber);
    return pNumber;
}


bohNumber bohNumberAdd(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) + BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft) + bohNumberGetF64(pRight));
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
        return bohNumberCreateF64(bohNumberGetF64(pLeft) - BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft) - bohNumberGetF64(pRight));
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
        return bohNumberCreateF64(bohNumberGetF64(pLeft) * BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft) * bohNumberGetF64(pRight));
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
        return bohNumberCreateF64(bohNumberGetF64(pLeft) / BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft) / bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) / bohNumberGetI64(pRight));
}


bohNumber* bohNumberDivAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberDiv(pDst, pValue);
    return pDst;
}


bohNumber bohNumberMod(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(fmod(bohNumberGetF64(pLeft), BOH_NUMBER_GET_UNDERLYING_VALUE(*pRight)));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(fmod(BOH_NUMBER_GET_UNDERLYING_VALUE(*pLeft), bohNumberGetF64(pRight)));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) % bohNumberGetI64(pRight));
}


bohNumber* bohNumberModAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberMod(pDst, pValue);
    return pDst;
}


bohNumber bohNumberXor(const bohNumber* pLeft, const bohNumber* pRight)
{
    assert(pLeft);
    assert(pRight);
    assert(bohNumberIsI64(pLeft));
    assert(bohNumberIsI64(pRight));

    return bohNumberCreateI64(bohNumberGetI64(pLeft) ^ bohNumberGetI64(pRight));
}


bohNumber* bohNumberXorAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberXor(pDst, pValue);
    return pDst;
}


bohNumber bohNumberLShift(const bohNumber* pValue, const bohNumber* pBits)
{
    assert(pValue);
    assert(pBits);
    assert(bohNumberIsI64(pValue));
    assert(bohNumberIsI64(pBits));

    return bohNumberCreateI64(bohNumberGetI64(pValue) << bohNumberGetI64(pBits));
}


bohNumber* bohNumberLShiftAssign(bohNumber* pDst, const bohNumber* pBits)
{
    *pDst = bohNumberLShift(pDst, pBits);
    return pDst;
}


bohNumber bohNumberRShift(const bohNumber* pValue, const bohNumber* pBits)
{
    assert(pValue);
    assert(pBits);
    assert(bohNumberIsI64(pValue));
    assert(bohNumberIsI64(pBits));

    return bohNumberCreateI64(bohNumberGetI64(pValue) >> bohNumberGetI64(pBits));
}


bohNumber *bohNumberRShiftAssign(bohNumber* pDst, const bohNumber* pBits)
{
    *pDst = bohNumberRShift(pDst, pBits);
    return pDst;
}
