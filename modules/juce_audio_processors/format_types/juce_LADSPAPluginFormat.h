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

#ifndef JUCE_LADSPAPLUGINFORMAT_H_INCLUDED
#define JUCE_LADSPAPLUGINFORMAT_H_INCLUDED

#include "../format/juce_AudioPluginFormat.h"

#if (JUCE_PLUGINHOST_LADSPA && JUCE_LINUX) || DOXYGEN

//==============================================================================
/**
    Implements a plugin format manager for LADSPA plugins.
*/
class JUCE_API  LADSPAPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    LADSPAPluginFormat();
    ~LADSPAPluginFormat();

    //==============================================================================
    String getName() const                { return "LADSPA"; }
    void findAllTypesForFile (OwnedArray <PluginDescription>&, const String& fileOrIdentifier);
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);
    bool fileMightContainThisPluginType (const String& fileOrIdentifier);
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier);
    bool pluginNeedsRescanning (const PluginDescription&);
    StringArray searchPathsForPlugins (const FileSearchPath&, bool recursive);
    bool doesPluginStillExist (const PluginDescription&);
    FileSearchPath getDefaultLocationsToSearch();
    bool canScanForPlugins() const        { return true; }

private:
    void recursiveFileSearch (StringArray&, const File&, bool recursive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAPluginFormat)
};


#endif

#endif   // JUCE_LADSPAPLUGINFORMAT_H_INCLUDED
