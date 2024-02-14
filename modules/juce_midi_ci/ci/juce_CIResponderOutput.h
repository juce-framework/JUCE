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
    Represents a destination into which MIDI-CI messages can be written.

    Each message should be written into the output buffer. Then, send() will
    send the current contents of the buffer to the specified group.

    @tags{Audio}
*/
class BufferOutput
{
public:
    BufferOutput() = default;
    virtual ~BufferOutput() = default;

    /** Returns the MUID of the responder. */
    virtual MUID getMuid() const = 0;

    /** Returns the buffer into which replies should be written. */
    virtual std::vector<std::byte>& getOutputBuffer() = 0;

    /** Sends the current contents of the buffer to the provided group. */
    virtual void send (uint8_t group) = 0;

    JUCE_DECLARE_NON_COPYABLE (BufferOutput)
    JUCE_DECLARE_NON_MOVEABLE (BufferOutput)
};

//==============================================================================
/**
    A buffer output that additionally provides information about an incoming message, so that
    an appropriate reply can be constructed for that message.

    @tags{Audio}
*/
class ResponderOutput : public BufferOutput
{
public:
    /** Returns the header of the message that was received. */
    virtual Message::Header getIncomingHeader() const = 0;

    /** Returns the group of the message that was received. */
    virtual uint8_t getIncomingGroup() const = 0;

    /** Returns the channel to which the incoming message was addressed. */
    ChannelAddress getChannelAddress() const;

    /** Returns a default header that can be used for outgoing replies.

        This always sets the destination MUID equal to the source MUID of the incoming header,
        so it's not suitable for broadcast messages.
    */
    Message::Header getReplyHeader (std::byte replySubID) const;
};

} // namespace juce::midi_ci
