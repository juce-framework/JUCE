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
    changed (true);
    flush();

    checkJUCEPaths();

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
        const std::unique_ptr<XmlElement> keys (commandManager->getKeyMappings()->createXml (true));

        if (keys != nullptr)
            getGlobalProperties().setValue ("keyMappings", keys.get());
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

    std::unique_ptr<XmlElement> projectDefaultsXml (propertyFiles.getFirst()->getXmlValue ("PROJECT_DEFAULT_SETTINGS"));

    if (projectDefaultsXml != nullptr)
        projectDefaults = ValueTree::fromXml (*projectDefaultsXml);

    std::unique_ptr<XmlElement> fallbackPathsXml (propertyFiles.getFirst()->getXmlValue ("FALLBACK_PATHS"));

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
            {
                f.moveFileTo (f.getSiblingFile (newProjectSettingsDir.getFileName()).getChildFile (newFileName));
            }
            else
            {
                auto newFile = f.getSiblingFile (newFileName);

                // don't overwrite newer settings file
                if (! newFile.existsAsFile())
                    f.moveFileTo (f.getSiblingFile (newFileName));
            }
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

StoredSettings::ColourSelectorWithSwatches::ColourSelectorWithSwatches() {}
StoredSettings::ColourSelectorWithSwatches::~ColourSelectorWithSwatches() {}

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
void StoredSettings::changed (bool isProjectDefaults)
{
    std::unique_ptr<XmlElement> data (isProjectDefaults ? projectDefaults.createXml()
                                                        : fallbackPaths.createXml());

    propertyFiles.getUnchecked (0)->setValue (isProjectDefaults ? "PROJECT_DEFAULT_SETTINGS" : "FALLBACK_PATHS",
                                              data.get());
}

