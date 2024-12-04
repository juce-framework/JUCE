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
    An interface to allow an AudioProcessor to implement extended VST2-specific functionality.

    To use this class, create an object that inherits from it, implement the methods, then return
    a pointer to the object in your AudioProcessor::getVST2ClientExtensions() method.

    @see AudioProcessor, AAXClientExtensions, VST3ClientExtensions

    @tags{Audio}
*/
struct VST2ClientExtensions
{
    virtual ~VST2ClientExtensions() = default;

    /** This is called by the VST plug-in wrapper when it receives unhandled
        plug-in "can do" calls from the host.
    */
    virtual pointer_sized_int handleVstPluginCanDo (int32 index,
                                                    pointer_sized_int value,
                                                    void* ptr,
                                                    float opt);

    /** This is called by the VST plug-in wrapper when it receives unhandled
        vendor specific calls from the host.
    */
    virtual pointer_sized_int handleVstManufacturerSpecific (int32 index,
                                                             pointer_sized_int value,
                                                             void* ptr,
                                                             float opt) = 0;

    /** The host callback function type. */
    using VstHostCallbackType = pointer_sized_int (int32 opcode,
                                                   int32 index,
                                                   pointer_sized_int value,
                                                   void* ptr,
                                                   float opt);

    /** This is called once by the VST plug-in wrapper after its constructor.
        You can use the supplied function to query the VST host.
    */
    virtual void handleVstHostCallbackAvailable (std::function<VstHostCallbackType>&& callback);
};

using VSTCallbackHandler [[deprecated ("replace with VST2ClientExtensions")]] = VST2ClientExtensions;

} // namespace juce
