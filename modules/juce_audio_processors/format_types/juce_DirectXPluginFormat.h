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

#ifndef __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__
#define __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__

#include "../format/juce_AudioPluginFormat.h"

#if JUCE_PLUGINHOST_DX && JUCE_WINDOWS


//   Sorry, this file is just a placeholder at the moment!...


//==============================================================================
/**
    Implements a plugin format manager for DirectX plugins.
*/
class JUCE_API  DirectXPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    DirectXPluginFormat();
    ~DirectXPluginFormat();

    //==============================================================================
    String getName() const                { return "DirectX"; }
    void findAllTypesForFile (OwnedArray <PluginDescription>&, const String& fileOrIdentifier);
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription&);
    bool fileMightContainThisPluginType (const String& fileOrIdentifier);
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier)  { return fileOrIdentifier; }
    FileSearchPath getDefaultLocationsToSearch();
    bool canScanForPlugins() const        { return true; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectXPluginFormat)
};

#endif

#endif   // __JUCE_DIRECTXPLUGINFORMAT_JUCEHEADER__
