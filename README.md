# MSU sMDT Dark Current Control Software

Welcome to the MSU sMDT Dark Current Control Software (MSU-SMDT-DCCS) GitHub 
repository. This is where we host the program that controls our Dark Current 
Testing Station. The purpose of this program specifically is to interface with 
our CAEN DT1471HET Power Supply Unit (PSU), as well as our Dark Current
Control Hardware (DCCH) board, in order to automate our testing.

As such, this program's main goals are as follows:
1. To offer an easy-to-use and intuitive Graphical User Interface (GUI)
2. To have a readable, complete, and extensive codebase 
3. To place the control software under version control with Git

This program is written in C++ and utilizes a various assortment of external
libraries such as the [Qt][] framework, as well as other libraries meant
to either aid development or aid the stability of the program.

This program was designed specifically for our PSU model. As well, this program
was designed specifically for Windows 10. Thus, our use-case is very specific,
and the code will reflect that. 

In order to encourage a maintainable and stable codebase, we will try to 
incorporate the principles of modern C++, such as: smart pointers, the STL
container library, the RAII paradigm, etc. 

[Qt]: https://www.qt.io/


## Status of the Project
The roadmap for development is the following:
- [x] Connect to the PSU using the CAEN HV Wrapper Library
- [ ] Create a basic PSUOBJ, DATAOBJ, LOGOBJ, and ERROROBJ
- [ ] Connect to a microcontroller to control the DCCH with code
- [ ] Create a basic DCCHCONOBJ
- [ ] Connect PSUOBJ to DCCHCONOBJ
- [ ] Ensure proper logging and error handling
- [ ] Create the GUI using Qt
- [ ] Ensure the program can execute a Dark Current Test


## Setting Up The Development Environment
### Obtaining Visual Studio 2022 and MSVC
Oh joy! The absolute fun part! (I mean this with as much sarcasm as I can 
conjure up). This part is terrible. And it is especially terrible on Windows.
Well...I suppose let's just dive in.

First things first, we need a C++ compiler. However, since we're on Windows,
we only have two choices: MSVC and Mingw. We will choose the former. 

Install [Visual Studio Community 2022][VS]. When in the installer, choose the
*Desktop development with C++* option, as shown below.

![IMG-1](documentation/images/desktop-development-cpp.PNG)

As well, opt for the Windows 10 SDK shown below, which can be found in the
*Individual components* tab.

![IMG-2](documentation/images/windows-10-sdk.PNG)

### Vcpkg
As we are on Windows, and seeing as setting up Conan was proving far too
difficult due to some internal mixups with Visual Studio and MSVC, we are opting
to use the [Vcpkg][] package manager instead. A plus is that this removes the
dependency on Python and Pip. 

Firstly, we need to figure out where we want to store Vcpkg. According to the
readme documentation, which can be found on the main [GitHub page][Vcpkg], the 
recommended location for Vcpkg is something like `C:\src\vcpkg` or 
`C:\dev\vcpkg`. So we enter into whatever directory you choose, and run the
following commands:

```cmd
> git clone https://github.com/microsoft/vcpkg
> .\vcpkg\bootstrap-vcpkg.bat
> .\vcpkg\vcpkg integrate install
```


### A Little Note on Vcpkg
A big part of the reason to use Vcpkg was that it made installing external
dependencies much simpler. However, Vcpkg itself has one notable flaw: it 
defaults to generating dynamic libraries. This is a huge pain, as this meant
having to package up all the DLL's with the executable. 

It took a great amount of time to figure out how in the world to overwrite this
default behavior. So it should be noted that, without directly specifying the
use of static libraries, Vcpkg will resign to create dynamic libraries. Just
something to be aware of.


### Testing The Development Environment
We can now check that everything has been set up properly. Navigate into the
following folder: `test/check-development-environment`. Inside, there is a demo
project which we can compile. The source code looks like the following:
```C++
// This file is test/check-development-environment/source/main.cpp

// We'll need to use Vcpkg to install this.
#include <fmt/core.h>

int main(int argc, char** argv)
{
    fmt::print("The build system has been successfully configured.\n");
    return 0;
}
```

We can compile this code by issuing the following commands. Do note that
multi-line commands are usually not accepted, and are parsed as separate
commands. In order to rectify this, delete the newline and paste as a single
command.
```cmd
> cd test/check-development-environment
> mkdir build
> cmake -B build -S ./
    -DCMAKE_TOOLCHAIN_FILE=C:\development\vcpkg\scripts\buildsystems\vcpkg.cmake
> cmake --build build
> cd build/Debug
> ./test_program.exe
```

It should be noted that the CMake build command is one entire line. It should
be noted that Windows treats multi-line commands as separate commands.

If the program installs the [fmt][] dependency, builds the program, executes,
and prints out the test string, then we know that the build system has been
properly set up. Congratulations!

[VS]: https://visualstudio.microsoft.com/
[Vcpkg]: https://github.com/microsoft/vcpkg
[fmt]: https://github.com/fmtlib/fmt


## Installing the CAEN HV Wrapper Library
The CAEN HV Wrapper Library download can be found [here][hv-wrapper]. It is 
recommended that the program be installed to: 
`C:\Program Files\CAEN\HV\CAENHVWrapper`, which happens to be the default
location.

[hv-wrapper]: https://www.caen.it/products/caen-hv-wrapper-library/


### Testing the CAEN HV Wrapper Library
The test project can be found at `test/channel-parameters`. This program can
be compiled using the following commands:
```cmd
> cd test/channel-parameters
> mkdir build
> cd build
> cmake ..
> cmake --build ./
> ./Debug/test.exe
```

The purpose of this program is to connect to the power supply, initialize the
power supply, and then make the attempt to read out the channel parameters.


## Building The Project
In order to build this project, MSVC, Vcpkg, and CMake must be installed. As
well, the CAEN HV Wrapper Library must be installed. Note that multi-line
commands are usually not parsed as a single command. The newlines must be
removed before execution. To build, issue the following commands:

```cmd
> mkdir build
> cmake -B build -S ./
    -DCMAKE_BUILD_TYPE=Debug 
    -DCMAKE_TOOLCHAIN_FILE=C:\development\vcpkg\scripts\buildsystems\vcpkg.cmake
    -DVCPKG_TARGET_TRIPLET=x64-windows-static 
    -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
> cmake --build build
> cd build/Debug
> ./dccs.exe
```
