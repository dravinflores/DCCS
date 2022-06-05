/* FakeHVLibrary.c */

#include "FakeHVLibrary.h"

#include <stdlib.h>
#include <string.h>

static const char* FakeHV_ValidParameters[] = {
    "VSet",
    "VMon",
    "ISet",
    "ImonRange",
    "IMonL",
    "IMonH",
    "MaxV",
    "RUp",
    "RDwn",
    "Trip",
    "PDwn",
    "Polarity",
    "ChStatus",
    "Pw"
};

enum {
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_UNSIGNED
};

static const int FakeHV_ParameterTypes[] = {
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_UNSIGNED,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_FLOAT,
    PARAMETER_TYPE_UNSIGNED,
    PARAMETER_TYPE_UNSIGNED,
    PARAMETER_TYPE_UNSIGNED,
    PARAMETER_TYPE_UNSIGNED
};

static int FakeHV_ErrorCode = 0;

enum {
    FAKEHV_NORMAL,
    FAKEHV_BAD_HANDLE,
    FAKEHV_UNKNOWN_TYPE,
    FAKEHV_BAD_REFERENCE,
    FAKEHV_BAD_ALLOCATION,
    FAKEHV_INCORRECT_SLOT,
    FAKEHV_POINTER_IS_NULL,
    FAKEHV_INVALID_PARAMETER,
    FAKEHV_TOO_MANY_CHANNELS
};

int FakeHV_InitializeSystem(
    /* In */ int system,
    /* In */ int linkType,
    /* In */ void* connectionString,
    /* In */ const char* username,
    /* In */ const char* password,
    /* Out */ int* handle
)
{
    if (handle == NULL)
    {
        FakeHV_ErrorCode = FAKEHV_BAD_HANDLE;
        return -1;
    }
    else
    {
        *handle = 0;
        return 0;
    }
}

int FakeHV_DeinitializeSystem(/* In */ int handle)
{
    return 0;
}

