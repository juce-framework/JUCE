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

namespace juce
{

//==============================================================================
struct OSCReceiver::Pimpl   : private Thread,
                              private MessageListener
{
    Pimpl (const String& oscThreadName)  : Thread (oscThreadName)
    {
    }

    ~Pimpl() override
    {
        disconnect();
    }

    //==============================================================================
    bool connectToPort (int portNumber)
    {
        if (! disconnect())
            return false;

        socket.setOwned (new DatagramSocket (false));

        if (! socket->bindToPort (portNumber))
            return false;

        startThread();
        return true;
    }

    bool connectToSocket (DatagramSocket& newSocket)
    {
        if (! disconnect())
            return false;

        socket.setNonOwned (&newSocket);
        startThread();
        return true;
    }

    bool disconnect()
    {
        if (socket != nullptr)
        {
            signalThreadShouldExit();

            if (socket.willDeleteObject())
                socket->shutdown();

            waitForThreadToExit (10000);
            socket.reset();
        }

        return true;
    }

    //==============================================================================
    void addListener (OSCReceiver::Listener<MessageLoopCallback>* listenerToAdd)
    {
        listeners.add (listenerToAdd);
    }

    void addListener (OSCReceiver::Listener<RealtimeCallback>* listenerToAdd)
    {
        realtimeListeners.add (listenerToAdd);
    }

    void addListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToAdd,
                      OSCAddress addressToMatch)
    {
        addListenerWithAddress (listenerToAdd, addressToMatch, listenersWithAddress);
    }

    void addListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToAdd, OSCAddress addressToMatch)
    {
        addListenerWithAddress (listenerToAdd, addressToMatch, realtimeListenersWithAddress);
    }

    void removeListener (OSCReceiver::Listener<MessageLoopCallback>* listenerToRemove)
    {
        listeners.remove (listenerToRemove);
    }

    void removeListener (OSCReceiver::Listener<RealtimeCallback>* listenerToRemove)
    {
        realtimeListeners.remove (listenerToRemove);
    }

    void removeListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToRemove)
    {
        removeListenerWithAddress (listenerToRemove, listenersWithAddress);
    }

    void removeListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToRemove)
    {
        removeListenerWithAddress (listenerToRemove, realtimeListenersWithAddress);
    }

    //==============================================================================
    struct CallbackMessage   : public Message
    {
        CallbackMessage (OSCBundle::Element oscElement)  : content (oscElement) {}

        // the payload of the message. Can be either an OSCMessage or an OSCBundle.
        OSCBundle::Element content;
    };

    //==============================================================================
    void handleBuffer (const char* data, size_t dataSize)
    {
        OSCInputStream inStream (data, dataSize);

        try
        {
            auto content = inStream.readElementWithKnownSize (dataSize);

            // realtime listeners should receive the OSC content first - and immediately
            // on this thread:
            callRealtimeListeners (content);

            if (content.isMessage())
                callRealtimeListenersWithAddress (content.getMessage());

            // now post the message that will trigger the handleMessage callback
            // dealing with the non-realtime listeners.
            if (listeners.size() > 0 || listenersWithAddress.size() > 0)
                postMessage (new CallbackMessage (content));
        }
        catch (const OSCFormatError&)
        {
            if (formatErrorHandler != nullptr)
                formatErrorHandler (data, (int) dataSize);
        }
    }

    //==============================================================================
    void registerFormatErrorHandler (OSCReceiver::FormatErrorHandler handler)
    {
        formatErrorHandler = handler;
    }

