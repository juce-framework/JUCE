/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci
{

/**
    An interface for types that implement responses for certain message types.

    @tags{Audio}
*/
class ResponderDelegate
{
public:
    ResponderDelegate() = default;
    virtual ~ResponderDelegate() = default;

    /** If the message is processed successfully, and a response sent, then
        this returns true. Otherwise, returns false, allowing other ResponderDelegates
        to attempt to handle the message if necessary.
    */
    virtual bool tryRespond (ResponderOutput& output, const Message::Parsed& message) = 0;

    JUCE_DECLARE_NON_COPYABLE (ResponderDelegate)
    JUCE_DECLARE_NON_MOVEABLE (ResponderDelegate)
};

} // namespace juce::midi_ci
