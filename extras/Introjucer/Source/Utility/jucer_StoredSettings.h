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

#ifndef __JUCER_STOREDSETTINGS_JUCEHEADER__
#define __JUCER_STOREDSETTINGS_JUCEHEADER__

#include "../Application/jucer_AppearanceSettings.h"


//==============================================================================
class StoredSettings
{
public:
    StoredSettings();
    ~StoredSettings();

    void initialise();

    PropertiesFile& getGlobalProperties();
    PropertiesFile& getProjectProperties (const String& projectUID);

    void flush();
    void reload();

    //==============================================================================
    RecentlyOpenedFilesList recentFiles;

    Array<File> getLastProjects();
    void setLastProjects (const Array<File>& files);

    //==============================================================================
    Array <Colour> swatchColours;

    class ColourSelectorWithSwatches    : public ColourSelector
    {
    public:
        ColourSelectorWithSwatches() {}

        int getNumSwatches() const;
        Colour getSwatchColour (int index) const;
        void setSwatchColour (int index, const Colour& newColour) const;
    };

    //==============================================================================
    AppearanceSettings appearance;

    StringArray monospacedFontNames;

private:
    OwnedArray<PropertiesFile> propertyFiles;

    void updateGlobalProps();
    void loadSwatchColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StoredSettings);
};

StoredSettings& getAppSettings();
PropertiesFile& getGlobalProperties();


#endif   // __JUCER_STOREDSETTINGS_JUCEHEADER__
