/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_WEB_BROWSER


//==============================================================================
class WebBrowserComponentInternal   : public ActiveXControlComponent
{
public:
    //==============================================================================
    WebBrowserComponentInternal()
        : browser (nullptr),
          connectionPoint (nullptr),
          adviseCookie (0)
    {
    }

    ~WebBrowserComponentInternal()
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

        IConnectionPointContainer* connectionPointContainer = (IConnectionPointContainer*) queryInterface (&IID_IConnectionPointContainer);

        if (connectionPointContainer != nullptr)
        {
            connectionPointContainer->FindConnectionPoint (DIID_DWebBrowserEvents2,
                                                           &connectionPoint);

            if (connectionPoint != nullptr)
            {
                WebBrowserComponent* const owner = dynamic_cast <WebBrowserComponent*> (getParentComponent());
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
                LPSAFEARRAY sa = SafeArrayCreateVector (VT_UI1, 0, postData->getSize());

                if (sa != 0)
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
                               &flags, &frame,
                               &postDataVar, &headersVar);

            if (sa != 0)
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
    class EventHandler  : public ComBaseClassHelper <IDispatch>,
                          public ComponentMovementWatcher
    {
    public:
        EventHandler (WebBrowserComponent& owner_)
            : ComponentMovementWatcher (&owner_),
              owner (owner_)
        {
        }

        //==============================================================================
        HRESULT __stdcall GetTypeInfoCount (UINT*)                                  { return E_NOTIMPL; }
        HRESULT __stdcall GetTypeInfo (UINT, LCID, ITypeInfo**)                     { return E_NOTIMPL; }
        HRESULT __stdcall GetIDsOfNames (REFIID, LPOLESTR*, UINT, LCID, DISPID*)    { return E_NOTIMPL; }

        HRESULT __stdcall Invoke (DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS* pDispParams,
                                  VARIANT* /*pVarResult*/, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/)
        {
            if (dispIdMember == DISPID_BEFORENAVIGATE2)
            {
                VARIANT* const vurl = pDispParams->rgvarg[5].pvarVal;
                String url;

                if ((vurl->vt & VT_BYREF) != 0)
                    url = *vurl->pbstrVal;
                else
                    url = vurl->bstrVal;

                *pDispParams->rgvarg->pboolVal
                    = owner.pageAboutToLoad (url) ? VARIANT_FALSE
                                                  : VARIANT_TRUE;

                return S_OK;
            }

            return E_NOTIMPL;
        }

        void componentMovedOrResized (bool, bool )  {}
        void componentPeerChanged()                 {}
        void componentVisibilityChanged()           { owner.visibilityChanged(); }

        //==============================================================================
    private:
        WebBrowserComponent& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventHandler);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComponentInternal);
};


//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadPageWhenBrowserIsHidden_)
    : browser (nullptr),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    setOpaque (true);
    addAndMakeVisible (browser = new WebBrowserComponentInternal());
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

    lastHeaders.clear();
    if (headers != nullptr)
        lastHeaders = *headers;

    lastPostData.setSize (0);
    if (postData != nullptr)
        lastPostData = *postData;

    blankPageShown = false;

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    if (browser->browser != nullptr)
        browser->browser->Stop();
}

void WebBrowserComponent::goBack()
{
    lastURL = String::empty;
    blankPageShown = false;

    if (browser->browser != nullptr)
        browser->browser->GoBack();
}

void WebBrowserComponent::goForward()
{
    lastURL = String::empty;

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
        g.fillAll (Colours::white);
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
        lastURL = String::empty;
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

bool WebBrowserComponent::pageAboutToLoad (const String&)
{
    return true;
}

#endif
