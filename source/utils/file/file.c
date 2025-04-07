#include "pch.h"

#include "file.h"


enum
{
    MAX_ERROR_MSG_SIZE = 512
};


static bohFileContent ReadFile(const char* pPath, const char* pMode)
{
    bohFileContent content = { 0 };

    if (!pPath) {
        const char pErrorMsg[] = "pPath is NULL";

        content.pErrorMsg = (char*)malloc(sizeof(pErrorMsg));
        strcpy_s(content.pErrorMsg, sizeof(pErrorMsg), pErrorMsg);

        return content;
    }

    FILE* pFile = NULL;
    errno_t result = fopen_s(&pFile, pPath, pMode);

    if (result != 0 || !pFile) {
        char pErrorMsg[MAX_ERROR_MSG_SIZE] = { 0 };
        sprintf_s(pErrorMsg, MAX_ERROR_MSG_SIZE, "Failed to open file: %s", pPath);

        const uint64_t errorLength = strlen(pErrorMsg) + 1;

        content.pErrorMsg = (char*)malloc(errorLength);
        strcpy_s(content.pErrorMsg, errorLength, pErrorMsg);

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
    return ReadFile(pPath, "r");
}


bohFileContent bohReadBinaryFile(const char* pPath)
{
    return ReadFile(pPath, "rb");
}


bool bohFileContentIsError(const bohFileContent* pContent)
{
    return pContent && pContent->pErrorMsg != NULL;
}


void bohFileContentFree(bohFileContent *pContent)
{
    if (!pContent) {
        return;
    }

    if (pContent->dataSize != 0) {
        free(pContent->pData);

        pContent->pData = NULL;
        pContent->dataSize = 0;
    }

    free(pContent->pErrorMsg);
    pContent->pErrorMsg = NULL;
}
