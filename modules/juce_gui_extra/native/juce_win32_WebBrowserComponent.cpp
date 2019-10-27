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

class WebBrowserComponent::Pimpl   : public ActiveXControlComponent
{
public:
    Pimpl() {}

    ~Pimpl()
    {
        if (connectionPoint != nullptr)
            connectionPoint->Unadvise (adviseCookie);

        if (browser != nullptr)
            browser->Release();
    }

    void createBrowser()
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
                auto* owner = dynamic_cast<WebBrowserComponent*> (getParentComponent());
                jassert (owner != nullptr);

                auto handler = new EventHandler (*owner);
                connectionPoint->Advise (handler, &adviseCookie);
                handler->Release();
            }
        }
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
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

    //==============================================================================
    IWebBrowser2* browser = nullptr;

private:
    IConnectionPoint* connectionPoint = nullptr;
    DWORD adviseCookie = 0;

    //==============================================================================
    struct EventHandler  : public ComBaseClassHelper<IDispatch>,
                           public ComponentMovementWatcher
    {
        EventHandler (WebBrowserComponent& w)  : ComponentMovementWatcher (&w), owner (w) {}

        JUCE_COMRESULT GetTypeInfoCount (UINT*)                                  { return E_NOTIMPL; }
        JUCE_COMRESULT GetTypeInfo (UINT, LCID, ITypeInfo**)                     { return E_NOTIMPL; }
        JUCE_COMRESULT GetIDsOfNames (REFIID, LPOLESTR*, UINT, LCID, DISPID*)    { return E_NOTIMPL; }

        JUCE_COMRESULT Invoke (DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS* pDispParams,
                               VARIANT* /*pVarResult*/, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/)
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
                // report only network erros
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadPageWhenBrowserIsHidden_)
    : browser (new Pimpl()),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    setOpaque (true);
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

    if (browser->browser == nullptr)
        checkWindowAssociation();

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    if (browser->browser != nullptr)
        browser->browser->Stop();
}

void WebBrowserComponent::goBack()
{
    lastURL.clear();
    blankPageShown = false;

    if (browser->browser != nullptr)
        browser->browser->GoBack();
}

void WebBrowserComponent::goForward()
{
    lastURL.clear();

    if (browser->browser != nullptr)
        browser->browser->GoForward();
}

void WebBrowserComponent::refresh()
{
    if (browser->browser != nullptr)
        browser->browser->Refresh();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
    if (browser->browser == nullptr)
    {
        g.fillAll (Colours::white);
        checkWindowAssociation();
    }
}

void WebBrowserComponent::checkWindowAssociation()
{
    if (isShowing())
    {
        if (browser->browser == nullptr && getPeer() != nullptr)
        {
            browser->createBrowser();
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
            browser->goToURL ("about:blank", 0, 0);
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
    browser->setSize (getWidth(), getHeight());
}

void WebBrowserComponent::visibilityChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::focusGained (FocusChangeType)
{
    auto iidOleObject = __uuidof (IOleObject);
    auto iidOleWindow = __uuidof (IOleWindow);

    if (auto oleObject = (IOleObject*) browser->queryInterface (&iidOleObject))
    {
        if (auto oleWindow = (IOleWindow*) browser->queryInterface (&iidOleWindow))
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
