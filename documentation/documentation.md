# Documentation
### Overview
Welcome to the MSU sMDT DCCS Project Documentation. Located next to the title of this documentation is a small 
table of contents icon, which can be used to navigate this documentation. 

### How to Use This Documentation
This documentation is meant to be read from chapter to chapter, much like a novel. However, this documentation will
often refer to files that are in the project itself. So the recommended use is to have this documentation opened 
alongside the the relevant source files. In this side-to-side manner, one can read the chapter and follow along with
the source code, simultaneously. 

### Appendices
Appendices are used for items that wouldn't fit in the chapter scheme (i.e. the section on bugs). 

## Chapter 1: The Outline of the Project
### The Source Tree
There are three main folders to consider in the source tree: `dcch`, `gui`, and `psu`. The `dcch` folder holds all the 
source files that relate to the Dark Current Control Hardware. The `gui` folder holds all the source files that
relate to the Graphical User Interface. The `psu` folder holds all the source files related to controlled the
Power Supply Unit. The last file located in the source tree is the `main.cpp` file, which is the entry point for
the entire program. 

## Chapter 2: The Power Supply
### Introduction
This project utilizes a special power supply: the CAEN DT1471HET power supply. In order to control this power supply,
this project utilizes a special CAEN HW Wrapper library, which is obtained from CAEN themselves. The source files which
control the power supply can be found within the `source/psu` directory. This documentation does not seek to explain
**how** these files manage to control the power supply, but rather it seeks to provide details on the intricacies of
the files, as well as the idiosyncrasies of the files located within. 

### Interfacing with the CAEN HV Wrapper Library
The CAEN HV Wrapper Library is library written in C. However, this project is written in C++. As such, we need to 
interface with C. This is performed in the `psu/hv_channel_interface.hpp` source file. Details of the interface can
be found within the file itself. In order to expedite this documentation, we shall refer to the CAEN HV Wrapper Library
simply as *HVLIB*. 

In the HVLIB, we find two paradigms: the board and the channel. Early development of the project made the attempt to
directly embed the channel-related functions into code, however this proved to be quite exhaustive and repetitive. As
well, many bugs were impinged to these embedded-functions. As such, the creation of the `hv_channel_interface` allowed
for the project to abstract away the particular interface to C. 

### The Single Channel Issue
Reading through the code reveals one interesting feature: the use of a single channel in the get and set functions. 
The power supply clearly has more than a single channel, however the code does not reflect that. The information of the
channels is abstracted away from the `hv_channel_interface`, and is instead placed with the `psu/channel.hpp` header
and `psu/channel.cpp` source file. As such, the `hv_channel_interface` only returns the information of a single
channel -- with the particular channel number passed in as an argument. 

### The Channel Class
The channel interface is as follows: a struct is used to encapsulate information about a singular channel (mostly
information related to bookkeeping), with a channel manager being used to house and control the array of channels.
While it seems that this approach is rather contrived, the benefit is the decentralization of error: any issues that
arise from the HVLIB itself can be corrected through the `hv_channel_interface.hpp` source file, without compromising
the `channel.cpp` code. At least, that is how it should work in principle. 

#### Shorts and Floats?
It should be noted that, as the project interfaces with C code, we should be mindful of C data types. As the library
uses the `short` and `float` data type, we conform to this by using that same data type in C++. While we ought to be
able to promote these data types to `int` and `double`, there were quite a few problems with the bookkeeping of these
promoted types. Therefore, we simply pass the `short` and `float` as-is. Given the precision of the data, we expect
these data types to be sufficient.

### The PSU Class
Given that we operate on the channel level, why is there the need for the PSU class? 

Well, that is a bit of a funny problem. Firstly, the PSU class existed before the channel class. Therefore, we retain
it as legacy. But secondly, there are a few things we need to do before we can even operate on any one channel: mostly,
we need to initialize the PSU. 

But the PSU class also offers the following interface: we can pre-designate a set of testing procedures, and then 
simply call the `psu::start_test()` member function. It would make little sense to house this functionality within the
`channel_manager` given that the idea of employing the channels for testing is outside the scope of managing the 
internal properties of every channel. Simply put: the `channel_manager` controls the channels, and the PSU class
controls the testing. 

## Chapter 3: TBD

## A-1: Hilarious Bugs
### CAEN HV Wrapper
This wrapper has introduced many bugs. Bug among the most annoying is the following: the library itself must define
a whole bunch of types, macros, and similar items. This was one of the most confusing bugs to identify due to its
relative obscurity. However, the following bug exists: if the CAEN HV Wrapper Library (HVLIB) is included before 
**any** Qt Libraries, the compiler will complain with seemingly nonsensical errors: 
`'char' followed by 'char' is illegal` and whatnot. The source of the errors is that the HVLIB defines a whole bunch
of things that Qt defines. Due to this, the HVLIB *must be the last library included* in order for the program to
compile. However, it is not known what kinds of bugs this might introduce down the line.