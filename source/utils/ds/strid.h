#pragma once

#include <stdint.h>

#include "core.h"


void bohStrIDEngineInit(void);
void bohStrIDEngineTerminate(void);
size_t bohStrIDEngineGetOccupiedMemorySize(void);


typedef struct StringView bohStringView;
typedef struct String bohString;


typedef struct StrID
{
#if defined(BOH_DEBUG)
    const char* pStr;
#endif

    uint64_t ID;
} bohStrID;


bohStrID bohStrIDCreate(void);
bohStrID bohStrIDCreateCStr(const char* pCStr);
bohStrID bohStrIDCreateString(const bohString* pString);
bohStrID bohStrIDCreateStringView(const bohStringView* pStrView);

bohStrID* bohStrIDAssign(bohStrID* pDst, const bohStrID* pSrc);
bohStrID* bohStrIDAssignCStr(bohStrID* pDst, const char* pSrc);
bohStrID* bohStrIDAssignString(bohStrID* pDst, const bohString* pSrc);
bohStrID* bohStrIDAssignStringView(bohStrID* pDst, const bohStringView* pSrc);

const char* bohStrIDGetCStr(const bohStrID* pStrID);

bool bohStrIDEqual(const bohStrID* pLeft, const bohStrID* pRight);
bool bohStrIDNotEqual(const bohStrID* pLeft, const bohStrID* pRight);
bool bohStrIDLess(const bohStrID* pLeft, const bohStrID* pRight);
bool bohStrIDGreater(const bohStrID* pLeft, const bohStrID* pRight);
bool bohStrIDLessEqual(const bohStrID* pLeft, const bohStrID* pRight);
bool bohStrIDGreaterEqual(const bohStrID* pLeft, const bohStrID* pRight);

uint64_t bohStrIDGetID(const bohStrID* pStrID);
uint64_t bohStrIDGetHash(const bohStrID* pStrID);

bool bohStrIDIsValid(const bohStrID* pStrID);