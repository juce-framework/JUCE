/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
struct OSCSender::Pimpl
{
    Pimpl() noexcept  {}
    ~Pimpl() noexcept { disconnect(); }

    //==============================================================================
    bool connect (const String& newTargetHost, int newTargetPort)
    {
        if (! disconnect())
            return false;

        socket.setOwned (new DatagramSocket (true));
        targetHostName = newTargetHost;
        targetPortNumber = newTargetPort;

        if (socket->bindToPort (0)) // 0 = use any local port assigned by the OS.
            return true;

        socket.reset();
        return false;
    }

    bool connectToSocket (DatagramSocket& newSocket, const String& newTargetHost, int newTargetPort)
    {
        if (! disconnect())
            return false;

        socket.setNonOwned (&newSocket);
        targetHostName = newTargetHost;
        targetPortNumber = newTargetPort;
        return true;
    }

    bool disconnect()
    {
        socket.reset();
        return true;
    }

    //==============================================================================
    bool send (const OSCMessage& message, const String& hostName, int portNumber)
    {
        OSCOutputStream outStream;

        return outStream.writeMessage (message)
            && sendOutputStream (outStream, hostName, portNumber);
    }

    bool send (const OSCBundle& bundle, const String& hostName, int portNumber)
    {
        OSCOutputStream outStream;

        return outStream.writeBundle (bundle)
            && sendOutputStream (outStream, hostName, portNumber);
    }

    bool send (const OSCMessage& message)   { return send (message, targetHostName, targetPortNumber); }
    bool send (const OSCBundle& bundle)     { return send (bundle,  targetHostName, targetPortNumber); }

private:
    //==============================================================================
    bool sendOutputStream (OSCOutputStream& outStream, const String& hostName, int portNumber)
    {
        if (socket != nullptr)
        {
            const int streamSize = (int) outStream.getDataSize();

            const int bytesWritten = socket->write (hostName, portNumber,
                                                    outStream.getData(), streamSize);
            return bytesWritten == streamSize;
        }

        // if you hit this, you tried to send some OSC data without being
        // connected to a port! You should call OSCSender::connect() first.
        jassertfalse;

        return false;
    }

    //==============================================================================
    OptionalScopedPointer<DatagramSocket> socket;
    String targetHostName;
    int targetPortNumber = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
OSCSender::OSCSender()   : pimpl (new Pimpl())
{
}

OSCSender::~OSCSender()
{
    pimpl->disconnect();
    pimpl.reset();
}

//==============================================================================
bool OSCSender::connect (const String& targetHostName, int targetPortNumber)
{
    return pimpl->connect (targetHostName, targetPortNumber);
}

bool OSCSender::connectToSocket (DatagramSocket& socket, const String& targetHostName, int targetPortNumber)
{
    return pimpl->connectToSocket (socket, targetHostName, targetPortNumber);
}

bool OSCSender::disconnect()
{
    return pimpl->disconnect();
}

//==============================================================================
bool OSCSender::send (const OSCMessage& message)    { return pimpl->send (message); }
bool OSCSender::send (const OSCBundle& bundle)      { return pimpl->send (bundle); }

bool OSCSender::sendToIPAddress (const String& host, int port, const OSCMessage& message) { return pimpl->send (message, host, port); }
bool OSCSender::sendToIPAddress (const String& host, int port, const OSCBundle& bundle)   { return pimpl->send (bundle,  host, port); }

} // namespace juce
