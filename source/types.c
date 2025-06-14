#include "pch.h"

#include "core.h"
#include "types.h"


bohBoharesString bohBoharesStringCreateStringView(void)
{
    bohBoharesString str;

    str.type = BOH_STRING_TYPE_VIEW;
    str.view = bohStringViewCreate();

    return str;
}


bohBoharesString bohBoharesStringCreateStringViewStringView(bohStringView strView)
{
    return bohBoharesStringCreateStringViewStringViewPtr(&strView);
}


bohBoharesString bohBoharesStringCreateStringViewStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohBoharesString str;

    str.type = BOH_STRING_TYPE_VIEW;
    str.view = bohStringViewCreateStringViewPtr(pStrView);

    return str;
}


bohBoharesString bohBoharesStringCreateString(void)
{
    bohBoharesString str;

    str.type = BOH_STRING_TYPE_STRING;
    str.string = bohStringCreate();

    return str;
}


bohBoharesString bohBoharesStringCreateStringString(const bohString* pString)
{
    BOH_ASSERT(pString);

    bohBoharesString str;

    str.type = BOH_STRING_TYPE_STRING;
    str.string = bohStringCreateString(pString);

    return str;
}


bohBoharesString bohBoharesStringCreateStringStringView(bohStringView strView)
{
    return bohBoharesStringCreateStringStringViewPtr(&strView);
}


bohBoharesString bohBoharesStringCreateStringStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohBoharesString str;

    str.type = BOH_STRING_TYPE_STRING;
    str.string = bohStringCreateStringViewPtr(pStrView);

    return str;
}


void bohBoharesStringDestroy(bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    if (bohBoharesStringIsString(pString)) {
        bohStringDestroy(&pString->string);
    } else {
        bohStringViewReset(&pString->view);
    }

    pString->type = BOH_STRING_TYPE_VIEW;
}


bohBoharesString* bohBoharesStringAssign(bohBoharesString* pDst, const bohBoharesString* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBoharesStringDestroy(pDst);

    if (bohBoharesStringIsString(pSrc)) {
        bohStringAssign(&pDst->string, &pSrc->string);
        pDst->type = BOH_STRING_TYPE_STRING;
    } else {
        bohStringViewAssignStringViewPtr(&pDst->view, &pSrc->view);
        pDst->type = BOH_STRING_TYPE_VIEW;
    }

    return pDst;
}


bohBoharesString* bohBoharesStringStringAssignStringView(bohBoharesString* pDst, bohStringView src)
{
    return bohBoharesStringStringAssignStringViewPtr(pDst, &src);
}


bohBoharesString* bohBoharesStringStringAssignStringViewPtr(bohBoharesString* pDst, const bohStringView* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBoharesStringDestroy(pDst);

    bohStringAssignStringViewPtr(&pDst->string, pSrc);
    pDst->type = BOH_STRING_TYPE_STRING;

    return pDst;
}


bohBoharesString* bohBoharesStringStringAssignString(bohBoharesString* pDst, const bohString* pSrc)
{
    return bohBoharesStringStringAssignStringView(pDst, bohStringViewCreateConstString(pSrc));
}


bohBoharesString* bohBoharesStringStringViewAssignStringView(bohBoharesString* pDst, bohStringView src)
{
    return bohBoharesStringStringViewAssignStringViewPtr(pDst, &src);
}


bohBoharesString* bohBoharesStringStringViewAssignStringViewPtr(bohBoharesString* pDst, const bohStringView* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBoharesStringDestroy(pDst);

    bohStringViewAssignStringViewPtr(&pDst->view, pSrc);
    pDst->type = BOH_STRING_TYPE_VIEW;

    return pDst;
}


bohBoharesString* bohBoharesStringStringViewAssignString(bohBoharesString* pDst, const bohString* pSrc)
{
    return bohBoharesStringStringViewAssignStringView(pDst, bohStringViewCreateConstString(pSrc));
}


bohBoharesString* bohBoharesStringResize(bohBoharesString* pString, size_t newSize)
{
    BOH_ASSERT(pString);

    if (bohBoharesStringIsString(pString)) {
        bohStringResize(&pString->string, newSize);
    } else {
        bohStringViewResize(&pString->view, newSize);
    }

    return pString;
}


bohBoharesString* bohBoharesStringMove(bohBoharesString* pDst, bohBoharesString* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBoharesStringDestroy(pDst);

    pDst->type = pSrc->type;

    if (bohBoharesStringIsString(pSrc)) {
        bohStringMove(&pDst->string, &pSrc->string);
    } else {
        bohStringViewMove(&pDst->view, &pSrc->view);
    }

    return pDst;
}


bool bohBoharesStringIsStringView(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);
    return pString->type == BOH_STRING_TYPE_VIEW;
}


