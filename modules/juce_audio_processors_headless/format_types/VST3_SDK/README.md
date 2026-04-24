<div style="text-align:center">
<img src="https://steinbergmedia.github.io/vst3_doc/gfx/vst3_logo.jpg" alt="VST 3 SDK" /></div>

# Welcome to VST SDK 3.8.x

## Table Of Contents

1. [The VST SDK package](#100)
1. [System requirements](#200)
1. [About VST plug-ins in general](#300)
1. [About VST 3](#400)
1. [How to build VST 3](#500)
1. [Contributing](#600)
1. [License & Usage guidelines](#700)

<div id='100'/>

## The VST SDK package contains

- VST 3 API
- VST 3 Implementation Helper Classes
- AAX, AUv3 and AU Wrappers
- VST 3 plug-ins Examples

The full **VST 3 SDK** is available [here!](https://www.steinberg.net/en/company/developers.html). It contains:

- VST 3 plug-in Test Host Application/Validator,
- the **Steinberg VST 3 Plug-In SDK Licensing Agreement** that you have to sign if you want to develop or host **VST 3** plug-ins.

<div id='200'/>

## System requirements

Supported Platforms:

| Operating System                  | Architecture                  | Compiler              | Notes                             |
| :-------------------------------- | :---------------------------- | :-------------------- | :-------------------------------- |
| Windows 11                        | x86, x86_64, arm64, arm64EC   | MSVC 2022             |                                   |
| Windows 8.1/10                    | x86, x86_64                   | MSVC 2019, MSVC 2022  |                                   |
| macOS 10.13 - 15                  | x86, x86_64, Apple Silicon    | Xcode 10 - 16         |                                   |
| iOS 13 - 18                       | arm64                         | Xcode 11 - 16         |                                   |
| Linux - Ubuntu 24.04 LTS          | x86_64                        | GCC 13.3 and higher   | Visual Studio Code, Qt Creator    |

---
<div id='300'/>

## About VST plug-ins in general

A VST plug-in is an audio processing component that is utilized within a host application. This host application provides the audio or/and event streams that are processed by the plug-in's code. Generally speaking, a VST plug-in can take a stream of audio data, apply a process to the audio, and return the result to the host application. A VST plug-in performs its process normally using the processor of the computer. The audio stream is broken down into a series of blocks. The host supplies the blocks in sequence. The host and its current environment control the block-size. The VST plug-in maintains the status of all its own parameters relating to the running process: The host does not maintain any information about what the plug-in did with the last block of data it processed.

From the host application's point of view, a VST plug-in is a black box with an arbitrary number of inputs, outputs (Event (MIDI) or Audio), and associated parameters. The host needs no implicit knowledge of the plug-in's process to be able to use it. The plug-in process can use whatever parameters it wishes, internally to the process, but depending on the capabilities of the host, it can allow the changes to user parameters to be automated by the host.

The source code of a VST plug-in is platform independent, but the delivery system depends on the platform architecture:

- On **Windows**, a VST plug-in is a multi-threaded DLL (Dynamic Link Library), recently packaged into a folder structure.
- On **Mac OS X**, a VST plug-in is a Mach-O Bundle
- On **Linux**, a VST plug-in is a package

To learn more about VST you can:

- subscribe to the [VST Developer Forum](https://sdk.steinberg.net)
- check the 3rd Party Developer Support section at [www.steinberg.net](https://www.steinberg.net/en/company/developers.html)
- check the VST 3 SDK online documentation under: [steinbergmedia.github.io/vst3_dev_portal](https://steinbergmedia.github.io/vst3_dev_portal/pages/index.html)
- check the online documentation under: [steinbergmedia.github.io/vst3_doc](https://steinbergmedia.github.io/vst3_doc)

 ---
<div id='400'/>

## About VST 3

VST 3 is a general rework of the long-serving VST plug-in interface. It is not compatible with the older VST versions, but it includes some new features and possibilities. We have redesigned the API to make it not only far easier and more reliable for developers to work with, but have also provided completely new possibilities for plug-ins. These include:

### 1. Improved Performance with the Silence Flag

Processing can optionally be applied to plug-ins only when audio signals are present on their respective inputs, so VST 3 plug-ins can apply their processing economically and only when it is needed.

### 2. Multiple Dynamic I/Os

VST 3 plug-ins are no longer limited to a fixed number of inputs and outputs, and their I/O configuration can dynamically adapt to the channel configuration. Side-chains are also very easily realizable. This includes the possibility to deactivate unused busses after loading and even reactivate those when needed. This cleans up the mixer and further helps to reduce CPU load.

### 3. Sample-accurate Automation

VST 3 also features vastly improved parameter automation with sample accuracy and support for ramped automation data, allowing completely accurate and rapid parameter automation changes.

### 4. Logical Parameter Organization

The VST 3 plug-in parameters are displayed in a tree structure. Parameters are grouped into sections which represent the structure of the plug-in. Plug-ins can communicate their internal structure for the purpose of overview, but also for some associated functionality (eg. program-lists).

### 5. Resizeable UI Editor

VST 3 defines a way to allow resizing of the plug-in editor by a user.

### 6. Mouse Over Support

The host could ask the plug-in which parameter is under the mouse.

### 7. Context Menu Support

VST 3 defines a way to allow the host to add its own entries in the plug-in context menu of a specific parameter.

### 8. Channel Context Information

A VST 3 plug-in could access some channel information where it is instantiated: name, color, ...

### 9. Note Expression

VST 3 defines with Note Expression a new way of event controller editing. The plug-in is able to break free from the limitations of MIDI controller events by providing access to new VST 3 controller events that circumvent the laws of MIDI and provide articulation information for each individual note (event) in a polyphonic arrangement according to its noteId.

### 10. 3D Support

VST 3 supports new speaker configurations like Ambisonic, Atmos, Auro 3D or 22.2.

### 11. Factory Concept

VST 3 plug-in library could export multiple plug-ins and in this way replaces the shell concept of VST 2 (kPlugCategShell).

### 12. Support Remote control Representation

VST 3 plug-in can deliver a specific parameter mapping for remote controls like Nuage.

### 13. Others

While designing VST 3, we performed a careful analysis of the existing functionality of VST and rewrote the interfaces from scratch. In doing so, we focused a lot on providing clear interfaces and their documentation in order to avoid usage errors from the deepest possible layer.
Some more features implemented specifically for developers include:

- More stable technical host/plug-in environment
- Advanced technical definition of the standard
- Modular approach
- Separation of UI and processing
- Advanced Preset System
- Multiple plug-ins per Library
- Test Host included
- Automated Testing Environment
- Validator (small command line Test Host) and plug-in examples code included

---
<div id='500'/>

## How to build VST 3

### Get the source code from GitHub

```c
git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
```

### Build the examples on Windows

- Create a folder for the build and move to this folder (using cd):

```c
mkdir build
cd build
```

- Generate the Solution/Projects: provide the path of the Project where CMakeLists.txt is located:

```c
// examples:
cmake.exe -G "Visual Studio 17 2022" -A x64 ..\vst3sdk
// or without symbolic links
cmake.exe -G "Visual Studio 17 2022" -A x64 ..\vst3sdk -DSMTG_CREATE_PLUGIN_LINK=0
// or by using the local user program folder (FOLDERID_UserProgramFilesCommon) as VST3 folder
cmake.exe -G "Visual Studio 17 2022" -A x64 -DSMTG_PLUGIN_TARGET_USER_PROGRAM_FILES_COMMON=1
```

- Now you can build the plug-in (you can use Visual Studio too):

```c
msbuild.exe vstsdk.sln
// (or alternatively for example for release)
cmake --build . --config Release
```
Note: If you have any issue with symbolic links, check [Preparation on Windows](https://steinbergmedia.github.io/vst3_dev_portal/pages/Getting+Started/Preparation+on+Windows.html) for potential solutions.

### Build the examples on macOS

- Create a folder for the build and move to this folder (using cd):

```c
mkdir build
cd build
```

- Generate the Solution/Projects: provide the path of the Project where CMakeLists.txt is located:

```c
// For XCode:
cmake -GXcode ../vst3sdk
// Without XCode (here debug variant):
cmake -DCMAKE_BUILD_TYPE=Debug ../
```

- Now you can build the plug-in (you can use XCode too):

```c
xcodebuild 
// (or alternatively for example for release)
cmake --build . --config Release
```

### Build the examples on Linux

- Install the required packages [Package Requirements](https://steinbergmedia.github.io/vst3_dev_portal/pages/Getting+Started/How+to+setup+my+system.html#for-linux)
- Create a folder for the build and move to this folder (using cd):

```c
mkdir build
cd build
```

- Generate the Solution/Projects: provide the path of the Project where CMakeLists.txt is located:

```c
cmake ../vst3sdk
```

- Now you can build the plug-in:

```c
make
// (or alternatively for example for release)
cmake --build . --config Release
```

### Build using cmake-gui

- start the cmake-gui Application
- **Browse Source...**: select the folder vst3sdk
- **Browse Build...**: select a folder where the outputs (projects/...) will be created. Typically, a folder named "build"
- you can check the SMTG Options
- Press **Configure**
- Press **Generate** and the project will be created

<div id='600'/>

## Contributing

For bug reports and features requests, please visit the [VST Developer Forum](https://sdk.steinberg.net)


<div id='700'/>

## License & Usage guidelines

- **VST 3 SDK** is under [MIT license](https://tlo.mit.edu/understand-ip/exploring-mit-open-source-license-comprehensive-guide).

- Developers can adopt the [MIT license](https://tlo.mit.edu/understand-ip/exploring-mit-open-source-license-comprehensive-guide) for full open-source integration.

- Licensing under **GPLv3** and the **Steinberg proprietary license** is no longer available.

- Code licensed under [MIT license](https://tlo.mit.edu/understand-ip/exploring-mit-open-source-license-comprehensive-guide) can be used, modified, and redistributed freely — including in commercial products — provided [MIT license](https://tlo.mit.edu/understand-ip/exploring-mit-open-source-license-comprehensive-guide) terms are followed. 

More details are found at [www.steinberg.net/sdklicenses_vst3](http://www.steinberg.net/sdklicenses_vst3)

### Trademark and Logo Usage 

#### General Principle 

Trademark usage (e.g. "VST" name or logo) is optional under [MIT license](https://tlo.mit.edu/understand-ip/exploring-mit-open-source-license-comprehensive-guide), but if used, must comply with Steinberg's official trademark rules. 

#### Permitted Use (if selected) 

If you choose to display the **VST Compatible Logo** or refer to the **VST trademark**: 

 - Logos must be used in unaltered form.
 - Use must follow the official usage guidelines provided in the SDK.
 - The trademark/logo must appear only in clear product-related contexts, such as: 
    - Splash screens
    - About boxes
    - Product websites
    - Documentation and manuals 

This ensures legal compliance and brand consistency. 

#### Prohibited Use 

- Using “VST” (or derivatives like “VSTi”) in company or product names.
- Applying the logo or trademark to non-VST-compatible products.
- Associating the trademark with offensive, obscene, or inappropriate content.
 
#### VST trademark and/or VST Compatible Logo 

If you choose to display the **VST trademark** and/or **VST Compatible Logo**, you must observe the *Steinberg VST Usage Guidelines*. 

*Purpose*: To assist Steinberg in enforcing and protecting the **VST trademark** in international jurisdictions. 

#### Notes for Developers 

- The *Steinberg VST Usage Guidelines* are included in the SDK.
- Use of branding is not required under [MIT license](https://tlo.mit.edu/understand-ip/exploring-mit-open-source-license-comprehensive-guide), but if you choose to use it, trademark compliance is mandatory.
- Non-compliance with logo guidelines does not affect your MIT rights (but may result in trademark violations).
 
#### Best Practice Recommendation 

Although optional, we encourage developers using VST under [MIT license](https://tlo.mit.edu/understand-ip/exploring-mit-open-source-license-comprehensive-guide) to follow the *Steinberg VST Usage Guidelines*.It fosters ecosystem-wide recognition, improves end-user trust, and supports the long-term visibility of the **VST** brand.
