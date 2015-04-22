/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

class WebBrowserComponent::Pimpl   : public ActiveXControlComponent
{
public:
    Pimpl()
      : browser (nullptr),
        connectionPoint (nullptr),
        adviseCookie (0)
    {
    }

    ~Pimpl()
    {
        if (connectionPoint != nullptr)
            connectionPoint->Unadvise (adviseCookie);

        if (browser != nullptr)
            browser->Release();
    }

    void createBrowser()
    {
        createControl (&CLSID_WebBrowser);
        browser = (IWebBrowser2*) queryInterface (&IID_IWebBrowser2);

        if (IConnectionPointContainer* connectionPointContainer
                = (IConnectionPointContainer*) queryInterface (&IID_IConnectionPointContainer))
        {
            connectionPointContainer->FindConnectionPoint (DIID_DWebBrowserEvents2, &connectionPoint);

            if (connectionPoint != nullptr)
            {
                WebBrowserComponent* const owner = dynamic_cast<WebBrowserComponent*> (getParentComponent());
                jassert (owner != nullptr);

                EventHandler* handler = new EventHandler (*owner);
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
            LPSAFEARRAY sa = nullptr;

            VARIANT flags, frame, postDataVar, headersVar;  // (_variant_t isn't available in all compilers)
            VariantInit (&flags);
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
                sa = SafeArrayCreateVector (VT_UI1, 0, (ULONG) postData->getSize());

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

                        postDataVar = postDataVar2;
                    }
                }
            }

            browser->Navigate ((BSTR) (const OLECHAR*) url.toWideCharPointer(),
                               &flags, &frame, &postDataVar, &headersVar);

            if (sa != nullptr)
                SafeArrayDestroy (sa);

            VariantClear (&flags);
            VariantClear (&frame);
            VariantClear (&postDataVar);
            VariantClear (&headersVar);
        }
    }

    //==============================================================================
    IWebBrowser2* browser;

private:
    IConnectionPoint* connectionPoint;
    DWORD adviseCookie;

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
    : browser (nullptr),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    setOpaque (true);
    addAndMakeVisible (browser = new Pimpl());
}

WebBrowserComponent::~WebBrowserComponent()
{
    delete browser;
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
    if (IOleObject* oleObject = (IOleObject*) browser->queryInterface (&IID_IOleObject))
    {
        if (IOleWindow* oleWindow = (IOleWindow*) browser->queryInterface (&IID_IOleWindow))
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