bool bohBoharesStringIsString(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);
    return pString->type == BOH_STRING_TYPE_STRING;
}


const bohStringView* bohBoharesStringGetStringView(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);
    BOH_ASSERT(bohBoharesStringIsStringView(pString));

    return &pString->view;
}


const bohString* bohBoharesStringGetString(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);
    BOH_ASSERT(bohBoharesStringIsString(pString));

    return &pString->string;
}


const char* bohBoharesStringGetData(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    if (bohBoharesStringIsString(pString)) {
        return bohStringGetCStr(&pString->string);
    } else {
        return bohStringViewGetData(&pString->view);
    }
}


size_t bohBoharesStringGetSize(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    if (bohBoharesStringIsString(pString)) {
        return bohStringGetSize(&pString->string);
    } else {
        return bohStringViewGetSize(&pString->view);
    }
}


char bohBoharesStringAt(const bohBoharesString* pString, size_t index)
{
    BOH_ASSERT(pString);

    if (bohBoharesStringIsString(pString)) {
        return bohStringAt(&pString->string, index);
    } else {
        return bohStringViewAt(&pString->view, index);
    }
}


void bohBoharesStringSetAt(bohBoharesString* pString, char ch, size_t index)
{
    if (bohBoharesStringIsString(pString)) {
        bohStringSetAt(&pString->string, ch, index);
    } else {
        bohStringViewSetAt(&pString->view, ch, index);
    }
}


bool bohBoharesStringIsEmpty(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    if (bohBoharesStringIsString(pString)) {
        return bohStringIsEmpty(&pString->string);
    } else {
        return bohStringViewIsEmpty(&pString->view);
    }
}


bool bohBoharesStringIsConstStringView(const bohBoharesString* pString)
{
    return bohBoharesStringIsStringView(pString) && bohStringViewIsConst(bohBoharesStringGetStringView(pString));
}


int32_t bohBoharesStringCmp(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    const bohStringView left = bohBoharesStringIsString(pLeft) ? bohStringViewCreateConstString(&pLeft->string) : pLeft->view;
    const bohStringView right = bohBoharesStringIsString(pRight) ? bohStringViewCreateConstString(&pRight->string) : pRight->view;

    return bohStringViewCmpPtr(&left, &right);
}


bool bohBoharesStringEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    return bohBoharesStringCmp(pLeft, pRight) == 0;
}


bool bohBoharesStringNotEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    return bohBoharesStringCmp(pLeft, pRight) != 0;
}


bool bohBoharesStringLess(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    return bohBoharesStringCmp(pLeft, pRight) < 0;
}


bool bohBoharesStringLessEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    return bohBoharesStringCmp(pLeft, pRight) <= 0;
}


bool bohBoharesStringGreater(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    return bohBoharesStringCmp(pLeft, pRight) > 0;
}


bool bohBoharesStringGreaterEqual(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    return bohBoharesStringCmp(pLeft, pRight) >= 0;
}


bohBoharesString bohBoharesStringAdd(const bohBoharesString* pLeft, const bohBoharesString* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    bohBoharesString result = bohBoharesStringCreateString();

    if (bohBoharesStringIsString(pLeft)) {
        if (bohBoharesStringIsString(pRight)) {
            result.string = bohStringAdd(&pLeft->string, &pRight->string);
        } else {
            result.string = bohStringAddStringView(&pLeft->string, &pRight->view);
        }
    } else {
        if (bohBoharesStringIsString(pRight)) {
            result.string = bohStringViewAddString(&pLeft->view, &pRight->string);
        } else {
            result.string = bohStringViewAddStringView(&pLeft->view, &pRight->view);
        }
    }

    return result;
}


bohNumber bohNumberCreate(void)
{
    bohNumber number;

    number.type = BOH_NUMBER_TYPE_INTEGER;
    number.i64 = 0;

    return number;
}


bohNumber bohNumberCreateI64(int64_t value)
{
    bohNumber number = bohNumberCreate();
    bohNumberSetI64(&number, value);

    return number;
}


bohNumber bohNumberCreateF64(double value)
{
    bohNumber number = bohNumberCreate();
    bohNumberSetF64(&number, value);

    return number;
}


bool bohNumberIsI64(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    return pNumber->type == BOH_NUMBER_TYPE_INTEGER;
}


bool bohNumberIsF64(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    return pNumber->type == BOH_NUMBER_TYPE_FLOAT;
}


int64_t bohNumberGetI64(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    BOH_ASSERT(bohNumberIsI64(pNumber));
    
    return pNumber->i64;
}


bool bohNumberIsIntegral(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    return bohNumberIsI64(pNumber);
}


bool bohNumberIsFloatingPoint(const bohNumber *pNumber)
{
    BOH_ASSERT(pNumber);
    return bohNumberIsF64(pNumber);
}


double bohNumberGetF64(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    BOH_ASSERT(bohNumberIsF64(pNumber));
    
    return pNumber->f64;
}


