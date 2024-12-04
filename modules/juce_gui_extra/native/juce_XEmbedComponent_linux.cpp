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

::Window juce_createKeyProxyWindow (ComponentPeer*);
void juce_deleteKeyProxyWindow (::Window);

//==============================================================================
enum
{
    maxXEmbedVersionToSupport = 0
};

enum
{
    XEMBED_MAPPED  = (1<<0)
};

enum
{
    XEMBED_EMBEDDED_NOTIFY        = 0,
    XEMBED_WINDOW_ACTIVATE        = 1,
    XEMBED_WINDOW_DEACTIVATE      = 2,
    XEMBED_REQUEST_FOCUS          = 3,
    XEMBED_FOCUS_IN               = 4,
    XEMBED_FOCUS_OUT              = 5,
    XEMBED_FOCUS_NEXT             = 6,
    XEMBED_FOCUS_PREV             = 7,
    XEMBED_MODALITY_ON            = 10,
    XEMBED_MODALITY_OFF           = 11,
    XEMBED_REGISTER_ACCELERATOR   = 12,
    XEMBED_UNREGISTER_ACCELERATOR = 13,
    XEMBED_ACTIVATE_ACCELERATOR   = 14
};

enum
{
    XEMBED_FOCUS_CURRENT = 0,
    XEMBED_FOCUS_FIRST   = 1,
    XEMBED_FOCUS_LAST    = 2
};

//==============================================================================
class XEmbedComponent::Pimpl  : private ComponentListener
{
public:
    //==============================================================================
    struct SharedKeyWindow final : public ReferenceCountedObject
    {
        SharedKeyWindow (ComponentPeer* peerToUse)
            : keyPeer (peerToUse),
              keyProxy (juce_createKeyProxyWindow (keyPeer)),
              association (peerToUse, keyProxy)
        {}

        ~SharedKeyWindow()
        {
            association = {};
            juce_deleteKeyProxyWindow (keyProxy);

            auto& keyWindows = getKeyWindows();
            keyWindows.remove (keyPeer);
        }

        using Ptr = ReferenceCountedObjectPtr<SharedKeyWindow>;

        //==============================================================================
        Window getHandle()    { return keyProxy; }

        static Window getCurrentFocusWindow (ComponentPeer* peerToLookFor)
        {
            auto& keyWindows = getKeyWindows();

            if (peerToLookFor != nullptr)
                if (auto* foundKeyWindow = keyWindows[peerToLookFor])
                    return foundKeyWindow->keyProxy;

            return {};
        }

        static SharedKeyWindow::Ptr getKeyWindowForPeer (ComponentPeer* peerToLookFor)
        {
            jassert (peerToLookFor != nullptr);

            auto& keyWindows = getKeyWindows();
            auto foundKeyWindow = keyWindows[peerToLookFor];

            if (foundKeyWindow == nullptr)
            {
                foundKeyWindow = new SharedKeyWindow (peerToLookFor);
                keyWindows.set (peerToLookFor, foundKeyWindow);
            }

            return foundKeyWindow;
        }

    private:
        //==============================================================================
        ComponentPeer* keyPeer;
        Window keyProxy;
        ScopedWindowAssociation association;

        static HashMap<ComponentPeer*, SharedKeyWindow*>& getKeyWindows()
        {
            // store a weak reference to the shared key windows
            static HashMap<ComponentPeer*, SharedKeyWindow*> keyWindows;
            return keyWindows;
        }
    };

public:
    //==============================================================================
    Pimpl (XEmbedComponent& parent, Window x11Window,
           bool wantsKeyboardFocus, bool isClientInitiated, bool shouldAllowResize)
        : owner (parent),
          infoAtom (XWindowSystem::getInstance()->getAtoms().XembedInfo),
          messageTypeAtom (XWindowSystem::getInstance()->getAtoms().XembedMsgType),
          clientInitiated (isClientInitiated),
          wantsFocus (wantsKeyboardFocus),
          allowResize (shouldAllowResize)
    {
        getWidgets().add (this);

        createHostWindow();

        if (clientInitiated)
            setClient (x11Window, true);

        owner.setWantsKeyboardFocus (wantsFocus);
        owner.addComponentListener (this);
    }

