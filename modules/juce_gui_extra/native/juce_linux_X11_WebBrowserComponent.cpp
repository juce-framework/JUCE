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
class CommandReceiver
{
public:
    struct Responder
    {
        virtual ~Responder() {}

        virtual void handleCommand (const String& cmd, const var& param) = 0;
        virtual void receiverHadError() = 0;
    };

    CommandReceiver (Responder* responderToUse, int inputChannelToUse)
        : responder (responderToUse), inChannel (inputChannelToUse)
    {
        setBlocking (inChannel, false);
    }

    static void setBlocking (int fd, bool shouldBlock)
    {
        int flags = fcntl (fd, F_GETFL);
        fcntl (fd, F_SETFL, (shouldBlock ? (flags & ~O_NONBLOCK)
                             : (flags | O_NONBLOCK)));
    }

    int getFd() const     { return inChannel; }

    void tryNextRead()
    {
        while (true)
        {
            size_t len = (receivingLength ? sizeof (size_t) : bufferLength.len);

            if (! receivingLength)
                buffer.realloc (len);

            char* dst  = (receivingLength ? bufferLength.data : buffer.getData());

            ssize_t actual = read (inChannel, &dst[pos], static_cast<size_t> (len - pos));

            if (actual < 0)
            {
                if (errno == EINTR)
                    continue;

                break;
            }

            pos += static_cast<size_t> (actual);

            if (pos == len)
            {
                pos = 0;

                if (! receivingLength)
                    parseJSON (String (buffer.getData(), bufferLength.len));

                receivingLength = (! receivingLength);
            }
        }

        if (errno != EAGAIN && errno != EWOULDBLOCK && responder != nullptr)
            responder->receiverHadError();
    }

    static void sendCommand (int outChannel, const String& cmd, const var& params)
    {
        DynamicObject::Ptr obj = new DynamicObject;

        obj->setProperty (getCmdIdentifier(), cmd);

        if (! params.isVoid())
            obj->setProperty (getParamIdentifier(), params);

        String json (JSON::toString (var (obj)));

        size_t jsonLength = static_cast<size_t> (json.length());
        size_t len        = sizeof (size_t) + jsonLength;

        HeapBlock<char> buffer (len);
        char* dst = buffer.getData();

        memcpy (dst, &jsonLength, sizeof (size_t));
        dst += sizeof (size_t);

        memcpy (dst, json.toRawUTF8(), jsonLength);

        ssize_t ret;

        do
        {
            ret = write (outChannel, buffer.getData(), len);
        } while (ret == -1 && errno == EINTR);
    }

private:
    void parseJSON (const String& json)
    {
        var object (JSON::fromString (json));

        if (! object.isVoid())
        {
            String cmd (object.getProperty (getCmdIdentifier(),   var()).toString());
            var params (object.getProperty (getParamIdentifier(), var()));

            if (responder != nullptr)
                responder->handleCommand (cmd, params);
        }
    }

    static Identifier getCmdIdentifier()    { static Identifier Id ("cmd");    return Id; }
    static Identifier getParamIdentifier()  { static Identifier Id ("params"); return Id; }

    Responder* responder;
    int inChannel;
    size_t pos = 0;
    bool receivingLength = true;
    union { char data [sizeof (size_t)]; size_t len; } bufferLength;
    HeapBlock<char> buffer;
};

//==============================================================================
class GtkChildProcess : private CommandReceiver::Responder
{
public:
    //==============================================================================
    GtkChildProcess (int inChannel, int outChannelToUse)
        : outChannel (outChannelToUse), receiver (this, inChannel)
    {}

    void entry()
    {
        gtk_init (nullptr, nullptr);
        GtkWidget *plug;

        plug = gtk_plug_new(0);
        GtkWidget* container;
        container = gtk_scrolled_window_new (nullptr, nullptr);

        webview = webkit_web_view_new();
        gtk_container_add (GTK_CONTAINER (container), webview);
        gtk_container_add (GTK_CONTAINER (plug),      container);

        webkit_web_view_load_uri (WEBKIT_WEB_VIEW (webview), "about:blank");

        g_signal_connect (WEBKIT_WEB_VIEW (webview), "navigation-policy-decision-requested",
                          G_CALLBACK (navigationPolicyDecisionCallback), this);

        g_signal_connect (WEBKIT_WEB_VIEW (webview), "new-window-policy-decision-requested",
                          G_CALLBACK (newWindowPolicyDecisionCallback), this);

        g_signal_connect (WEBKIT_WEB_VIEW (webview), "document-load-finished",
                          G_CALLBACK (documentLoadFinishedCallback), this);

        gtk_widget_show_all (plug);
        unsigned long wID = (unsigned long) gtk_plug_get_id (GTK_PLUG (plug));

        CommandReceiver::setBlocking (outChannel,      true);

        ssize_t ret;

        do {
            ret = write (outChannel, &wID, sizeof (wID));
        } while (ret == -1 && errno == EINTR);

        g_unix_fd_add (receiver.getFd(), G_IO_IN, pipeReadyStatic, this);
        receiver.tryNextRead();

        gtk_main();
    }