void bohNumberSetI64(bohNumber* pNumber, int64_t value)
{
    BOH_ASSERT(pNumber);
    
    pNumber->type = BOH_NUMBER_TYPE_INTEGER;
    pNumber->i64 = value;
}


void bohNumberSetF64(bohNumber* pNumber, double value)
{
    BOH_ASSERT(pNumber);
    
    pNumber->type = BOH_NUMBER_TYPE_FLOAT;
    pNumber->f64 = value;
}


bohNumber* bohNumberAssign(bohNumber* pDst, const bohNumber* pNumber)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pNumber);

    if (bohNumberIsI64(pNumber)) {
        bohNumberSetI64(pDst, bohNumberGetI64(pNumber));
    } else {
        bohNumberSetF64(pDst, bohNumberGetF64(pNumber));
    }

    return pDst;
}


bool bohNumberEqual(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);
    
    return BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft) == BOH_NUMBER_GET_UNDERLYING_VALUE(pRight);
}


bool bohNumberNotEqual(const bohNumber* pLeft, const bohNumber* pRight)
{
    return !bohNumberEqual(pLeft, pRight);
}


bool bohNumberLess(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);
    
    return BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft) < BOH_NUMBER_GET_UNDERLYING_VALUE(pRight);
}


bool bohNumberGreater(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);
    
    return BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft) > BOH_NUMBER_GET_UNDERLYING_VALUE(pRight);
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
    BOH_ASSERT(pNumber);
    
    if (bohNumberIsF64(pNumber)) {
        const bohNumber zeroF64 = bohNumberCreateF64(0.0);
        return bohNumberEqual(pNumber, &zeroF64);
    } else {
        const bohNumber zeroI64 = bohNumberCreateI64(0);
        return bohNumberEqual(pNumber, &zeroI64);
    }
}


bohNumber bohNumberGetOpposite(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    
    if (bohNumberIsF64(pNumber)) {
        return bohNumberCreateF64(-bohNumberGetF64(pNumber));
    } else {
        return bohNumberCreateI64(-bohNumberGetI64(pNumber));
    }
}


bohNumber* bohNumberMakeOpposite(bohNumber* pNumber)
{
    const bohNumber newValue = bohNumberGetOpposite(pNumber);
    bohNumberAssign(pNumber, &newValue);
    
    return pNumber;
}


bohNumber bohNumberGetNegation(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    
    if (bohNumberIsF64(pNumber)) {
        return bohNumberCreateF64(!bohNumberGetF64(pNumber));
    } else {
        return bohNumberCreateI64(!bohNumberGetI64(pNumber));
    }
}


bohNumber* bohNumberMakeNegation(bohNumber* pNumber)
{
    const bohNumber newValue = bohNumberGetNegation(pNumber);
    bohNumberAssign(pNumber, &newValue);

    return pNumber;
}


bohNumber bohNumberAdd(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) + BOH_NUMBER_GET_UNDERLYING_VALUE(pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft) + bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) + bohNumberGetI64(pRight));
}


bohNumber* bohNumberAddAssign(bohNumber* pDst, const bohNumber* pValue)
{
    const bohNumber newValue = bohNumberAdd(pDst, pValue);
    bohNumberAssign(pDst, &newValue);

    return pDst;
}


bohNumber bohNumberSub(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) - BOH_NUMBER_GET_UNDERLYING_VALUE(pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft) - bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) - bohNumberGetI64(pRight));
}


bohNumber* bohNumberSubAssign(bohNumber* pDst, const bohNumber* pValue)
{
    const bohNumber newValue = bohNumberSub(pDst, pValue);
    bohNumberAssign(pDst, &newValue);
    
    return pDst;
}


bohNumber bohNumberMult(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) * BOH_NUMBER_GET_UNDERLYING_VALUE(pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft) * bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) * bohNumberGetI64(pRight));
}


bohNumber* bohNumberMultAssign(bohNumber* pDst, const bohNumber* pValue)
{
    const bohNumber newValue = bohNumberMult(pDst, pValue);
    bohNumberAssign(pDst, &newValue);

    return pDst;
}


bohNumber bohNumberDiv(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(bohNumberGetF64(pLeft) / BOH_NUMBER_GET_UNDERLYING_VALUE(pRight));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft) / bohNumberGetF64(pRight));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) / bohNumberGetI64(pRight));
}


bohNumber* bohNumberDivAssign(bohNumber* pDst, const bohNumber* pValue)
{
    const bohNumber newValue = bohNumberDiv(pDst, pValue);
    bohNumberAssign(pDst, &newValue);
    
    return pDst;
}