    ~Pimpl() override
    {
        owner.removeComponentListener (this);
        setClient (0, true);

        if (host != 0)
        {
            auto dpy = getDisplay();

            X11Symbols::getInstance()->xDestroyWindow (dpy, host);
            X11Symbols::getInstance()->xSync (dpy, false);

            auto mask = NoEventMask | KeyPressMask | KeyReleaseMask
                      | EnterWindowMask | LeaveWindowMask | PointerMotionMask
                      | KeymapStateMask | ExposureMask | StructureNotifyMask
                      | FocusChangeMask;

            XEvent event;
            while (X11Symbols::getInstance()->xCheckWindowEvent (dpy, host, mask, &event) == True)
            {}

            host = 0;
        }

        getWidgets().removeAllInstancesOf (this);
    }

    //==============================================================================
    void setClient (Window xembedClient, bool shouldReparent)
    {
        removeClient();

        if (xembedClient != 0)
        {
            auto dpy = getDisplay();

            client = xembedClient;

            // if the client has initiated the component then keep the clients size
            // otherwise the client should use the host's window' size
            if (clientInitiated)
            {
                configureNotify();
            }
            else
            {
                auto newBounds = getX11BoundsFromJuce();
                X11Symbols::getInstance()->xResizeWindow (dpy, client, static_cast<unsigned int> (newBounds.getWidth()),
                                                          static_cast<unsigned int> (newBounds.getHeight()));
            }

            auto eventMask = StructureNotifyMask | PropertyChangeMask | FocusChangeMask;

            XWindowAttributes clientAttr;
            X11Symbols::getInstance()->xGetWindowAttributes (dpy, client, &clientAttr);

            if ((eventMask & clientAttr.your_event_mask) != eventMask)
                X11Symbols::getInstance()->xSelectInput (dpy, client, clientAttr.your_event_mask | eventMask);

            getXEmbedMappedFlag();

            if (shouldReparent)
                X11Symbols::getInstance()->xReparentWindow (dpy, client, host, 0, 0);

            if (supportsXembed)
                sendXEmbedEvent (CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, (long) host, xembedVersion);

            updateMapping();
        }
    }

    void focusGained (FocusChangeType changeType, FocusChangeDirection direction)
    {
        if (client != 0 && supportsXembed && wantsFocus)
        {
            updateKeyFocus();

            const auto xembedDirection = [&]
            {
                if (direction == FocusChangeDirection::forward)
                    return XEMBED_FOCUS_FIRST;

                if (direction == FocusChangeDirection::backward)
                    return XEMBED_FOCUS_LAST;

                return XEMBED_FOCUS_CURRENT;
            }();

            sendXEmbedEvent (CurrentTime, XEMBED_FOCUS_IN,
                             (changeType == focusChangedByTabKey ? xembedDirection : XEMBED_FOCUS_CURRENT));
        }
    }

    void focusLost (FocusChangeType)
    {
        if (client != 0 && supportsXembed && wantsFocus)
        {
            sendXEmbedEvent (CurrentTime, XEMBED_FOCUS_OUT);
            updateKeyFocus();
        }
    }

    void broughtToFront()
    {
        if (client != 0 && supportsXembed)
            sendXEmbedEvent (CurrentTime, XEMBED_WINDOW_ACTIVATE);
    }

    unsigned long getHostWindowID()
    {
        // You are using the client initiated version of the protocol. You cannot
        // retrieve the window id of the host. Please read the documentation for
        // the XEmebedComponent class.
        jassert (! clientInitiated);

        return host;
    }

    void updateEmbeddedBounds()
    {
        componentMovedOrResized (owner, true, true);
    }

private:
    //==============================================================================
    XEmbedComponent& owner;
    Window client = 0, host = 0;
    Atom infoAtom, messageTypeAtom;

    bool clientInitiated;
    bool wantsFocus        = false;
    bool allowResize       = false;
    bool supportsXembed    = false;
    bool hasBeenMapped     = false;
    int xembedVersion      = maxXEmbedVersionToSupport;

    ComponentPeer* lastPeer = nullptr;
    SharedKeyWindow::Ptr keyWindow;

    //==============================================================================
    void componentParentHierarchyChanged (Component&) override   { peerChanged (owner.getPeer()); }
    void componentMovedOrResized (Component&, bool, bool) override
    {
        if (host != 0 && lastPeer != nullptr)
        {
            auto dpy = getDisplay();
            auto newBounds = getX11BoundsFromJuce();
            XWindowAttributes attr;

            if (X11Symbols::getInstance()->xGetWindowAttributes (dpy, host, &attr))
            {
                Rectangle<int> currentBounds (attr.x, attr.y, attr.width, attr.height);
                if (currentBounds != newBounds)
                {
                    X11Symbols::getInstance()->xMoveResizeWindow (dpy, host, newBounds.getX(), newBounds.getY(),
                                                                  static_cast<unsigned int> (newBounds.getWidth()),
                                                                  static_cast<unsigned int> (newBounds.getHeight()));
                }
            }

            if (client != 0 && X11Symbols::getInstance()->xGetWindowAttributes (dpy, client, &attr))
            {
                Rectangle<int> currentBounds (attr.x, attr.y, attr.width, attr.height);

                if ((currentBounds.getWidth() != newBounds.getWidth()
                     || currentBounds.getHeight() != newBounds.getHeight()))
                {
                    X11Symbols::getInstance()->xMoveResizeWindow (dpy, client, 0, 0,
                                                                  static_cast<unsigned int> (newBounds.getWidth()),
                                                                  static_cast<unsigned int> (newBounds.getHeight()));
                }
            }
        }
    }

