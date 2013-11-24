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

#ifndef __JUCER_STOREDSETTINGS_JUCEHEADER__
#define __JUCER_STOREDSETTINGS_JUCEHEADER__

#include "../Application/jucer_AppearanceSettings.h"


//==============================================================================
class StoredSettings
{
public:
    StoredSettings();
    ~StoredSettings();

    PropertiesFile& getGlobalProperties();
    PropertiesFile& getProjectProperties (const String& projectUID);

    void flush();
    void reload();

    //==============================================================================
    RecentlyOpenedFilesList recentFiles;

    Array<File> getLastProjects();
    void setLastProjects (const Array<File>& files);

    //==============================================================================
    Array<Colour> swatchColours;

    class ColourSelectorWithSwatches    : public ColourSelector
    {
    public:
        ColourSelectorWithSwatches() {}

        int getNumSwatches() const override;
        Colour getSwatchColour (int index) const override;
        void setSwatchColour (int index, const Colour& newColour) const override;
    };

    //==============================================================================
    AppearanceSettings appearance;

    StringArray monospacedFontNames;

private:
    OwnedArray<PropertiesFile> propertyFiles;

    void updateGlobalProps();
    void loadSwatchColours();
    void saveSwatchColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StoredSettings)
};

StoredSettings& getAppSettings();
PropertiesFile& getGlobalProperties();


#endif   // __JUCER_STOREDSETTINGS_JUCEHEADER__
