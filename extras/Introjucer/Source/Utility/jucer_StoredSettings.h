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

    PropertiesFile& getProps();
    void flush();
    void reload();

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

        int getNumSwatches() const;
        Colour getSwatchColour (int index) const;
        void setSwatchColour (int index, const Colour& newColour) const;
    };

    //==============================================================================
    AppearanceSettings appearance;

    const char* getSchemeFileSuffix() const     { return ".editorscheme"; }
    File getSchemesFolder();

private:
    ScopedPointer<PropertiesFile> props;
    StringArray fontNames;

    void loadSwatchColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StoredSettings);
};

StoredSettings& getAppSettings();
PropertiesFile& getAppProperties();


//==============================================================================
class Icons
{
public:
    Icons();

    void reload (const Colour& backgroundColour);

    const Drawable* folder;
    const Drawable* document;
    const Drawable* imageDoc;
    const Drawable* config;
    const Drawable* exporter;
    const Drawable* juceLogo;
    const Drawable* graph;
    const Drawable* jigsaw;
    const Drawable* info;
    const Drawable* warning;

private:
    OwnedArray<Drawable> drawables;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Icons);
};

const Icons& getIcons();


#endif   // __JUCER_STOREDSETTINGS_JUCEHEADER__
