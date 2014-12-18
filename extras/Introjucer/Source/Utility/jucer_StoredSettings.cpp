/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    reload();
}

StoredSettings::~StoredSettings()
{
    flush();
}

PropertiesFile& StoredSettings::getGlobalProperties()
{
    return *propertyFiles.getUnchecked (0);
}

static PropertiesFile* createPropsFile (const String& filename)
{
    return new PropertiesFile (IntrojucerApp::getApp()
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

void StoredSettings::updateGlobalProps()
{
    PropertiesFile& props = getGlobalProperties();

    {
        const ScopedPointer<XmlElement> xml (appearance.settings.createXml());
        props.setValue ("editorColours", xml);
    }

    props.setValue ("recentFiles", recentFiles.toString());

    props.removeValue ("keyMappings");

    if (ApplicationCommandManager* commandManager = IntrojucerApp::getApp().commandManager)
    {
        const ScopedPointer <XmlElement> keys (commandManager->getKeyMappings()->createXml (true));

        if (keys != nullptr)
            props.setValue ("keyMappings", keys);
    }
}

void StoredSettings::flush()
{
    updateGlobalProps();
    saveSwatchColours();

    for (int i = propertyFiles.size(); --i >= 0;)
        propertyFiles.getUnchecked(i)->saveIfNeeded();
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
