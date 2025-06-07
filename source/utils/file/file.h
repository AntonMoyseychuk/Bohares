#pragma once

#include <stdint.h>


typedef enum FileContentErrorCode
{
    BOH_FILE_CONTENT_ERROR_NONE,
    BOH_FILE_CONTENT_ERROR_NULL_FILEPATH,
    BOH_FILE_CONTENT_ERROR_OPEN_FAILED,
} bohFileContentErrorCode;


typedef struct FileContent
{
    void* pData;
    size_t dataSize;
    size_t unescapedDataSize;

    bohFileContentErrorCode error;
} bohFileContent;


bohFileContent bohReadTextFile(const char* pPath);
bohFileContent bohReadBinaryFile(const char* pPath);

bohFileContentErrorCode bohFileContentGetErrorCode(const bohFileContent* pContent);
void bohFileContentFree(bohFileContent* pContent);
