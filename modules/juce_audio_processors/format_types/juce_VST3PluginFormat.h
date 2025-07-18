/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if (JUCE_PLUGINHOST_VST3 && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD)) || DOXYGEN

/**
    Implements a plugin format for VST3s.

    @tags{Audio}
*/
class JUCE_API VST3PluginFormat   : public AudioPluginFormat
{
public:
    /** Constructor */
    VST3PluginFormat();

    /** Destructor */
    ~VST3PluginFormat() override;

    //==============================================================================
    /** @cond */
    /** Attempts to reload a VST3 plugin's state from some preset file data.

        @see VSTPluginFormat::loadFromFXBFile
    */
    [[deprecated ("Instead of using this function, use AudioPluginInstance::getExtensions() "
                 "to visit the ExtensionsVisitor::VST3 struct for the instance, if it exists. "
                 "Then, call ExtensionsVisitor::VST3::setPreset() to set the state using the "
                 "contents of a vstpreset file.")]]
    static bool setStateFromVSTPresetFile (AudioPluginInstance*, const MemoryBlock&);
    /** @endcond */

    //==============================================================================
    static String getFormatName()                   { return "VST3"; }
    String getName() const override                 { return getFormatName(); }
    bool canScanForPlugins() const override         { return true; }
    bool isTrivialToScan() const override           { return false; }

    void findAllTypesForFile (OwnedArray<PluginDescription>&, const String& fileOrIdentifier) override;
    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;
    bool pluginNeedsRescanning (const PluginDescription&) override;
    StringArray searchPathsForPlugins (const FileSearchPath&, bool recursive, bool) override;
    bool doesPluginStillExist (const PluginDescription&) override;
    FileSearchPath getDefaultLocationsToSearch() override;
    void createARAFactoryAsync (const PluginDescription&, ARAFactoryCreationCallback callback) override;

private:
    //==============================================================================
    void createPluginInstance (const PluginDescription&, double initialSampleRate,
                               int initialBufferSize, PluginCreationCallback) override;
    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    void recursiveFileSearch (StringArray&, const File&, bool recursive);
    StringArray getLibraryPaths (const String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginFormat)
};

#endif // JUCE_PLUGINHOST_VST3

} // namespace juce
