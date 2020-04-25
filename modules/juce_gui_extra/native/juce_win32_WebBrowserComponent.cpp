/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
};

#if JUCE_USE_WINRT_WEBVIEW

extern RTL_OSVERSIONINFOW getWindowsVersionInfo();

using namespace Microsoft::WRL;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Storage::Streams;
using namespace ABI::Windows::Web;
using namespace ABI::Windows::Web::UI;
using namespace ABI::Windows::Web::UI::Interop;
using namespace ABI::Windows::Web::Http;
using namespace ABI::Windows::Web::Http::Headers;

//==============================================================================
class WinRTWebView  : public InternalWebViewType,
                      public Component,
                      public ComponentMovementWatcher
{
public:
    WinRTWebView (WebBrowserComponent& o)
        : ComponentMovementWatcher (&o),
          owner (o)
    {
        if (! WinRTWrapper::getInstance()->isInitialised())
            throw std::runtime_error ("Failed to initialise the WinRT wrapper");

        if (! createWebViewProcess())
            throw std::runtime_error ("Failed to create the WebViewControlProcess");

        owner.addAndMakeVisible (this);
    }

    ~WinRTWebView() override
    {
        if (webViewControl != nullptr)
            webViewControl->Stop();

        removeEventHandlers();

        webViewProcess->Terminate();
    }

    void createBrowser() override
    {
        if (webViewControl == nullptr)
            createWebViewControl();
    }

    bool hasBrowserBeenCreated() override
    {
        return webViewControl != nullptr || isCreating;
    }

    void goToURL (const String& url, const StringArray* headers, const MemoryBlock* postData) override
    {
        if (webViewControl != nullptr)
        {
            if ((headers != nullptr && ! headers->isEmpty())
                || (postData != nullptr && postData->getSize() > 0))
            {
                auto requestMessage = createHttpRequestMessage (url, headers, postData);
                webViewControl->NavigateWithHttpRequestMessage (requestMessage.get());
            }
            else
            {
                auto uri = createURI (url);
                webViewControl->Navigate (uri.get());
            }
        }
    }

    void stop() override
    {
        if (webViewControl != nullptr)
            webViewControl->Stop();
    }

    void goBack() override
    {
        if (webViewControl != nullptr)
        {
            boolean canGoBack = false;
            webViewControl->get_CanGoBack (&canGoBack);

            if (canGoBack)
                webViewControl->GoBack();
        }
    }

    void goForward() override
    {
        if (webViewControl != nullptr)
        {
            boolean canGoForward = false;
            webViewControl->get_CanGoForward (&canGoForward);

            if (canGoForward)
                webViewControl->GoForward();
        }
    }

    void refresh() override
    {
        if (webViewControl != nullptr)
            webViewControl->Refresh();
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
    template<typename OperationResultType, typename ResultsType>
    static HRESULT waitForCompletion (IAsyncOperation<OperationResultType>* op, ResultsType* results)
    {
        using OperationType = IAsyncOperation<OperationResultType>;
        using DelegateType  = IAsyncOperationCompletedHandler<OperationResultType>;

        struct EventDelegate : public RuntimeClass<RuntimeClassFlags<RuntimeClassType::Delegate>,
                                                   DelegateType,
                                                   FtmBase>
        {
            EventDelegate() = default;

            ~EventDelegate()
            {
                CloseHandle (eventCompleted);
            }

            HRESULT RuntimeClassInitialize()
            {
                eventCompleted = CreateEventEx (nullptr, nullptr, 0, EVENT_ALL_ACCESS);
                return eventCompleted == nullptr ? HRESULT_FROM_WIN32 (GetLastError()) : S_OK;
            }

            HRESULT Invoke (OperationType*, AsyncStatus newStatus)
            {
                status = newStatus;
                SetEvent (eventCompleted);

                return S_OK;
            }

            AsyncStatus status = AsyncStatus::Started;
            HANDLE eventCompleted = nullptr;
        };

        WinRTWrapper::ComPtr<OperationType> operation = op;
        WinRTWrapper::ComPtr<EventDelegate> eventCallback;

        auto hr = MakeAndInitialize<EventDelegate> (eventCallback.resetAndGetPointerAddress());

        if (SUCCEEDED (hr))
        {
            hr = operation->put_Completed (eventCallback.get());

            if (SUCCEEDED (hr))
            {
                HANDLE waitForEvents[1] { eventCallback->eventCompleted };
                auto handleCount = (ULONG) ARRAYSIZE (waitForEvents);
                DWORD handleIndex = 0;

                hr = CoWaitForMultipleHandles (COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS | COWAIT_INPUTAVAILABLE,
                                               INFINITE, handleCount, waitForEvents, &handleIndex);

                if (SUCCEEDED (hr))
                {
                    if (eventCallback->status == AsyncStatus::Completed)
                    {
                        hr = operation->GetResults (results);
                    }
                    else
                    {
                        WinRTWrapper::ComPtr<IAsyncInfo> asyncInfo;

                        if (SUCCEEDED (operation->QueryInterface (asyncInfo.resetAndGetPointerAddress())))
                            asyncInfo->get_ErrorCode (&hr);
                    }
                }
            }
        }

        return hr;
    }

    //==============================================================================
    template<class ArgsType>
    String getURIStringFromArgs (ArgsType& args)
    {
        WinRTWrapper::ComPtr<IUriRuntimeClass> uri;
        args.get_Uri (uri.resetAndGetPointerAddress());

        if (uri != nullptr)
        {
            HSTRING uriString;
            uri->get_AbsoluteUri (&uriString);

            return WinRTWrapper::getInstance()->hStringToString (uriString);
        }

        return {};
    }

    void addEventHandlers()
    {
        if (webViewControl != nullptr)
        {
            webViewControl->add_NavigationStarting (Callback<ITypedEventHandler<IWebViewControl*, WebViewControlNavigationStartingEventArgs*>> (
                [this] (IWebViewControl*, IWebViewControlNavigationStartingEventArgs* args)
                {
                    auto uriString = getURIStringFromArgs (*args);

                    if (uriString.isNotEmpty())
                        args->put_Cancel (! owner.pageAboutToLoad (uriString));

                    return S_OK;
                }
            ).Get(), &navigationStartingToken);

            webViewControl->add_NewWindowRequested (Callback<ITypedEventHandler<IWebViewControl*, WebViewControlNewWindowRequestedEventArgs*>> (
                [this] (IWebViewControl*, IWebViewControlNewWindowRequestedEventArgs* args)
                {
                    auto uriString = getURIStringFromArgs (*args);

                    if (uriString.isNotEmpty())
                    {
                        owner.newWindowAttemptingToLoad (uriString);
                        args->put_Handled (true);
                    }

                    return S_OK;
                }
            ).Get(), &newWindowRequestedToken);

            webViewControl->add_NavigationCompleted (Callback<ITypedEventHandler<IWebViewControl*, WebViewControlNavigationCompletedEventArgs*>> (
                [this] (IWebViewControl*, IWebViewControlNavigationCompletedEventArgs* args)
                {
                    auto uriString = getURIStringFromArgs (*args);

                    if (uriString.isNotEmpty())
                    {
                        boolean success;
                        args->get_IsSuccess (&success);

                        if (success)
                        {
                            owner.pageFinishedLoading (uriString);
                        }
                        else
                        {
                            WebErrorStatus status;
                            args->get_WebErrorStatus (&status);

                            owner.pageLoadHadNetworkError ("Error code: " + String (status));
                        }
                    }

                    return S_OK;
                }
            ).Get(), &navigationCompletedToken);
        }
    }

    void removeEventHandlers()
    {
        if (webViewControl != nullptr)
        {
            if (navigationStartingToken.value != 0)
                webViewControl->remove_NavigationStarting (navigationStartingToken);

            if (newWindowRequestedToken.value != 0)
                webViewControl->remove_NewWindowRequested (newWindowRequestedToken);

            if (navigationCompletedToken.value != 0)
                webViewControl->remove_NavigationCompleted (navigationCompletedToken);
        }
    }

    bool createWebViewProcess()
    {
        auto webViewControlProcessFactory
            = WinRTWrapper::getInstance()->getWRLFactory<IWebViewControlProcessFactory> (RuntimeClass_Windows_Web_UI_Interop_WebViewControlProcess);

        if (webViewControlProcessFactory == nullptr)
        {
            jassertfalse;
            return false;
        }

        auto webViewProcessOptions
            = WinRTWrapper::getInstance()->activateInstance<IWebViewControlProcessOptions> (RuntimeClass_Windows_Web_UI_Interop_WebViewControlProcessOptions,
                                                                                            __uuidof (IWebViewControlProcessOptions));

        webViewProcessOptions->put_PrivateNetworkClientServerCapability (WebViewControlProcessCapabilityState_Enabled);
        webViewControlProcessFactory->CreateWithOptions (webViewProcessOptions.get(), webViewProcess.resetAndGetPointerAddress());

        return webViewProcess != nullptr;
    }

    void createWebViewControl()
    {
        if (auto* peer = getPeer())
        {
            ScopedValueSetter<bool> svs (isCreating, true);

            WinRTWrapper::ComPtr<IAsyncOperation<WebViewControl*>> createWebViewAsyncOperation;

            webViewProcess->CreateWebViewControlAsync ((INT64) peer->getNativeHandle(), {},
                                                       createWebViewAsyncOperation.resetAndGetPointerAddress());

            waitForCompletion (createWebViewAsyncOperation.get(), webViewControl.resetAndGetPointerAddress());

            addEventHandlers();
            componentMovedOrResized (true, true);
        }
    }

    //==============================================================================
    WinRTWrapper::ComPtr<IUriRuntimeClass> createURI (const String& url)
    {
        auto uriRuntimeFactory
            = WinRTWrapper::getInstance()->getWRLFactory <IUriRuntimeClassFactory> (RuntimeClass_Windows_Foundation_Uri);

        if (uriRuntimeFactory == nullptr)
        {
            jassertfalse;
            return {};
        }

        WinRTWrapper::ScopedHString hstr (url);
        WinRTWrapper::ComPtr<IUriRuntimeClass> uriRuntimeClass;
        uriRuntimeFactory->CreateUri (hstr.get(), uriRuntimeClass.resetAndGetPointerAddress());

        return uriRuntimeClass;
    }

    WinRTWrapper::ComPtr<IHttpContent> getPOSTContent (const MemoryBlock& postData)
    {
        auto factory = WinRTWrapper::getInstance()->getWRLFactory<IHttpStringContentFactory> (RuntimeClass_Windows_Web_Http_HttpStringContent);

        if (factory == nullptr)
        {
            jassertfalse;
            return {};
        }

        WinRTWrapper::ScopedHString hStr (postData.toString());

        WinRTWrapper::ComPtr<IHttpContent> content;
        factory->CreateFromString (hStr.get(), content.resetAndGetPointerAddress());

        return content;
    }

    WinRTWrapper::ComPtr<IHttpMethod> getMethod (bool isPOST)
    {
        auto methodFactory = WinRTWrapper::getInstance()->getWRLFactory<IHttpMethodStatics> (RuntimeClass_Windows_Web_Http_HttpMethod);

        if (methodFactory == nullptr)
        {
            jassertfalse;
            return {};
        }

        WinRTWrapper::ComPtr<IHttpMethod> method;

        if (isPOST)
            methodFactory->get_Post (method.resetAndGetPointerAddress());
        else
            methodFactory->get_Get (method.resetAndGetPointerAddress());

        return method;
    }

    void addHttpHeaders (WinRTWrapper::ComPtr<IHttpRequestMessage>& requestMessage, const StringArray& headers)
    {
        WinRTWrapper::ComPtr<IHttpRequestHeaderCollection> headerCollection;
        requestMessage->get_Headers (headerCollection.resetAndGetPointerAddress());

        for (int i = 0; i < headers.size(); ++i)
        {
            WinRTWrapper::ScopedHString headerName  (headers[i].upToFirstOccurrenceOf (":", false, false).trim());
            WinRTWrapper::ScopedHString headerValue (headers[i].fromFirstOccurrenceOf (":", false, false).trim());

            headerCollection->Append (headerName.get(), headerValue.get());
        }
    }

    WinRTWrapper::ComPtr<IHttpRequestMessage> createHttpRequestMessage (const String& url,
                                                                        const StringArray* headers,
                                                                        const MemoryBlock* postData)
    {
        auto requestFactory
            = WinRTWrapper::getInstance()->getWRLFactory<IHttpRequestMessageFactory> (RuntimeClass_Windows_Web_Http_HttpRequestMessage);

        if (requestFactory == nullptr)
        {
            jassertfalse;
            return {};
        }

        bool isPOSTRequest = (postData != nullptr && postData->getSize() > 0);
        auto method = getMethod (isPOSTRequest);

        auto uri = createURI (url);

        WinRTWrapper::ComPtr<IHttpRequestMessage> requestMessage;
        requestFactory->Create (method.get(), uri.get(), requestMessage.resetAndGetPointerAddress());

        if (isPOSTRequest)
        {
            auto content = getPOSTContent (*postData);
            requestMessage->put_Content (content.get());
        }

        if (headers != nullptr && ! headers->isEmpty())
            addHttpHeaders (requestMessage, *headers);

        return requestMessage;
    }

    //==============================================================================
    void setControlBounds (Rectangle<int> newBounds) const
    {
        if (webViewControl != nullptr)
        {
           #if JUCE_WIN_PER_MONITOR_DPI_AWARE
            if (auto* peer = owner.getTopLevelComponent()->getPeer())
                newBounds = (newBounds.toDouble() * peer->getPlatformScaleFactor()).toNearestInt();
           #endif

            WinRTWrapper::ComPtr<IWebViewControlSite> site;

            if (SUCCEEDED (webViewControl->QueryInterface (site.resetAndGetPointerAddress())))
                site->put_Bounds ({ static_cast<FLOAT> (newBounds.getX()), static_cast<FLOAT> (newBounds.getY()),
                                    static_cast<FLOAT> (newBounds.getWidth()), static_cast<FLOAT> (newBounds.getHeight()) });
        }

    }

    void setControlVisible (bool shouldBeVisible) const
    {
        if (webViewControl != 0)
        {
            WinRTWrapper::ComPtr<IWebViewControlSite> site;

            if (SUCCEEDED (webViewControl->QueryInterface (site.resetAndGetPointerAddress())))
                site->put_IsVisible (shouldBeVisible);
        }
    }

    //==============================================================================
    WebBrowserComponent& owner;

    WinRTWrapper::ComPtr<IWebViewControlProcess> webViewProcess;
    WinRTWrapper::ComPtr<IWebViewControl> webViewControl;

    EventRegistrationToken navigationStartingToken  { 0 },
                           newWindowRequestedToken  { 0 },
                           navigationCompletedToken { 0 };

    bool isCreating = false;
};

#endif

//==============================================================================
class WebBrowserComponent::Pimpl
{
public:
    Pimpl (WebBrowserComponent& owner)
    {
        #if JUCE_USE_WINRT_WEBVIEW
         auto windowsVersionInfo = getWindowsVersionInfo();

         if (windowsVersionInfo.dwMajorVersion >= 10 && windowsVersionInfo.dwBuildNumber >= 17763)
         {
             try
             {
                 internal.reset (new WinRTWebView (owner));
             }
             catch (std::runtime_error&) {}
         }
        #endif

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
    : browser (new Pimpl (*this)),
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
