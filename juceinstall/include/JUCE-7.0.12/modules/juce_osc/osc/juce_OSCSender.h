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

namespace juce
{

//==============================================================================
/**
    An OSC message sender.

    An OSCSender object can connect to a network port. It then can send OSC
    messages and bundles to a specified host over an UDP socket.

    @tags{OSC}
*/
class JUCE_API  OSCSender
{
public:
    //==============================================================================
    /** Constructs a new OSCSender. */
    OSCSender();

    /** Destructor. */
    ~OSCSender();

    //==============================================================================
    /** Connects to a datagram socket and prepares the socket for sending OSC
        packets to the specified target.

        Note: The operating system will choose which specific network adapter(s)
        to bind your socket to, and which local port to use for the sender.

        @param  targetHostName   The remote host to which messages will be send.
        @param  targetPortNumber The remote UDP port number on which the host will
                                 receive the messages.

        @returns true if the connection was successful; false otherwise.
        @see send, disconnect.
    */
    bool connect (const String& targetHostName, int targetPortNumber);

    /** Uses an existing datagram socket for sending OSC packets to the specified target.

        @param  socket           An existing datagram socket. Make sure this doesn't
                                 get deleted while this class is still using it!
        @param  targetHostName   The remote host to which messages will be send.
        @param  targetPortNumber The remote UDP port number on which the host will
                                 receive the messages.

        @returns true if the connection was successful; false otherwise.
        @see connect, send, disconnect.
    */
    bool connectToSocket (DatagramSocket& socket, const String& targetHostName, int targetPortNumber);

    //==============================================================================
    /** Disconnects from the currently used UDP port.
        @returns true if the disconnection was successful; false otherwise.
        @see connect.
    */
    bool disconnect();

    //==============================================================================
    /** Sends an OSC message to the target.
        @param  message   The OSC message to send.
        @returns true if the operation was successful.
    */
    bool send (const OSCMessage& message);

    /** Send an OSC bundle to the target.
        @param  bundle    The OSC bundle to send.
        @returns true if the operation was successful.
    */
    bool send (const OSCBundle& bundle);

    /** Sends an OSC message to a specific IP address and port.
        This overrides the address and port that was originally set for this sender.
        @param  targetIPAddress   The IP address to send to
        @param  targetPortNumber  The target port number
        @param  message           The OSC message to send.
        @returns true if the operation was successful.
    */
    bool sendToIPAddress (const String& targetIPAddress, int targetPortNumber,
                          const OSCMessage& message);

    /** Sends an OSC bundle to a specific IP address and port.
        This overrides the address and port that was originally set for this sender.
        @param  targetIPAddress   The IP address to send to
        @param  targetPortNumber  The target port number
        @param  bundle            The OSC bundle to send.
        @returns true if the operation was successful.
    */
    bool sendToIPAddress (const String& targetIPAddress, int targetPortNumber,
                          const OSCBundle& bundle);

    /** Creates a new OSC message with the specified address pattern and list
        of arguments, and sends it to the target.

        @param  address  The OSC address pattern of the message
                         (you can use a string literal here).
        @param  args     The list of arguments for the message.
    */
    template <typename... Args>
    bool send (const OSCAddressPattern& address, Args&&... args);

    /** Creates a new OSC message with the specified address pattern and list
        of arguments, and sends it to the target.

        @param  targetIPAddress   The IP address to send to
        @param  targetPortNumber  The target port number
        @param  address  The OSC address pattern of the message
                         (you can use a string literal here).
        @param  args     The list of arguments for the message.
    */
    template <typename... Args>
    bool sendToIPAddress (const String& targetIPAddress, int targetPortNumber,
                          const OSCAddressPattern& address, Args&&... args);

private:
    //==============================================================================
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCSender)
};


//==============================================================================
template <typename... Args>
bool OSCSender::send (const OSCAddressPattern& address, Args&&... args)
{
    return send (OSCMessage (address, std::forward<Args> (args)...));
}

template <typename... Args>
bool OSCSender::sendToIPAddress (const String& targetIPAddress, int targetPortNumber,
                                 const OSCAddressPattern& address, Args&&... args)
{
    return sendToIPAddress (targetIPAddress, targetPortNumber, OSCMessage (address, std::forward<Args> (args)...));
}

} // namespace juce
