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

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor,         "<init>",              "(Landroid/content/Context;)V") \
  METHOD (getSettings,         "getSettings",         "()Landroid/webkit/WebSettings;") \
  METHOD (goBack,              "goBack",              "()V") \
  METHOD (goForward,           "goForward",           "()V") \
  METHOD (loadDataWithBaseURL, "loadDataWithBaseURL", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V") \
  METHOD (loadUrl,             "loadUrl",             "(Ljava/lang/String;Ljava/util/Map;)V") \
  METHOD (postUrl,             "postUrl",             "(Ljava/lang/String;[B)V") \
  METHOD (reload,              "reload",              "()V") \
  METHOD (setWebChromeClient,  "setWebChromeClient",  "(Landroid/webkit/WebChromeClient;)V") \
  METHOD (setWebViewClient,    "setWebViewClient",    "(Landroid/webkit/WebViewClient;)V") \
  METHOD (stopLoading,         "stopLoading",         "()V")

DECLARE_JNI_CLASS (AndroidWebView, "android/webkit/WebView")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>", "()V")

DECLARE_JNI_CLASS (AndroidWebChromeClient, "android/webkit/WebChromeClient");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>", "()V")

DECLARE_JNI_CLASS (AndroidWebViewClient, "android/webkit/WebViewClient");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (getInstance, "getInstance", "()Landroid/webkit/CookieManager;")

DECLARE_JNI_CLASS (AndroidCookieManager, "android/webkit/CookieManager");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>",      "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";J)V")

DECLARE_JNI_CLASS (JuceWebChromeClient, JUCE_ANDROID_ACTIVITY_CLASSPATH "$JuceWebChromeClient");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>",      "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";J)V") \
  METHOD (hostDeleted, "hostDeleted", "()V")

DECLARE_JNI_CLASS (JuceWebViewClient, JUCE_ANDROID_ACTIVITY_CLASSPATH "$JuceWebViewClient");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (setBuiltInZoomControls,    "setBuiltInZoomControls",    "(Z)V") \
  METHOD (setDisplayZoomControls,    "setDisplayZoomControls",    "(Z)V") \
  METHOD (setJavaScriptEnabled,      "setJavaScriptEnabled",      "(Z)V") \
  METHOD (setSupportMultipleWindows, "setSupportMultipleWindows", "(Z)V")