    void goToURL (const var& params)
    {
        static Identifier urlIdentifier ("url");
        String url (params.getProperty (urlIdentifier, var()).toString());

        webkit_web_view_load_uri (WEBKIT_WEB_VIEW (webview), url.toRawUTF8());
    }

    void handleDecisionResponse (const var& params)
    {
        WebKitWebPolicyDecision* decision
            = (WebKitWebPolicyDecision*) ((int64) params.getProperty ("decision_id", var (0)));
        bool allow = params.getProperty ("allow", var (false));

        if (decision != nullptr && decisions.contains (decision))
        {
            if (allow)
                webkit_web_policy_decision_use (decision);
            else
                webkit_web_policy_decision_ignore (decision);

            decisions.removeAllInstancesOf (decision);
            g_object_unref (decision);
        }
    }

    //==============================================================================
    void handleCommand (const String& cmd, const var& params) override
    {
        if      (cmd == "quit")      quit();
        else if (cmd == "goToURL")   goToURL (params);
        else if (cmd == "goBack")    webkit_web_view_go_back      (WEBKIT_WEB_VIEW (webview));
        else if (cmd == "goForward") webkit_web_view_go_forward   (WEBKIT_WEB_VIEW (webview));
        else if (cmd == "refresh")   webkit_web_view_reload       (WEBKIT_WEB_VIEW (webview));
        else if (cmd == "stop")      webkit_web_view_stop_loading (WEBKIT_WEB_VIEW (webview));
        else if (cmd == "decision")  handleDecisionResponse (params);
    }

    void receiverHadError() override
    {
        exit (-1);
    }

    //==============================================================================
    bool pipeReady (gint fd, GIOCondition)
    {
        if (fd == receiver.getFd())
        {
            receiver.tryNextRead();
            return true;
        }

        return false;
    }

    void quit()
    {
        exit (-1);
    }

    bool onNavigation (WebKitWebFrame* webFrame,
                       WebKitNetworkRequest* /*request*/,
                       WebKitWebNavigationAction* action,
                       WebKitWebPolicyDecision* decision)
    {
        if (decision != nullptr && webkit_web_frame_find_frame (webFrame, "_top") == webFrame)
        {
            g_object_ref (decision);
            decisions.add (decision);

            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", String (webkit_web_navigation_action_get_original_uri (action)));
            params->setProperty ("decision_id", (int64) decision);
            CommandReceiver::sendCommand (outChannel, "pageAboutToLoad", var (params));

            return true;
        }

        return false;
    }

    bool onNewWindow (WebKitWebFrame* /*webFrame*/,
                      WebKitNetworkRequest* /*request*/,
                      WebKitWebNavigationAction* action,
                      WebKitWebPolicyDecision* decision)
    {
        if (decision != nullptr)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", String (webkit_web_navigation_action_get_original_uri (action)));
            CommandReceiver::sendCommand (outChannel, "newWindowAttemptingToLoad", var (params));

            // never allow new windows
            webkit_web_policy_decision_ignore (decision);

            return true;
        }

        return false;
    }

    void onLoadFinished (WebKitWebFrame* webFrame)
    {
        if (webkit_web_frame_find_frame (webFrame, "_top") == webFrame)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", String (webkit_web_frame_get_uri (webFrame)));
            CommandReceiver::sendCommand (outChannel, "pageFinishedLoading", var (params));
        }
    }