    //==============================================================================
    void createHostWindow()
    {
        auto dpy = getDisplay();
        int defaultScreen = X11Symbols::getInstance()->xDefaultScreen (dpy);
        Window root = X11Symbols::getInstance()->xRootWindow (dpy, defaultScreen);

        XSetWindowAttributes swa;
        swa.border_pixel = 0;
        swa.background_pixmap = None;
        swa.override_redirect = True;
        swa.event_mask = SubstructureNotifyMask | StructureNotifyMask | FocusChangeMask;

        host = X11Symbols::getInstance()->xCreateWindow (dpy, root, 0, 0, 1, 1, 0, CopyFromParent,
                                                         InputOutput, CopyFromParent,
                                                         CWEventMask | CWBorderPixel | CWBackPixmap | CWOverrideRedirect,
                                                         &swa);
    }

    void removeClient()
    {
        if (client != 0)
        {
            auto dpy = getDisplay();
            X11Symbols::getInstance()->xSelectInput (dpy, client, 0);

            keyWindow = nullptr;

            int defaultScreen = X11Symbols::getInstance()->xDefaultScreen (dpy);
            Window root = X11Symbols::getInstance()->xRootWindow (dpy, defaultScreen);

            if (hasBeenMapped)
            {
                X11Symbols::getInstance()->xUnmapWindow (dpy, client);
                hasBeenMapped = false;
            }

            X11Symbols::getInstance()->xReparentWindow (dpy, client, root, 0, 0);
            client = 0;

            X11Symbols::getInstance()->xSync (dpy, False);
        }
    }

    void updateMapping()
    {
        if (client != 0)
        {
            const bool shouldBeMapped = getXEmbedMappedFlag();

            if (shouldBeMapped != hasBeenMapped)
            {
                hasBeenMapped = shouldBeMapped;

                if (shouldBeMapped)
                    X11Symbols::getInstance()->xMapWindow (getDisplay(), client);
                else
                    X11Symbols::getInstance()->xUnmapWindow (getDisplay(), client);
            }
        }
    }

    Window getParentX11Window()
    {
        if (auto* peer = owner.getPeer())
            return reinterpret_cast<Window> (peer->getNativeHandle());

        return {};
    }

    Display* getDisplay()   { return XWindowSystem::getInstance()->getDisplay(); }

    //==============================================================================
    bool getXEmbedMappedFlag()
    {
        XWindowSystemUtilities::GetXProperty embedInfo (getDisplay(), client, infoAtom, 0, 2, false, infoAtom);

        if (embedInfo.success && embedInfo.actualFormat == 32
             && embedInfo.numItems >= 2 && embedInfo.data != nullptr)
        {
            long version;
            memcpy (&version, embedInfo.data, sizeof (long));

            supportsXembed = true;
            xembedVersion = jmin ((int) maxXEmbedVersionToSupport, (int) version);

            long flags;
            memcpy (&flags, embedInfo.data + sizeof (long), sizeof (long));

            return ((flags & XEMBED_MAPPED) != 0);
        }
        else
        {
            supportsXembed = false;
            xembedVersion = maxXEmbedVersionToSupport;
        }

        return true;
    }

    //==============================================================================
    void propertyChanged (const Atom& a)
    {
        if (a == infoAtom)
            updateMapping();
    }

