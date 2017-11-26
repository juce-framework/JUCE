/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"
#include "jucer_StoredSettings.h"
#include "../Application/jucer_Application.h"

//==============================================================================
StoredSettings& getAppSettings()
{
    return *ProjucerApplication::getApp().settings;
}

PropertiesFile& getGlobalProperties()
{
    return getAppSettings().getGlobalProperties();
}

//==============================================================================
StoredSettings::StoredSettings()
    : appearance (true),
      projectDefaults ("PROJECT_DEFAULT_SETTINGS"),
      fallbackPaths ("FALLBACK_PATHS")
{
    updateOldProjectSettingsFiles();
    reload();
    projectDefaults.addListener (this);
    fallbackPaths.addListener (this);
}

StoredSettings::~StoredSettings()
{
    projectDefaults.removeListener (this);
    fallbackPaths.removeListener (this);
    flush();
}

PropertiesFile& StoredSettings::getGlobalProperties()
{
    return *propertyFiles.getUnchecked (0);
}

static PropertiesFile* createPropsFile (const String& filename, bool isProjectSettings)
{
    return new PropertiesFile (ProjucerApplication::getApp()
                                .getPropertyFileOptionsFor (filename, isProjectSettings));
}

PropertiesFile& StoredSettings::getProjectProperties (const String& projectUID)
{
    const auto filename = String ("Projucer_Project_" + projectUID);

    for (auto i = propertyFiles.size(); --i >= 0;)
    {
        auto* const props = propertyFiles.getUnchecked(i);
        if (props->getFile().getFileNameWithoutExtension() == filename)
            return *props;
    }

    auto* p = createPropsFile (filename, true);
    propertyFiles.add (p);
    return *p;
}

void StoredSettings::updateGlobalPreferences()
{
    // update 'invisible' global settings
    updateRecentFiles();
    updateLastWizardFolder();
    updateKeyMappings();
}

void StoredSettings::updateRecentFiles()
{
    getGlobalProperties().setValue ("recentFiles", recentFiles.toString());
}

void StoredSettings::updateLastWizardFolder()
{
    getGlobalProperties().setValue ("lastWizardFolder", lastWizardFolder.getFullPathName());
}

void StoredSettings::updateKeyMappings()
{
    getGlobalProperties().removeValue ("keyMappings");

    if (auto* commandManager = ProjucerApplication::getApp().commandManager.get())
    {
        const ScopedPointer<XmlElement> keys (commandManager->getKeyMappings()->createXml (true));

        if (keys != nullptr)
            getGlobalProperties().setValue ("keyMappings", keys);
    }
}

void StoredSettings::flush()
{
    updateGlobalPreferences();
    saveSwatchColours();

    for (auto i = propertyFiles.size(); --i >= 0;)
        propertyFiles.getUnchecked(i)->saveIfNeeded();
}

void StoredSettings::reload()
{
    propertyFiles.clear();
    propertyFiles.add (createPropsFile ("Projucer", false));

    ScopedPointer<XmlElement> projectDefaultsXml (propertyFiles.getFirst()->getXmlValue ("PROJECT_DEFAULT_SETTINGS"));
    if (projectDefaultsXml != nullptr)
        projectDefaults = ValueTree::fromXml (*projectDefaultsXml);

    ScopedPointer<XmlElement> fallbackPathsXml (propertyFiles.getFirst()->getXmlValue ("FALLBACK_PATHS"));
    if (fallbackPathsXml != nullptr)
        fallbackPaths = ValueTree::fromXml (*fallbackPathsXml);

    // recent files...
    recentFiles.restoreFromString (getGlobalProperties().getValue ("recentFiles"));
    recentFiles.removeNonExistentFiles();

    lastWizardFolder = getGlobalProperties().getValue ("lastWizardFolder");

    loadSwatchColours();
}

Array<File> StoredSettings::getLastProjects()
{
    StringArray s;
    s.addTokens (getGlobalProperties().getValue ("lastProjects"), "|", "");

    Array<File> f;
    for (int i = 0; i < s.size(); ++i)
        f.add (File (s[i]));

    return f;
}

void StoredSettings::setLastProjects (const Array<File>& files)
{
    StringArray s;
    for (int i = 0; i < files.size(); ++i)
        s.add (files.getReference(i).getFullPathName());

    getGlobalProperties().setValue ("lastProjects", s.joinIntoString ("|"));
}

void StoredSettings::updateOldProjectSettingsFiles()
{
    // Global properties file hasn't been created yet so create a dummy file
    auto projucerSettingsDirectory = ProjucerApplication::getApp().getPropertyFileOptionsFor ("Dummy", false)
                                                                  .getDefaultFile().getParentDirectory();

    auto newProjectSettingsDir = projucerSettingsDirectory.getChildFile ("ProjectSettings");
    newProjectSettingsDir.createDirectory();

    DirectoryIterator iter (projucerSettingsDirectory, false, "*.settings");
    while (iter.next())
    {
        auto f = iter.getFile();
        auto oldFileName = f.getFileName();

        if (oldFileName.contains ("Introjucer"))
        {
            auto newFileName = oldFileName.replace ("Introjucer", "Projucer");

            if (oldFileName.contains ("_Project"))
                f.moveFileTo (f.getSiblingFile (newProjectSettingsDir.getFileName()).getChildFile (newFileName));
            else
                f.moveFileTo (f.getSiblingFile (newFileName));
        }
    }
}

