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

#ifndef __JUCE_MAINHOSTWINDOW_JUCEHEADER__xxxxx
#define __JUCE_MAINHOSTWINDOW_JUCEHEADER__xxxxx

#include "FilterGraph.h"


//==============================================================================
/**
    Manages the internal plugin types.
*/
class InternalPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    InternalPluginFormat();
    ~InternalPluginFormat() {}

    //==============================================================================
    enum InternalFilterType
    {
        audioInputFilter = 0,
        audioOutputFilter,
        midiInputFilter,

        endOfFilterTypes
    };

    const PluginDescription* getDescriptionFor (const InternalFilterType type);

    void getAllTypes (OwnedArray <PluginDescription>& results);

    //==============================================================================
    const String getName() const                                { return "Internal"; }
    bool fileMightContainThisPluginType (const String&)         { return false; }
    const FileSearchPath getDefaultLocationsToSearch()          { return FileSearchPath(); }
    void findAllTypesForFile (OwnedArray <PluginDescription>&, const String&)     {}
    bool doesPluginStillExist (const PluginDescription&)        { return true; }
    const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier)   { return fileOrIdentifier; }
    const StringArray searchPathsForPlugins (const FileSearchPath&, const bool)   { return StringArray(); }
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PluginDescription audioInDesc;
    PluginDescription audioOutDesc;
    PluginDescription midiInDesc;
};


#endif
