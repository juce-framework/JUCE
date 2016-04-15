/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_OSCSENDER_H_INCLUDED
#define JUCE_OSCSENDER_H_INCLUDED


//==============================================================================
/**
    An OSC message sender.

    An OSCSender object can connect to a network port. It then can send OSC
    messages and bundles to a specified host over an UDP socket.
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

        @param  targetHostName   The remote host to which messages will be send.
        @param  targetPortNumber The remote UDP port number on which the host will
                                 receive the messages.

        @returns true if the connection was successful; false otherwise.

        Note: the operating system will choose which specific network adapter(s)
        to bind your socket to, and which local port to use for the sender.

        @see send, disconnect.
    */
    bool connect (const String& targetHostName, int targetPortNumber);

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

   #if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
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
   #endif

private:
    //==============================================================================
    struct Pimpl;
    friend struct Pimpl;
    friend struct ContainerDeletePolicy<Pimpl>;
    ScopedPointer<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCSender)
};


//==============================================================================
#if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
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
#endif // JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS

#endif // JUCE_OSCSENDER_H_INCLUDED
