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

#ifndef JUCE_VST3PLUGINFORMAT_H_INCLUDED
#define JUCE_VST3PLUGINFORMAT_H_INCLUDED

#if JUCE_PLUGINHOST_VST3
/**
    Implements a plugin format for VST3s.
*/
class JUCE_API VST3PluginFormat : public AudioPluginFormat
{
public:
    /** Constructor */
    VST3PluginFormat();

    /** Destructor */
    ~VST3PluginFormat();

    //==============================================================================
    /** @internal */
    String getName() const override { return "VST3"; }
    /** @internal */
    void findAllTypesForFile (OwnedArray<PluginDescription>& results, const String& fileOrIdentifier) override;
    /** @internal */
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription& description, double, int) override;
    /** @internal */
    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;
    /** @internal */
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;
    /** @internal */
    bool pluginNeedsRescanning (const PluginDescription& description) override;
    /** @internal */
    StringArray searchPathsForPlugins (const FileSearchPath& searchPath, bool recursive) override;
    /** @internal */
    bool doesPluginStillExist (const PluginDescription& description) override;
    /** @internal */
    FileSearchPath getDefaultLocationsToSearch() override;
    /** @internal */
    bool canScanForPlugins() const override { return true; }

private:
    //==============================================================================
    void recursiveFileSearch (StringArray&, const File&, bool recursive);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginFormat)
};

#endif   // JUCE_PLUGINHOST_VST3
#endif   // JUCE_VST3PLUGINFORMAT_H_INCLUDED