DECLARE_JNI_CLASS (WebSettings, "android/webkit/WebSettings");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (toString, "toString", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (SslError, "android/net/http/SslError")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (encode, "encode", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;")

DECLARE_JNI_CLASS (URLEncoder, "java/net/URLEncoder")
#undef JNI_CLASS_MEMBERS

//==============================================================================
class WebBrowserComponent::Pimpl    : public AndroidViewComponent,
                                      public AsyncUpdater
{
public:
    Pimpl (WebBrowserComponent& o)
        : AndroidViewComponent (true),
          owner (o)
    {
        auto* env = getEnv();

        setView (env->NewObject (AndroidWebView, AndroidWebView.constructor, android.activity.get()));

        auto settings = LocalRef<jobject> (env->CallObjectMethod ((jobject) getView(), AndroidWebView.getSettings));
        env->CallVoidMethod (settings, WebSettings.setJavaScriptEnabled, true);
        env->CallVoidMethod (settings, WebSettings.setBuiltInZoomControls, true);
        env->CallVoidMethod (settings, WebSettings.setDisplayZoomControls, false);
        env->CallVoidMethod (settings, WebSettings.setSupportMultipleWindows, true);

        juceWebChromeClient = GlobalRef (LocalRef<jobject> (env->NewObject (JuceWebChromeClient, JuceWebChromeClient.constructor,
                                                                            android.activity.get(),
                                                                            reinterpret_cast<jlong>(&owner))));
        env->CallVoidMethod ((jobject) getView(), AndroidWebView.setWebChromeClient, juceWebChromeClient.get());

        juceWebViewClient = GlobalRef (LocalRef<jobject> (env->NewObject (JuceWebViewClient, JuceWebViewClient.constructor,
                                                                          android.activity.get(),
                                                                          reinterpret_cast<jlong>(&owner))));
        env->CallVoidMethod ((jobject) getView(), AndroidWebView.setWebViewClient, juceWebViewClient.get());
    }

    ~Pimpl()
    {
        auto* env = getEnv();

        env->CallVoidMethod ((jobject) getView(), AndroidWebView.stopLoading);

        auto defaultChromeClient = LocalRef<jobject> (env->NewObject (AndroidWebChromeClient, AndroidWebChromeClient.constructor));
        auto defaultViewClient   = LocalRef<jobject> (env->NewObject (AndroidWebViewClient,   AndroidWebViewClient  .constructor));

        env->CallVoidMethod ((jobject) getView(), AndroidWebView.setWebChromeClient, defaultChromeClient.get());
        env->CallVoidMethod ((jobject) getView(), AndroidWebView.setWebViewClient,   defaultViewClient  .get());

        masterReference.clear();

        // if other Java thread is waiting for us to respond to page load request
        // wake it up immediately (false answer will be sent), so that it releases
        // the lock we need when calling hostDeleted.
        responseReadyEvent.signal();

        env->CallVoidMethod (juceWebViewClient, JuceWebViewClient.hostDeleted);
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        auto* env = getEnv();

        if (headers == nullptr && postData == nullptr)
        {
            env->CallVoidMethod ((jobject) getView(), AndroidWebView.loadUrl, javaString (url).get(), 0);
        }
        else if (headers != nullptr && postData == nullptr)
        {
            auto headersMap = LocalRef<jobject> (env->NewObject (JavaHashMap,
                                                                 JavaHashMap.constructorWithCapacity,
                                                                 headers->size()));

            for (const auto& header : *headers)
            {
                auto name  = header.upToFirstOccurrenceOf (":", false, false).trim();
                auto value = header.fromFirstOccurrenceOf (":", false, false).trim();

                env->CallObjectMethod (headersMap, JavaMap.put,
                                       javaString (name).get(),
                                       javaString (value).get());
            }

            env->CallVoidMethod ((jobject) getView(), AndroidWebView.loadUrl,
                                 javaString (url).get(), headersMap.get());
        }
        else if (headers == nullptr && postData != nullptr)
        {
            auto dataStringJuce = postData->toString();
            auto dataStringJava = javaString (dataStringJuce);
            auto encodingString = LocalRef<jobject> (env->CallStaticObjectMethod (URLEncoder, URLEncoder.encode,
                                                                                  dataStringJava.get(), javaString ("utf-8").get()));

            auto bytes = LocalRef<jbyteArray> ((jbyteArray) env->CallObjectMethod (encodingString, JavaString.getBytes));

            env->CallVoidMethod ((jobject) getView(), AndroidWebView.postUrl,
                                 javaString (url).get(), bytes.get());
        }
        else if (headers != nullptr && postData != nullptr)
        {
            // There is no support for both extra headers and post data in Android WebView, so
            // we need to open URL manually.

            URL urlToUse = URL (url).withPOSTData (*postData);
            connectionThread.reset (new ConnectionThread (*this, urlToUse, *headers));
        }
    }

    void stop()
    {
        connectionThread = nullptr;

        getEnv()->CallVoidMethod ((jobject) getView(), AndroidWebView.stopLoading);
    }

    void goBack()
    {
        connectionThread = nullptr;

        getEnv()->CallVoidMethod ((jobject) getView(), AndroidWebView.goBack);
    }

    void goForward()
    {
        connectionThread = nullptr;

        getEnv()->CallVoidMethod ((jobject) getView(), AndroidWebView.goForward);
    }

    void refresh()
    {
        connectionThread = nullptr;

        getEnv()->CallVoidMethod ((jobject) getView(), AndroidWebView.reload);
    }

    void handleAsyncUpdate()
    {
        jassert (connectionThread != nullptr);

        if (connectionThread == nullptr)
            return;

        auto& result = connectionThread->getResult();

        if (result.statusCode >= 200 && result.statusCode < 300)
        {
            auto url = javaString (result.url);
            auto data = javaString (result.data);
            auto mimeType = javaString ("text/html");
            auto encoding = javaString ("utf-8");

            getEnv()->CallVoidMethod ((jobject) getView(), AndroidWebView.loadDataWithBaseURL,
                                      url.get(), data.get(), mimeType.get(),
                                      encoding.get(), 0);
        }
        else
        {
            owner.pageLoadHadNetworkError (result.description);
        }
    }

    bool handlePageAboutToLoad (const String& url)
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
            return owner.pageAboutToLoad (url);

        WeakReference<Pimpl> weakRef (this);

        if (weakRef == nullptr)
            return false;

        responseReadyEvent.reset();

        bool shouldLoad = false;

        MessageManager::callAsync ([weakRef, url, &shouldLoad]
        {
            if (weakRef == nullptr)
                return;

            shouldLoad = weakRef->owner.pageAboutToLoad (url);

            weakRef->responseReadyEvent.signal();
        });

        responseReadyEvent.wait (-1);

        return shouldLoad;
    }

private:
    class ConnectionThread  : private Thread
    {
    public:
        struct Result
        {
            String url;
            int statusCode = 0;
            String description;
            String data;
        };

        ConnectionThread (Pimpl& ownerToUse,
                          URL& url,
                          const StringArray& headers)
            : Thread ("WebBrowserComponent::Pimpl::ConnectionThread"),
              owner (ownerToUse),
              webInputStream (new WebInputStream (url, true))
        {
            webInputStream->withExtraHeaders (headers.joinIntoString ("\n"));
            webInputStream->withConnectionTimeout (10000);

            result.url = url.toString (true);

            startThread();
        }

        ~ConnectionThread()
        {
            webInputStream->cancel();
            signalThreadShouldExit();
            waitForThreadToExit (10000);

            webInputStream = nullptr;
        }

        void run() override
        {
            if (! webInputStream->connect (nullptr))
            {
                result.description = "Could not establish connection";
                owner.triggerAsyncUpdate();
                return;
            }

            result.statusCode = webInputStream->getStatusCode();
            result.description = "Status code: " + String (result.statusCode);
            readFromInputStream();
            owner.triggerAsyncUpdate();
        }

        const Result& getResult() { return result; }

    private:
        void readFromInputStream()
        {
            MemoryOutputStream ostream;

            while (true)
            {
                if (threadShouldExit())
                    return;

                char buffer [8192];
                const int num = webInputStream->read (buffer, sizeof (buffer));

                if (num <= 0)
                    break;

                ostream.write (buffer, (size_t) num);
            }

            result.data = ostream.toUTF8();
        }

        Pimpl& owner;
        std::unique_ptr<WebInputStream> webInputStream;
        Result result;
    };


    WebBrowserComponent& owner;
    GlobalRef juceWebChromeClient;
    GlobalRef juceWebViewClient;
    std::unique_ptr<ConnectionThread> connectionThread;
    WaitableEvent responseReadyEvent;

    WeakReference<Pimpl>::Master masterReference;
    friend class WeakReference<Pimpl>;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadWhenHidden)
    : blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadWhenHidden)
{
    setOpaque (true);

    browser.reset (new Pimpl (*this));
    addAndMakeVisible (browser.get());
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
    if (isShowing())
    {
        if (blankPageShown)
            goBack();
    }
    else
    {
        if (unloadPageWhenBrowserIsHidden && ! blankPageShown)
        {
            // when the component becomes invisible, some stuff like flash
            // carries on playing audio, so we need to force it onto a blank
            // page to avoid this, (and send it back when it's made visible again).

            blankPageShown = true;
            browser->goToURL ("about:blank", 0, 0);
        }
    }
}

