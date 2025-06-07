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


static void bohFileContentUnescapeFileData(bohFileContent* pContent)
{
    BOH_ASSERT(pContent);

    char* pRead = (char*)pContent->pData;
    char* pWrite = (char*)pContent->pData;

    size_t unescapedDataSize = 0;

    for (size_t i = 0; i < pContent->dataSize; ) {
        if (pRead[i] == '\\' && (i < pContent->dataSize && pRead[i + 1] == 'n')) {
            pWrite[unescapedDataSize] = '\n';  // Unescape '\\n' to '\n'
            i += 2;  // Skip the '\\' and 'n'
        } else if (pRead[i] == '\\' && (i < pContent->dataSize && pRead[i + 1] == 't')) {
            pWrite[unescapedDataSize] = '\t';  // Unescape '\\t' to '\t'
            i += 2;  // Skip the '\\' and 't'
        } else if (pRead[i] == '\\' && (i < pContent->dataSize && pRead[i + 1] == '\"')) {
            pWrite[unescapedDataSize] = '\"';  // Unescape '\\\"' to '\"'
            i += 2;  // Skip the '\\' and '\"'
        } else {
            pWrite[unescapedDataSize] = pRead[i];
            ++i;
        }

        ++unescapedDataSize;
    }

    pWrite[unescapedDataSize] = '\0';
    pContent->unescapedDataSize = unescapedDataSize;
}


bohFileContent bohReadTextFile(const char* pPath)
{
    bohFileContent content = ReadFile(pPath, "r");
    bohFileContentUnescapeFileData(&content);

    return content;
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