//==============================================================================
void StoredSettings::loadSwatchColours()
{
    swatchColours.clear();

    #define COL(col)  Colours::col,

    const Colour colours[] =
    {
        #include "../Utility/Helpers/jucer_Colours.h"
        Colours::transparentBlack
    };

    #undef COL

    const auto numSwatchColours = 24;
    auto& props = getGlobalProperties();

    for (auto i = 0; i < numSwatchColours; ++i)
        swatchColours.add (Colour::fromString (props.getValue ("swatchColour" + String (i),
                                                               colours [2 + i].toString())));
}

void StoredSettings::saveSwatchColours()
{
    auto& props = getGlobalProperties();

    for (auto i = 0; i < swatchColours.size(); ++i)
        props.setValue ("swatchColour" + String (i), swatchColours.getReference(i).toString());
}

int StoredSettings::ColourSelectorWithSwatches::getNumSwatches() const
{
    return getAppSettings().swatchColours.size();
}

Colour StoredSettings::ColourSelectorWithSwatches::getSwatchColour (int index) const
{
    return getAppSettings().swatchColours [index];
}

void StoredSettings::ColourSelectorWithSwatches::setSwatchColour (int index, const Colour& newColour)
{
    getAppSettings().swatchColours.set (index, newColour);
}

//==============================================================================
Value StoredSettings::getStoredPath (const Identifier& key)
{
    auto v = projectDefaults.getPropertyAsValue (key, nullptr);

    if (v.toString().isEmpty())
        v = getFallbackPathForOS (key, TargetOS::getThisOS()).toString();

    return v;
}

Value StoredSettings::getFallbackPathForOS (const Identifier& key, DependencyPathOS os)
{
    auto id = Identifier();

    if      (os == TargetOS::osx)     id = Ids::osxFallback;
    else if (os == TargetOS::windows) id = Ids::windowsFallback;
    else if (os == TargetOS::linux)   id = Ids::linuxFallback;

    if (id == Identifier())
        jassertfalse;

    auto v = fallbackPaths.getOrCreateChildWithName (id, nullptr)
                          .getPropertyAsValue (key, nullptr);

    if (v.toString().isEmpty())
    {
        if (key == Ids::defaultJuceModulePath)
        {
            v = (os == TargetOS::windows ? "C:\\JUCE\\modules"
                                         : "~/JUCE/modules");
        }
        else if (key == Ids::defaultUserModulePath)
        {
            v = (os == TargetOS::windows ? "C:\\modules"
                                         : "~/modules");
        }
        else if (key == Ids::vst3Path)
        {
            v = (os == TargetOS::windows ? "C:\\SDKs\\VST_SDK\\VST3_SDK"
                                         : "~/SDKs/VST_SDK/VST3_SDK");
        }
        else if (key == Ids::rtasPath)
        {
            if      (os == TargetOS::windows)  v = "C:\\SDKs\\PT_90_SDK";
            else if (os == TargetOS::osx)      v = "~/SDKs/PT_90_SDK";
            else                               jassertfalse; // no RTAS on this OS!
        }
        else if (key == Ids::aaxPath)
        {
            if      (os == TargetOS::windows)  v = "C:\\SDKs\\AAX";
            else if (os == TargetOS::osx)      v = "~/SDKs/AAX" ;
            else                               jassertfalse; // no AAX on this OS!
        }
        else if (key == Ids::androidSDKPath)
        {
            v = "${user.home}/Library/Android/sdk";
        }
        else if (key == Ids::androidNDKPath)
        {
            v = "${user.home}/Library/Android/sdk/ndk-bundle";
        }
    }

    return v;
}

static bool doesSDKPathContainFile (const File& relativeTo, const String& path, const String& fileToCheckFor)
{
    auto actualPath = path.replace ("${user.home}", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    return relativeTo.getChildFile (actualPath + "/" + fileToCheckFor).exists();
}

bool StoredSettings::isGlobalPathValid (const File& relativeTo, const Identifier& key, const String& path)
{
    String fileToCheckFor;

    if (key == Ids::vst3Path)
    {
        fileToCheckFor = "base/source/baseiids.cpp";
    }
    else if (key == Ids::rtasPath)
    {
        fileToCheckFor = "AlturaPorts/TDMPlugIns/PlugInLibrary/EffectClasses/CEffectProcessMIDI.cpp";
    }
    else if (key == Ids::aaxPath)
    {
        fileToCheckFor = "Interfaces/AAX_Exports.cpp";
    }
    else if (key == Ids::androidSDKPath)
    {
       #if JUCE_WINDOWS
        fileToCheckFor = "platform-tools/adb.exe";
       #else
        fileToCheckFor = "platform-tools/adb";
       #endif
    }
    else if (key == Ids::androidNDKPath)
    {
       #if JUCE_WINDOWS
        fileToCheckFor = "ndk-depends.cmd";
       #else
        fileToCheckFor = "ndk-depends";
       #endif
    }
    else if (key == Ids::defaultJuceModulePath)
    {
        fileToCheckFor = "juce_core";
    }
    else if (key == Ids::defaultUserModulePath)
    {
        fileToCheckFor = {};
    }
    else
    {
        // didn't recognise the key provided!
        jassertfalse;
        return false;
    }

    return doesSDKPathContainFile (relativeTo, path, fileToCheckFor);
}
