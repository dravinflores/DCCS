/* FakeHVLibrary.h */

#ifndef SOURCE_PSU_FAKE_HV_LIBRARY_H
#define SOURCE_PSU_FAKE_HV_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int FakeHV_InitializeSystem(
    /* In */ int system,
    /* In */ int linkType,
    /* In */ void* connectionString,
    /* In */ const char* username,
    /* In */ const char* password,
    /* Out */ int* handle
);

int FakeHV_DeinitializeSystem(/* In */ int handle);

int FakeHV_GetCrateMap(
    /* In */ int handle,
    /* Out */ unsigned short* numberOfSlots,
    /* Out-Allocated */ unsigned short** listOfChannelsIndexedBySlot,
    /* Out-Allocated */ char** listOfModelsIndexedBySlot,
    /* Out-Allocated */ char** descriptionList,
    /* Out-Allocated */ unsigned short** listOfSerialNumbersIndexedBySlot,
    /* Out-Allocated */ unsigned char** listOfFirmwareSuffixesIndexedBySlot,
    /* Out-Allocated */ unsigned char** listOfFirmwarePrefixesIndexedBySlot
);

int FakeHV_GetChannelParameter(
    /* In */ int handle,
    /* In */ unsigned short slot,
    /* In */ const char* parameter,
    /* In */ unsigned short channelListSize,
    /* In */ const unsigned short* listOfChannelsToRead,
    /* Out */ void* listOfParameterValues
);

int FakeHV_SetChannelParameter(
    /* In */ int handle,
    /* In */ unsigned short slot,
    /* In */ const char* parameter,
    /* In */ unsigned short channelListSize,
    /* In */ const unsigned short* listOfChannelsToWrite,
    /* In */ void* newParameterValue
);

char* FakeHV_GetError(/* In */ int handle);

int FakeHV_Free(/* In */ void* resource);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SOURCE_PSU_FAKE_HV_LIBRARY_H */