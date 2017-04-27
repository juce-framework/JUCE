# The BLOCKS SDK

## Overview

This repository contains the source code of the BLOCKS SDK, which is licensed under the permissive ISC license.

Much more comprehensive documentation can be found at http://developer.roli.com/documentation/the_standalone_blocks_sdk.html.

The `SDK` directory contains the source code required to compile a library containing the BLOCKS SDK functionality. In the `SDK/Build` directory you will find an Xcode project, a Visual Studio solution and a Linux Makefile which are configured to create a static library on the MacOS, Windows and Linux platforms respectively. Simply use the one appropriate for your system. Once you have created this library you can access BLOCKS functionality by linking this library into your application and `#include`-ing the provided header file, `BlocksHeader.h`.

The `examples` directory contains some sample code which uses the BLOCKS SDK library. Here we again provide an Xcode project, a Visual Studio solution and a Linux Makefile to allow the examples to be built on the corresponding platform. The Xcode project and Visual Studio solution compile the BLOCKS SDK library automatically, whereas the Linux Makefile requires the BLOCKS SDK library to be manually build first.

## Quick start guide

### MacOS with Xcode

Open `examples/BlockFinder/MacOS/BlockFinder.xcodeproj` with Xcode and compile and execute the example application. This will both create the BLOCKS SDK library and provide a very simple demonstration of how to use it. Use this as a base to develop your own BLOCKS creation!

### Windows with Visual Studio

Use the same procedure as MacOS, but instead open `examples/BlockFinder/Windows/BlockFinder.vcxproj`.

### Linux

Use the following procedure to compile the demo app:

```shell
cd SDK/Build/Linux
make
cd ../../../examples/BlockFinder/Linux/
make
```

This will produce the executable `BlockFinder` in the `Debug` directory.

## Using the SDK library

### Compiling the library

The source code for the BLOCKS SDK library is contained within the `SDK` directory of this repository.  Here you will find header files that you can include in your own projects and the `Build` subdirectory contains an XCode project, a Visual Studio project and a Linux Makefile for compiling the SDK source code into a static library. Use the appropriate choice for your platform, select either the "Debug" or "Release" configuration, and build the project.

For MacOS this will produce `libBLOCKS-SDK.a` in either the `SDK/Build/MacOS/Debug/` or `SDK/Build/MacOS/Release/` directory, for Linux this will produce `libBLOCKS-SDK.a` in either the `SDK/Build/Linux/Debug/` or `SDK/Build/Linux/Release/` directory, and for Windows this will produce `BLOCKS-SDK.lib` in either the `SDK\Build\Windows\x64\Debug` or `SDK\Build\Windows\x64\Release` folder.

### Using the SDK header file

To use BLOCKS classes and functions in your application you must include the `BlocksHeader.h` file in your source code. You also need to tell the compiler to look in the `SDK` directory for additional header files, which you can configure inside your XCode or Visual Studio project. If you are using the command line to compile your application then you can see an example of how to do this in `examples/BLOCKS-SDK/BlockFinder/Linux/Makefile` (which is also configured for MacOS, despite being located inside the Linux directory).

### Linking against the SDK library

You must also tell your compiler where to find the SDK static library before your BLOCKS application will compile, and include all of the dependencies for your platform, which are listed in the Dependencies section of this README. Again, this is configured in your XCode or Visual Studio project, but if you are using the command line you can see an example of how to do this in `examples/BLOCKS-SDK/BlockFinder/Linux/Makefile` (which, again, is also configured for MacOS).

## Dependencies

- A C++11 compatible compiler

### MacOS frameworks

- Accelerate
- AudioToolbox
- CoreAudio
- CoreMIDI

### Linux packages

- x11
- alsa
- libcurl

