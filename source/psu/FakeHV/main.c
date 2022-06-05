#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "FakeHVLibrary.h"

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

void test_valid_connection()
{
    int system = 0;
    int linkType = 0;
    char* connectionString = "";
    const char* username = "";
    const char* password = "";

    int handle = 55;
    int result = FakeHV_InitializeSystem(
        system, 
        linkType, 
        (void*) connectionString, 
        username, 
        password, 
        &handle
    );

    assert(result == FAKEHV_NORMAL);
    puts("[TEST] test_valid_connection: PASSED");
}

void test_invalid_connection()
{
    int system = 0;
    int linkType = 0;
    char* connectionString = "";
    const char* username = "";
    const char* password = "";

    int* handle = NULL;
    int result = FakeHV_InitializeSystem(
        system, 
        linkType, 
        (void*) connectionString, 
        username, 
        password, 
        handle
    );

    assert(result == -1);
    puts("[TEST] test_invalid_connection: PASSED");
}

void test_crate_map_bad_handle()
{
    unsigned short numberOfSlots;
    unsigned short* listOfChannelsIndexedBySlot;
    char* listOfModelsIndexedBySlot;
    char* descriptionList;
    unsigned short* listOfSerialNumbersIndexedBySlot;
    unsigned char* listOfFirmwareSuffixesIndexedBySlot;
    unsigned char* listOfFirmwarePrefixesIndexedBySlot;
    int handle = -1;

    int result = FakeHV_GetCrateMap(
        handle,
        &numberOfSlots,
        &listOfChannelsIndexedBySlot,
        &listOfModelsIndexedBySlot,
        &descriptionList,
        &listOfSerialNumbersIndexedBySlot,
        &listOfFirmwareSuffixesIndexedBySlot,
        &listOfFirmwarePrefixesIndexedBySlot
    );

    assert(result == -1);
    puts("[TEST] test_crate_map_bad_handle: PASSED");
};

void test_crate_map_bad_number_of_slots()
{
    int handle = 0;
    unsigned short* numberOfSlots = NULL;
    unsigned short* listOfChannelsIndexedBySlot;
    char* listOfModelsIndexedBySlot;
    char* descriptionList;
    unsigned short* listOfSerialNumbersIndexedBySlot;
    unsigned char* listOfFirmwareSuffixesIndexedBySlot;
    unsigned char* listOfFirmwarePrefixesIndexedBySlot;
    
    int result = FakeHV_GetCrateMap(
        handle,
        numberOfSlots,
        &listOfChannelsIndexedBySlot,
        &listOfModelsIndexedBySlot,
        &descriptionList,
        &listOfSerialNumbersIndexedBySlot,
        &listOfFirmwareSuffixesIndexedBySlot,
        &listOfFirmwarePrefixesIndexedBySlot
    );

    assert(result == -1);
    puts("[TEST] test_crate_map_bad_number_of_slots: PASSED");
}

void test_crate_map_return_values()
{
    int handle = 0;
    unsigned short numberOfSlots;
    unsigned short* listOfChannelsIndexedBySlot;
    char* listOfModelsIndexedBySlot;
    char* descriptionList;
    unsigned short* listOfSerialNumbersIndexedBySlot;
    unsigned char* listOfFirmwareSuffixesIndexedBySlot;
    unsigned char* listOfFirmwarePrefixesIndexedBySlot;
    
    int result = FakeHV_GetCrateMap(
        handle,
        &numberOfSlots,
        &listOfChannelsIndexedBySlot,
        &listOfModelsIndexedBySlot,
        &descriptionList,
        &listOfSerialNumbersIndexedBySlot,
        &listOfFirmwareSuffixesIndexedBySlot,
        &listOfFirmwarePrefixesIndexedBySlot
    );

    assert(listOfChannelsIndexedBySlot[0] == 4);
    assert(strcmp(listOfModelsIndexedBySlot, "VIRTUAL") == 0);
    assert(strcmp(descriptionList, "VIRTUAL") == 0);
    assert(listOfSerialNumbersIndexedBySlot[0] == 0);
    assert(listOfFirmwareSuffixesIndexedBySlot[0] == 0);
    assert(listOfFirmwarePrefixesIndexedBySlot[0] == 0);
    assert(result == 0);
    puts("[TEST] test_crate_map_return_values: PASSED");

    free(listOfChannelsIndexedBySlot);
    free(listOfModelsIndexedBySlot);
    free(descriptionList);
    free(listOfSerialNumbersIndexedBySlot);
    free(listOfFirmwareSuffixesIndexedBySlot);
    free(listOfFirmwarePrefixesIndexedBySlot);
}

