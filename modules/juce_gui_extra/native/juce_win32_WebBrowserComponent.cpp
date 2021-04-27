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

struct InternalWebViewType
{
    InternalWebViewType() {}
    virtual ~InternalWebViewType() {}

    virtual void createBrowser() = 0;
    virtual bool hasBrowserBeenCreated() = 0;

    virtual void goToURL (const String&, const StringArray*, const MemoryBlock*) = 0;

    virtual void stop() = 0;
    virtual void goBack() = 0;
    virtual void goForward() = 0;
    virtual void refresh() = 0;

    virtual void focusGained() {}
    virtual void setWebViewSize (int, int) = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InternalWebViewType)
};

#if JUCE_MINGW
 JUCE_DECLARE_UUID_GETTER (IOleClientSite,           "00000118-0000-0000-c000-000000000046")
 JUCE_DECLARE_UUID_GETTER (IDispatch,                "00020400-0000-0000-c000-000000000046")

 #ifndef WebBrowser
  class WebBrowser;
 #endif
#endif

JUCE_DECLARE_UUID_GETTER (DWebBrowserEvents2,        "34A715A0-6587-11D0-924A-0020AFC7AC4D")
JUCE_DECLARE_UUID_GETTER (IConnectionPointContainer, "B196B284-BAB4-101A-B69C-00AA00341D07")
JUCE_DECLARE_UUID_GETTER (IWebBrowser2,              "D30C1661-CDAF-11D0-8A3E-00C04FC9E26E")
JUCE_DECLARE_UUID_GETTER (WebBrowser,                "8856F961-340A-11D0-A96B-00C04FD705A2")

