/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
    InternalWebViewType() = default;
    virtual ~InternalWebViewType() = default;

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

//==============================================================================
class Win32WebView   : public InternalWebViewType,
                       public ActiveXControlComponent
{
public:
    Win32WebView (WebBrowserComponent& parent, const String& userAgentToUse)
        : owner (parent),
          userAgent (userAgentToUse)
    {
        owner.addAndMakeVisible (this);
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

    void createBrowser() override
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

    bool hasBrowserBeenCreated() override
    {
        return browser != nullptr;
    }

    void goToURL (const String& url, const StringArray* requestedHeaders, const MemoryBlock* postData) override
    {
        if (browser != nullptr)
        {
            VARIANT headerFlags, frame, postDataVar, headersVar;  // (_variant_t isn't available in all compilers)
            VariantInit (&headerFlags);
            VariantInit (&frame);
            VariantInit (&postDataVar);
            VariantInit (&headersVar);

            StringArray headers;

            if (userAgent.isNotEmpty())
                headers.add("User-Agent: " + userAgent);

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

    void focusGained() override
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

    using ActiveXControlComponent::focusGained;

    void setWebViewSize (int width, int height) override
    {
        setSize (width, height);
    }

private:
    WebBrowserComponent& owner;
    IWebBrowser2* browser = nullptr;
    IConnectionPoint* connectionPoint = nullptr;
    DWORD adviseCookie = 0;
    String userAgent;

    //==============================================================================
    struct EventHandler  : public ComBaseClassHelper<IDispatch>,
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
                V_VT( pVarResult ) = VT_BSTR;
                V_BSTR( pVarResult ) = SysAllocString ((const OLECHAR*) String(owner.userAgent).toWideCharPointer());;
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

#include <winuser.h>

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

class WebView2  : public InternalWebViewType,
                  public Component,
                  public ComponentMovementWatcher,
                  private AsyncUpdater
{
public:
    WebView2 (WebBrowserComponent& o, const WebBrowserComponent::Options& prefs)
         : ComponentMovementWatcher (&o),
           owner (o),
           preferences (prefs.getWinWebView2BackendOptions()),
           userAgent (prefs.getUserAgent())
    {
        if (auto handle = createWebViewHandle (preferences))
            webViewHandle = std::move (*handle);
        else
            throw std::runtime_error ("Failed to create the CoreWebView2Environment");

        owner.addAndMakeVisible (this);
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
        removeEventHandlers();
        closeWebView();
    }

    void createBrowser() override
    {
        if (webView == nullptr)
        {
            jassert (webViewHandle.environment != nullptr);
            createWebView();
        }
    }

    bool hasBrowserBeenCreated() override
    {
        return    webView != nullptr
               || webView2ConstructionHelper.webView2BeingCreated == this
               || webView2ConstructionHelper.viewsWaitingForCreation.count (this) > 0;
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

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
    }

    //==============================================================================
    struct WebViewHandle
    {
        using LibraryRef = std::unique_ptr<typename std::pointer_traits<HMODULE>::element_type, decltype(&::FreeLibrary)>;
        LibraryRef loaderHandle {nullptr, &::FreeLibrary};
        ComSmartPtr<ICoreWebView2Environment> environment;
    };

    static std::optional<WebViewHandle> createWebViewHandle (const WebBrowserComponent::Options::WinWebView2& options)
    {
        using CreateWebViewEnvironmentWithOptionsFunc = HRESULT (*) (PCWSTR, PCWSTR,
                                                                     ICoreWebView2EnvironmentOptions*,
                                                                     ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

        auto dllPath = options.getDLLLocation().getFullPathName();

        if (dllPath.isEmpty())
            dllPath = "WebView2Loader.dll";

        WebViewHandle result;

        result.loaderHandle = WebViewHandle::LibraryRef (LoadLibraryA (dllPath.toUTF8()), &::FreeLibrary);

        if (result.loaderHandle == nullptr)
            return {};

        auto* createWebViewEnvironmentWithOptions = (CreateWebViewEnvironmentWithOptionsFunc) GetProcAddress (result.loaderHandle.get(),
                                                                                                              "CreateCoreWebView2EnvironmentWithOptions");
        if (createWebViewEnvironmentWithOptions == nullptr)
            return {};

        auto webViewOptions = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
        const auto userDataFolder = options.getUserDataFolder().getFullPathName();

        auto hr = createWebViewEnvironmentWithOptions (nullptr,
                                                       userDataFolder.isNotEmpty() ? userDataFolder.toWideCharPointer() : nullptr,
                                                       webViewOptions.Get(),
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [&result] (HRESULT, ICoreWebView2Environment* env) -> HRESULT
                {
                    result.environment = env;
                    return S_OK;
                }).Get());

        if (! SUCCEEDED (hr))
            return {};

        return result;
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
        }

        if (webViewController != nullptr)
        {
            if (moveFocusRequestedToken.value != 0)
                webViewController->remove_MoveFocusRequested (moveFocusRequestedToken);
        }
    }

    void setWebViewPreferences()
    {
        ComSmartPtr<ICoreWebView2Controller2> controller2;
        webViewController->QueryInterface (controller2.resetAndGetPointerAddress());

        if (controller2 != nullptr)
        {
            const auto bgColour = preferences.getBackgroundColour();

            controller2->put_DefaultBackgroundColor ({ (BYTE) bgColour.getAlpha(),
                                                       (BYTE) bgColour.getRed(),
                                                       (BYTE) bgColour.getGreen(),
                                                       (BYTE) bgColour.getBlue() });
        }

        ComSmartPtr<ICoreWebView2Settings> settings;
        webView->get_Settings (settings.resetAndGetPointerAddress());

        if (settings != nullptr)
        {
            settings->put_IsStatusBarEnabled (! preferences.getIsStatusBarDisabled());
            settings->put_IsBuiltInErrorPageEnabled (! preferences.getIsBuiltInErrorPageDisabled());

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

            WeakReference<WebView2> weakThis (this);

            webViewHandle.environment->CreateCoreWebView2Controller ((HWND) peer->getNativeHandle(),
                Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler> (
                    [weakThis = WeakReference<WebView2> { this }] (HRESULT, ICoreWebView2Controller* controller) -> HRESULT
                    {
                        if (weakThis != nullptr)
                        {
                            webView2ConstructionHelper.webView2BeingCreated = nullptr;

                            if (controller != nullptr)
                            {
                                weakThis->webViewController = controller;
                                controller->get_CoreWebView2 (weakThis->webView.resetAndGetPointerAddress());

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
        createWebView();
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
    WebBrowserComponent::Options::WinWebView2 preferences;
    String userAgent;

    WebViewHandle webViewHandle;
    ComSmartPtr<ICoreWebView2Controller> webViewController;
    ComSmartPtr<ICoreWebView2> webView;

    EventRegistrationToken navigationStartingToken   { 0 },
                           newWindowRequestedToken   { 0 },
                           windowCloseRequestedToken { 0 },
                           navigationCompletedToken  { 0 },
                           webResourceRequestedToken { 0 },
                           moveFocusRequestedToken   { 0 };

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

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (WebView2)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebView2)
};

#endif

//==============================================================================
class WebBrowserComponent::Pimpl
{
public:
    Pimpl (WebBrowserComponent& owner,
           [[maybe_unused]] const Options& preferences,
           bool useWebView2,
           const String& userAgent)
    {
        if (useWebView2)
        {
           #if JUCE_USE_WIN_WEBVIEW2
            try
            {
                internal.reset (new WebView2 (owner, preferences));
            }
            catch (const std::runtime_error&) {}
           #endif
        }

        if (internal == nullptr)
            internal.reset (new Win32WebView (owner, userAgent));
    }

    InternalWebViewType& getInternalWebView()
    {
        return *internal;
    }

private:
    std::unique_ptr<InternalWebViewType> internal;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const Options& options)
    : browser (new Pimpl (*this, options,
                          options.getBackend() == Options::Backend::webview2, options.getUserAgent())),
      unloadPageWhenHidden (! options.keepsPageLoadedWhenBrowserIsHidden())
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
        if (browser != nullptr && unloadPageWhenHidden && ! blankPageShown)
        {
            // when the component becomes invisible, some stuff like flash
            // carries on playing audio, so we need to force it onto a blank
            // page to avoid this..

            blankPageShown = true;
            browser->getInternalWebView().goToURL ("about:blank", nullptr, nullptr);
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

void WebBrowserComponent::focusGainedWithDirection (FocusChangeType type, FocusChangeDirection dir)
{
    ignoreUnused (type, dir);

   #if JUCE_USE_WIN_WEBVIEW2
    if (auto* webView2 = dynamic_cast<WebView2*> (&browser->getInternalWebView()))
        webView2->focusGainedWithDirection (type, dir);
    else
   #endif
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

//==============================================================================
bool WebBrowserComponent::areOptionsSupported (const Options& options)
{
    if (options.getBackend() == Options::Backend::defaultBackend || options.getBackend() == Options::Backend::ie)
        return true;

   #if JUCE_USE_WIN_WEBVIEW2
    if (options.getBackend() != Options::Backend::webview2)
        return false;

    if (auto webView = WebView2::createWebViewHandle (options.getWinWebView2BackendOptions()))
        return true;
   #endif

    return false;
}

} // namespace juce