private:
    static gboolean pipeReadyStatic (gint fd, GIOCondition condition, gpointer user)
    {
        return (reinterpret_cast<GtkChildProcess*> (user)->pipeReady (fd, condition) ? TRUE : FALSE);
    }

    static gboolean navigationPolicyDecisionCallback (WebKitWebView*,
                                                      WebKitWebFrame* webFrame,
                                                      WebKitNetworkRequest* request,
                                                      WebKitWebNavigationAction* action,
                                                      WebKitWebPolicyDecision* decision,
                                                      gpointer user)
    {
        GtkChildProcess& owner = *reinterpret_cast<GtkChildProcess*> (user);
        return (owner.onNavigation (webFrame, request, action, decision) ? TRUE : FALSE);
    }

    static gboolean newWindowPolicyDecisionCallback (WebKitWebView*,
                                                     WebKitWebFrame* webFrame,
                                                     WebKitNetworkRequest* request,
                                                     WebKitWebNavigationAction* action,
                                                     WebKitWebPolicyDecision* decision,
                                                     gpointer user)
    {
        GtkChildProcess& owner = *reinterpret_cast<GtkChildProcess*> (user);
        return (owner.onNewWindow (webFrame, request, action, decision) ? TRUE : FALSE);
    }

    static void documentLoadFinishedCallback (WebKitWebView*,
                                              WebKitWebFrame* webFrame,
                                              gpointer        user)
    {
        GtkChildProcess& owner = *reinterpret_cast<GtkChildProcess*> (user);
        owner.onLoadFinished (webFrame);
    }

    int outChannel;
    CommandReceiver receiver;
    GtkWidget* webview = nullptr;
    Array<WebKitWebPolicyDecision*> decisions;
};

//==============================================================================
class WebBrowserComponent::Pimpl : private Thread, private CommandReceiver::Responder
{
public:
    Pimpl (WebBrowserComponent& parent)
        : Thread ("Webview"), owner (parent)
    {}

    ~Pimpl()
    {
        quit();
    }

    //==============================================================================
    void init()
    {
        launchChild();

        int ret = pipe (threadControl);

        ignoreUnused (ret);
        jassert (ret == 0);

        CommandReceiver::setBlocking (inChannel,        true);
        CommandReceiver::setBlocking (outChannel,       true);
        CommandReceiver::setBlocking (threadControl[0], false);
        CommandReceiver::setBlocking (threadControl[1], true);

        unsigned long windowHandle;
        ssize_t actual = read (inChannel, &windowHandle, sizeof (windowHandle));
        if (actual != sizeof (windowHandle))
        {
            killChild();
            return;
        }

        receiver = new CommandReceiver (this, inChannel);
        startThread();

        xembed = new XEmbedComponent (windowHandle);
        owner.addAndMakeVisible (xembed);
    }

    void quit()
    {
        if (isThreadRunning())
        {
            signalThreadShouldExit();

            char ignore = 0;
            ssize_t ret;

            do
            {
                ret = write (threadControl[1], &ignore, 1);
            } while (ret == -1 && errno == EINTR);

            waitForThreadToExit (-1);
            receiver = nullptr;
        }

        if (childProcess != 0)
        {
            CommandReceiver::sendCommand (outChannel, "quit", var());
            killChild();
        }
    }

    //==============================================================================
    void goToURL (const String& url, const StringArray* headers, const MemoryBlock* postData)
    {
        DynamicObject::Ptr params = new DynamicObject;

        params->setProperty ("url", url);

        if (headers != nullptr)
            params->setProperty ("headers", var (*headers));

        if (postData != nullptr)
            params->setProperty ("postData", var (*postData));

        CommandReceiver::sendCommand (outChannel, "goToURL", var (params));
    }

    void goBack()      { CommandReceiver::sendCommand (outChannel, "goBack",    var()); }
    void goForward()   { CommandReceiver::sendCommand (outChannel, "goForward", var()); }
    void refresh()     { CommandReceiver::sendCommand (outChannel, "refresh",   var()); }
    void stop()        { CommandReceiver::sendCommand (outChannel, "stop",      var()); }

    void resized()
    {
        if (xembed != nullptr)
            xembed->setBounds (owner.getLocalBounds());
    }
private:
    //==============================================================================
    void killChild()
    {
        if (childProcess != 0)
        {
            xembed = nullptr;

            kill (childProcess, SIGTERM);

            int status = 0;

            while (! WIFEXITED(status))
                waitpid (childProcess, &status, 0);

            childProcess = 0;
        }
    }

    void launchChild()
    {
        int ret;
        int inPipe[2], outPipe[2];

        ret = pipe (inPipe);
        ignoreUnused (ret); jassert (ret == 0);

        ret = pipe (outPipe);
        ignoreUnused (ret); jassert (ret == 0);

        int pid = fork();
        if (pid == 0)
        {
            close (inPipe[0]);
            close (outPipe[1]);

            GtkChildProcess child (outPipe[0], inPipe[1]);
            child.entry();
            exit (0);
        }

        close (inPipe[1]);
        close (outPipe[0]);

        inChannel  = inPipe[0];
        outChannel = outPipe[1];

        childProcess = pid;
    }

