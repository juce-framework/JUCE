/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_StoredSettings.h"
#include "../Application/jucer_Application.h"
#include "../Application/jucer_GlobalPreferences.h"

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
    : appearance (true), projectDefaults ("PROJECT_DEFAULT_SETTINGS")
{
    reload();
    projectDefaults.addListener (this);
}

StoredSettings::~StoredSettings()
{
    projectDefaults.removeListener (this);
    flush();
}

PropertiesFile& StoredSettings::getGlobalProperties()
{
    return *propertyFiles.getUnchecked (0);
}

static PropertiesFile* createPropsFile (const String& filename)
{
    return new PropertiesFile (ProjucerApplication::getApp()
                                .getPropertyFileOptionsFor (filename));
}

PropertiesFile& StoredSettings::getProjectProperties (const String& projectUID)
{
    const String filename ("Introjucer_Project_" + projectUID);

    for (int i = propertyFiles.size(); --i >= 0;)
    {
        PropertiesFile* const props = propertyFiles.getUnchecked(i);
        if (props->getFile().getFileNameWithoutExtension() == filename)
            return *props;
    }

    PropertiesFile* p = createPropsFile (filename);
    propertyFiles.add (p);
    return *p;
}

void StoredSettings::updateGlobalPreferences()
{
    // update global settings editable from the global preferences window
    updateAppearanceSettings();

    // update 'invisible' global settings
    updateRecentFiles();
    updateKeyMappings();
}

void StoredSettings::updateAppearanceSettings()
{
    const ScopedPointer<XmlElement> xml (appearance.settings.createXml());
    getGlobalProperties().setValue ("editorColours", xml);
}

void StoredSettings::updateRecentFiles()
{
    getGlobalProperties().setValue ("recentFiles", recentFiles.toString());
}

void StoredSettings::updateKeyMappings()
{
    getGlobalProperties().removeValue ("keyMappings");

    if (ApplicationCommandManager* commandManager = ProjucerApplication::getApp().commandManager)
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

    for (int i = propertyFiles.size(); --i >= 0;)
        propertyFiles.getUnchecked(i)->saveIfNeeded();
}

void StoredSettings::reload()
{
    propertyFiles.clear();
    propertyFiles.add (createPropsFile ("Introjucer"));

    ScopedPointer<XmlElement> projectDefaultsXml (propertyFiles.getFirst()->getXmlValue ("PROJECT_DEFAULT_SETTINGS"));

    if (projectDefaultsXml != nullptr)
        projectDefaults = ValueTree::fromXml (*projectDefaultsXml);

    // recent files...
    recentFiles.restoreFromString (getGlobalProperties().getValue ("recentFiles"));
    recentFiles.removeNonExistentFiles();

    ScopedPointer<XmlElement> xml (getGlobalProperties().getXmlValue ("editorColours"));

    if (xml == nullptr)
    {
        xml = XmlDocument::parse (BinaryData::colourscheme_dark_xml);
        jassert (xml != nullptr);
    }

    appearance.readFromXML (*xml);

    appearance.updateColourScheme();
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

//==============================================================================
void StoredSettings::loadSwatchColours()
{
    swatchColours.clear();

    #define COL(col)  Colours::col,

    const Colour colours[] =
    {
        #include "jucer_Colours.h"
        Colours::transparentBlack
    };

    #undef COL

    const int numSwatchColours = 24;
    PropertiesFile& props = getGlobalProperties();

    for (int i = 0; i < numSwatchColours; ++i)
        swatchColours.add (Colour::fromString (props.getValue ("swatchColour" + String (i),
                                                               colours [2 + i].toString())));
}

void StoredSettings::saveSwatchColours()
{
    PropertiesFile& props = getGlobalProperties();

    for (int i = 0; i < swatchColours.size(); ++i)
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

void StoredSettings::ColourSelectorWithSwatches::setSwatchColour (int index, const Colour& newColour) const
{
    getAppSettings().swatchColours.set (index, newColour);
}

//==============================================================================
static bool doesSDKPathContainFile (const File& relativeTo, const String& path, const String& fileToCheckFor)
{
    String actualPath = path.replace ("${user.home}", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    return relativeTo.getChildFile (actualPath + "/" + fileToCheckFor).existsAsFile();
}

Value StoredSettings::getGlobalPath (const Identifier& key, DependencyPathOS os)
{
    Value v (projectDefaults.getPropertyAsValue (key, nullptr));

    if (v.toString().isEmpty())
        v = getFallbackPath (key, os);

    return v;
}

String StoredSettings::getFallbackPath (const Identifier& key, DependencyPathOS os)
{
    if (key == Ids::vst3Path)
        return os == TargetOS::windows ? "c:\\SDKs\\VST3 SDK"
                                       : "~/SDKs/VST3 SDK";

    if (key == Ids::rtasPath)
    {
        if (os == TargetOS::windows)   return "c:\\SDKs\\PT_90_SDK";
        if (os == TargetOS::osx)       return "~/SDKs/PT_90_SDK";

        // no RTAS on this OS!
        jassertfalse;
        return String();
    }

    if (key == Ids::aaxPath)
    {
        if (os == TargetOS::windows)   return "c:\\SDKs\\AAX";
        if (os == TargetOS::osx)       return "~/SDKs/AAX" ;

        // no AAX on this OS!
        jassertfalse;
        return String();
    }

    if (key == Ids::androidSDKPath)
        return "${user.home}/Library/Android/sdk";

    if (key == Ids::androidNDKPath)
        return "${user.home}/Library/Android/sdk/ndk-bundle";

    // didn't recognise the key provided!
    jassertfalse;
    return String();
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
    else
    {
        // didn't recognise the key provided!
        jassertfalse;
        return false;
    }

    return doesSDKPathContainFile (relativeTo, path, fileToCheckFor);
}