//==============================================================================
class Win32WebView   : public InternalWebViewType,
                       public ActiveXControlComponent
{
public:
    Win32WebView (WebBrowserComponent& owner)
    {
        owner.addAndMakeVisible (this);
    }

    ~Win32WebView() override
    {
        if (connectionPoint != nullptr)
            connectionPoint->Unadvise (adviseCookie);

        if (browser != nullptr)
            browser->Release();
    }

    void createBrowser() override
    {
        auto webCLSID = __uuidof (WebBrowser);
        createControl (&webCLSID);

        auto iidWebBrowser2              = __uuidof (IWebBrowser2);
        auto iidConnectionPointContainer = __uuidof (IConnectionPointContainer);

        browser = (IWebBrowser2*) queryInterface (&iidWebBrowser2);

        if (auto connectionPointContainer = (IConnectionPointContainer*) queryInterface (&iidConnectionPointContainer))
        {
            connectionPointContainer->FindConnectionPoint (__uuidof (DWebBrowserEvents2), &connectionPoint);

            if (connectionPoint != nullptr)
            {
                auto* owner = dynamic_cast<WebBrowserComponent*> (Component::getParentComponent());
                jassert (owner != nullptr);

                auto handler = new EventHandler (*owner);
                connectionPoint->Advise (handler, &adviseCookie);
                handler->Release();
            }
        }
    }

    bool hasBrowserBeenCreated() override
    {
        return browser != nullptr;
    }

    void goToURL (const String& url, const StringArray* headers, const MemoryBlock* postData) override
    {
        if (browser != nullptr)
        {
            VARIANT headerFlags, frame, postDataVar, headersVar;  // (_variant_t isn't available in all compilers)
            VariantInit (&headerFlags);
            VariantInit (&frame);
            VariantInit (&postDataVar);
            VariantInit (&headersVar);

            if (headers != nullptr)
            {
                V_VT (&headersVar) = VT_BSTR;
                V_BSTR (&headersVar) = SysAllocString ((const OLECHAR*) headers->joinIntoString ("\r\n").toWideCharPointer());
            }

            if (postData != nullptr && postData->getSize() > 0)
            {
                auto sa = SafeArrayCreateVector (VT_UI1, 0, (ULONG) postData->getSize());

                if (sa != nullptr)
                {
                    void* data = nullptr;
                    SafeArrayAccessData (sa, &data);
                    jassert (data != nullptr);

                    if (data != nullptr)
                    {
                        postData->copyTo (data, 0, postData->getSize());
                        SafeArrayUnaccessData (sa);

                        VARIANT postDataVar2;
                        VariantInit (&postDataVar2);
                        V_VT (&postDataVar2) = VT_ARRAY | VT_UI1;
                        V_ARRAY (&postDataVar2) = sa;

                        sa = nullptr;
                        postDataVar = postDataVar2;
                    }
                    else
                    {
                        SafeArrayDestroy (sa);
                    }
                }
            }

            auto urlBSTR = SysAllocString ((const OLECHAR*) url.toWideCharPointer());
            browser->Navigate (urlBSTR, &headerFlags, &frame, &postDataVar, &headersVar);
            SysFreeString (urlBSTR);

            VariantClear (&headerFlags);
            VariantClear (&frame);
            VariantClear (&postDataVar);
            VariantClear (&headersVar);
        }
    }

    void stop() override
    {
        if (browser != nullptr)
            browser->Stop();
    }

    void goBack() override
    {
        if (browser != nullptr)
            browser->GoBack();
    }

    void goForward() override
    {
        if (browser != nullptr)
            browser->GoForward();
    }

    void refresh() override
    {
        if (browser != nullptr)
            browser->Refresh();
    }

    void focusGained() override
    {
        auto iidOleObject = __uuidof (IOleObject);
        auto iidOleWindow = __uuidof (IOleWindow);

        if (auto oleObject = (IOleObject*) queryInterface (&iidOleObject))
        {
            if (auto oleWindow = (IOleWindow*) queryInterface (&iidOleWindow))
            {
                IOleClientSite* oleClientSite = nullptr;

                if (SUCCEEDED (oleObject->GetClientSite (&oleClientSite)))
                {
                    HWND hwnd;
                    oleWindow->GetWindow (&hwnd);
                    oleObject->DoVerb (OLEIVERB_UIACTIVATE, nullptr, oleClientSite, 0, hwnd, nullptr);
                    oleClientSite->Release();
                }

                oleWindow->Release();
            }

            oleObject->Release();
        }
    }

    using ActiveXControlComponent::focusGained;

    void setWebViewSize (int width, int height) override
    {
        setSize (width, height);
    }

private:
    IWebBrowser2* browser = nullptr;
    IConnectionPoint* connectionPoint = nullptr;
    DWORD adviseCookie = 0;

    //==============================================================================
    struct EventHandler  : public ComBaseClassHelper<IDispatch>,
                           public ComponentMovementWatcher
    {
        EventHandler (WebBrowserComponent& w)  : ComponentMovementWatcher (&w), owner (w) {}

        JUCE_COMRESULT GetTypeInfoCount (UINT*) override                                 { return E_NOTIMPL; }
        JUCE_COMRESULT GetTypeInfo (UINT, LCID, ITypeInfo**) override                    { return E_NOTIMPL; }
        JUCE_COMRESULT GetIDsOfNames (REFIID, LPOLESTR*, UINT, LCID, DISPID*) override   { return E_NOTIMPL; }

        JUCE_COMRESULT Invoke (DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS* pDispParams,
                               VARIANT* /*pVarResult*/, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/) override
        {
            if (dispIdMember == DISPID_BEFORENAVIGATE2)
            {
                *pDispParams->rgvarg->pboolVal
                    = owner.pageAboutToLoad (getStringFromVariant (pDispParams->rgvarg[5].pvarVal)) ? VARIANT_FALSE
                                                                                                    : VARIANT_TRUE;
                return S_OK;
            }

            if (dispIdMember == 273 /*DISPID_NEWWINDOW3*/)
            {
                owner.newWindowAttemptingToLoad (pDispParams->rgvarg[0].bstrVal);
                *pDispParams->rgvarg[3].pboolVal = VARIANT_TRUE;
                return S_OK;
            }

            if (dispIdMember == DISPID_DOCUMENTCOMPLETE)
            {
                owner.pageFinishedLoading (getStringFromVariant (pDispParams->rgvarg[0].pvarVal));
                return S_OK;
            }

            if (dispIdMember == 271 /*DISPID_NAVIGATEERROR*/)
            {
                int statusCode = pDispParams->rgvarg[1].pvarVal->intVal;
                *pDispParams->rgvarg[0].pboolVal = VARIANT_FALSE;

                // IWebBrowser2 also reports http status codes here, we need
                // report only network errors
                if (statusCode < 0)
                {
                    LPTSTR messageBuffer = nullptr;
                    auto size = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                               nullptr, statusCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                               (LPTSTR) &messageBuffer, 0, nullptr);

                    String message (messageBuffer, size);
                    LocalFree (messageBuffer);

                    if (! owner.pageLoadHadNetworkError (message))
                        *pDispParams->rgvarg[0].pboolVal = VARIANT_TRUE;
                }

                return S_OK;
            }

            if (dispIdMember == 263 /*DISPID_WINDOWCLOSING*/)
            {
                owner.windowCloseRequest();

                // setting this bool tells the browser to ignore the event - we'll handle it.
                if (pDispParams->cArgs > 0 && pDispParams->rgvarg[0].vt == (VT_BYREF | VT_BOOL))
                    *pDispParams->rgvarg[0].pboolVal = VARIANT_TRUE;

                return S_OK;
            }

            return E_NOTIMPL;
        }

        void componentMovedOrResized (bool, bool) override   {}
        void componentPeerChanged() override                 {}
        void componentVisibilityChanged() override           { owner.visibilityChanged(); }

    private:
        WebBrowserComponent& owner;

        static String getStringFromVariant (VARIANT* v)
        {
            return (v->vt & VT_BYREF) != 0 ? *v->pbstrVal
                                           : v->bstrVal;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventHandler)
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32WebView)
};

