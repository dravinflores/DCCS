cmake_minimum_required(VERSION 3.20)
project(FakeHV LANGUAGES C)

if (TEST_FAKEHV)
    add_executable(FakeHV main.c FakeHVLibrary.c)
else()
    add_library(
        FakeHV
        STATIC
            FakeHVLibrary.h
            FakeHVLibrary.c
    )
endif()