int FakeHV_GetCrateMap(
    /* In */ int handle,
    /* Out */ unsigned short* numberOfSlots,
    /* Out-Allocated */ unsigned short** listOfChannelsIndexedBySlot,
    /* Out-Allocated */ char** listOfModelsIndexedBySlot,
    /* Out-Allocated */ char** descriptionList,
    /* Out-Allocated */ unsigned short** listOfSerialNumbersIndexedBySlot,
    /* Out-Allocated */ unsigned char** listOfFirmwareSuffixesIndexedBySlot,
    /* Out-Allocated */ unsigned char** listOfFirmwarePrefixesIndexedBySlot
)
{
    if (handle)
    {
        FakeHV_ErrorCode = FAKEHV_BAD_HANDLE;
        return -1;
    }

    if (numberOfSlots == NULL)
    {
        FakeHV_ErrorCode = FAKEHV_BAD_REFERENCE;
        return -1;
    }

    *numberOfSlots = 1;
    int n = (*numberOfSlots);

    typedef unsigned short UShort;
    typedef unsigned char UChar;

    // For allocating the right amount of characters.
    const char* message = "VIRTUAL";
    const int m = strlen(message) + 1;

    // Time for some fun with the pointers.
    (*listOfChannelsIndexedBySlot) = (UShort*) malloc(n * sizeof(UShort));
    (*listOfModelsIndexedBySlot) = (char*) malloc(n * m * sizeof(char));
    (*descriptionList) = (char*) malloc(n * m * sizeof(char));
    (*listOfSerialNumbersIndexedBySlot) = (UShort*) malloc(n * sizeof(UShort));
    (*listOfFirmwareSuffixesIndexedBySlot) = (UChar*) malloc(n * sizeof(UChar));
    (*listOfFirmwarePrefixesIndexedBySlot) = (UChar*) malloc(n * sizeof(UChar));

    int parameterIsNull[] = { 0, 0, 0, 0, 0, 0 };

    // Now we need to check if these things are even allocated properly.
    if (*listOfChannelsIndexedBySlot == NULL)
        parameterIsNull[0] = 1;
    if (*listOfModelsIndexedBySlot == NULL)
        parameterIsNull[1] = 1;
    if (*descriptionList == NULL)
        parameterIsNull[2] = 1;
    if (*listOfSerialNumbersIndexedBySlot == NULL)
        parameterIsNull[3] = 1;
    if (*listOfFirmwareSuffixesIndexedBySlot == NULL)
        parameterIsNull[4] = 1;
    if (*listOfFirmwarePrefixesIndexedBySlot == NULL)
        parameterIsNull[5] = 1;

    int badAllocation = 0;

    // Caller should expect a NULL pointer.
    if (parameterIsNull[0])
    {
        free(*listOfChannelsIndexedBySlot);
        *listOfChannelsIndexedBySlot = NULL;
        badAllocation = 1;
    }
    if (parameterIsNull[1])
    {
        free(*listOfModelsIndexedBySlot);
        *listOfModelsIndexedBySlot = NULL;
        badAllocation = 1;
    }
    if (parameterIsNull[2])
    {
        free(*descriptionList);
        *descriptionList = NULL;
        badAllocation = 1;
    }
    if (parameterIsNull[3])
    {
        free(*listOfSerialNumbersIndexedBySlot);
        *listOfSerialNumbersIndexedBySlot = NULL;
        badAllocation = 1;
    }
    if (parameterIsNull[4])
    {
        free(*listOfFirmwareSuffixesIndexedBySlot);
        *listOfFirmwareSuffixesIndexedBySlot = NULL;
        badAllocation = 1;
    }
    if (parameterIsNull[5])
    {
        free(*listOfFirmwarePrefixesIndexedBySlot);
        *listOfFirmwarePrefixesIndexedBySlot = NULL;
        badAllocation = 1;
    }

    if (badAllocation)
    {
        FakeHV_ErrorCode = FAKEHV_BAD_ALLOCATION;
        return -1;
    }

    // Now for assignment. This is confusing because of pass by reference.
    // Remember that we have references, so we'll need to dereference first.
    // These are hardcoded as they are known ahead of time.
    (*listOfChannelsIndexedBySlot)[0] = 4;

    // (*listOfModelsIndexedBySlot) = "VIRTUAL";
    // (*descriptionList) = "VIRTUAL";
    strcpy(*listOfModelsIndexedBySlot, "VIRTUAL");
    strcpy(*descriptionList, "VIRTUAL");

    (*listOfSerialNumbersIndexedBySlot)[0] = 0;
    (*listOfFirmwareSuffixesIndexedBySlot)[0] = 0;
    (*listOfFirmwarePrefixesIndexedBySlot)[0] = 0;

    return 0;
}


int FakeHV_GetChannelParameter(
    /* In */ int handle,
    /* In */ unsigned short slot,
    /* In */ const char* parameter,
    /* In */ unsigned short channelListSize,
    /* In */ const unsigned short* listOfChannelsToRead,
    /* Out */ void* listOfParameterValues
)
{
    if (handle)
    {
        FakeHV_ErrorCode = FAKEHV_BAD_HANDLE;
        return -1;
    }

    // Particular value is hardcoded. 
    if (slot)
    {
        FakeHV_ErrorCode = FAKEHV_INCORRECT_SLOT;
        return -1;
    }

    int isInList = 0;
    int index = 0;
    for (int i = 0; i < 14; ++i)
    {
        if (strcmp(parameter, FakeHV_ValidParameters[i]) == 0)
        {
            isInList = 1;
            index = 0;
            break;
        }
    }

    if (!isInList)
    {
        FakeHV_ErrorCode = FAKEHV_INVALID_PARAMETER;
        return -1;
    }

    if (listOfChannelsToRead == NULL)
    {
        FakeHV_ErrorCode = FAKEHV_POINTER_IS_NULL;
        return -1;
    }

    if (listOfParameterValues == NULL)
    {
        FakeHV_ErrorCode = FAKEHV_POINTER_IS_NULL;
        return -1;
    }

    // We cannot really enforce this. We could end up accessing memory that
    // isn't ours if we get the wrong channelListSize in comparison to the
    // memory of listOfChannelsToRead...would prefer std::span if we had it.
    if (channelListSize > 4)
    {
        FakeHV_ErrorCode = FAKEHV_TOO_MANY_CHANNELS;
        return -1;
    }

    for (int i = 0; i < channelListSize; ++i)
    {
        if (FakeHV_ParameterTypes[index] == PARAMETER_TYPE_FLOAT)
        {
            ((float*) listOfParameterValues)[i] = 0.00f;
        }
        else if (FakeHV_ParameterTypes[index] == PARAMETER_TYPE_UNSIGNED)
        {
            ((unsigned long*) listOfParameterValues)[i] = 0;
        }
        else
        {
            FakeHV_ErrorCode = FAKEHV_UNKNOWN_TYPE;
            return -1;
        }
    }

    return 0;
}

