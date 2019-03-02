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

extern int juce_gtkWebkitMain (int argc, const char* argv[]);

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
        for (;;)
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

        String json (JSON::toString (var (obj.get())));

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

    typedef void (*SetHardwareAcclPolicyFunctionPtr) (WebKitSettings*, int);

    int entry()
    {
        CommandReceiver::setBlocking (outChannel,      true);

        gtk_init (nullptr, nullptr);

        WebKitSettings* settings = webkit_settings_new();

        // webkit_settings_set_hardware_acceleration_policy was only added recently to webkit2
        // but is needed when running a WebBrowserComponent in a Parallels VM with 3D acceleration enabled
        auto setHardwarePolicy
            = reinterpret_cast<SetHardwareAcclPolicyFunctionPtr> (dlsym (RTLD_DEFAULT, "webkit_settings_set_hardware_acceleration_policy"));

        if (setHardwarePolicy != nullptr)
            setHardwarePolicy (settings, 2 /*WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER*/);

        GtkWidget *plug;

        plug = gtk_plug_new(0);
        GtkWidget* container;
        container = gtk_scrolled_window_new (nullptr, nullptr);

        GtkWidget* webviewWidget = webkit_web_view_new_with_settings (settings);
        webview = WEBKIT_WEB_VIEW (webviewWidget);


        gtk_container_add (GTK_CONTAINER (container), webviewWidget);
        gtk_container_add (GTK_CONTAINER (plug),      container);

        webkit_web_view_load_uri (webview, "about:blank");

        g_signal_connect (webview, "decide-policy",
                          G_CALLBACK (decidePolicyCallback), this);

        g_signal_connect (webview, "load-changed",
                          G_CALLBACK (loadChangedCallback), this);

        g_signal_connect (webview, "load-failed",
                          G_CALLBACK (loadFailedCallback), this);

        gtk_widget_show_all (plug);
        unsigned long wID = (unsigned long) gtk_plug_get_id (GTK_PLUG (plug));


        ssize_t ret;

        do {
            ret = write (outChannel, &wID, sizeof (wID));
        } while (ret == -1 && errno == EINTR);

        g_unix_fd_add (receiver.getFd(), G_IO_IN, pipeReadyStatic, this);
        receiver.tryNextRead();

        gtk_main();
        return 0;
    }

    void goToURL (const var& params)
    {
        static Identifier urlIdentifier ("url");
        String url (params.getProperty (urlIdentifier, var()).toString());

        webkit_web_view_load_uri (webview, url.toRawUTF8());
    }

    void handleDecisionResponse (const var& params)
    {
        WebKitPolicyDecision* decision
            = (WebKitPolicyDecision*) ((int64) params.getProperty ("decision_id", var (0)));
        bool allow = params.getProperty ("allow", var (false));

        if (decision != nullptr && decisions.contains (decision))
        {
            if (allow)
                webkit_policy_decision_use (decision);
            else
                webkit_policy_decision_ignore (decision);

            decisions.removeAllInstancesOf (decision);
            g_object_unref (decision);
        }
    }

    //==============================================================================
    void handleCommand (const String& cmd, const var& params) override
    {
        if      (cmd == "quit")      quit();
        else if (cmd == "goToURL")   goToURL (params);
        else if (cmd == "goBack")    webkit_web_view_go_back      (webview);
        else if (cmd == "goForward") webkit_web_view_go_forward   (webview);
        else if (cmd == "refresh")   webkit_web_view_reload       (webview);
        else if (cmd == "stop")      webkit_web_view_stop_loading (webview);
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
        gtk_main_quit();
    }

    bool onNavigation (String frameName,
                       WebKitNavigationAction* action,
                       WebKitPolicyDecision* decision)
    {
        if (decision != nullptr && frameName.isEmpty())
        {
            g_object_ref (decision);
            decisions.add (decision);

            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", String (webkit_uri_request_get_uri (webkit_navigation_action_get_request (action))));
            params->setProperty ("decision_id", (int64) decision);
            CommandReceiver::sendCommand (outChannel, "pageAboutToLoad", var (params.get()));

            return true;
        }

        return false;
    }

    bool onNewWindow (String /*frameName*/,
                      WebKitNavigationAction* action,
                      WebKitPolicyDecision* decision)
    {
        if (decision != nullptr)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", String (webkit_uri_request_get_uri (webkit_navigation_action_get_request (action))));
            CommandReceiver::sendCommand (outChannel, "newWindowAttemptingToLoad", var (params.get()));

            // never allow new windows
            webkit_policy_decision_ignore (decision);

            return true;
        }

        return false;
    }

    void onLoadChanged (WebKitLoadEvent loadEvent)
    {
        if (loadEvent == WEBKIT_LOAD_FINISHED)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", String (webkit_web_view_get_uri (webview)));
            CommandReceiver::sendCommand (outChannel, "pageFinishedLoading", var (params.get()));
        }
    }

    bool onDecidePolicy (WebKitPolicyDecision*    decision,
                         WebKitPolicyDecisionType decisionType)
    {
        switch (decisionType)
        {
        case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
            {
                WebKitNavigationPolicyDecision* navigationDecision = WEBKIT_NAVIGATION_POLICY_DECISION (decision);
                const char* frameName = webkit_navigation_policy_decision_get_frame_name (navigationDecision);

                return onNavigation (String (frameName != nullptr ? frameName : ""),
                                     webkit_navigation_policy_decision_get_navigation_action (navigationDecision),
                                     decision);
            }
            break;
        case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
            {
                WebKitNavigationPolicyDecision* navigationDecision = WEBKIT_NAVIGATION_POLICY_DECISION (decision);
                const char* frameName = webkit_navigation_policy_decision_get_frame_name (navigationDecision);

                return onNewWindow  (String (frameName != nullptr ? frameName : ""),
                                     webkit_navigation_policy_decision_get_navigation_action (navigationDecision),
                                     decision);
            }
            break;
        case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
            {
                WebKitResponsePolicyDecision *response = WEBKIT_RESPONSE_POLICY_DECISION (decision);

                // for now just always allow response requests
                ignoreUnused (response);
                webkit_policy_decision_use (decision);
                return true;
            }
            break;
        default:
            break;
        }

        return false;
    }

    void onLoadFailed (GError* error)
    {
        DynamicObject::Ptr params = new DynamicObject;

        params->setProperty ("error", String (error != nullptr ? error->message : "unknown error"));
        CommandReceiver::sendCommand (outChannel, "pageLoadHadNetworkError", var (params.get()));
    }

