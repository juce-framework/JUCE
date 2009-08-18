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

#ifndef __JUCER_STOREDSETTINGS_JUCEHEADER__
#define __JUCER_STOREDSETTINGS_JUCEHEADER__


//==============================================================================
/**
    A singleton to hold the jucer's persistent settings, and to save them in a
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

    const File getTemplatesDir() const;
    void setTemplatesDir (const File& newDir);

    Array <Colour> swatchColours;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PropertiesFile* props;
};


#endif   // __JUCER_STOREDSETTINGS_JUCEHEADER__
