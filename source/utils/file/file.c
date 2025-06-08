#include "pch.h"

#include "core.h"

#include "file.h"


static bohFileContent ReadFile(const char* pPath, const char* pMode)
{
    bohFileContent content;
    content.pData = NULL;
    content.dataSize = 0;
    content.error = BOH_FILE_CONTENT_ERROR_NONE;

    if (!pPath) {
        content.error = BOH_FILE_CONTENT_ERROR_NULL_FILEPATH;
        return content;
    }

    FILE* pFile = NULL;
    errno_t result = fopen_s(&pFile, pPath, pMode);

    if (!pFile || result != 0) {
        content.error = BOH_FILE_CONTENT_ERROR_OPEN_FAILED;
        return content;
    }

    fseek(pFile, 0, SEEK_END);
    const uint64_t fileSizeInBytes = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (fileSizeInBytes == 0) {
        content.pData = "";
        return content;
    }

    content.pData = malloc(fileSizeInBytes);
    memset(content.pData, 0, fileSizeInBytes);

    fread_s(content.pData, fileSizeInBytes, sizeof(uint8_t), fileSizeInBytes, pFile);
    fclose(pFile);

    content.dataSize = fileSizeInBytes;

    return content;
}


bohFileContent bohReadTextFile(const char* pPath)
{
    return  ReadFile(pPath, "r");
}


bohFileContent bohReadBinaryFile(const char* pPath)
{
    return ReadFile(pPath, "rb");
}


bohFileContentErrorCode bohFileContentGetErrorCode(const bohFileContent* pContent)
{
    BOH_ASSERT(pContent);
    return pContent->error;
}


void bohFileContentFree(bohFileContent *pContent)
{
    BOH_ASSERT(pContent);

    if (pContent->dataSize != 0) {
        free(pContent->pData);

        pContent->pData = NULL;
        pContent->dataSize = 0;
    }

    pContent->error = BOH_FILE_CONTENT_ERROR_NONE;
}
