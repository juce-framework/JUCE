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

#ifndef DOXYGEN

// Forward declaration to avoid leaking implementation details.
namespace Steinberg
{
    class FUnknown;
    using TUID = char[16];
} // namespace Steinberg

#endif

namespace juce
{

/** An interface to allow an AudioProcessor to implement extended VST3-specific
    functionality.

    To use this class, ensure that your AudioProcessor publicly inherits
    from VST3ClientExtensions.

    @see VSTCallbackHandler

    @tags{Audio}
*/
struct VST3ClientExtensions
{
    virtual ~VST3ClientExtensions() = default;

    /** This function may be used by implementations of queryInterface()
        in the VST3's implementation of IEditController to return
        additional supported interfaces.
    */
    virtual int32_t queryIEditController (const Steinberg::TUID, void** obj)
    {
        *obj = nullptr;
        return -1;
    }

    /** This function may be used by implementations of queryInterface()
        in the VST3's implementation of IAudioProcessor to return
        additional supported interfaces.
    */
    virtual int32_t queryIAudioProcessor (const Steinberg::TUID, void** obj)
    {
        *obj = nullptr;
        return -1;
    }

    /** This may be called by the VST3 wrapper when the host sets an
        IComponentHandler for the plugin to use.

        You should not make any assumptions about how and when this will be
        called - this function may not be called at all!
    */
    virtual void setIComponentHandler (Steinberg::FUnknown*) {}

    /** This may be called shortly after the AudioProcessor is constructed
        with the current IHostApplication.

        You should not make any assumptions about how and when this will be
        called - this function may not be called at all!
    */
    virtual void setIHostApplication  (Steinberg::FUnknown*) {}

    /** This function will be called to check whether the first input bus
        should be designated as "kMain" or "kAux". Return true if the
        first bus should be kMain, or false if the bus should be kAux.

        All other input buses will always be designated kAux.
    */
    virtual bool getPluginHasMainInput() const  { return true; }
};

} // namespace juce
