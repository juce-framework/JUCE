
# ModuleInfoLib

This is a c++17 library to parse and create the Steinberg moduleinfo.json files.

## Parsing

To parse a moduleinfo.json file you need to include the following files to your project:

* moduleinfoparser.cpp
* moduleinfoparser.h
* moduleinfo.h
* json.h
* jsoncxx.h

And add a header search path to the root folder of the VST SDK.

Now to parse a moduleinfo.json file in code you need to read the moduleinfo.json into a memory buffer and call

``` c++
auto moduleInfo = ModuleInfoLib::parseCompatibilityJson (std::string_view (buffer, bufferSize), &std::cerr);
```

Afterwards if parsing succeeded the moduleInfo optional has a value containing the ModuleInfo.

## Creating

The VST3 SDK contains the moduleinfotool utility that can create moduleinfo.json files from VST3 modules.

To add this capability to your own project you need to link to the sdk_hosting library from the SDK and include the following files to your project:

* moduleinfocreator.cpp
* moduleinfocreator.h
* moduleinfo.h

Additionally you need to add the module platform implementation from the hosting directory (module_win32.cpp, module_mac.mm or module_linux.cpp).

Now you can use the two methods in moduleinfocreator.h to create a moduleinfo.json file:

``` c++
auto moduleInfo = ModuleInfoLib::createModuleInfo (module, false);
ModuleInfoLib::outputJson (moduleInfo, std::cout);
```
