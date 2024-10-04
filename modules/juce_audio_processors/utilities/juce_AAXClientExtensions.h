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

/**
    An interface to allow an AudioProcessor to implement extended AAX-specific functionality.

    To use this class, create an object that inherits from it, implement the methods, then return
    a pointer to the object in your AudioProcessor `getAAXClientExtensions()` method.

    @see AudioProcessor, VST2ClientExtensions, VST3ClientExtensions

    @tags{Audio}
*/
struct JUCE_API AAXClientExtensions
{
    virtual ~AAXClientExtensions() = default;

    /** AAX plug-ins need to report a unique "plug-in id" for every audio layout
        configuration that your AudioProcessor supports on the main bus. Override this
        function if you want your AudioProcessor to use a custom "plug-in id" (for example
        to stay backward compatible with older versions of JUCE).

        The default implementation will compute a unique integer from the input and output
        layout and add this value to the 4 character code 'jcaa' (for native AAX) or 'jyaa'
        (for AudioSuite plug-ins).
    */
    virtual int32 getPluginIDForMainBusConfig (const AudioChannelSet& mainInputLayout,
                                               const AudioChannelSet& mainOutputLayout,
                                               bool idForAudioSuite) const;

    /** Returns an optional filename (including extension) for a page file to be used.

        A page file allows an AAX plugin to specify how its parameters are displayed on
        various control surfaces. For more information read the Page Table Guide in the
        AAX SDK documentation.

        By default this file will be searched for in `*.aaxplugin/Contents/Resources`.

        @see getPageFileSearchPath
    */
    virtual String getPageFileName() const;

    /** Optionally returns a search path for finding a page table file.

        This can be useful for specifying a location outside the plugin bundle so users can
        make changes to a page table file without breaking any code signatures.

        If this function returns a default-constructed File, then a default location will be used.
        The AAX SDK states this location will be `*.aaxplugin/Contents/Resources`.

        @note The returned path should be an absolute path to a directory.

        @see getPageFileName
    */
    virtual File getPageFileSearchPath() const { return {}; }
};

} // namespace juce
