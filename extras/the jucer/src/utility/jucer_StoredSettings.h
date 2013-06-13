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

private:
    ScopedPointer<PropertiesFile> props;
};


#endif   // __JUCER_STOREDSETTINGS_JUCEHEADER__