    void configureNotify()
    {
        XWindowAttributes attr;
        auto dpy = getDisplay();

        if (X11Symbols::getInstance()->xGetWindowAttributes (dpy, client, &attr))
        {
            XWindowAttributes hostAttr;

            if (X11Symbols::getInstance()->xGetWindowAttributes (dpy, host, &hostAttr))
                if (attr.width != hostAttr.width || attr.height != hostAttr.height)
                    X11Symbols::getInstance()->xResizeWindow (dpy, host, (unsigned int) attr.width, (unsigned int) attr.height);

            // as the client window is not on any screen yet, we need to guess
            // on which screen it might appear to get a scaling factor :-(
            auto& displays = Desktop::getInstance().getDisplays();
            auto* peer = owner.getPeer();
            const double scale = (peer != nullptr ? peer->getPlatformScaleFactor()
                                                  : displays.getPrimaryDisplay()->scale);

            Point<int> topLeftInPeer
                = (peer != nullptr ? peer->getComponent().getLocalPoint (&owner, Point<int> (0, 0))
                   : owner.getBounds().getTopLeft());

            Rectangle<int> newBounds (topLeftInPeer.getX(), topLeftInPeer.getY(),
                                      static_cast<int> (static_cast<double> (attr.width)  / scale),
                                      static_cast<int> (static_cast<double> (attr.height) / scale));


            if (peer != nullptr)
                newBounds = owner.getLocalArea (&peer->getComponent(), newBounds);

            jassert (newBounds.getX() == 0 && newBounds.getY() == 0);

            if (newBounds != owner.getLocalBounds())
                owner.setSize (newBounds.getWidth(), newBounds.getHeight());
        }
    }

    void peerChanged (ComponentPeer* newPeer)
    {
        if (newPeer != lastPeer)
        {
            if (lastPeer != nullptr)
                keyWindow = nullptr;

            auto dpy = getDisplay();
            Window rootWindow = X11Symbols::getInstance()->xRootWindow (dpy, DefaultScreen (dpy));
            Rectangle<int> newBounds = getX11BoundsFromJuce();

            if (newPeer == nullptr)
                X11Symbols::getInstance()->xUnmapWindow (dpy, host);

            Window newParent = (newPeer != nullptr ? getParentX11Window() : rootWindow);
            X11Symbols::getInstance()->xReparentWindow (dpy, host, newParent, newBounds.getX(), newBounds.getY());

            lastPeer = newPeer;

            if (newPeer != nullptr)
            {
                if (wantsFocus)
                {
                    keyWindow = SharedKeyWindow::getKeyWindowForPeer (newPeer);
                    updateKeyFocus();
                }

                componentMovedOrResized (owner, true, true);
                X11Symbols::getInstance()->xMapWindow (dpy, host);

                broughtToFront();
            }
        }
    }

    void updateKeyFocus()
    {
        if (lastPeer != nullptr && lastPeer->isFocused())
            X11Symbols::getInstance()->xSetInputFocus (getDisplay(), getCurrentFocusWindow (lastPeer), RevertToParent, CurrentTime);
    }

    //==============================================================================
    void handleXembedCmd (const ::Time& /*xTime*/, long opcode, long /*detail*/, long /*data1*/, long /*data2*/)
    {
        if (auto* peer = owner.getPeer())
            peer->getCurrentModifiersRealtime();

        switch (opcode)
        {
            case XEMBED_REQUEST_FOCUS:
                if (wantsFocus)
                    owner.grabKeyboardFocus();
                break;

            case XEMBED_FOCUS_NEXT:
                if (wantsFocus)
                    owner.moveKeyboardFocusToSibling (true);
                break;

            case XEMBED_FOCUS_PREV:
                if (wantsFocus)
                    owner.moveKeyboardFocusToSibling (false);
                break;

            default:
                break;
        }
    }

    bool handleX11Event (const XEvent& e)
    {
        if (e.xany.window == client && client != 0)
        {
            switch (e.type)
            {
                case PropertyNotify:
                    propertyChanged (e.xproperty.atom);
                    return true;

                case ConfigureNotify:
                    if (allowResize)
                        configureNotify();
                    else
                        MessageManager::callAsync ([this] {componentMovedOrResized (owner, true, true);});

                    return true;

                default:
                    break;
            }
        }
        else if (e.xany.window == host && host != 0)
        {
            switch (e.type)
            {
                case ReparentNotify:
                    if (e.xreparent.parent == host && e.xreparent.window != client)
                    {
                        setClient (e.xreparent.window, false);
                        return true;
                    }
                    break;

                case CreateNotify:
                    if (e.xcreatewindow.parent != e.xcreatewindow.window && e.xcreatewindow.parent == host && e.xcreatewindow.window != client)
                    {
                        setClient (e.xcreatewindow.window, false);
                        return true;
                    }
                    break;

                case GravityNotify:
                    componentMovedOrResized (owner, true, true);
                    return true;

                case ClientMessage:
                    if (e.xclient.message_type == messageTypeAtom && e.xclient.format == 32)
                    {
                        handleXembedCmd ((::Time) e.xclient.data.l[0], e.xclient.data.l[1],
                                         e.xclient.data.l[2], e.xclient.data.l[3],
                                         e.xclient.data.l[4]);

                        return true;
                    }
                    break;

                default:
                    break;
            }
        }

        return false;
    }