#if JUCE_USE_WIN_WEBVIEW2

using namespace Microsoft::WRL;

class WebView2  : public InternalWebViewType,
                  public Component,
                  public ComponentMovementWatcher
{
public:
    WebView2 (WebBrowserComponent& o, const File& dllLocation, const File& userDataFolder)
         : ComponentMovementWatcher (&o),
           owner (o)
    {
        if (! createWebViewEnvironment (dllLocation, userDataFolder))
            throw std::runtime_error ("Failed to create the CoreWebView2Environemnt");

        owner.addAndMakeVisible (this);
    }

    ~WebView2() override
    {
        removeEventHandlers();
        closeWebView();

        if (webView2LoaderHandle != nullptr)
            ::FreeLibrary (webView2LoaderHandle);
    }

    void createBrowser() override
    {
        if (webView == nullptr)
        {
            jassert (webViewEnvironment != nullptr);
            createWebView();
        }
    }

    bool hasBrowserBeenCreated() override
    {
        return webView != nullptr || isCreating;
    }

    void goToURL (const String& url, const StringArray* headers, const MemoryBlock* postData) override
    {
        urlRequest = { url,
                       headers != nullptr ? *headers : StringArray(),
                       postData != nullptr && postData->getSize() > 0 ? *postData : MemoryBlock() };

        if (webView != nullptr)
            webView->Navigate (urlRequest.url.toWideCharPointer());
    }

    void stop() override
    {
        if (webView != nullptr)
            webView->Stop();
    }

    void goBack() override
    {
        if (webView != nullptr)
        {
            BOOL canGoBack = false;
            webView->get_CanGoBack (&canGoBack);

            if (canGoBack)
                webView->GoBack();
        }
    }

    void goForward() override
    {
        if (webView != nullptr)
        {
            BOOL canGoForward = false;
            webView->get_CanGoForward (&canGoForward);

            if (canGoForward)
                webView->GoForward();
        }
    }

    void refresh() override
    {
        if (webView != nullptr)
            webView->Reload();
    }

    void setWebViewSize (int width, int height) override
    {
        setSize (width, height);
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
            setControlBounds (peer->getAreaCoveredBy (owner));
    }

    void componentPeerChanged() override
    {
        componentMovedOrResized (true, true);
    }

    void componentVisibilityChanged() override
    {
        setControlVisible (owner.isShowing());

        componentPeerChanged();
        owner.visibilityChanged();
    }

private:
    //==============================================================================
    template <class ArgType>
    static String getUriStringFromArgs (ArgType* args)
    {
        if (args != nullptr)
        {
            LPWSTR uri;
            args->get_Uri (&uri);

            return uri;
        }

        return {};
    }

    //==============================================================================
    void addEventHandlers()
    {
        if (webView != nullptr)
        {
            webView->add_NavigationStarting (Callback<ICoreWebView2NavigationStartingEventHandler> (
                [this] (ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT
                {
                    auto uriString = getUriStringFromArgs (args);

                    if (uriString.isNotEmpty() && ! owner.pageAboutToLoad (uriString))
                        args->put_Cancel (true);

                    return S_OK;
                }).Get(), &navigationStartingToken);

            webView->add_NewWindowRequested (Callback<ICoreWebView2NewWindowRequestedEventHandler> (
                [this] (ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT
                {
                    auto uriString = getUriStringFromArgs (args);

                    if (uriString.isNotEmpty())
                    {
                        owner.newWindowAttemptingToLoad (uriString);
                        args->put_Handled (true);
                    }

                    return S_OK;
                }).Get(), &newWindowRequestedToken);

            webView->add_WindowCloseRequested (Callback<ICoreWebView2WindowCloseRequestedEventHandler> (
                [this] (ICoreWebView2*, IUnknown*) -> HRESULT
                {
                    owner.windowCloseRequest();
                    return S_OK;
                }).Get(), &windowCloseRequestedToken);

            webView->add_NavigationCompleted (Callback<ICoreWebView2NavigationCompletedEventHandler> (
                [this] (ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
                {
                    LPWSTR uri;
                    sender->get_Source (&uri);

                    String uriString (uri);

                    if (uriString.isNotEmpty())
                    {
                        BOOL success = false;
                        args->get_IsSuccess (&success);

                        COREWEBVIEW2_WEB_ERROR_STATUS errorStatus;
                        args->get_WebErrorStatus (&errorStatus);

                        if (success
                            || errorStatus == COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED) // this error seems to happen erroneously so ignore
                        {
                            owner.pageFinishedLoading (uriString);
                        }
                        else
                        {
                            auto errorString = "Error code: " + String (errorStatus);

                            if (owner.pageLoadHadNetworkError (errorString))
                                owner.goToURL ("data:text/plain;charset=UTF-8," + errorString);
                        }
                    }

                    return S_OK;
                }).Get(), &navigationCompletedToken);

            webView->AddWebResourceRequestedFilter (L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_DOCUMENT);

            webView->add_WebResourceRequested (Callback<ICoreWebView2WebResourceRequestedEventHandler> (
                [this] (ICoreWebView2*, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT
                {
                    if (urlRequest.url.isEmpty())
                        return S_OK;

                    ComSmartPtr<ICoreWebView2WebResourceRequest> request;
                    args->get_Request (request.resetAndGetPointerAddress());

                    auto uriString = getUriStringFromArgs<ICoreWebView2WebResourceRequest> (request);

                    if (uriString == urlRequest.url
                        || (uriString.endsWith ("/") && uriString.upToLastOccurrenceOf ("/", false, false) == urlRequest.url))
                    {
                        String method ("GET");

                        if (! urlRequest.postData.isEmpty())
                        {
                            method = "POST";

                            ComSmartPtr<IStream> content (SHCreateMemStream ((BYTE*) urlRequest.postData.getData(),
                                                                                      (UINT) urlRequest.postData.getSize()));
                            request->put_Content (content);
                        }

                        if (! urlRequest.headers.isEmpty())
                        {
                            ComSmartPtr<ICoreWebView2HttpRequestHeaders> headers;
                            request->get_Headers (headers.resetAndGetPointerAddress());

                            for (auto& header : urlRequest.headers)
                            {
                                headers->SetHeader (header.upToFirstOccurrenceOf (":", false, false).trim().toWideCharPointer(),
                                                    header.fromFirstOccurrenceOf (":", false, false).trim().toWideCharPointer());
                            }
                        }

                        request->put_Method (method.toWideCharPointer());

                        urlRequest = {};
                    }

                    return S_OK;
                }).Get(), &webResourceRequestedToken);
        }
    }

    void removeEventHandlers()
    {
        if (webView != nullptr)
        {
            if (navigationStartingToken.value != 0)
                webView->remove_NavigationStarting (navigationStartingToken);

            if (newWindowRequestedToken.value != 0)
                webView->remove_NewWindowRequested (newWindowRequestedToken);

            if (windowCloseRequestedToken.value != 0)
                webView->remove_WindowCloseRequested (windowCloseRequestedToken);

            if (navigationCompletedToken.value != 0)
                webView->remove_NavigationCompleted (navigationCompletedToken);

            if (webResourceRequestedToken.value != 0)
            {
                webView->RemoveWebResourceRequestedFilter (L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_DOCUMENT);
                webView->remove_WebResourceRequested (webResourceRequestedToken);
            }
        }
    }

    bool createWebViewEnvironment (const File& dllLocation, const File& userDataFolder)
    {
        using CreateWebViewEnvironmentWithOptionsFunc = HRESULT (*) (PCWSTR, PCWSTR,
                                                                     ICoreWebView2EnvironmentOptions*,
                                                                     ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

        auto dllPath = dllLocation.getFullPathName();

        if (dllPath.isEmpty())
            dllPath = "WebView2Loader.dll";

        webView2LoaderHandle = LoadLibraryA (dllPath.toUTF8());

        if (webView2LoaderHandle == nullptr)
            return false;

        auto* createWebViewEnvironmentWithOptions = (CreateWebViewEnvironmentWithOptionsFunc) GetProcAddress (webView2LoaderHandle,
                                                                                                              "CreateCoreWebView2EnvironmentWithOptions");
        if (createWebViewEnvironmentWithOptions == nullptr)
        {
            // failed to load WebView2Loader.dll
            jassertfalse;
            return false;
        }

        auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

        WeakReference<WebView2> weakThis (this);
        auto hr = createWebViewEnvironmentWithOptions (nullptr,
                                                       userDataFolder != File() ? userDataFolder.getFullPathName().toWideCharPointer() : nullptr,
                                                       options.Get(),
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this, weakThis] (HRESULT, ICoreWebView2Environment* env) -> HRESULT
                {
                    if (weakThis != nullptr)
                        webViewEnvironment = env;

                    return S_OK;
                }).Get());

        return SUCCEEDED (hr);
    }

    void createWebView()
    {
        if (auto* peer = getPeer())
        {
            isCreating = true;

            WeakReference<WebView2> weakThis (this);

            webViewEnvironment->CreateCoreWebView2Controller ((HWND) peer->getNativeHandle(),
                Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler> (
                    [this, weakThis] (HRESULT, ICoreWebView2Controller* controller) -> HRESULT
                    {
                        if (weakThis != nullptr)
                        {
                            isCreating = false;

                            if (controller != nullptr)
                            {
                                webViewController = controller;
                                controller->get_CoreWebView2 (webView.resetAndGetPointerAddress());

                                addEventHandlers();
                                componentMovedOrResized (true, true);

                                if (webView != nullptr && urlRequest.url.isNotEmpty())
                                    webView->Navigate (urlRequest.url.toWideCharPointer());
                            }
                        }

                        return S_OK;
                    }).Get());
        }
    }

    void closeWebView()
    {
        if (webViewController != nullptr)
        {
            webViewController->Close();
            webViewController = nullptr;
            webView = nullptr;
        }

        webViewEnvironment = nullptr;
    }

    //==============================================================================
    void setControlBounds (Rectangle<int> newBounds) const
    {
        if (webViewController != nullptr)
        {
           #if JUCE_WIN_PER_MONITOR_DPI_AWARE
            if (auto* peer = owner.getTopLevelComponent()->getPeer())
                newBounds = (newBounds.toDouble() * peer->getPlatformScaleFactor()).toNearestInt();
           #endif

            webViewController->put_Bounds({ newBounds.getX(), newBounds.getY(),
                                            newBounds.getRight(), newBounds.getBottom() });
        }
    }

    void setControlVisible (bool shouldBeVisible) const
    {
        if (webViewController != nullptr)
            webViewController->put_IsVisible (shouldBeVisible);
    }

    //==============================================================================
    WebBrowserComponent& owner;

    HMODULE webView2LoaderHandle = nullptr;

    ComSmartPtr<ICoreWebView2Environment> webViewEnvironment;
    ComSmartPtr<ICoreWebView2Controller> webViewController;
    ComSmartPtr<ICoreWebView2> webView;

    EventRegistrationToken navigationStartingToken   { 0 },
                           newWindowRequestedToken   { 0 },
                           windowCloseRequestedToken { 0 },
                           navigationCompletedToken  { 0 },
                           webResourceRequestedToken { 0 };

    struct URLRequest
    {
        String url;
        StringArray headers;
        MemoryBlock postData;
    };
    URLRequest urlRequest;

    bool isCreating = false;

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (WebView2)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebView2)
};

#endif

//==============================================================================
class WebBrowserComponent::Pimpl
{
public:
    Pimpl (WebBrowserComponent& owner, const File& dllLocation, const File& userDataFolder, bool useWebView2)
    {
        if (useWebView2)
        {
           #if JUCE_USE_WIN_WEBVIEW2
            try
            {
                internal.reset (new WebView2 (owner, dllLocation, userDataFolder));
            }
            catch (std::runtime_error&) {}
           #endif
        }

        ignoreUnused (dllLocation, userDataFolder);

        if (internal == nullptr)
            internal.reset (new Win32WebView (owner));
    }

    InternalWebViewType& getInternalWebView()
    {
        return *internal;
    }

private:
    std::unique_ptr<InternalWebViewType> internal;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (bool unloadWhenHidden)
    : browser (new Pimpl (*this, {}, {}, false)),
      unloadPageWhenBrowserIsHidden (unloadWhenHidden)
{
    setOpaque (true);
}

WebBrowserComponent::WebBrowserComponent (bool unloadWhenHidden,
                                          const File& dllLocation,
                                          const File& userDataFolder)
    : browser (new Pimpl (*this, dllLocation, userDataFolder, true)),
      unloadPageWhenBrowserIsHidden (unloadWhenHidden)
{
    setOpaque (true);
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

    if (! browser->getInternalWebView().hasBrowserBeenCreated())
        checkWindowAssociation();

    browser->getInternalWebView().goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    browser->getInternalWebView().stop();
}

void WebBrowserComponent::goBack()
{
    lastURL.clear();
    blankPageShown = false;

    browser->getInternalWebView().goBack();
}

void WebBrowserComponent::goForward()
{
    lastURL.clear();

    browser->getInternalWebView().goForward();
}

void WebBrowserComponent::refresh()
{
    browser->getInternalWebView().refresh();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
    if (! browser->getInternalWebView().hasBrowserBeenCreated())
    {
        g.fillAll (Colours::white);
        checkWindowAssociation();
    }
}

void WebBrowserComponent::checkWindowAssociation()
{
    if (isShowing())
    {
        if (! browser->getInternalWebView().hasBrowserBeenCreated() && getPeer() != nullptr)
        {
            browser->getInternalWebView().createBrowser();
            reloadLastURL();
        }
        else
        {
            if (blankPageShown)
                goBack();
        }
    }
    else
    {
        if (browser != nullptr && unloadPageWhenBrowserIsHidden && ! blankPageShown)
        {
            // when the component becomes invisible, some stuff like flash
            // carries on playing audio, so we need to force it onto a blank
            // page to avoid this..

            blankPageShown = true;
            browser->getInternalWebView().goToURL ("about:blank", 0, 0);
        }
    }
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
    browser->getInternalWebView().setWebViewSize (getWidth(), getHeight());
}

void WebBrowserComponent::visibilityChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::focusGained (FocusChangeType)
{
    browser->getInternalWebView().focusGained();
}

void WebBrowserComponent::clearCookies()
{
    HeapBlock<::INTERNET_CACHE_ENTRY_INFOA> entry;
    ::DWORD entrySize = sizeof (::INTERNET_CACHE_ENTRY_INFOA);
    ::HANDLE urlCacheHandle = ::FindFirstUrlCacheEntryA ("cookie:", entry.getData(), &entrySize);

    if (urlCacheHandle == nullptr && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        entry.realloc (1, entrySize);
        urlCacheHandle = ::FindFirstUrlCacheEntryA ("cookie:", entry.getData(), &entrySize);
    }

    if (urlCacheHandle != nullptr)
    {
        for (;;)
        {
            ::DeleteUrlCacheEntryA (entry.getData()->lpszSourceUrlName);

            if (::FindNextUrlCacheEntryA (urlCacheHandle, entry.getData(), &entrySize) == 0)
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    entry.realloc (1, entrySize);

                    if (::FindNextUrlCacheEntryA (urlCacheHandle, entry.getData(), &entrySize) != 0)
                        continue;
                }

                break;
            }
        }

        FindCloseUrlCache (urlCacheHandle);
    }
}

} // namespace juce
