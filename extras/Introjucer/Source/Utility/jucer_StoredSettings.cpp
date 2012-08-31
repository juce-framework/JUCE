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
    return *IntrojucerApp::getApp().settings;
}

PropertiesFile& getGlobalProperties()
{
    return getAppSettings().getGlobalProperties();
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

PropertiesFile& StoredSettings::getGlobalProperties()
{
    return *propertyFiles.getUnchecked (0);
}

static PropertiesFile* createPropsFile (const String& filename)
{
    PropertiesFile::Options options;
    options.applicationName     = filename;
    options.filenameSuffix      = "settings";
    options.osxLibrarySubFolder = "Application Support";
   #if JUCE_LINUX
    options.folderName          = ".introjucer";
   #else
    options.folderName          = "Introjucer";
   #endif

    return new PropertiesFile (options);
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

void StoredSettings::flush()
{
    for (int i = propertyFiles.size(); --i >= 0;)
    {
        PropertiesFile* const props = propertyFiles.getUnchecked(i);

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
    propertyFiles.clear();
    propertyFiles.add (createPropsFile ("Introjucer"));

    // recent files...
    recentFiles.restoreFromString (getGlobalProperties().getValue ("recentFiles"));
    recentFiles.removeNonExistentFiles();

    ScopedPointer<XmlElement> xml (getGlobalProperties().getXmlValue ("editorColours"));
    if (xml == nullptr)
        xml = XmlDocument::parse (BinaryData::colourscheme_dark_xml);

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