int FakeHV_SetChannelParameter(
    /* In */ int handle,
    /* In */ unsigned short slot,
    /* In */ const char* parameter,
    /* In */ unsigned short channelListSize,
    /* In */ const unsigned short* listOfChannelsToWrite,
    /* In */ void* newParameterValue
)
{
    // Yes this is totally a copy and paste.
    if (handle)
    {
        FakeHV_ErrorCode = FAKEHV_BAD_HANDLE;
        return -1;
    }

    // Particular value is hardcoded. 
    if (slot)
    {
        FakeHV_ErrorCode = FAKEHV_INCORRECT_SLOT;
        return -1;
    }

    int isInList = 0;
    int index = 0;
    for (int i = 0; i < 14; ++i)
    {
        if (strcmp(parameter, FakeHV_ValidParameters[i]) != 0)
        {
            isInList = 1;
            index = 0;
            break;
        }
    }

    if (!isInList)
    {
        FakeHV_ErrorCode = FAKEHV_INVALID_PARAMETER;
        return -1;
    }

    if (listOfChannelsToWrite == NULL)
    {
        FakeHV_ErrorCode = FAKEHV_POINTER_IS_NULL;
        return -1;
    }

    if (channelListSize > 4)
    {
        FakeHV_ErrorCode = FAKEHV_TOO_MANY_CHANNELS;
        return -1;
    }

    if (newParameterValue == NULL)
    {
        FakeHV_ErrorCode = FAKEHV_POINTER_IS_NULL;
        return -1;
    }

    return 0;
}

char* FakeHV_GetError(/* In */ int handle)
{
    switch(FakeHV_ErrorCode)
    {
    case FAKEHV_NORMAL:
        return "Error [0]: No Error.";
    case FAKEHV_BAD_HANDLE:
        return "Error [1]: Bad Handle Received.";
    case FAKEHV_UNKNOWN_TYPE:
        return "Error [2]: Unknown Value Type Received.";
    case FAKEHV_BAD_REFERENCE:
        return "Error [3]: Bad Reference Received.";
    case FAKEHV_BAD_ALLOCATION:
        return "Error [4]: Cannot Allocate.";
    case FAKEHV_INCORRECT_SLOT:
        return "Error [5]: Incorrect Slot Received.";
    case FAKEHV_POINTER_IS_NULL:
        return "Error [6]: Null Pointer Received";
    case FAKEHV_INVALID_PARAMETER:
        return "Error [7]: Invalid Parameter Received";
    case FAKEHV_TOO_MANY_CHANNELS:
        return "Error [8]: Invalid Number of Channels Received";
    default:
        return "Error [?]: Unknown Error Code";
    }
}

int FakeHV_Free(/* In */ void* resource)
{
    if (resource == NULL)
    {
        FakeHV_ErrorCode = FAKEHV_POINTER_IS_NULL;
        return -1;
    }

    free(resource);
    return 0;
}