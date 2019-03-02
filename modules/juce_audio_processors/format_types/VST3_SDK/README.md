# Welcome to VST SDK 3.6.12

## Table Of Contents
1. [The VST SDK package](#100)
1. [System requirements](#200)
1. [About VST Plug-ins in general](#300)
1. [About VST 3](#400)
1. [How to build VST 3](#500)
1. [License & Usage guidelines](#600)

<div id='100'/>

## The VST SDK package contains:
- The VST 3 API
- VST 3 Implementation Helper Classes
- AAX, AU and VST 2 wrappers
- VST 3 Plug-ins Examples

The full VST 3 SDK is available [here!](https://www.steinberg.net/en/company/developers.html). It contains :
- a VST 3 Plug-in Test Host Application/Validator,
- the <b>Steinberg VST 3 Plug-In SDK Licensing Agreement</b> that you have to sign if you want to develop or host VST 3 Plug-Ins.


<div id='200'/>

## System requirements

Supported OS:

- Microsoft Windows 7-10
- Apple OSX 10.9-10.13
- Apple iOS 8-9
- Linux (Beta version)

Supported IDE:
- Visual Studio 2015/2017
- minimum Xcode 7
- Qt Creator

---
<div id='300'/>

## About VST Plug-ins in general
A VST Plug-in is an audio processing component that is utilized within a host application. This host application provides the audio or/and event streams that are processed by the Plug-in's code. Generally speaking, a VST Plug-in can take a stream of audio data, apply a process to the audio, and return the result to the host application. A VST Plug-in performs its process normally using the processor of the computer. The audio stream is broken down into a series of blocks. The host supplies the blocks in sequence. The host and its current environment control the block-size. The VST Plug-in maintains the status of all its own parameters relating to the running process: The host does not maintain any information about what the Plug-in did with the last block of data it processed.

From the host application's point of view, a VST Plug-in is a black box with an arbitrary number of inputs, outputs (Event (MIDI) or Audio), and associated parameters. The host needs no implicit knowledge of the Plug-in's process to be able to use it. The Plug-in process can use whatever parameters it wishes, internally to the process, but depending on the capabilities of the host, it can allow the changes to user parameters to be automated by the host.

The source code of a VST Plug-in is platform independent, but the delivery system depends on the platform architecture:
- On **Windows**, a VST Plug-in is a multi-threaded DLL (Dynamic Link Library).
- On **Mac OS X**, a VST Plug-in is a Mach-O Bundle
- On **Linux**, a VST Plug-in is a package

To learn more about VST you can subscribe to the [VST Developer Forum](https://sdk.steinberg.net) - check the 3rd Party Developer Support section at [www.steinberg.net](http://www.steinberg.net). 
 
 ---
<div id='400'/>

## About VST 3
VST 3 is a general rework of the long-serving VST Plug-in interface. It is not compatible with the older VST versions, but it includes some new features and possibilities. We have redesigned the API to make it not only far easier and more reliable for developers to work with, but have also provided completely new possibilities for Plug-ins. These include:

### 1. Improved Performance with the Silence Flag 
Processing can optionally be applied to Plug-ins only when audio signals are present on their respective inputs, so VST 3 Plug-ins can apply their processing economically and only when it is needed. 

### 2. Multiple Dynamic I/Os 
VST 3 Plug-ins are no longer limited to a fixed number of inputs and outputs, and their I/O configuration can dynamically adapt to the channel configuration. Side-chains are also very easily realizable. This includes the possibility to deactivate unused buses after loading and even reactivate those when needed. This cleans up the mixer and further helps to reduce CPU load. 

### 3. Sample-accurate Automation 
VST 3 also features vastly improved parameter automation with sample accuracy and support for ramped automation data, allowing completely accurate and rapid parameter automation changes. 

### 4. Logical Parameter Organization 
The VST 3 Plug-in parameters are displayed in a tree structure. Parameters are grouped into sections which represent the structure of the Plug-in. Plug-ins can communicate their internal structure for the purpose of overview, but also for some associated functionality (eg. program-lists). 

### 5. Resizeable UI Editor 
VST 3 defines a way to allow resizing of the Plug-in editor by a user. 

### 6. Mouse Over Support 
The Host could ask the Plug-in which parameter is under the mouse. 

### 7. Context Menu Support
VST 3 defines a way to allow the host to add its own entries in the Plug-in context menu of a specific parameter.

### 8. Channel Context Information
A VST 3 Plug-in could access some channel information where it is instantiated: name, color,...

### 9. Note Expression 
VST 3 defines with Note Expression a new way of event controller editing. The Plug-in is able to break free from the limitations of MIDI controller events by providing access to new VST 3 controller events that circumvent the laws of MIDI and provide articulation information for each individual note (event) in a polyphonic arrangement according to its noteId. 

### 10. 3D Support
VST 3 supports new speaker configurations like Ambisonic, Atmos, Auro 3D or 22.2.

### 11. Factory Concept 
VST 3 Plug-in library could export multiple Plug-ins and in this way replaces the shell concept of VST 2 (kPlugCategShell).

### 12. Support Remote control Representation
VST 3 Plug-in can deliver a specific parameter mapping for remote controls like Nuage.

### 13. Others
While designing VST 3, we performed a careful analysis of the existing functionality of VST and rewrote the interfaces from scratch. In doing so, we focused a lot on providing clear interfaces and their documentation in order to avoid usage errors from the deepest possible layer.
Some more features implemented specifically for developers include:
- More stable technical Host/Plug-in environment
- Advanced technical definition of the standard
- Modular approach
- Separation of UI and processing
- Advanced Preset System
- Multiple Plug-ins per Library
- Test Host included
- Automated Testing Environment
- Validator (small command line Test Host) and Plug-in examples code included

---
<div id='500'/>

## How to build VST3

### Get the source code from GitHub
<pre>git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
</pre>

### Adding VST2 version
The VST2 SDK is not part anymore of the VST3 SDK, you have to use an older version of the SDK and copy the VST2_SDK folder into the VST_SDK folder.
In order to build a VST2 version of the Plug-in and a VST3 at the same time, you need to copy the VST2 folder into the VST3 folder, simply run the following commands:
- for macOS:
<pre>
cd TheFolderWhereYouDownloadTheSDK
./copy_vst2_to_vst3_sdk.sh
</pre>
- for Windows:
<pre>
cd TheFolderWhereYouDownloadTheSDK
copy_vst2_to_vst3_sdk.bat
</pre>

### Build the examples on Linux
- Create a folder for the build and move to this folder (using cd):
<pre>
    mkdir build
    cd build
</pre>
- Generate the Solution/Projects: provide the path of the Project where CMakeLists.txt is located:
<pre>
    cmake ../vst3sdk
</pre>
- Now you can build the Plug-in:
<pre>
    make 
 (or alternatively for example for release)
    cmake --build . --config Release
</pre>

### Build the examples on macOS
- Create a folder for the build and move to this folder (using cd):
<pre>
    mkdir build
    cd build
</pre>
- Generate the Solution/Projects: provide the path of the Project where CMakeLists.txt is located:
<pre>
  For XCode:
    cmake -GXcode ../vst3sdk
  Without XCode (here debug variant):
    cmake -DCMAKE_BUILD_TYPE=Debug ../
</pre>
- Now you can build the Plug-in (you can use XCode too):
<pre>
    xcodebuild 
 (or alternatively for example for release)
    cmake --build . --config Release
</pre>

### Build the examples on Windows
- Create a folder for the build and move to this folder (using cd):
<pre>
    mkdir build
    cd build
</pre>
- Generate the Solution/Projects: provide the path of the Project where CMakeLists.txt is located:
<pre>
    cmake.exe -G"Visual Studio 15 2017 Win64" ../vst3sdk
</pre>
- Now you can build the Plug-in (you can use Visual Studio too):
<pre>
    msbuild.exe vstsdk.sln
  (or alternatively for example for release)
    cmake --build . --config Release
</pre>

### Build using cmake-gui
* start the cmake-gui Application
* "Browse Source...": select the folder VST3_SDK
* "Browse Build...": select a folder where the outputs (projects/...) will be created. Typically a folder named "build"
* you can check the SMTG Options
* Press "Configure"
* Press "Generate" and the project will be created

---
<div id='600'/>

## License & Usage guidelines
More details are found at [www.steinberg.net/sdklicenses_vst3](http://www.steinberg.net/sdklicenses_vst3)