void test_get_channel_bad_slot()
{
    int handle = 0;
    unsigned short slot = 1;
    const char* parameter;
    unsigned short channelListSize;
    const unsigned short* listOfChannelsToRead;
    int listOfParameterValues[4];

    int result = FakeHV_GetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );

    assert(result == -1);
    puts("[TEST] test_get_channel_bad_slot: PASSED");
}

void test_get_channel_bad_parameter()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "Bad";
    unsigned short channelListSize;
    const unsigned short* listOfChannelsToRead;
    int listOfParameterValues[4];

    int result = FakeHV_GetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );

    assert(result == -1);
    puts("[TEST] test_get_channel_bad_parameter: PASSED");
}

void test_get_channel_bad_channels_list()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "VMon";
    unsigned short channelListSize = 4;
    const unsigned short* listOfChannelsToRead = NULL;
    int listOfParameterValues[4];

    int result = FakeHV_GetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );

    assert(result == -1);
    puts("[TEST] test_get_channel_bad_channels_list: PASSED");
}

void test_get_channel_bad_parameters_list()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "VMon";
    unsigned short channelListSize = 4;
    const unsigned short listOfChannelsToRead[] = { 0, 1, 2, 3 };
    int* listOfParameterValues = NULL;

    int result = FakeHV_GetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );

    assert(result == -1);
    puts("[TEST] test_get_channel_bad_parameters_list: PASSED");
}

void test_get_channel_float_value_type()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "VMon";
    unsigned short channelListSize = 4;
    const unsigned short listOfChannelsToRead[] = { 0, 1, 2, 3 };
    float listOfParameterValues[4] = {1, 1, 1, 1};

    int result = FakeHV_GetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );

    // Hard to compare floats due to precision.
    assert(result == 0);
    assert(listOfParameterValues[0] > -0.001);
    assert(listOfParameterValues[0] < 0.010);
    puts("[TEST] test_get_channel_float_value_type: PASSED");
}

void test_get_channel_unsigned_value_type()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "VMon";
    unsigned short channelListSize = 4;
    const unsigned short listOfChannelsToRead[] = { 0, 1, 2, 3 };
    int listOfParameterValues[4] = {1, 1, 1, 1};

    int result = FakeHV_GetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );

    assert(result == 0);
    assert(listOfParameterValues[0] == 0);
    puts("[TEST] test_get_channel_unsigned_value_type: PASSED");
}

void test_set_channel_bad_handle()
{
    int handle = -1;
    unsigned short slot = 1;
    const char* parameter;
    unsigned short channelListSize;
    const unsigned short* listOfChannelsToWrite;
    int newParameterValue;

    int result = FakeHV_SetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToWrite,
        &newParameterValue
    );

    assert(result == -1);
    puts("[TEST] test_set_channel_bad_handle: PASSED");
}

void test_set_channel_bad_slot()
{
    int handle;
    unsigned short slot = 1;
    const char* parameter;
    unsigned short channelListSize;
    const unsigned short* listOfChannelsToWrite;
    int newParameterValue;

    int result = FakeHV_SetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToWrite,
        &newParameterValue
    );

    assert(result == -1);
    puts("[TEST] test_set_channel_bad_slot: PASSED");
}

