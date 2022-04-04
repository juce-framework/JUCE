/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
#define FILE_EXT .h

#define EXPAND(x) x
#define CREATE_FILEPATH(DemoName, category) JUCE_STRINGIFY(EXPAND(category)/EXPAND(DemoName)EXPAND(FILE_EXT))

#define REGISTER_DEMO(DemoName, category, heavyweight) JUCEDemos::registerDemo ([] { return new DemoName(); }, CREATE_FILEPATH(DemoName, category), JUCE_STRINGIFY (category), heavyweight);

//==============================================================================
struct JUCEDemos
{
    struct FileAndCallback
    {
        File demoFile;
        std::function<Component*()> callback;
        bool isHeavyweight;
    };

    struct DemoCategory
    {
        String name;
        std::vector<FileAndCallback> demos;
    };

    static std::vector<DemoCategory>& getCategories();
    static DemoCategory& getCategory (const String& name);

    static void registerDemo (std::function<Component*()> constructorCallback, const String& filePath, const String& category, bool isHeavyweight);
    static File findExamplesDirectoryFromExecutable (File exec);
};

void registerDemos_One() noexcept;
void registerDemos_Two() noexcept;

//==============================================================================
// used by child-process demo
bool invokeChildProcessDemo (const String& commandLine);
void registerAllDemos() noexcept;

Component* createIntroDemo();
bool isComponentIntroDemo (Component*) noexcept;

CodeEditorComponent::ColourScheme getDarkColourScheme();
CodeEditorComponent::ColourScheme getLightColourScheme();

//==============================================================================
extern std::unique_ptr<AudioDeviceManager> sharedAudioDeviceManager;

AudioDeviceManager& getSharedAudioDeviceManager (int numInputChannels = -1, int numOutputChannels = -1);
ApplicationCommandManager& getGlobalCommandManager();