    void sendXEmbedEvent (const ::Time& xTime, long opcode,
                          long opcodeMinor = 0, long data1 = 0, long data2 = 0)
    {
        XClientMessageEvent msg;
        auto dpy = getDisplay();

        ::memset (&msg, 0, sizeof (XClientMessageEvent));
        msg.window = client;
        msg.type = ClientMessage;
        msg.message_type = messageTypeAtom;
        msg.format = 32;
        msg.data.l[0] = (long) xTime;
        msg.data.l[1] = opcode;
        msg.data.l[2] = opcodeMinor;
        msg.data.l[3] = data1;
        msg.data.l[4] = data2;

        X11Symbols::getInstance()->xSendEvent (dpy, client, False, NoEventMask, (XEvent*) &msg);
        X11Symbols::getInstance()->xSync (dpy, False);
    }

    Rectangle<int> getX11BoundsFromJuce()
    {
        if (auto* peer = owner.getPeer())
        {
            auto r = peer->getComponent().getLocalArea (&owner, owner.getLocalBounds());
            return r * peer->getPlatformScaleFactor() * peer->getComponent().getDesktopScaleFactor();
        }

        return owner.getLocalBounds();
    }

    //==============================================================================
    friend bool juce::juce_handleXEmbedEvent (ComponentPeer*, void*);
    friend unsigned long juce::juce_getCurrentFocusWindow (ComponentPeer*);

    static Array<Pimpl*>& getWidgets()
    {
        static Array<Pimpl*> i;
        return i;
    }

    static bool dispatchX11Event (ComponentPeer* p, const XEvent* eventArg)
    {
        if (eventArg != nullptr)
        {
            auto& e = *eventArg;

            if (auto w = e.xany.window)
                for (auto* widget : getWidgets())
                    if (w == widget->host || w == widget->client)
                        return widget->handleX11Event (e);
        }
        else
        {
            for (auto* widget : getWidgets())
                if (widget->owner.getPeer() == p)
                    widget->peerChanged (nullptr);
        }

        return false;
    }

    static Window getCurrentFocusWindow (ComponentPeer* p)
    {
        if (p != nullptr)
        {
            for (auto* widget : getWidgets())
                if (widget->owner.getPeer() == p && widget->owner.hasKeyboardFocus (false))
                    return widget->client;
        }

        return SharedKeyWindow::getCurrentFocusWindow (p);
    }
};

//==============================================================================
XEmbedComponent::XEmbedComponent (bool wantsKeyboardFocus, bool allowForeignWidgetToResizeComponent)
    : pimpl (new Pimpl (*this, 0, wantsKeyboardFocus, false, allowForeignWidgetToResizeComponent))
{
    setOpaque (true);
}

XEmbedComponent::XEmbedComponent (unsigned long wID, bool wantsKeyboardFocus, bool allowForeignWidgetToResizeComponent)
    : pimpl (new Pimpl (*this, wID, wantsKeyboardFocus, true, allowForeignWidgetToResizeComponent))
{
    setOpaque (true);
}

XEmbedComponent::~XEmbedComponent() {}

void XEmbedComponent::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);
}

void XEmbedComponent::focusGainedWithDirection (FocusChangeType changeType, FocusChangeDirection direction)
{
    pimpl->focusGained (changeType, direction);
}

void XEmbedComponent::focusLost   (FocusChangeType changeType)     { pimpl->focusLost   (changeType); }
void XEmbedComponent::broughtToFront()                             { pimpl->broughtToFront(); }
unsigned long XEmbedComponent::getHostWindowID()                   { return pimpl->getHostWindowID(); }
void XEmbedComponent::removeClient()                               { pimpl->setClient (0, true); }
void XEmbedComponent::updateEmbeddedBounds()                       { pimpl->updateEmbeddedBounds(); }

//==============================================================================
bool juce_handleXEmbedEvent (ComponentPeer* p, void* e)
{
    return XEmbedComponent::Pimpl::dispatchX11Event (p, reinterpret_cast<const XEvent*> (e));
}

unsigned long juce_getCurrentFocusWindow (ComponentPeer* peer)
{
    return (unsigned long) XEmbedComponent::Pimpl::getCurrentFocusWindow (peer);
}

} // namespace juce