private:
    static gboolean pipeReadyStatic (gint fd, GIOCondition condition, gpointer user)
    {
        return (reinterpret_cast<GtkChildProcess*> (user)->pipeReady (fd, condition) ? TRUE : FALSE);
    }

    static gboolean decidePolicyCallback (WebKitWebView*,
                                          WebKitPolicyDecision*    decision,
                                          WebKitPolicyDecisionType decisionType,
                                          gpointer user)
    {
        GtkChildProcess& owner = *reinterpret_cast<GtkChildProcess*> (user);
        return (owner.onDecidePolicy (decision, decisionType) ? TRUE : FALSE);
    }

    static void loadChangedCallback (WebKitWebView*,
                                     WebKitLoadEvent loadEvent,
                                     gpointer        user)
    {
        GtkChildProcess& owner = *reinterpret_cast<GtkChildProcess*> (user);
        owner.onLoadChanged (loadEvent);
    }

    static void loadFailedCallback (WebKitWebView*,
                                    WebKitLoadEvent /*loadEvent*/,
                                    gchar*          /*failing_uri*/,
                                    GError*         error,
                                    gpointer        user)
    {
        GtkChildProcess& owner = *reinterpret_cast<GtkChildProcess*> (user);
        owner.onLoadFailed (error);
    }

    int outChannel;
    CommandReceiver receiver;
    WebKitWebView* webview = nullptr;
    Array<WebKitPolicyDecision*> decisions;
};

//==============================================================================
class WebBrowserComponent::Pimpl  : private Thread,
                                    private CommandReceiver::Responder
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

        receiver.reset (new CommandReceiver (this, inChannel));
        startThread();

        xembed.reset (new XEmbedComponent (windowHandle));
        owner.addAndMakeVisible (xembed.get());
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

        CommandReceiver::sendCommand (outChannel, "goToURL", var (params.get()));
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

            int status = 0, result;

            result = waitpid (childProcess, &status, WNOHANG);
            for (int i = 0; i < 15 && (! WIFEXITED(status) || result != childProcess); ++i)
            {
                Thread::sleep (100);
                result = waitpid (childProcess, &status, WNOHANG);
            }

            // clean-up any zombies
            status = 0;
            if (! WIFEXITED(status) || result != childProcess)
            {
                do
                {
                    kill (childProcess, SIGTERM);
                    waitpid (childProcess, &status, 0);
                } while (! WIFEXITED(status));
            }

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

            HeapBlock<const char*> argv (5);
            StringArray arguments;

            arguments.add (File::getSpecialLocation (File::currentExecutableFile).getFullPathName());
            arguments.add ("--juce-gtkwebkitfork-child");
            arguments.add (String (outPipe[0]));
            arguments.add (String (inPipe [1]));

            for (int i = 0; i < arguments.size(); ++i)
                argv[i] = arguments[i].toRawUTF8();

            argv[4] = nullptr;

           #if JUCE_STANDALONE_APPLICATION
            execv (arguments[0].toRawUTF8(), (char**) argv.getData());
           #else
            juce_gtkWebkitMain (4, (const char**) argv.getData());
           #endif
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
        else if (cmd == "pageLoadHadNetworkError")   handlePageLoadHadNetworkError (params);

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

            CommandReceiver::sendCommand (outChannel, "decision", var (params.get()));
        }
    }

    void handlePageLoadHadNetworkError (const var& params)
    {
        String error = params.getProperty ("error", "Unknown error");

        if (owner.pageLoadHadNetworkError (error))
            goToURL (String ("data:text/plain,") + error, nullptr, nullptr);
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
    std::unique_ptr<CommandReceiver> receiver;
    int childProcess = 0, inChannel = 0, outChannel = 0;
    int threadControl[2];
    std::unique_ptr<XEmbedComponent> xembed;
    WaitableEvent threadBlocker;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadPageWhenBrowserIsHidden_)
    : browser (new Pimpl (*this)),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    setOpaque (true);

    browser->init();
}

WebBrowserComponent::~WebBrowserComponent()
{
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

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    browser->stop();
}

void WebBrowserComponent::goBack()
{
    lastURL.clear();

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

void WebBrowserComponent::clearCookies()
{
    // Currently not implemented on linux as WebBrowserComponent currently does not
    // store cookies on linux
    jassertfalse;
}

int juce_gtkWebkitMain (int argc, const char* argv[])
{
    if (argc != 4) return -1;


    GtkChildProcess child (String (argv[2]).getIntValue(),
                           String (argv[3]).getIntValue());
    return child.entry();
}

} // namespace juce
