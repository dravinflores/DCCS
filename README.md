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


## Setting Up The Development Environment
### Revised
The original project utilized Vcpkg in order to install dependencies. This
method brought with it many issues that proved time consuming to fix.
Furthermore, it was not possible to use a pre-installed version of Qt alongside
the installed Vcpkg packages. In light of this, the build system was updated
to no longer require Vcpkg. The new build system requires an up-to-date 
installation of CMake, as well as Git. 

It should be noted that the two test programs located in the `test` folder
used the Vcpkg build system. The `test/check-development-environment` test
project is retained for legacy reasons. The `test/channel-parameters` test
project does not require Vcpkg, and remains as is. 

### Obtaining Visual Studio 2022 and MSVC
First things first, we need a C++ compiler. As we are on Windows, we will opt
to use MSVC.

Install [Visual Studio Community 2022][VS]. When in the installer, choose the
*Desktop development with C++* option, as shown below.

![IMG-1](documentation/images/desktop-development-cpp.PNG)

As well, opt for the Windows 10 SDK shown below, which can be found in the
*Individual components* tab.

![IMG-2](documentation/images/windows-10-sdk.PNG)


### Testing The Development Environment
This test is retained for legacy reasons. It will require Vcpkg to build and
execute.

## Installing the CAEN HV Wrapper Library
The CAEN HV Wrapper Library download can be found [here][hv-wrapper]. The 
project assumes that it will be installed in the directory
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
To build the project, execute the following commands

```cmd
> mkdir build
> cmake -B build -S ./ -DSPDLOG_FMT_EXTERNAL=true
> cmake --build build
> ./build/Debug/dccs.exe
```

## Documentation
Further documentation on the project can be found as the markdown document
`documentation/documentation.md`.