void test_set_channel_bad_parameter()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "Bad";
    unsigned short channelListSize;
    const unsigned short* listOfChannelsToWrite;
    int newParameterValue;

    int result = FakeHV_SetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToWrite,
        &newParameterValue
    );

    assert(result == -1);
    puts("[TEST] test_set_channel_bad_parameter: PASSED");
}

void test_set_channel_bad_list_of_channels()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "Bad";
    unsigned short channelListSize;
    const unsigned short* listOfChannelsToWrite = NULL;
    int newParameterValue;

    int result = FakeHV_SetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToWrite,
        &newParameterValue
    );

    assert(result == -1);
    puts("[TEST] test_set_channel_bad_list_of_channels: PASSED");
}

void test_set_channel_new_parameter_is_null()
{
    int handle = 0;
    unsigned short slot = 0;
    const char* parameter = "Bad";
    unsigned short channelListSize;
    const unsigned short listOfChannelsToWrite[] = { 0, 1, 2, 3 };
    int* newParameterValue = NULL;

    int result = FakeHV_SetChannelParameter(
        handle,
        slot,
        parameter,
        channelListSize,
        listOfChannelsToWrite,
        newParameterValue
    );

    assert(result == -1);
    puts("[TEST] test_set_channel_new_parameter_is_null: PASSED");
}

void test_get_error()
{
    printf("%s\n", FakeHV_GetError(0));
}

void test_free()
{
    int handle = 0;
    unsigned short numberOfSlots;
    unsigned short* listOfChannelsIndexedBySlot;
    char* listOfModelsIndexedBySlot;
    char* descriptionList;
    unsigned short* listOfSerialNumbersIndexedBySlot;
    unsigned char* listOfFirmwareSuffixesIndexedBySlot;
    unsigned char* listOfFirmwarePrefixesIndexedBySlot;
    
    int result = FakeHV_GetCrateMap(
        handle,
        &numberOfSlots,
        &listOfChannelsIndexedBySlot,
        &listOfModelsIndexedBySlot,
        &descriptionList,
        &listOfSerialNumbersIndexedBySlot,
        &listOfFirmwareSuffixesIndexedBySlot,
        &listOfFirmwarePrefixesIndexedBySlot
    );

    assert(listOfChannelsIndexedBySlot[0] == 4);
    assert(strcmp(listOfModelsIndexedBySlot, "VIRTUAL") == 0);
    assert(strcmp(descriptionList, "VIRTUAL") == 0);
    assert(listOfSerialNumbersIndexedBySlot[0] == 0);
    assert(listOfFirmwareSuffixesIndexedBySlot[0] == 0);
    assert(listOfFirmwarePrefixesIndexedBySlot[0] == 0);
    assert(result == 0);
    puts("[TEST] test_crate_map_return_values: PASSED");

    FakeHV_Free(listOfChannelsIndexedBySlot);
    FakeHV_Free(listOfModelsIndexedBySlot);
    FakeHV_Free(descriptionList);
    FakeHV_Free(listOfSerialNumbersIndexedBySlot);
    FakeHV_Free(listOfFirmwareSuffixesIndexedBySlot);
    FakeHV_Free(listOfFirmwarePrefixesIndexedBySlot);

    puts("Values freed");
}

int main(int argc, char** argv)
{
    test_valid_connection();
    test_invalid_connection();
    test_crate_map_bad_handle();
    test_crate_map_bad_number_of_slots();
    test_crate_map_return_values();
    test_get_channel_bad_slot();
    test_get_channel_bad_parameter();
    test_get_channel_bad_channels_list();
    test_get_channel_bad_parameters_list();
    test_get_channel_float_value_type();
    test_get_channel_unsigned_value_type();
    test_set_channel_bad_handle();
    test_set_channel_bad_slot();
    test_set_channel_bad_parameter();
    test_set_channel_bad_list_of_channels();
    test_set_channel_new_parameter_is_null();
    test_get_error();
    test_free();
    puts("Testing complete.");
    return 0;
}