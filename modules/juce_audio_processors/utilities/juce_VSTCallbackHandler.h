/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** An interface to allow an AudioProcessor to send and receive VST specific calls from
    the host.

    To use this class, ensure that your AudioProcessor publicly inherits
    from VSTCallbackHandler.

    @see VST3ClientExtensions

    @tags{Audio}
*/
struct VSTCallbackHandler
{
    virtual ~VSTCallbackHandler() = default;

    /** This is called by the VST plug-in wrapper when it receives unhandled
        plug-in "can do" calls from the host.
    */
    virtual pointer_sized_int handleVstPluginCanDo (int32 index,
                                                    pointer_sized_int value,
                                                    void* ptr,
                                                    float opt)
    {
        ignoreUnused (index, value, ptr, opt);
        return 0;
    }

    /** This is called by the VST plug-in wrapper when it receives unhandled
        vendor specific calls from the host.
    */
    virtual pointer_sized_int handleVstManufacturerSpecific (int32 index,
                                                             pointer_sized_int value,
                                                             void* ptr,
                                                             float opt) = 0;

    // Note: VS2013 prevents a "using" declaration here
    /** The host callback function type. */
    typedef pointer_sized_int (VstHostCallbackType) (int32 opcode,
                                                     int32 index,
                                                     pointer_sized_int value,
                                                     void* ptr,
                                                     float opt);

    /** This is called once by the VST plug-in wrapper after its constructor.
        You can use the supplied function to query the VST host.
    */
    virtual void handleVstHostCallbackAvailable (std::function<VstHostCallbackType>&& callback)
    {
        ignoreUnused (callback);
    }
};

} // namespace juce
