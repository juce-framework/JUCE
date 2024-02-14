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

namespace juce::midi_ci::detail
{

/*
    Parses individual messages, and additionally gives ResponderDelegates a chance to formulate
    a response to any message that would normally necessitate a reply.
*/
struct Responder
{
    Responder() = delete;

    /*  Parses the message, then calls tryParse on each ResponderDelegate in
        turn until one returns true, indicating that the message has been
        handled. Most 'inquiry' messages should emit one or more reply messages.
        These replies will be written to the provided BufferOutput.
        If none of the provided delegates are able to handle the message, then
        a generic NAK will be written to the BufferOutput.
    */
    static Parser::Status processCompleteMessage (BufferOutput& output,
                                                  ump::BytesOnGroup message,
                                                  Span<ResponderDelegate* const> delegates);
};

} // namespace juce::midi_ci
