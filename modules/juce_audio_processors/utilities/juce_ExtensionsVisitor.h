/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/** Create a derived implementation of this class and pass it to
    AudioPluginInstance::getExtensions() to retrieve format-specific
    information about a plugin instance.

    Note that the references passed to the visit member functions are only
    guaranteed to live for the duration of the function call, so don't
    store pointers to these objects! If you need to store and reuse
    format-specific information, it is recommended to copy the result
    of the function calls that you care about. For example, you should
    store the result of VST::getAEffectPtr() rather than storing a pointer
    to the VST instance.

    @tags{Audio}
*/
struct ExtensionsVisitor
{
    /** Indicates that there is no platform specific information available. */
    struct Unknown {};

    /** Can be used to retrieve information about a VST3 that is wrapped by an AudioProcessor. */
    struct VST3Client
    {
        virtual ~VST3Client() = default;
        virtual void* getIComponentPtr() const noexcept = 0;

        virtual MemoryBlock getPreset() const = 0;
        virtual bool setPreset (const MemoryBlock&) const = 0;
    };

    /** Can be used to retrieve information about an AudioUnit that is wrapped by an AudioProcessor. */
    struct AudioUnitClient
    {
        virtual ~AudioUnitClient() = default;
        virtual void* getAudioUnitHandle() const noexcept = 0;
    };

    /** Can be used to retrieve information about a VST that is wrapped by an AudioProcessor. */
    struct VSTClient
    {
        virtual ~VSTClient() = default;
        virtual void* getAEffectPtr() const noexcept = 0;
    };

    virtual ~ExtensionsVisitor() = default;

    /** Will be called if there is no platform specific information available. */
    virtual void visitUnknown           (const Unknown&)         {}

    /** Called with VST3-specific information. */
    virtual void visitVST3Client        (const VST3Client&)      {}

    /** Called with VST-specific information. */
    virtual void visitVSTClient         (const VSTClient&)       {}

    /** Called with AU-specific information. */
    virtual void visitAudioUnitClient   (const AudioUnitClient&) {}
};

} // namespace juce
