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


//==============================================================================
/**
    A singleton to hold persistent settings, and to save them in a
    suitable PropertiesFile.
*/
class StoredSettings
{
public:
    //==============================================================================
    StoredSettings();
    ~StoredSettings();

    juce_DeclareSingleton (StoredSettings, false);

    PropertiesFile& getProps();
    void flush();

    //==============================================================================
    RecentlyOpenedFilesList recentFiles;

    Array<File> getLastProjects() const;
    void setLastProjects (const Array<File>& files);

    const StringArray& getFontNames();

    //==============================================================================
    Array <Colour> swatchColours;

    class ColourSelectorWithSwatches    : public ColourSelector
    {
    public:
        ColourSelectorWithSwatches() {}

        int getNumSwatches() const                                      { return StoredSettings::getInstance()->swatchColours.size(); }
        Colour getSwatchColour (int index) const                        { return StoredSettings::getInstance()->swatchColours [index]; }
        void setSwatchColour (int index, const Colour& newColour) const { StoredSettings::getInstance()->swatchColours.set (index, newColour); }
    };


    //==============================================================================
    Image getFallbackImage();
    const Drawable* getImageFileIcon();
    const Drawable* getCogIcon();


private:
    ScopedPointer<PropertiesFile> props;
    StringArray fontNames;

    ScopedPointer<Drawable> imageFileIcon, cogIcon;
    Image fallbackImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StoredSettings);
};


#endif   // __JUCER_STOREDSETTINGS_JUCEHEADER__
