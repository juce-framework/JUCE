/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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


//==============================================================================
StoredSettings::StoredSettings()
    : props (0)
{
    flush();
}

StoredSettings::~StoredSettings()
{
    flush();
    props = 0;
    clearSingletonInstance();
}

juce_ImplementSingleton (StoredSettings);


//==============================================================================
PropertiesFile& StoredSettings::getProps()
{
    jassert (props != 0);
    return *props;
}

void StoredSettings::flush()
{
    if (props != 0)
    {
        props->setValue ("recentFiles", recentFiles.toString());

        props->removeValue ("keyMappings");

        if (commandManager != 0)
        {
            ScopedPointer <XmlElement> keys (commandManager->getKeyMappings()->createXml (true));

            if (keys != 0)
                props->setValue ("keyMappings", (XmlElement*) keys);
        }
    }

    props = 0;
    props = PropertiesFile::createDefaultAppPropertiesFile ("Jucer2",
                                                            "settings",
                                                            String::empty,
                                                            false, 3000,
                                                            PropertiesFile::storeAsXML);

    // recent files...
    recentFiles.restoreFromString (props->getValue ("recentFiles"));
    recentFiles.removeNonExistentFiles();

    // swatch colours...
    swatchColours.clear();

    #define COL(col)  Colours::col,

    const Colour colours[] =
    {
        #include "jucer_Colours.h"
        Colours::transparentBlack
    };

    #undef COL

    for (int i = 0; i < numSwatchColours; ++i)
    {
        Colour defaultCol (colours [2 + i]);
        swatchColours.add (Colour (props->getValue ("swatchColour" + String (i),
                                                    hexString8Digits (defaultCol.getARGB())).getHexValue32()));
    }
}

const File StoredSettings::getLastProject() const
{
    return props->getValue ("lastProject");
}

void StoredSettings::setLastProject (const File& file)
{
    props->setValue ("lastProject", file.getFullPathName());
}

const File StoredSettings::getLastKnownJuceFolder() const
{
    File defaultJuceFolder (FileHelpers::findDefaultJuceFolder());
    File f (props->getValue ("lastJuceFolder", defaultJuceFolder.getFullPathName()));

    if ((! FileHelpers::isJuceFolder (f)) && FileHelpers::isJuceFolder (defaultJuceFolder))
        f = defaultJuceFolder;

    return f;
}

void StoredSettings::setLastKnownJuceFolder (const File& file)
{
    jassert (FileHelpers::isJuceFolder (file));
    props->setValue ("lastJuceFolder", file.getFullPathName());
}

const StringArray& StoredSettings::getFontNames()
{
    if (fontNames.size() == 0)
        fontNames = Font::findAllTypefaceNames();

    return fontNames;
}

const Image StoredSettings::getFallbackImage()
{
    if (fallbackImage.isNull())
        fallbackImage = ImageFileFormat::loadFrom (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize);

    return fallbackImage;
}