    void run() override
    {
        while (! threadShouldExit())
        {
            if (shouldExit())
                return;

            receiver->tryNextRead();

            fd_set set;
            FD_ZERO (&set);
            FD_SET (threadControl[0],  &set);
            FD_SET (receiver->getFd(), &set);

            int max_fd = jmax (threadControl[0], receiver->getFd());

            int result = 0;

            while (result == 0 || (result < 0 && errno == EINTR))
                result = select (max_fd + 1, &set, NULL, NULL, NULL);

            if (result < 0)
                break;
        }
    }

    bool shouldExit()
    {
        char ignore;
        ssize_t result = read (threadControl[0], &ignore, 1);

        return (result != -1 || (errno != EAGAIN && errno != EWOULDBLOCK));
    }

    //==============================================================================
    void handleCommandOnMessageThread (const String& cmd, const var& params)
    {
        String url (params.getProperty ("url", var()).toString());

        if      (cmd == "pageAboutToLoad")           handlePageAboutToLoad (url, params);
        else if (cmd == "pageFinishedLoading")       owner.pageFinishedLoading (url);
        else if (cmd == "windowCloseRequest")        owner.windowCloseRequest();
        else if (cmd == "newWindowAttemptingToLoad") owner.newWindowAttemptingToLoad (url);

        threadBlocker.signal();
    }

    void handlePageAboutToLoad (const String& url, const var& inputParams)
    {
        int64 decision_id = inputParams.getProperty ("decision_id", var (0));

        if (decision_id != 0)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("decision_id", decision_id);
            params->setProperty ("allow", owner.pageAboutToLoad (url));

            CommandReceiver::sendCommand (outChannel, "decision", var (params));
        }
    }

    void handleCommand (const String& cmd, const var& params) override
    {
        threadBlocker.reset();

        (new HandleOnMessageThread (this, cmd, params))->post();

        // wait until the command has executed on the message thread
        // this ensures that Pimpl can never be deleted while the
        // message has not been executed yet
        threadBlocker.wait (-1);
    }

    void receiverHadError() override {}

    //==============================================================================
    struct HandleOnMessageThread : public CallbackMessage
    {
        HandleOnMessageThread (Pimpl* pimpl, const String& cmdToUse, const var& params)
            : owner (pimpl), cmdToSend (cmdToUse), paramsToSend (params)
        {}

        void messageCallback() override
        {
            owner->handleCommandOnMessageThread (cmdToSend, paramsToSend);
        }

        Pimpl* owner;
        String cmdToSend;
        var paramsToSend;
    };

private:
    WebBrowserComponent& owner;
    ScopedPointer<CommandReceiver> receiver;
    int childProcess = 0, inChannel = 0, outChannel = 0;
    int threadControl[2];
    ScopedPointer<XEmbedComponent> xembed;
    WaitableEvent threadBlocker;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadPageWhenBrowserIsHidden_)
    : browser (new Pimpl (*this)),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    setOpaque (true);

    browser->init();
}

WebBrowserComponent::~WebBrowserComponent()
{
    if (browser != nullptr)
    {
        delete browser;
        browser = nullptr;
    }
}

//==============================================================================
void WebBrowserComponent::goToURL (const String& url,
                                   const StringArray* headers,
                                   const MemoryBlock* postData)
{
    lastURL = url;

    if (headers != nullptr)
        lastHeaders = *headers;
    else
        lastHeaders.clear();

    if (postData != nullptr)
        lastPostData = *postData;
    else
        lastPostData.reset();

    blankPageShown = false;

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    browser->stop();
}

void WebBrowserComponent::goBack()
{
    lastURL.clear();
    blankPageShown = false;

    browser->goBack();
}

void WebBrowserComponent::goForward()
{
    lastURL.clear();
    browser->goForward();
}

void WebBrowserComponent::refresh()
{
    browser->refresh();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void WebBrowserComponent::checkWindowAssociation()
{
}

void WebBrowserComponent::reloadLastURL()
{
    if (lastURL.isNotEmpty())
    {
        goToURL (lastURL, &lastHeaders, &lastPostData);
        lastURL.clear();
    }
}

void WebBrowserComponent::parentHierarchyChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::resized()
{
    if (browser != nullptr)
        browser->resized();
}

void WebBrowserComponent::visibilityChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::focusGained (FocusChangeType)
{
}
