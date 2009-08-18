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

#ifndef __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
#define __JUCE_VSTPLUGINFORMAT_JUCEHEADER__

#include "../juce_AudioPluginFormat.h"


#if JUCE_PLUGINHOST_VST

//==============================================================================
/**
    Implements a plugin format manager for VSTs.
*/
class JUCE_API  VSTPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    VSTPluginFormat();
    ~VSTPluginFormat();

    //==============================================================================
    const String getName() const                { return "VST"; }
    void findAllTypesForFile (OwnedArray <PluginDescription>& results, const String& fileOrIdentifier);
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);
    bool fileMightContainThisPluginType (const String& fileOrIdentifier);
    const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier);
    const StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive);
    bool doesPluginStillExist (const PluginDescription& desc);
    const FileSearchPath getDefaultLocationsToSearch();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    VSTPluginFormat (const VSTPluginFormat&);
    const VSTPluginFormat& operator= (const VSTPluginFormat&);

    void recursiveFileSearch (StringArray& results, const File& dir, const bool recursive);
};


#endif
#endif   // __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
