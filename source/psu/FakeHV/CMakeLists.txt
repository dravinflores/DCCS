cmake_minimum_required(VERSION 3.20)
project(FakeHV LANGUAGES C)

if (TEST_FAKEHV)
    add_executable(FakeHVTest main.c FakeHVLibrary.c)
endif()

add_library(
    FakeHV
    STATIC
        FakeHVLibrary.h
        FakeHVLibrary.c
)