# ARA plugin support

JUCE supports the development of ARA enabled hosts and plugins. Since the ARA SDK is not included
in JUCE there are some steps you need to take to enable all ARA related functionality.

## External dependencies

- ARA SDK 2.2.0

You can download the ARA SDK from Celemony's Github. The command below will recursively clone the
right version into the `ARA_SDK` directory

    git clone --recursive --branch releases/2.2.0 https://github.com/Celemony/ARA_SDK

## Enabling ARA features in JUCE

Once you have downloaded the ARA SDK you need to configure JUCE to use it.

### The Projucer

Add the path to the Global Paths settings.

### CMake

Use the `juce_set_ara_sdk_path` function in your CMakeLists file.

Alternatively, if you are building the examples and extras with CMake from the JUCE repo directory
you can also specify `-DJUCE_GLOBAL_ARA_SDK_PATH=/your/path/to/ARA_SDK` parameter to CMake to 
enable ARA.

## Building the AudioPluginHost with ARA

The AudioPluginHost has simple ARA hosting features, but you need to modify its build configuration
to enable them.

### The Projucer

After opening `AudioPluginHost.jucer` go to *Modules* → *juce_audio_processors* and enable the 
*JUCE_PLUGINHOST_ARA* setting.

### CMake

Set `JUCE_PLUGINHOST_ARA=1` inside `AudioPluginHost/CMakeLists.txt`.

### Loading ARA plugins 

ARA capable plugins will now have two entries in the Create plugin menu, and the one saying (ARA) 
will activate additional ARA features. If you right click on the plugin in the graph, you can use
the “Show ARA host control” item to assign an audio file that the plugin can read through the ARA
interfaces.

## Adding ARA features to existing plugins

### The Projucer

Check the Enable ARA option in the Plugin Formats settings. ARA is an extension to VST3 and AU 
plugins, hence you need to have at least one of those options enabled too for valid build targets.

### CMake

Add the property `IS_ARA_EFFECT TRUE` to your `juce_add_plugin` call.

### Modifying the plugin code

In addition to the `createPluginFilter()` function that is needed for all audio plugins, you will 
now need to provide an implementation to the `createARAFactory()` function as well. You can do this
by inheriting from `juce::ARADocumentControllerSpecialisation` and using its helper function. The 
class documentation should make this clear by also providing an example. You can also find an 
example in the ARAPluginDemo.

## Learning about ARA

ARA provides an extensive API that allows you to exchange information with the host in completely
new ways. To understand the basics and to build up your knowledge it’s best to read the official 
ARA documentation, which has approachable introductory sections. You can find the documentation in
the SDK directory at `ARA_SDK/ARA_Library/html_docs/index.html`.
