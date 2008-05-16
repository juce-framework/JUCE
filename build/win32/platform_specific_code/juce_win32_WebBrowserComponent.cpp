/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

//==============================================================================
#ifdef _MSC_VER
  #pragma warning (disable: 4514)
  #pragma warning (push)
#endif

#include "win32_headers.h"
#include <comutil.h>
#include <Exdisp.h>
#include <exdispid.h>

#ifdef _MSC_VER
  #pragma warning (pop)
  #pragma warning (disable: 4312 4244)
#endif


#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/gui/components/special/juce_WebBrowserComponent.h"
#include "../../../src/juce_appframework/gui/components/special/juce_ActiveXControlComponent.h"


//==============================================================================
class WebBrowserComponentInternal   : public ActiveXControlComponent
{
public:
    //==============================================================================
    WebBrowserComponentInternal()
        : browser (0),
          connectionPoint (0),
          adviseCookie (0)
    {
    }

    ~WebBrowserComponentInternal()
    {
        if (connectionPoint != 0)
            connectionPoint->Unadvise (adviseCookie);

        if (browser != 0)
            browser->Release();
    }

    void createBrowser()
    {
        createControl (&CLSID_WebBrowser);
        browser = (IWebBrowser2*) queryInterface (&IID_IWebBrowser2);

        IConnectionPointContainer* connectionPointContainer = (IConnectionPointContainer*) queryInterface (&IID_IConnectionPointContainer);

        if (connectionPointContainer != 0)
        {
            connectionPointContainer->FindConnectionPoint (DIID_DWebBrowserEvents2,
                                                           &connectionPoint);

            if (connectionPoint != 0)
            {
                WebBrowserComponent* const owner = dynamic_cast <WebBrowserComponent*> (getParentComponent());
                jassert (owner != 0);

                EventHandler* handler = new EventHandler (owner);
                connectionPoint->Advise (handler, &adviseCookie);
            }
        }
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        if (browser != 0)
        {
            LPSAFEARRAY sa = 0;
            _variant_t flags, frame, postDataVar, headersVar;

            if (headers != 0)
                headersVar = (const tchar*) headers->joinIntoString ("\r\n");

            if (postData != 0 && postData->getSize() > 0)
            {
                LPSAFEARRAY sa = SafeArrayCreateVector (VT_UI1, 0, postData->getSize());

                if (sa != 0)
                {
                    void* data = 0;
                    SafeArrayAccessData (sa, &data);
                    jassert (data != 0);

                    if (data != 0)
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

            browser->Navigate ((BSTR) (const OLECHAR*) url,
                               &flags, &frame,
                               &postDataVar, &headersVar);

            if (sa != 0)
                SafeArrayDestroy (sa);
        }
    }

    //==============================================================================
    IWebBrowser2* browser;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    IConnectionPoint* connectionPoint;
    DWORD adviseCookie;

    //==============================================================================
    class EventHandler  : public IDispatch
    {
    public:
        EventHandler (WebBrowserComponent* owner_)
            : owner (owner_),
              refCount (0)
        {
        }

        ~EventHandler()
        {
        }

        //==============================================================================
        HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
        {
            if (id == IID_IUnknown || id == IID_IDispatch || id == DIID_DWebBrowserEvents2)
            {
                AddRef();
                *result = this;
                return S_OK;
            }

            *result = 0;
            return E_NOINTERFACE;
        }

        ULONG __stdcall AddRef()    { return ++refCount; }
        ULONG __stdcall Release()   { jassert (refCount > 0); const int r = --refCount; if (r == 0) delete this; return r; }

        HRESULT __stdcall GetTypeInfoCount (UINT __RPC_FAR*)                                            { return E_NOTIMPL; }
        HRESULT __stdcall GetTypeInfo (UINT, LCID, ITypeInfo __RPC_FAR *__RPC_FAR*)                     { return E_NOTIMPL; }
        HRESULT __stdcall GetIDsOfNames (REFIID, LPOLESTR __RPC_FAR*, UINT, LCID, DISPID __RPC_FAR*)    { return E_NOTIMPL; }

        HRESULT __stdcall Invoke (DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/,
                                  WORD /*wFlags*/, DISPPARAMS __RPC_FAR* pDispParams,
                                  VARIANT __RPC_FAR* /*pVarResult*/, EXCEPINFO __RPC_FAR* /*pExcepInfo*/,
                                  UINT __RPC_FAR* /*puArgErr*/)
        {
            switch (dispIdMember)
            {
                case DISPID_BEFORENAVIGATE2:
                {
                    VARIANT* const vurl = pDispParams->rgvarg[5].pvarVal;

                    String url;

                    if ((vurl->vt & VT_BYREF) != 0)
                        url = *vurl->pbstrVal;
                    else
                        url = vurl->bstrVal;

                    *pDispParams->rgvarg->pboolVal
                        = owner->pageAboutToLoad (url) ? VARIANT_FALSE
                                                       : VARIANT_TRUE;

                    return S_OK;
                }

                default:
                    break;
            }

            return E_NOTIMPL;
        }

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        WebBrowserComponent* const owner;
        int refCount;

        EventHandler (const EventHandler&);
        const EventHandler& operator= (const EventHandler&);
    };
};



//==============================================================================
WebBrowserComponent::WebBrowserComponent()
    : browser (0),
      blankPageShown (false)
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
    if (headers != 0)
        lastHeaders = *headers;

    lastPostData.setSize (0);
    if (postData != 0)
        lastPostData = *postData;

    blankPageShown = false;

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    if (browser->browser != 0)
        browser->browser->Stop();
}

void WebBrowserComponent::goBack()
{
    lastURL = String::empty;
    blankPageShown = false;

    if (browser->browser != 0)
        browser->browser->GoBack();
}

void WebBrowserComponent::goForward()
{
    lastURL = String::empty;

    if (browser->browser != 0)
        browser->browser->GoForward();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
    if (browser->browser == 0)
        g.fillAll (Colours::white);
}

void WebBrowserComponent::checkWindowAssociation()
{
    if (isShowing())
    {
        if (blankPageShown)
            goBack();

        if (browser->browser == 0 && getPeer() != 0)
        {
            browser->createBrowser();
            reloadLastURL();
        }
    }
    else
    {
        if (browser != 0 && ! blankPageShown)
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

void WebBrowserComponent::moved()
{
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

END_JUCE_NAMESPACE
