#pragma once

#include <stdint.h>


typedef struct FileContent
{
    void* pData;
    size_t dataSize;

    char* pErrorMsg;
} bohFileContent;


bohFileContent bohReadTextFile(const char* pPath);
bohFileContent bohReadBinaryFile(const char* pPath);

bool bohFileContentIsError(const bohFileContent* pContent);
void bohFileContentFree(bohFileContent* pContent);
