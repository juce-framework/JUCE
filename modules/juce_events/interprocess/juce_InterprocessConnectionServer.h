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

//==============================================================================
/**
    An object that waits for client sockets to connect to a port on this host, and
    creates InterprocessConnection objects for each one.

    To use this, create a class derived from it which implements the createConnectionObject()
    method, so that it creates suitable connection objects for each client that tries
    to connect.

    @see InterprocessConnection

    @tags{Events}
*/
class JUCE_API  InterprocessConnectionServer    : private Thread
{
public:
    //==============================================================================
    /** Creates an uninitialised server object.
    */
    InterprocessConnectionServer();

    /** Destructor. */
    ~InterprocessConnectionServer() override;

    //==============================================================================
    /** Starts an internal thread which listens on the given port number.

        While this is running, if another process tries to connect with the
        InterprocessConnection::connectToSocket() method, this object will call
        createConnectionObject() to create a connection to that client.

        Use stop() to stop the thread running.

        @param portNumber    The port on which the server will receive
                             connections
        @param bindAddress   The address on which the server will listen
                             for connections. An empty string indicates
                             that it should listen on all addresses
                             assigned to this machine.

        @see createConnectionObject, stop
    */
    bool beginWaitingForSocket (int portNumber, const String& bindAddress = String());

    /** Terminates the listener thread, if it's active.

        @see beginWaitingForSocket
    */
    void stop();

    /** Returns the local port number to which this server is currently bound.

        This is useful if you need to know to which port the OS has actually bound your
        socket when calling beginWaitingForSocket with a port number of zero.

        Returns -1 if the function fails.
    */
    int getBoundPort() const noexcept;

protected:
    /** Creates a suitable connection object for a client process that wants to
        connect to this one.

        This will be called by the listener thread when a client process tries
        to connect, and must return a new InterprocessConnection object that will
        then run as this end of the connection.

        @see InterprocessConnection
    */
    virtual InterprocessConnection* createConnectionObject() = 0;

private:
    //==============================================================================
    std::unique_ptr<StreamingSocket> socket;

    void run() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InterprocessConnectionServer)
};

} // namespace juce
