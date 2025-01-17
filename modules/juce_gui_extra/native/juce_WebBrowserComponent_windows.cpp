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

struct WebBrowserComponent::Impl::Platform  : public PlatformInterface
{
    class Win32WebView;
    class WebView2;
};

//==============================================================================
class WebBrowserComponent::Impl::Platform::Win32WebView final : public WebBrowserComponent::Impl::PlatformInterface,
                                                                public ActiveXControlComponent
{
public:
    Win32WebView (WebBrowserComponent& parent, const String& userAgentToUse)
        : owner (parent),
          userAgent (userAgentToUse)
    {
        owner.addAndMakeVisible (this);
    }

    void checkWindowAssociation() override
    {
        if (owner.isShowing())
        {
            if (! hasBrowserBeenCreated() && owner.getPeer() != nullptr)
            {
                createBrowser();
                owner.reloadLastURL();
            }
            else
            {
                if (owner.blankPageShown)
                    goBack();
            }
        }
        else
        {
            if (owner.unloadPageWhenHidden && ! owner.blankPageShown)
            {
                // when the component becomes invisible, some stuff like flash
                // carries on playing audio, so we need to force it onto a blank
                // page to avoid this..

                owner.blankPageShown = true;
                goToURL ("about:blank", nullptr, nullptr);
            }
        }
    }

    ~Win32WebView() override
    {
        if (connectionPoint != nullptr)
        {
            connectionPoint->Unadvise (adviseCookie);
            connectionPoint->Release();
        }

        if (browser != nullptr)
            browser->Release();
    }

    void createBrowser()
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        auto webCLSID = __uuidof (WebBrowser);
        createControl (&webCLSID);

        auto iidWebBrowser2              = __uuidof (IWebBrowser2);
        auto iidConnectionPointContainer = __uuidof (IConnectionPointContainer);
        auto iidOleControl               = __uuidof (IOleControl);

        browser = (IWebBrowser2*) queryInterface (&iidWebBrowser2);

        if (auto connectionPointContainer = (IConnectionPointContainer*) queryInterface (&iidConnectionPointContainer))
        {
            connectionPointContainer->FindConnectionPoint (__uuidof (DWebBrowserEvents2), &connectionPoint);

            if (connectionPoint != nullptr)
            {
                auto handler = new EventHandler (*this);
                connectionPoint->Advise (handler, &adviseCookie);
                setEventHandler (handler);
                handler->Release();
            }

            connectionPointContainer->Release();
        }

        if (auto oleControl = (IOleControl*) queryInterface (&iidOleControl))
        {
            oleControl->OnAmbientPropertyChange (/*DISPID_AMBIENT_USERAGENT*/-5513);
            oleControl->Release();
        }

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    bool hasBrowserBeenCreated()
    {
        return browser != nullptr;
    }

    void fallbackPaint (Graphics& webBrowserComponentContext) override
    {
        if (! hasBrowserBeenCreated())
        {
            webBrowserComponentContext.fillAll (Colours::white);
            checkWindowAssociation();
        }
    }

    void goToURL (const String& url, const StringArray* requestedHeaders, const MemoryBlock* postData) override
    {
        checkWindowAssociation();

        if (browser != nullptr)
        {
            VARIANT headerFlags, frame, postDataVar, headersVar;  // (_variant_t isn't available in all compilers)
            VariantInit (&headerFlags);
            VariantInit (&frame);
            VariantInit (&postDataVar);
            VariantInit (&headersVar);

            StringArray headers;

            if (userAgent.isNotEmpty())
                headers.add ("User-Agent: " + userAgent);

            if (requestedHeaders != nullptr)
                headers.addArray (*requestedHeaders);

            if (headers.size() > 0)
            {
                V_VT (&headersVar) = VT_BSTR;
                V_BSTR (&headersVar) = SysAllocString ((const OLECHAR*) headers.joinIntoString ("\r\n").toWideCharPointer());
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

    void focusGainedWithDirection (FocusChangeType, FocusChangeDirection) override
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

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

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    void setWebViewSize (int width, int height) override
    {
        setSize (width, height);
    }

    void evaluateJavascript (const String&, WebBrowserComponent::EvaluationCallback) override
    {
        // This feature is only supported when using WebView2
        jassertfalse;
    }

private:
    WebBrowserComponent& owner;
    IWebBrowser2* browser = nullptr;
    IConnectionPoint* connectionPoint = nullptr;
    DWORD adviseCookie = 0;
    String userAgent;

    //==============================================================================
    struct EventHandler final : public ComBaseClassHelper<IDispatch>,
                                public ComponentMovementWatcher
    {
        EventHandler (Win32WebView& w)  : ComponentMovementWatcher (&w.owner), owner (w) {}

        JUCE_COMRESULT GetTypeInfoCount (UINT*) override                                 { return E_NOTIMPL; }
        JUCE_COMRESULT GetTypeInfo (UINT, LCID, ITypeInfo**) override                    { return E_NOTIMPL; }
        JUCE_COMRESULT GetIDsOfNames (REFIID, LPOLESTR*, UINT, LCID, DISPID*) override   { return E_NOTIMPL; }

        JUCE_COMRESULT Invoke (DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS* pDispParams,
                               VARIANT* pVarResult, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/) override
        {

            if (dispIdMember == /*DISPID_AMBIENT_USERAGENT*/-5513)
            {
                V_VT ( pVarResult ) = VT_BSTR;
                V_BSTR ( pVarResult ) = SysAllocString ((const OLECHAR*) String (owner.userAgent).toWideCharPointer());;
                return S_OK;
            }

            if (dispIdMember == DISPID_BEFORENAVIGATE2)
            {
                *pDispParams->rgvarg->pboolVal
                    = owner.owner.pageAboutToLoad (getStringFromVariant (pDispParams->rgvarg[5].pvarVal)) ? VARIANT_FALSE
                                                                                                    : VARIANT_TRUE;
                return S_OK;
            }

            if (dispIdMember == 273 /*DISPID_NEWWINDOW3*/)
            {
                owner.owner.newWindowAttemptingToLoad (pDispParams->rgvarg[0].bstrVal);
                *pDispParams->rgvarg[3].pboolVal = VARIANT_TRUE;
                return S_OK;
            }

            if (dispIdMember == DISPID_DOCUMENTCOMPLETE)
            {
                owner.owner.pageFinishedLoading (getStringFromVariant (pDispParams->rgvarg[0].pvarVal));
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
                                               nullptr, (DWORD) statusCode, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                                               (LPTSTR) &messageBuffer, 0, nullptr);

                    String message (messageBuffer, size);
                    LocalFree (messageBuffer);

                    if (! owner.owner.pageLoadHadNetworkError (message))
                        *pDispParams->rgvarg[0].pboolVal = VARIANT_TRUE;
                }

                return S_OK;
            }

            if (dispIdMember == 263 /*DISPID_WINDOWCLOSING*/)
            {
                owner.owner.windowCloseRequest();

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

        using ComponentMovementWatcher::componentVisibilityChanged;
        using ComponentMovementWatcher::componentMovedOrResized;

    private:
        Win32WebView& owner;

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

static std::vector<HWND> getDirectChildWindows (HWND hwnd)
{
    std::vector<HWND> result;

    const auto getNextChildWindow = [hwnd, &result]
    {
        return FindWindowExA (hwnd, result.empty() ? nullptr : result.back(), nullptr, nullptr);
    };

    for (auto* next = getNextChildWindow(); next != nullptr; next = getNextChildWindow())
        result.push_back (next);

    return result;
}

static void forEachChildWindowRecursive (HWND hwnd, std::function<bool (HWND)> callback)
{
    // EnumChildWindows itself provides the recursion
    EnumChildWindows (hwnd,
                      [] (HWND hwnd, LPARAM lParam)
                      {
                          auto* callbackPtr = reinterpret_cast<std::function<bool (HWND)>*> (lParam);
                          return (*callbackPtr) (hwnd) ? TRUE : FALSE;
                      },
                      reinterpret_cast<LPARAM> (&callback));
}

static bool anyChildWindow (HWND hwnd, std::function<bool (HWND)> predicate)
{
    auto result = false;

    forEachChildWindowRecursive (hwnd,
                                 [&predicate, &result] (auto* child)
                                 {
                                     result = predicate (child);
                                     const auto keepGoing = ! result;
                                     return keepGoing;
                                 });

    return result;
}

static constexpr const char* platformSpecificIntegrationScript = R"(
window.__JUCE__ = {
  postMessage: function(object) {
    window.chrome.webview.postMessage(object);
  },
};
)";

class WebBrowserComponent::Impl::Platform::WebView2 final : public WebBrowserComponent::Impl::PlatformInterface,
                                                            public Component,
                                                            public ComponentMovementWatcher,
                                                            private AsyncUpdater
{
public:
    static std::unique_ptr<WebView2> tryConstruct (WebBrowserComponent& o,
                                                   const WebBrowserComponent::Options& prefs,
                                                   const StringArray& userScriptsIn)
    {
        if (auto handle = createWebViewHandle (prefs))
            return rawToUniquePtr (new WebView2 (o, prefs, userScriptsIn, std::move (*handle)));

        return nullptr;
    }

    void checkWindowAssociation() override
    {
        if (owner.isShowing())
        {
            if (! hasBrowserBeenCreated() && owner.getPeer() != nullptr)
            {
                createBrowser();
                owner.reloadLastURL();
            }
            else
            {
                if (owner.blankPageShown)
                    goBack();
            }
        }
        else
        {
            if (webView != nullptr && owner.unloadPageWhenHidden && ! owner.blankPageShown)
            {
                // when the component becomes invisible, some stuff like flash
                // carries on playing audio, so we need to force it onto a blank
                // page to avoid this..

                owner.blankPageShown = true;
                goToURL ("about:blank", nullptr, nullptr);
            }
        }

        if (! hasBrowserBeenCreated())
            createBrowser();
    }

    void fallbackPaint (Graphics& webBrowserComponentContext) override
    {
        webBrowserComponentContext.fillAll (Colours::white);

        if (! hasBrowserBeenCreated())
            checkWindowAssociation();
    }

    void focusGainedWithDirection (FocusChangeType, FocusChangeDirection direction) override
    {
        if (inMoveFocusRequested)
            return;

        const auto moveFocusReason = [&]
        {
            if (direction == FocusChangeDirection::backward)
                return COREWEBVIEW2_MOVE_FOCUS_REASON_PREVIOUS;

            if (direction == FocusChangeDirection::forward)
                return COREWEBVIEW2_MOVE_FOCUS_REASON_NEXT;

            return COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC;
        }();

        if (webViewController != nullptr)
            webViewController->MoveFocus (moveFocusReason);
    }

    ~WebView2() override
    {
        if (webView2ConstructionHelper.webView2BeingCreated == this)
            webView2ConstructionHelper.webView2BeingCreated = nullptr;

        webView2ConstructionHelper.viewsWaitingForCreation.erase (this);

        cancelPendingUpdate();
        removeEventHandlers();
        closeWebView();
    }

    void createBrowser()
    {
        if (webView == nullptr)
        {
            jassert (webViewHandle.environment != nullptr);
            createWebView();
        }
    }

    bool hasBrowserBeenCreated()
    {
        return    webView != nullptr
               || webView2ConstructionHelper.webView2BeingCreated == this
               || webView2ConstructionHelper.viewsWaitingForCreation.count (this) > 0;
    }

    void goToURL (const String& url, const StringArray* headers, const MemoryBlock* postData) override
    {
        checkWindowAssociation();

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

    void componentMovedOrResized (bool, bool) override
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

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
    }

    //==============================================================================
    struct WebViewHandle
    {
        using LibraryRef = std::unique_ptr<typename std::pointer_traits<HMODULE>::element_type, decltype (&::FreeLibrary)>;
        LibraryRef loaderHandle {nullptr, &::FreeLibrary};
        ComSmartPtr<ICoreWebView2Environment> environment;
    };

    static std::optional<WebViewHandle> createWebViewHandle (const WebBrowserComponent::Options& options)
    {
        using CreateWebViewEnvironmentWithOptionsFunc = HRESULT (__stdcall *) (PCWSTR, PCWSTR,
                                                                               ICoreWebView2EnvironmentOptions*,
                                                                               ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

        auto dllPath = options.getWinWebView2BackendOptions().getDLLLocation().getFullPathName();

        if (dllPath.isEmpty())
            dllPath = "WebView2Loader.dll";

        WebViewHandle result;

        auto* createWebViewEnvironmentWithOptions = [&]() -> CreateWebViewEnvironmentWithOptionsFunc
        {
           #if JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING
            return &CreateCoreWebView2EnvironmentWithOptions;
           #else
            result.loaderHandle = WebViewHandle::LibraryRef (LoadLibraryA (dllPath.toUTF8()), &::FreeLibrary);

            if (result.loaderHandle == nullptr)
                return nullptr;

            return (CreateWebViewEnvironmentWithOptionsFunc) GetProcAddress (result.loaderHandle.get(),
                                                                             "CreateCoreWebView2EnvironmentWithOptions");
           #endif
        }();

        if (createWebViewEnvironmentWithOptions == nullptr)
            return {};

        auto webViewOptions = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

        const auto userDataFolder = options.getWinWebView2BackendOptions().getUserDataFolder().getFullPathName();

        auto hr = createWebViewEnvironmentWithOptions (nullptr,
                                                       userDataFolder.isNotEmpty() ? userDataFolder.toWideCharPointer() : nullptr,
                                                       webViewOptions.Get(),
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler> (
                [&result] (HRESULT, ICoreWebView2Environment* env) -> HRESULT
                {
                    result.environment = addComSmartPtrOwner (env);
                    return S_OK;
                }).Get());

        if (! SUCCEEDED (hr))
            return {};

        return result;
    }

    void evaluateJavascript (const String& script, EvaluationCallback callbackIn) override
    {
        if (webView == nullptr)
        {
            scriptsWaitingForExecution.push_back ({ script, std::move (callbackIn) });
            return;
        }

        webView->ExecuteScript (script.toUTF16(),
                                Callback<ICoreWebView2ExecuteScriptCompletedHandler> (
                                    [callback = std::move (callbackIn)] (HRESULT error, PCWSTR result) -> HRESULT
                                    {
                                        if (callback == nullptr)
                                            return S_OK;

                                        const auto callbackArg = [&]
                                        {
                                            using Error = WebBrowserComponent::EvaluationResult::Error;

                                            if (error != S_OK)
                                                return EvaluationResult { Error { Error::Type::unknown, "Error code: " + String { error } } };

                                            return EvaluationResult { JSON::fromString (StringRef { CharPointer_UTF16 { result } }) };
                                        }();

                                        callback (callbackArg);

                                        return S_OK;
                                    }).Get());
    }

private:
    //==============================================================================
    WebView2 (WebBrowserComponent& o,
              const WebBrowserComponent::Options& prefs,
              const StringArray& userScriptsIn,
              WebViewHandle handle)
        : ComponentMovementWatcher (&o),
          owner (o),
          preferences (prefs),
          userAgent (prefs.getUserAgent()),
          userScripts (userScriptsIn),
          webViewHandle (std::move (handle))
    {
        owner.addAndMakeVisible (this);
    }

    //==============================================================================
    template <typename ArgType>
    static std::optional<String> callMethodWithLpwstrResult (ArgType* args, HRESULT (ArgType::* method) (LPWSTR*))
    {
        // According to the API reference for WebView2, the result of any method with an LPWSTR
        // out-parameter should be freed by the caller using CoTaskMemFree.
        if (LPWSTR result{}; args != nullptr && SUCCEEDED ((args->*method) (&result)))
        {
            const ScopeGuard scope { [&] { CoTaskMemFree (result); } };
            return String { CharPointer_UTF16 { result } };
        }

        return {};
    }

    template <typename ArgType>
    static String getUriStringFromArgs (ArgType* args)
    {
        return callMethodWithLpwstrResult (args, &ArgType::get_Uri).value_or ("");
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
                    const auto uriString = callMethodWithLpwstrResult (sender, &ICoreWebView2::get_Source).value_or ("");

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
                            {
                                const auto adhocErrorPageUrl = "data:text/plain;charset=UTF-8," + errorString;

                                if (owner.lastURL == adhocErrorPageUrl)
                                {
                                    // We encountered an error while trying to navigate to the adhoc
                                    // error page. Trying to navigate to the error page again would
                                    // likely end us up in an infinite error callback loop, so we
                                    // early exit.
                                    //
                                    // Override WebBrowserComponent::pageLoadHadNetworkError and
                                    // return false to avoid such a loop, while still being able to
                                    // take action on the error if necessary.
                                    //
                                    // Receiving Error code: 9 can often be ignored safely with the
                                    // current WebView2 implementation.
                                    jassertfalse;

                                    return S_OK;
                                }

                                owner.goToURL (adhocErrorPageUrl);
                            }
                        }
                    }

                    return S_OK;
                }).Get(), &navigationCompletedToken);

            webView->AddWebResourceRequestedFilter (L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

            webView->add_WebResourceRequested (Callback<ICoreWebView2WebResourceRequestedEventHandler> (
                [this] (ICoreWebView2*, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT
                {
                    ComSmartPtr<ICoreWebView2WebResourceRequest> request;
                    args->get_Request (request.resetAndGetPointerAddress());

                    auto uriString = getUriStringFromArgs (request.get());

                    if (! urlRequest.url.isEmpty() && uriString == urlRequest.url
                        || (uriString.endsWith ("/") && uriString.upToLastOccurrenceOf ("/", false, false) == urlRequest.url))
                    {
                        String method ("GET");

                        if (! urlRequest.postData.isEmpty())
                        {
                            method = "POST";

                            auto content = becomeComSmartPtrOwner (SHCreateMemStream ((BYTE*) urlRequest.postData.getData(),
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

                    if (const auto resourceRequestUri = uriString.fromFirstOccurrenceOf ("https://juce.backend", false, false);
                        resourceRequestUri.isNotEmpty())
                    {
                        if (auto responseData = owner.impl->handleResourceRequest (resourceRequestUri))
                        {
                            auto stream = becomeComSmartPtrOwner (SHCreateMemStream ((BYTE*) responseData->data.data(),
                                                                                     (UINT) responseData->data.size()));

                            StringArray headers { "Content-Type: " + responseData->mimeType };

                            if (const auto allowedOrigin = owner.impl->options.getAllowedOrigin())
                                headers.add ("Access-Control-Allow-Origin: " + *allowedOrigin);

                            ComSmartPtr<ICoreWebView2WebResourceResponse> response;

                            if (webViewHandle.environment->CreateWebResourceResponse (stream,
                                                                                      200,
                                                                                      L"OK",
                                                                                      headers.joinIntoString ("\n").toUTF16(),
                                                                                      response.resetAndGetPointerAddress())
                                    != S_OK)
                            {
                                return E_FAIL;
                            }

                            args->put_Response (response);
                        }
                    }

                    return S_OK;
                }).Get(), &webResourceRequestedToken);

            webView->add_WebMessageReceived (Callback<ICoreWebView2WebMessageReceivedEventHandler> (
                                                 [this] (ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                                                 {
                                                     if (const auto str = callMethodWithLpwstrResult (args, &ICoreWebView2WebMessageReceivedEventArgs::TryGetWebMessageAsString))
                                                         owner.impl->handleNativeEvent (JSON::fromString (*str));

                                                     return S_OK;
                                                 }).Get(), &webMessageReceivedToken);
        }

        if (webViewController != nullptr)
        {
            webViewController->add_MoveFocusRequested (Callback<ICoreWebView2MoveFocusRequestedEventHandler> (
                [this] (ICoreWebView2Controller*, ICoreWebView2MoveFocusRequestedEventArgs* args) -> HRESULT
                {
                    ScopedValueSetter scope { inMoveFocusRequested, true };

                    auto* comp = [&]() -> Component*
                    {
                        auto* c = owner.getParentComponent();

                        if (c == nullptr)
                            return nullptr;

                        const auto traverser = c->createFocusTraverser();

                        if (COREWEBVIEW2_MOVE_FOCUS_REASON reason;
                            args->get_Reason (&reason) == S_OK && reason == COREWEBVIEW2_MOVE_FOCUS_REASON_PREVIOUS)
                        {
                            // The previous Component to the embedded WebView2 Component is the
                            // WebBrowserComponent. Here we want to skip that and jump to the
                            // Component that comes before it.
                            return traverser->getPreviousComponent (&owner);
                        }

                        // The Component that comes immediately after the WebBrowserComponent is the
                        // embedded WebView2. We want to jump to the Component that comes after that.
                        return traverser->getNextComponent (this);
                    }();

                    if (comp != nullptr)
                        comp->getAccessibilityHandler()->grabFocus();
                    else
                        giveAwayKeyboardFocus();

                    return S_OK;
                }).Get(), &moveFocusRequestedToken);
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

            if (webMessageReceivedToken.value != 0)
                webView->remove_WebMessageReceived (webMessageReceivedToken);
        }

        if (webViewController != nullptr)
        {
            if (moveFocusRequestedToken.value != 0)
                webViewController->remove_MoveFocusRequested (moveFocusRequestedToken);
        }
    }

    void setWebViewPreferences()
    {
        setControlVisible (owner.isShowing());

        ComSmartPtr<ICoreWebView2Controller2> controller2;
        webViewController->QueryInterface (controller2.resetAndGetPointerAddress());

        if (controller2 != nullptr)
        {
            const auto bgColour = preferences.getWinWebView2BackendOptions().getBackgroundColour();

            controller2->put_DefaultBackgroundColor ({ (BYTE) bgColour.getAlpha(),
                                                       (BYTE) bgColour.getRed(),
                                                       (BYTE) bgColour.getGreen(),
                                                       (BYTE) bgColour.getBlue() });
        }

        ComSmartPtr<ICoreWebView2Settings> settings;
        webView->get_Settings (settings.resetAndGetPointerAddress());

        if (settings != nullptr)
        {
           #if ! JUCE_DEBUG
            settings->put_AreDevToolsEnabled (false);
           #endif

            settings->put_IsStatusBarEnabled (! preferences.getWinWebView2BackendOptions().getIsStatusBarDisabled());
            settings->put_IsBuiltInErrorPageEnabled (! preferences.getWinWebView2BackendOptions().getIsBuiltInErrorPageDisabled());

            if (userAgent.isNotEmpty())
            {
                ComSmartPtr<ICoreWebView2Settings2> settings2;

                settings.QueryInterface (settings2);

                if (settings2 != nullptr)
                    settings2->put_UserAgent (userAgent.toWideCharPointer());
            }
        }
    }

    void createWebView()
    {
        if (auto* peer = getPeer())
        {
            // We enforce the serial creation of WebView2 instances so that our HWND association
            // logic can work. Multiple HWNDs can belong to the same browser process, so the only
            // way to identify which belongs to which WebView2 is to associate them with each other
            // in the order of creation.
            if (webView2ConstructionHelper.webView2BeingCreated != nullptr)
            {
                webView2ConstructionHelper.viewsWaitingForCreation.insert (this);
                return;
            }

            webView2ConstructionHelper.viewsWaitingForCreation.erase (this);
            webView2ConstructionHelper.webView2BeingCreated = this;

            webViewHandle.environment->CreateCoreWebView2Controller ((HWND) peer->getNativeHandle(),
                Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler> (
                    [weakThis = WeakReference<WebView2> { this }] (HRESULT, ICoreWebView2Controller* controller) -> HRESULT
                    {
                        if (weakThis != nullptr)
                        {
                            weakThis->triggerAsyncUpdate();
                            webView2ConstructionHelper.webView2BeingCreated = nullptr;

                            if (controller != nullptr)
                            {
                                weakThis->webViewController = addComSmartPtrOwner (controller);
                                controller->get_CoreWebView2 (weakThis->webView.resetAndGetPointerAddress());

                                auto allUserScripts = weakThis->userScripts;
                                allUserScripts.insert (0, platformSpecificIntegrationScript);

                                for (const auto& script : allUserScripts)
                                {
                                    weakThis->webView->AddScriptToExecuteOnDocumentCreated (script.toUTF16(),
                                                                                            Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler> (
                                                                                                [] (HRESULT error, PCWSTR) -> HRESULT
                                                                                                {
                                                                                                    if (error != S_OK)
                                                                                                        jassertfalse;

                                                                                                    return S_OK;
                                                                                                }).Get());
                                }

                                if (weakThis->webView != nullptr)
                                {
                                    if (UINT32 browserProcessId;
                                        weakThis->webView->get_BrowserProcessId (&browserProcessId) == S_OK)
                                    {
                                        auto* self = weakThis.get();
                                        auto* webView2WindowHandle = static_cast<HWND> (self->getWindowHandle());

                                        // There is no WebView2 API for getting the HWND hosting
                                        // the WebView2 content. So we iterate over all child
                                        // windows of the JUCE peer HWND, and try to figure out
                                        // which one belongs to a WebView2. What we are looking for
                                        // is a window that has a child window that belongs to the
                                        // browserProcessId.
                                        const auto directChildWindows = getDirectChildWindows (webView2WindowHandle);

                                        for (auto* childWindow : directChildWindows)
                                        {
                                            if (self->webView2ConstructionHelper.associatedWebViewNativeWindows.count (childWindow) == 0)
                                            {
                                                if (anyChildWindow (childWindow,
                                                                    [browserProcessId] (auto* childOfChild)
                                                                    {
                                                                        if (DWORD procId; GetWindowThreadProcessId (childOfChild, &procId) != 0)
                                                                            return (UINT32) procId == browserProcessId;

                                                                        return false;
                                                                    }))
                                                {
                                                    webView2ConstructionHelper.associatedWebViewNativeWindows.insert (childWindow);
                                                    AccessibilityHandler::setNativeChildForComponent (*self, childWindow);
                                                }
                                            }
                                        }
                                    }

                                    weakThis->addEventHandlers();
                                    weakThis->setWebViewPreferences();
                                    weakThis->componentMovedOrResized (true, true);

                                    if (weakThis->urlRequest.url.isNotEmpty())
                                        weakThis->webView->Navigate (weakThis->urlRequest.url.toWideCharPointer());
                                }
                            }

                            if (! weakThis->webView2ConstructionHelper.viewsWaitingForCreation.empty())
                                (*weakThis->webView2ConstructionHelper.viewsWaitingForCreation.begin())->triggerAsyncUpdate();
                        }

                        return S_OK;
                    }).Get());
        }
    }

    void closeWebView()
    {
        if (auto* webViewNativeWindow = AccessibilityHandler::getNativeChildForComponent (*this))
            webView2ConstructionHelper.associatedWebViewNativeWindows.erase (webViewNativeWindow);

        AccessibilityHandler::setNativeChildForComponent (*this, nullptr);

        if (webViewController != nullptr)
        {
            webViewController->Close();
            webViewController = nullptr;
            webView = nullptr;
        }

        webViewHandle.environment = nullptr;
    }

    //==============================================================================
    void handleAsyncUpdate() override
    {
        if (webView == nullptr)
        {
            createWebView();
            return;
        }

        while (! scriptsWaitingForExecution.empty())
        {
            auto [script, callback] = scriptsWaitingForExecution.front();
            scriptsWaitingForExecution.pop_front();
            evaluateJavascript (script, std::move (callback));
        }
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

            webViewController->put_Bounds ({ newBounds.getX(), newBounds.getY(),
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
    WebBrowserComponent::Options preferences;
    String userAgent;
    StringArray userScripts;

    WebViewHandle webViewHandle;
    ComSmartPtr<ICoreWebView2Controller> webViewController;
    ComSmartPtr<ICoreWebView2> webView;

    EventRegistrationToken navigationStartingToken   { 0 },
                           newWindowRequestedToken   { 0 },
                           windowCloseRequestedToken { 0 },
                           navigationCompletedToken  { 0 },
                           webResourceRequestedToken { 0 },
                           moveFocusRequestedToken   { 0 },
                           webMessageReceivedToken   { 0 };

    bool inMoveFocusRequested = false;

    struct URLRequest
    {
        String url;
        StringArray headers;
        MemoryBlock postData;
    };

    URLRequest urlRequest;

    struct WebView2ConstructionHelper
    {
        WebView2* webView2BeingCreated;
        std::set<WebView2*> viewsWaitingForCreation;
        std::set<void*> associatedWebViewNativeWindows;
    };

    inline static WebView2ConstructionHelper webView2ConstructionHelper;
    std::deque<std::pair<String, EvaluationCallback>> scriptsWaitingForExecution;

    NativeScaleFactorNotifier scaleFactorNotifier { this,
                                                    [this] (auto)
                                                    {
                                                        componentMovedOrResized (true, true);
                                                    } };

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (WebView2)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebView2)
};

#endif

//==============================================================================
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

//==============================================================================
bool WebBrowserComponent::areOptionsSupported (const Options& options)
{
    if (options.getBackend() == Options::Backend::defaultBackend || options.getBackend() == Options::Backend::ie)
        return true;

   #if JUCE_USE_WIN_WEBVIEW2
    if (options.getBackend() != Options::Backend::webview2)
        return false;

    if (auto webView = WebBrowserComponent::Impl::Platform::WebView2::createWebViewHandle (options))
        return true;
   #endif

    return false;
}

auto WebBrowserComponent::Impl::createAndInitPlatformDependentPart (WebBrowserComponent::Impl& impl,
                                                                    const WebBrowserComponent::Options& options,
                                                                    [[maybe_unused]] const StringArray& userScripts)
    -> std::unique_ptr<PlatformInterface>
{
    if (options.getBackend() == WebBrowserComponent::Options::Backend::webview2)
    {
       #if JUCE_USE_WIN_WEBVIEW2
        if (auto constructed = Platform::WebView2::tryConstruct (impl.owner, options, userScripts))
            return constructed;
       #endif
    }

    return std::make_unique<Platform::Win32WebView> (impl.owner, options.getUserAgent());
}

} // namespace juce