bohNumber bohNumberMod(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    if (bohNumberIsF64(pLeft)) {
        return bohNumberCreateF64(fmod(bohNumberGetF64(pLeft), BOH_NUMBER_GET_UNDERLYING_VALUE(pRight)));
    } else if (bohNumberIsF64(pRight)) {
        return bohNumberCreateF64(fmod(BOH_NUMBER_GET_UNDERLYING_VALUE(pLeft), bohNumberGetF64(pRight)));
    }

    return bohNumberCreateI64(bohNumberGetI64(pLeft) % bohNumberGetI64(pRight));
}


bohNumber* bohNumberModAssign(bohNumber* pDst, const bohNumber* pValue)
{
    const bohNumber newValue = bohNumberMod(pDst, pValue);
    bohNumberAssign(pDst, &newValue);
    
    return pDst;
}


bohNumber bohNumberGetBitwiseNegation(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);
    BOH_ASSERT(bohNumberIsI64(pNumber));

    return bohNumberCreateI64(~bohNumberGetI64(pNumber));
}


bohNumber* bohNumberMakeBitwiseNegation(bohNumber* pNumber)
{
    *pNumber = bohNumberGetBitwiseNegation(pNumber);
    return pNumber;
}


bohNumber bohNumberBitwiseAnd(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);
    BOH_ASSERT(bohNumberIsI64(pLeft));
    BOH_ASSERT(bohNumberIsI64(pRight));

    return bohNumberCreateI64(bohNumberGetI64(pLeft) & bohNumberGetI64(pRight));
}


bohNumber* bohNumberBitwiseAndAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberBitwiseAnd(pDst, pValue);
    return pDst;
}


bohNumber bohNumberBitwiseOr(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);
    BOH_ASSERT(bohNumberIsI64(pLeft));
    BOH_ASSERT(bohNumberIsI64(pRight));

    return bohNumberCreateI64(bohNumberGetI64(pLeft) | bohNumberGetI64(pRight));
}


bohNumber* bohNumberBitwiseOrAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberBitwiseOr(pDst, pValue);
    return pDst;
}


bohNumber bohNumberBitwiseXor(const bohNumber* pLeft, const bohNumber* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);
    BOH_ASSERT(bohNumberIsI64(pLeft));
    BOH_ASSERT(bohNumberIsI64(pRight));

    return bohNumberCreateI64(bohNumberGetI64(pLeft) ^ bohNumberGetI64(pRight));
}


bohNumber* bohNumberBitwiseXorAssign(bohNumber* pDst, const bohNumber* pValue)
{
    *pDst = bohNumberBitwiseXor(pDst, pValue);
    return pDst;
}


bohNumber bohNumberBitwiseLShift(const bohNumber* pValue, const bohNumber* pBits)
{
    BOH_ASSERT(pValue);
    BOH_ASSERT(pBits);
    BOH_ASSERT(bohNumberIsI64(pValue));
    BOH_ASSERT(bohNumberIsI64(pBits));

    return bohNumberCreateI64(bohNumberGetI64(pValue) << bohNumberGetI64(pBits));
}


bohNumber* bohNumberBitwiseLShiftAssign(bohNumber* pDst, const bohNumber* pBits)
{
    *pDst = bohNumberBitwiseLShift(pDst, pBits);
    return pDst;
}


bohNumber bohNumberBitwiseRShift(const bohNumber* pValue, const bohNumber* pBits)
{
    BOH_ASSERT(pValue);
    BOH_ASSERT(pBits);
    BOH_ASSERT(bohNumberIsI64(pValue));
    BOH_ASSERT(bohNumberIsI64(pBits));

    return bohNumberCreateI64(bohNumberGetI64(pValue) >> bohNumberGetI64(pBits));
}


bohNumber *bohNumberBitwiseRShiftAssign(bohNumber* pDst, const bohNumber* pBits)
{
    *pDst = bohNumberBitwiseRShift(pDst, pBits);
    return pDst;
}


bohString bohNumberToString(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);

    char buff[256] = { 0 };
    sprintf_s(buff, sizeof(buff) - 1, bohNumberIsF64(pNumber) ? "%.10f" : "%lld", BOH_NUMBER_GET_UNDERLYING_VALUE(pNumber));

    return bohStringCreateCStr(buff);
}


bool bohNumberToBool(const bohNumber* pNumber)
{
    return bohNumberIsI64(pNumber) ? pNumber->i64 != 0 : pNumber->f64 != 0.0;
}


bohNumber* bohNumberMove(bohNumber* pDst, bohNumber* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohNumberAssign(pDst, pSrc);

    switch (pSrc->type) {
        case BOH_NUMBER_TYPE_INTEGER:
            bohNumberSetI64(pSrc, 0);
            break;
        case BOH_NUMBER_TYPE_FLOAT:
            bohNumberSetF64(pSrc, 0.0);
            break;
        default:
            BOH_ASSERT(false && "Invalid number type");
            break;
    }

    return pDst;
}
