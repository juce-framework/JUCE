/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_StoredSettings.h"
#include "../Application/jucer_Application.h"

//==============================================================================
StoredSettings& getAppSettings()
{
    return *JucerApplication::getApp().settings;
}

PropertiesFile& getAppProperties()
{
    return getAppSettings().getProps();
}

//==============================================================================
StoredSettings::StoredSettings()
    : appearance (true)
{
}

StoredSettings::~StoredSettings()
{
    flush();
}

void StoredSettings::initialise()
{
    reload();
}

PropertiesFile& StoredSettings::getProps()
{
    jassert (props != nullptr);
    return *props;
}

void StoredSettings::flush()
{
    if (props != nullptr)
    {
        {
            const ScopedPointer<XmlElement> xml (appearance.settings.createXml());
            props->setValue ("editorColours", xml);
        }

        props->setValue ("recentFiles", recentFiles.toString());

        props->removeValue ("keyMappings");

        if (commandManager != nullptr)
        {
            ScopedPointer <XmlElement> keys (commandManager->getKeyMappings()->createXml (true));

            if (keys != nullptr)
                props->setValue ("keyMappings", (XmlElement*) keys);
        }

        props->saveIfNeeded();
    }
}

void StoredSettings::reload()
{
    props = nullptr;

    {
        // These settings are used in defining the properties file's location.
        PropertiesFile::Options options;
        options.applicationName     = "Introjucer";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Application Support";
       #if JUCE_LINUX
        options.folderName          = ".introjucer";
       #else
        options.folderName          = "Introjucer";
       #endif

        props = new PropertiesFile (options);
    }

    // recent files...
    recentFiles.restoreFromString (props->getValue ("recentFiles"));
    recentFiles.removeNonExistentFiles();

    const ScopedPointer<XmlElement> xml (props->getXmlValue ("editorColours"));
    if (xml != nullptr)
        appearance.readFromXML (*xml);

    appearance.updateColourScheme();
    loadSwatchColours();
}

Array<File> StoredSettings::getLastProjects() const
{
    StringArray s;
    s.addTokens (props->getValue ("lastProjects"), "|", "");

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

    props->setValue ("lastProjects", s.joinIntoString ("|"));
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

    for (int i = 0; i < numSwatchColours; ++i)
        swatchColours.add (Colour::fromString (props->getValue ("swatchColour" + String (i),
                                                                colours [2 + i].toString())));
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
