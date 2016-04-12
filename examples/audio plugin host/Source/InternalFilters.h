/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef __INTERNALFILTERS_JUCEHEADER__
#define __INTERNALFILTERS_JUCEHEADER__

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
    String getName() const override                                      { return "Internal"; }
    bool fileMightContainThisPluginType (const String&) override         { return true; }
    FileSearchPath getDefaultLocationsToSearch() override                { return FileSearchPath(); }
    bool canScanForPlugins() const override                              { return false; }
    void findAllTypesForFile (OwnedArray <PluginDescription>&, const String&) override     {}
    bool doesPluginStillExist (const PluginDescription&) override        { return true; }
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override   { return fileOrIdentifier; }
    bool pluginNeedsRescanning (const PluginDescription&) override       { return false; }
    StringArray searchPathsForPlugins (const FileSearchPath&, bool, bool) override         { return StringArray(); }

private:
    //==============================================================================
    void createPluginInstance (const PluginDescription& description,
                               double initialSampleRate,
                               int initialBufferSize,
                               void* userData,
                               void (*callback) (void*, AudioPluginInstance*, const String&)) override;
    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept override;
private:
    //==============================================================================
    PluginDescription audioInDesc;
    PluginDescription audioOutDesc;
    PluginDescription midiInDesc;
};


#endif   // __INTERNALFILTERS_JUCEHEADER__