void WebBrowserComponent::reloadLastURL()
{
    if (lastURL.isNotEmpty())
    {
        goToURL (lastURL, &lastHeaders, lastPostData.getSize() == 0 ? nullptr : &lastPostData);
        lastURL.clear();
    }
}

void WebBrowserComponent::parentHierarchyChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::resized()
{
    browser->setSize (getWidth(), getHeight());
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
    auto* env = getEnv();

    auto cookieManager = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidCookieManager,
                                                                         AndroidCookieManager.getInstance));

    const bool apiAtLeast21 = env->CallStaticIntMethod (JuceAppActivity, JuceAppActivity.getAndroidSDKVersion) >= 21;

    jmethodID clearCookiesMethod = 0;

    if (apiAtLeast21)
    {
        clearCookiesMethod = env->GetMethodID (AndroidCookieManager, "removeAllCookies", "(Landroid/webkit/ValueCallback;)V");
        env->CallVoidMethod (cookieManager, clearCookiesMethod, 0);
    }
    else
    {
        clearCookiesMethod = env->GetMethodID (AndroidCookieManager, "removeAllCookie", "()V");
        env->CallVoidMethod (cookieManager, clearCookiesMethod);
    }
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, webViewPageLoadStarted, bool, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*webView*/, jobject url))
{
    setEnv (env);

    return juce_webViewPageLoadStarted (reinterpret_cast<WebBrowserComponent*> (host),
                                        juceString (static_cast<jstring> (url)));
}

