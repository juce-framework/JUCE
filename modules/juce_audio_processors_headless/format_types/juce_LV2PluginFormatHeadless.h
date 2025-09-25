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

#if JUCE_INTERNAL_HAS_LV2 || DOXYGEN

/**
    Implements a plugin format for LV2 plugins.

    @tags{Audio}
*/
class JUCE_API LV2PluginFormatHeadless  : public AudioPluginFormat
{
public:
    LV2PluginFormatHeadless();
    ~LV2PluginFormatHeadless() override;

    static String getFormatName()       { return "LV2"; }
    String getName() const override     { return getFormatName(); }

    void findAllTypesForFile (OwnedArray<PluginDescription>& results,
                              const String& fileOrIdentifier) override;

    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;

    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;

    bool pluginNeedsRescanning (const PluginDescription&) override;

    bool doesPluginStillExist (const PluginDescription&) override;

    bool canScanForPlugins() const override;

    bool isTrivialToScan() const override;

    StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                       bool recursive,
                                       bool allowPluginsWhichRequireAsynchronousInstantiation = false) override;

    FileSearchPath getDefaultLocationsToSearch() override;

    /** @internal */
    class Pimpl;

private:
    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    void createPluginInstance (const PluginDescription&, double, int, PluginCreationCallback) override;

    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LV2PluginFormatHeadless)
};

#endif

} // namespace juce
