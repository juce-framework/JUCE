/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
    deleteAndZero (props);
    clearSingletonInstance();
}

juce_ImplementSingleton (StoredSettings);


//==============================================================================
PropertiesFile& StoredSettings::getProps()
{
    return *props;
}

void StoredSettings::flush()
{
    if (props != 0)
    {
        props->setValue (T("recentFiles"), recentFiles.toString());

        props->removeValue (T("keyMappings"));

        XmlElement* keys = commandManager->getKeyMappings()->createXml (true);

        if (keys != 0)
        {
            props->setValue (T("keyMappings"), keys);
            delete keys;
        }

        for (int i = 0; i < swatchColours.size(); ++i)
            props->setValue (T("swatchColour") + String (i), colourToHex (swatchColours [i]));
    }

    deleteAndZero (props);

    props = PropertiesFile::createDefaultAppPropertiesFile (T("Jucer"),
                                                            T("settings"),
                                                            String::empty,
                                                            false, 3000,
                                                            PropertiesFile::storeAsXML);

    // recent files...
    recentFiles.restoreFromString (props->getValue (T("recentFiles")));
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

        swatchColours.add (Colour (props->getValue (T("swatchColour") + String (i),
                                                    colourToHex (defaultCol)).getHexValue32()));
    }
}

const File StoredSettings::getTemplatesDir() const
{
    File defaultTemplateDir (File::getSpecialLocation (File::currentExecutableFile)
                                .getParentDirectory());

    return File (props->getValue (T("templateDir"),
                                  defaultTemplateDir.getFullPathName()));
}

void StoredSettings::setTemplatesDir (const File& newDir)
{
    props->setValue (T("templateDir"), newDir.getFullPathName());
}