bool juce_webViewPageLoadStarted (WebBrowserComponent* browserComponent, const String& url)
{
    return browserComponent->browser->handlePageAboutToLoad (url);
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, webViewPageLoadFinished, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*webView*/, jobject url))
{
    setEnv (env);

    reinterpret_cast<WebBrowserComponent*> (host)->pageFinishedLoading (juceString (static_cast<jstring> (url)));
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, webViewReceivedError, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*webView*/, jobject /*request*/, jobject error))
{
    setEnv (env);

    jclass errorClass = env->FindClass ("android/webkit/WebResourceError");

    if (errorClass != 0)
    {
        jmethodID method = env->GetMethodID (errorClass, "getDescription", "()Ljava/lang/CharSequence;");

        if (method != 0)
        {
            auto sequence = LocalRef<jobject> (env->CallObjectMethod (error, method));
            auto errorString = LocalRef<jstring> ((jstring) env->CallObjectMethod (sequence, JavaCharSequence.toString));

            reinterpret_cast<WebBrowserComponent*> (host)->pageLoadHadNetworkError (juceString (errorString));
            return;
        }
    }

    // Should never get here!
    jassertfalse;
    reinterpret_cast<WebBrowserComponent*> (host)->pageLoadHadNetworkError ({});
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, webViewReceivedHttpError, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*webView*/, jobject /*request*/, jobject errorResponse))
{
    setEnv (env);

    jclass responseClass = env->FindClass ("android/webkit/WebResourceResponse");

    if (responseClass != 0)
    {
        jmethodID method = env->GetMethodID (responseClass, "getReasonPhrase", "()Ljava/lang/String;");

        if (method != 0)
        {
            auto errorString = LocalRef<jstring> ((jstring) env->CallObjectMethod (errorResponse, method));

            reinterpret_cast<WebBrowserComponent*> (host)->pageLoadHadNetworkError (juceString (errorString));
            return;
        }
    }

    // Should never get here!
    jassertfalse;
    reinterpret_cast<WebBrowserComponent*> (host)->pageLoadHadNetworkError ({});
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, webViewReceivedSslError, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*webView*/, jobject /*sslErrorHandler*/, jobject sslError))
{
    setEnv (env);

    auto errorString = LocalRef<jstring> ((jstring) env->CallObjectMethod (sslError, SslError.toString));

    reinterpret_cast<WebBrowserComponent*> (host)->pageLoadHadNetworkError (juceString (errorString));
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, webViewCloseWindowRequest, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*webView*/))
{
    setEnv (env);

    reinterpret_cast<WebBrowserComponent*> (host)->windowCloseRequest();
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, webViewCreateWindowRequest, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*webView*/))
{
    setEnv (env);

    reinterpret_cast<WebBrowserComponent*> (host)->newWindowAttemptingToLoad ({});
}


} // namespace juce