private:
    //==============================================================================
    void run() override
    {
        int bufferSize = 65535;
        HeapBlock<char> oscBuffer (bufferSize);

        while (! threadShouldExit())
        {
            jassert (socket != nullptr);
            auto ready = socket->waitUntilReady (true, 100);

            if (ready < 0 || threadShouldExit())
                return;

            if (ready == 0)
                continue;

            auto bytesRead = (size_t) socket->read (oscBuffer.getData(), bufferSize, false);

            if (bytesRead >= 4)
                handleBuffer (oscBuffer.getData(), bytesRead);
        }
    }

    //==============================================================================
    template <typename ListenerType>
    void addListenerWithAddress (ListenerType* listenerToAdd,
                                 OSCAddress address,
                                 Array<std::pair<OSCAddress, ListenerType*>>& array)
    {
        for (auto& i : array)
            if (address == i.first && listenerToAdd == i.second)
                return;

        array.add (std::make_pair (address, listenerToAdd));
    }

    //==============================================================================
    template <typename ListenerType>
    void removeListenerWithAddress (ListenerType* listenerToRemove,
                                    Array<std::pair<OSCAddress, ListenerType*>>& array)
    {
        for (int i = 0; i < array.size(); ++i)
        {
            if (listenerToRemove == array.getReference (i).second)
            {
                // aarrgh... can't simply call array.remove (i) because this
                // requires a default c'tor to be present for OSCAddress...
                // luckily, we don't care about methods preserving element order:
                array.swap (i, array.size() - 1);
                array.removeLast();
                break;
            }
        }
    }

    //==============================================================================
    void handleMessage (const Message& msg) override
    {
        if (auto* callbackMessage = dynamic_cast<const CallbackMessage*> (&msg))
        {
            auto& content = callbackMessage->content;

            callListeners (content);

            if (content.isMessage())
                callListenersWithAddress (content.getMessage());
        }
    }

    //==============================================================================
    void callListeners (const OSCBundle::Element& content)
    {
        using OSCListener = OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>;

        if (content.isMessage())
        {
            auto&& message = content.getMessage();
            listeners.call ([&] (OSCListener& l) { l.oscMessageReceived (message); });
        }
        else if (content.isBundle())
        {
            auto&& bundle = content.getBundle();
            listeners.call ([&] (OSCListener& l) { l.oscBundleReceived (bundle); });
        }
    }

    void callRealtimeListeners (const OSCBundle::Element& content)
    {
        using OSCListener = OSCReceiver::Listener<OSCReceiver::RealtimeCallback>;

        if (content.isMessage())
        {
            auto&& message = content.getMessage();
            realtimeListeners.call ([&] (OSCListener& l) { l.oscMessageReceived (message); });
        }
        else if (content.isBundle())
        {
            auto&& bundle = content.getBundle();
            realtimeListeners.call ([&] (OSCListener& l) { l.oscBundleReceived (bundle); });
        }
    }

    //==============================================================================
    void callListenersWithAddress (const OSCMessage& message)
    {
        for (auto& entry : listenersWithAddress)
            if (auto* listener = entry.second)
                if (message.getAddressPattern().matches (entry.first))
                    listener->oscMessageReceived (message);
    }

    void callRealtimeListenersWithAddress (const OSCMessage& message)
    {
        for (auto& entry : realtimeListenersWithAddress)
            if (auto* listener = entry.second)
                if (message.getAddressPattern().matches (entry.first))
                    listener->oscMessageReceived (message);
    }

    //==============================================================================
    ListenerList<OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>> listeners;
    ListenerList<OSCReceiver::Listener<OSCReceiver::RealtimeCallback>>    realtimeListeners;

    Array<std::pair<OSCAddress, OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>*>> listenersWithAddress;
    Array<std::pair<OSCAddress, OSCReceiver::ListenerWithOSCAddress<OSCReceiver::RealtimeCallback>*>>    realtimeListenersWithAddress;

    OptionalScopedPointer<DatagramSocket> socket;
    OSCReceiver::FormatErrorHandler formatErrorHandler { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
OSCReceiver::OSCReceiver (const String& threadName)   : pimpl (new Pimpl (threadName))
{
}

OSCReceiver::OSCReceiver()  : OSCReceiver ("JUCE OSC server")
{
}

OSCReceiver::~OSCReceiver()
{
    pimpl.reset();
}

bool OSCReceiver::connect (int portNumber)
{
    return pimpl->connectToPort (portNumber);
}

bool OSCReceiver::connectToSocket (DatagramSocket& socket)
{
    return pimpl->connectToSocket (socket);
}

bool OSCReceiver::disconnect()
{
    return pimpl->disconnect();
}

void OSCReceiver::addListener (OSCReceiver::Listener<MessageLoopCallback>* listenerToAdd)
{
    pimpl->addListener (listenerToAdd);
}

void OSCReceiver::addListener (Listener<RealtimeCallback>* listenerToAdd)
{
    pimpl->addListener (listenerToAdd);
}

void OSCReceiver::addListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToAdd, OSCAddress addressToMatch)
{
    pimpl->addListener (listenerToAdd, addressToMatch);
}

void OSCReceiver::addListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToAdd, OSCAddress addressToMatch)
{
    pimpl->addListener (listenerToAdd, addressToMatch);
}

void OSCReceiver::removeListener (Listener<MessageLoopCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

void OSCReceiver::removeListener (Listener<RealtimeCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

void OSCReceiver::removeListener (ListenerWithOSCAddress<MessageLoopCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

void OSCReceiver::removeListener (ListenerWithOSCAddress<RealtimeCallback>* listenerToRemove)
{
    pimpl->removeListener (listenerToRemove);
}

void OSCReceiver::registerFormatErrorHandler (FormatErrorHandler handler)
{
    pimpl->registerFormatErrorHandler (handler);
}

} // namespace juce