//==============================================================================
static bool doesSDKPathContainFile (const File& relativeTo, const String& path, const String& fileToCheckFor) noexcept
{
    auto actualPath = path.replace ("${user.home}", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    return relativeTo.getChildFile (actualPath + "/" + fileToCheckFor).exists();
}

static bool isGlobalPathValid (const File& relativeTo, const Identifier& key, const String& path)
{
    String fileToCheckFor;

    if (key == Ids::vstLegacyPath)
    {
        fileToCheckFor = "pluginterfaces/vst2.x/aeffect.h";
    }
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
    else if (key == Ids::clionExePath)
    {
       #if JUCE_MAC
        fileToCheckFor = path.trim().endsWith (".app") ? "Contents/MacOS/clion" : "../clion";
       #elif JUCE_WINDOWS
        fileToCheckFor = "../clion64.exe";
       #else
        fileToCheckFor = "../clion.sh";
       #endif
    }
    else if (key == Ids::androidStudioExePath)
    {
       #if JUCE_MAC
        fileToCheckFor = "Android Studio.app";
       #elif JUCE_WINDOWS
        fileToCheckFor = "studio64.exe";
       #endif
    }
    else if (key == Ids::jucePath)
    {
        fileToCheckFor = "ChangeList.txt";
    }
    else
    {
        // didn't recognise the key provided!
        jassertfalse;
        return false;
    }

    return doesSDKPathContainFile (relativeTo, path, fileToCheckFor);
}

void StoredSettings::checkJUCEPaths()
{
    auto moduleFolder = getStoredPath (Ids::defaultJuceModulePath, TargetOS::getThisOS()).get().toString();
    auto juceFolder   = getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get().toString();

    auto validModuleFolder = isGlobalPathValid ({}, Ids::defaultJuceModulePath, moduleFolder);
    auto validJuceFolder   = isGlobalPathValid ({}, Ids::jucePath, juceFolder);

    if (validModuleFolder && ! validJuceFolder)
        projectDefaults.getPropertyAsValue (Ids::jucePath, nullptr) = File (moduleFolder).getParentDirectory().getFullPathName();
    else if (! validModuleFolder && validJuceFolder)
        projectDefaults.getPropertyAsValue (Ids::defaultJuceModulePath, nullptr) = File (juceFolder).getChildFile ("modules").getFullPathName();
}

bool StoredSettings::shouldAskUserToSetJUCEPath() noexcept
{
    if (! isGlobalPathValid ({}, Ids::jucePath, getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get().toString())
        && getGlobalProperties().getValue ("dontAskAboutJUCEPath", {}).isEmpty())
        return true;

    return false;
}

void StoredSettings::setDontAskAboutJUCEPathAgain() noexcept
{
    getGlobalProperties().setValue ("dontAskAboutJUCEPath", 1);
}

static String getFallbackPathForOS (const Identifier& key, DependencyPathOS os)
{
    if (key == Ids::jucePath)
    {
        return (os == TargetOS::windows ? "C:\\JUCE" : "~/JUCE");
    }
    else if (key == Ids::defaultJuceModulePath)
    {
        return (os == TargetOS::windows ? "C:\\JUCE\\modules" : "~/JUCE/modules");
    }
    else if (key == Ids::defaultUserModulePath)
    {
        return (os == TargetOS::windows ? "C:\\modules" : "~/modules");
    }
    else if (key == Ids::vstLegacyPath || key == Ids::vst3Path)
    {
        return {};
    }
    else if (key == Ids::rtasPath)
    {
        if      (os == TargetOS::windows)  return "C:\\SDKs\\PT_90_SDK";
        else if (os == TargetOS::osx)      return "~/SDKs/PT_90_SDK";
        else                               return {}; // no RTAS on this OS!
    }
    else if (key == Ids::aaxPath)
    {
        if      (os == TargetOS::windows)  return "C:\\SDKs\\AAX";
        else if (os == TargetOS::osx)      return "~/SDKs/AAX";
        else                               return {}; // no AAX on this OS!
    }
    else if (key == Ids::androidSDKPath)
    {
        return "${user.home}/Library/Android/sdk";
    }
    else if (key == Ids::androidNDKPath)
    {
        return "${user.home}/Library/Android/sdk/ndk-bundle";
    }
    else if (key == Ids::clionExePath)
    {
        if (os == TargetOS::windows)
        {
          #if JUCE_WINDOWS
            auto regValue = WindowsRegistry::getValue ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\clion64.exe\\shell\\open\\command\\", {}, {});
            auto openCmd = StringArray::fromTokens (regValue, true);

            if (! openCmd.isEmpty())
                return openCmd[0].unquoted();
          #endif

            return "C:\\Program Files\\JetBrains\\CLion YYYY.MM.DD\\bin\\clion64.exe";
        }
        else if (os == TargetOS::osx)
        {
            return "/Applications/CLion.app";
        }
        else
        {
            return "${user.home}/clion/bin/clion.sh";
        }
    }
    else if (key == Ids::androidStudioExePath)
    {
        if (os == TargetOS::windows)
        {
           #if JUCE_WINDOWS
            auto path = WindowsRegistry::getValue ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Android Studio\\Path", {}, {});

            if (! path.isEmpty())
                return path.unquoted() + "\\bin\\studio64.exe";
           #endif

            return "C:\\Program Files\\Android\\Android Studio\\bin\\studio64.exe";
        }
        else if (os == TargetOS::osx)
        {
            return "/Applications/Android Studio.app";
        }
        else
        {
            return {}; // no Android Studio on this OS!
        }
    }

    // unknown key!
    jassertfalse;
    return {};
}

static Identifier identifierForOS (DependencyPathOS os) noexcept
{
    if      (os == TargetOS::osx)     return Ids::osxFallback;
    else if (os == TargetOS::windows) return Ids::windowsFallback;
    else if (os == TargetOS::linux)   return Ids::linuxFallback;

    jassertfalse;
    return {};
}

ValueWithDefault StoredSettings::getStoredPath (const Identifier& key, DependencyPathOS os)
{
    auto tree = (os == TargetOS::getThisOS() ? projectDefaults
                                             : fallbackPaths.getOrCreateChildWithName (identifierForOS (os), nullptr));

    return { tree, key, nullptr, getFallbackPathForOS (key, os) };
}
