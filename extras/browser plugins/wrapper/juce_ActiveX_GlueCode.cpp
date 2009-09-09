/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

//==============================================================================
/*
    This file contains all the gubbins to create an ActiveX browser plugin that
    wraps your BrowserPluginComponent object.

*/
//==============================================================================
#if _MSC_VER

//==============================================================================
#include <windows.h>
#include <windowsx.h>
#include <olectl.h>
#include <objsafe.h>
#include <exdisp.h>
#pragma warning (disable:4584)

#include "../../../juce_amalgamated.h"
#include "juce_BrowserPluginComponent.h"
#include "juce_IncludeBrowserPluginInfo.h"

#ifndef JuceBrowserPlugin_ActiveXCLSID
#error "For an activeX plugin, you need to define JuceBrowserPlugin_ActiveXCLSID in your BrowserPluginCharacteristics.h file!"
#endif

//==============================================================================
#if JUCE_DEBUG
static int numDOWID = 0, numJuceSO = 0;
#endif

#define log(a) DBG(a)

// Cunning trick used to add functions to export list without messing about with .def files.
#define EXPORTED_FUNCTION comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

//==============================================================================
static void juceVarToVariant (const var& v, VARIANT& dest);
static const var variantTojuceVar (const VARIANT& v);

//==============================================================================
// Takes care of the logic in invoking var methods from IDispatch callbacks.
class IDispatchHelper
{
public:
    IDispatchHelper() {}
    ~IDispatchHelper() {}

    var::identifier getId (const int hash) const
    {
        for (int i = knownIdentifiers.size(); --i >= 0;)
            if (knownIdentifiers.getUnchecked(i)->hashCode == hash)
                return *knownIdentifiers.getUnchecked(i);

        return var::identifier (String::empty);
    }

    var::identifier getId (const String& name)
    {
        for (int i = knownIdentifiers.size(); --i >= 0;)
            if (knownIdentifiers.getUnchecked(i)->name == name)
                return *knownIdentifiers.getUnchecked(i);

        const var::identifier v (name);
        knownIdentifiers.add (new var::identifier (v));
        return v;
    }

    HRESULT doGetIDsOfNames (LPOLESTR* rgszNames, UINT cNames, DISPID* rgDispId)
    {
        for (unsigned int i = 0; i < cNames; ++i)
        {
            var::identifier id (getId (rgszNames[i]));
            rgDispId[i] = (DISPID) id.hashCode;
        }

        return S_OK;
    }

    HRESULT doInvoke (const var& v,
                      DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
                      VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
    {
        const var::identifier memberId (getId ((int) dispIdMember));

        if (memberId.name.isEmpty() || v.getObject() == 0)
            return DISP_E_MEMBERNOTFOUND;

        if ((wFlags & DISPATCH_METHOD) != 0)
        {
            if (! v.getObject()->hasMethod (memberId))
                return DISP_E_MEMBERNOTFOUND;

            const int numArgs = pDispParams == 0 ? 0 : pDispParams->cArgs;
            var result;

            if (numArgs == 0)
            {
                result = v.call (memberId);
            }
            else
            {
                var* args = (var*) juce_calloc (sizeof (var) * numArgs);
                for (int j = 0; j < numArgs; ++j)
                    args[(numArgs - 1) - j] = variantTojuceVar (pDispParams->rgvarg[j]);

                result = v.invoke (memberId, args, numArgs);

                for (int j = 0; j < numArgs; ++j)
                    args[j] = var();

                juce_free (args);
            }

            if (pVarResult != 0)
                juceVarToVariant (result, *pVarResult);

            return S_OK;
        }
        else if ((wFlags & DISPATCH_PROPERTYGET) != 0)
        {
            if (! v.getObject()->hasProperty (memberId))
                return DISP_E_MEMBERNOTFOUND;

            if (pVarResult != 0)
            {
                juceVarToVariant (v.getObject()->getProperty (memberId), *pVarResult);
                return S_OK;
            }
        }
        else if ((wFlags & DISPATCH_PROPERTYPUT) != 0)
        {
            if (pDispParams != 0 && pDispParams->cArgs > 0)
            {
                v.getObject()->setProperty (memberId, variantTojuceVar (pDispParams->rgvarg[0]));
                return S_OK;
            }
        }

        return DISP_E_MEMBERNOTFOUND;
    }

private:
    OwnedArray <var::identifier> knownIdentifiers;

    IDispatchHelper (const IDispatchHelper&);
    const IDispatchHelper& operator= (const IDispatchHelper&);
};

//==============================================================================
// Makes a var look like an IDispatch
class IDispatchWrappingDynamicObject   : public IDispatch
{
public:
    IDispatchWrappingDynamicObject (const var& object_)
        : object (object_),
          refCount (0)
    {
        DBG ("num Juce wrapper objs: " + String (++numJuceSO));
    }

    virtual ~IDispatchWrappingDynamicObject()
    {
        DBG ("num Juce wrapper objs: " + String (--numJuceSO));
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown)         { AddRef(); *result = (IUnknown*) this; return S_OK; }
        else if (id == IID_IDispatch)   { AddRef(); *result = (IDispatch*) this; return S_OK; }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

    HRESULT __stdcall GetTypeInfoCount (UINT*)                  { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo (UINT, LCID, ITypeInfo**)     { return E_NOTIMPL; }

    HRESULT __stdcall GetIDsOfNames (REFIID riid, LPOLESTR* rgszNames, UINT cNames,
                                     LCID lcid, DISPID* rgDispId)
    {
        return iDispatchHelper.doGetIDsOfNames (rgszNames, cNames, rgDispId);
    }

    HRESULT __stdcall Invoke (DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                              DISPPARAMS* pDispParams, VARIANT* pVarResult,
                              EXCEPINFO* pExcepInfo, UINT* puArgErr)
    {
        return iDispatchHelper.doInvoke (object, dispIdMember, riid, lcid, wFlags, pDispParams,
                                         pVarResult, pExcepInfo, puArgErr);
    }

private:
    //==============================================================================
    var object;
    int refCount;
    IDispatchHelper iDispatchHelper;
};


//==============================================================================
// Makes an IDispatch look like a var
class DynamicObjectWrappingIDispatch   : public DynamicObject
{
    IDispatch* const source;

public:
    DynamicObjectWrappingIDispatch (IDispatch* const source_)
        : source (source_)
    {
        source->AddRef();
        log ("num IDispatch wrapper objs: " + String (++numDOWID));
    }

    ~DynamicObjectWrappingIDispatch()
    {
        source->Release();
        log ("num IDispatch wrapper objs: " + String (--numDOWID));
    }

    const var getProperty (const var::identifier& propertyName) const
    {
        LPCOLESTR name = (LPCOLESTR) propertyName.name;
        DISPID id = 0;
        if (source->GetIDsOfNames (IID_NULL, (LPOLESTR*)&name, 1, 0, &id) == S_OK)
        {
            EXCEPINFO excepInfo;
            DISPPARAMS params;
            zerostruct (params);
            UINT argError;
            VARIANT result;
            zerostruct (result);

            if (source->Invoke (id, IID_NULL, 0, DISPATCH_PROPERTYGET,
                                &params, &result, &excepInfo, &argError) == S_OK)
            {
                var v (variantTojuceVar (result));
                VariantClear (&result);
                return v;
            }
        }

        return var();
    }

    bool hasProperty (const var::identifier& propertyName) const
    {
        LPCOLESTR name = (LPCOLESTR) propertyName.name;
        DISPID id = 0;
        return source->GetIDsOfNames (IID_NULL, (LPOLESTR*)&name, 1, 0, &id) == S_OK;
    }

    void setProperty (const var::identifier& propertyName, const var& newValue)
    {
        LPCOLESTR name = (LPCOLESTR) propertyName.name;
        DISPID id = 0;
        if (source->GetIDsOfNames (IID_NULL, (LPOLESTR*)&name, 1, 0, &id) == S_OK)
        {
            VARIANT param;
            zerostruct (param);
            juceVarToVariant (newValue, param);

            DISPPARAMS dispParams;
            zerostruct (dispParams);
            dispParams.cArgs = 1;
            dispParams.rgvarg = &param;
            EXCEPINFO excepInfo;
            zerostruct (excepInfo);

            VARIANT result;
            zerostruct (result);
            UINT argError = 0;

            if (source->Invoke (id, IID_NULL, 0, DISPATCH_PROPERTYPUT,
                                &dispParams, &result, &excepInfo, &argError) == S_OK)
            {
                VariantClear (&result);
            }

            VariantClear (&param);
        }
    }

    void removeProperty (const var::identifier& propertyName)
    {
        setProperty (propertyName, var());
    }

    bool hasMethod (const var::identifier& methodName) const
    {
        LPCOLESTR name = (LPCOLESTR) methodName.name;
        DISPID id = 0;
        return source->GetIDsOfNames (IID_NULL, (LPOLESTR*)&name, 1, 0, &id) == S_OK;
    }

    const var invokeMethod (const var::identifier& methodName,
                            const var* parameters,
                            int numParameters)
    {
        var returnValue;
        LPCOLESTR name = (LPCOLESTR) methodName.name;
        DISPID id = 0;
        if (source->GetIDsOfNames (IID_NULL, (LPOLESTR*)&name, 1, 0, &id) == S_OK)
        {
            VARIANT* params = (VARIANT*) juce_calloc (sizeof (VARIANT) * (numParameters + 1));

            for (int i = 0; i < numParameters; ++i)
                juceVarToVariant (parameters[(numParameters - 1) - i], params[i]);

            DISPPARAMS dispParams;
            zerostruct (dispParams);
            dispParams.cArgs = numParameters;
            dispParams.rgvarg = params;

            EXCEPINFO excepInfo;
            zerostruct (excepInfo);

            VARIANT result;
            zerostruct (result);
            UINT argError = 0;

            if (source->Invoke (id, IID_NULL, 0, DISPATCH_METHOD,
                                &dispParams, &result, &excepInfo, &argError) == S_OK)
            {
                returnValue = variantTojuceVar (result);
                VariantClear (&result);
            }

            juce_free (params);
        }

        return returnValue;
    }
};


//==============================================================================
void juceVarToVariant (const var& v, VARIANT& dest)
{
    if (v.isVoid())
    {
        dest.vt = VT_EMPTY;
    }
    else if (v.isInt())
    {
        dest.vt = VT_INT;
        dest.intVal = (int) v;
    }
    else if (v.isBool())
    {
        dest.vt = VT_BOOL;
        dest.boolVal = (int) v;
    }
    else if (v.isDouble())
    {
        dest.vt = VT_R8;
        dest.dblVal = (double) v;
    }
    else if (v.isString())
    {
        dest.vt = VT_BSTR;
        dest.bstrVal = SysAllocString (v.toString());
    }
    else if (v.isObject())
    {
        dest.vt = VT_DISPATCH;
        dest.pdispVal = new IDispatchWrappingDynamicObject (v);
    }
    else if (v.isMethod())
    {
        dest.vt = VT_EMPTY;
    }
}

const var variantTojuceVar (const VARIANT& v)
{
    if ((v.vt & VT_ARRAY) != 0)
    {
        //xxx
    }
    else
    {
        switch (v.vt & ~VT_BYREF)
        {
        case VT_VOID:
        case VT_EMPTY:      return var();
        case VT_I1:         return var ((int) v.cVal);
        case VT_I2:         return var ((int) v.iVal);
        case VT_I4:         return var ((int) v.lVal);
        case VT_I8:         return var (String (v.llVal));
        case VT_UI1:        return var ((int) v.bVal);
        case VT_UI2:        return var ((int) v.uiVal);
        case VT_UI4:        return var ((int) v.ulVal);
        case VT_UI8:        return var (String (v.ullVal));
        case VT_INT:        return var ((int) v.intVal);
        case VT_UINT:       return var ((int) v.uintVal);
        case VT_R4:         return var ((double) v.fltVal);
        case VT_R8:         return var ((double) v.dblVal);
        case VT_BSTR:       return var (v.bstrVal);
        case VT_BOOL:       return var (v.boolVal ? true : false);
        case VT_DISPATCH:   return var (new DynamicObjectWrappingIDispatch (v.pdispVal));
        default:
            break;
        }
    }

    return var();
}

//==============================================================================
// This acts as the embedded HWND
class AXBrowserPluginHolderComponent    : public Component
{
public:
    //==============================================================================
    AXBrowserPluginHolderComponent()
        : child (0),
          parentHWND (0),
          browser (0)
    {
        setOpaque (true);
        setWantsKeyboardFocus (false);

        addAndMakeVisible (child = createBrowserPlugin());
        jassert (child != 0);   // You have to create one of these!
    }

    ~AXBrowserPluginHolderComponent()
    {
        setWindow (0);
        deleteAndZero (child);
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        if (child == 0 || ! child->isOpaque())
            g.fillAll (Colours::white);
    }

    void resized()
    {
        if (child != 0)
            child->setBounds (0, 0, getWidth(), getHeight());
    }

    const var getObject()   { return child->getJavascriptObject(); }

    void setWindow (IOleInPlaceSite* site)
    {
        if (browser != 0)
        {
            browser->Release();
            browser = 0;
        }

        HWND newHWND = 0;

        if (site != 0)
        {
            site->GetWindow (&newHWND);

            IServiceProvider* sp = 0;
            site->QueryInterface (IID_IServiceProvider, (void**) &sp);

            if (sp != 0)
            {
                sp->QueryService (IID_IWebBrowserApp, IID_IWebBrowser2, (void**) &browser);
                sp->Release();
            }
        }

        if (parentHWND != newHWND)
        {
            removeFromDesktop();
            setVisible (false);

            parentHWND = newHWND;

            if (parentHWND != 0)
            {
                addToDesktop (0);

                HWND ourHWND = (HWND) getWindowHandle();
                SetParent (ourHWND, parentHWND);
                DWORD val = GetWindowLong (ourHWND, GWL_STYLE);
                val = (val & ~WS_POPUP) | WS_CHILD;
                SetWindowLong (ourHWND, GWL_STYLE, val);

                setVisible (true);
            }
        }

        if (site != 0)
            site->OnInPlaceActivate();
    }

    const String getBrowserURL() const
    {
        if (browser == 0)
            return String::empty;

        BSTR url = 0;
        browser->get_LocationURL (&url);
        return URL::removeEscapeChars (url);
    }

private:
    //==============================================================================
    BrowserPluginComponent* child;
    HWND parentHWND;
    IWebBrowser2* browser;
};

//==============================================================================
extern String browserVersionDesc;

static const String getExePath()
{
    TCHAR moduleFile [2048];
    moduleFile[0] = 0;
    GetModuleFileName (0, moduleFile, 2048);
    return moduleFile;
}

static const String getExeVersion (const String& exeFileName, const String& fieldName)
{
    String resultString;
    DWORD pointlessWin32Variable;
    DWORD size = GetFileVersionInfoSize (exeFileName, &pointlessWin32Variable);

    if (size > 0)
    {
        void* const exeInfo = juce_calloc (size);

        if (GetFileVersionInfo (exeFileName, 0, size, exeInfo))
        {
            TCHAR* result = 0;
            unsigned int resultLen = 0;

            // try the 1200 codepage (Unicode)
            String queryStr ("\\StringFileInfo\\040904B0\\" + fieldName);

            if (! VerQueryValue (exeInfo, (LPTSTR) (LPCTSTR) queryStr, (void**) &result, &resultLen))
            {
                // try the 1252 codepage (Windows Multilingual)
                queryStr = "\\StringFileInfo\\040904E4\\" + fieldName;
                VerQueryValue (exeInfo, (LPTSTR) (LPCTSTR) queryStr, (void**) &result, &resultLen);
            }

            resultString = String (result, resultLen);
        }

        juce_free (exeInfo);
    }

    return resultString;
}

static int numActivePlugins = 0;

class JuceActiveXObject     : public IUnknown,
                              public IDispatch,
                              public IObjectWithSite,
                              public IObjectSafety,
                              public IOleInPlaceObject
{
public:
    JuceActiveXObject()
        : refCount (0)
    {
        log ("JuceActiveXObject");
        site = 0;
        holderComp = 0;
    }

    ~JuceActiveXObject()
    {
        deleteHolderComp();
        log ("~JuceActiveXObject");
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown)                 { AddRef(); *result = (IUnknown*) this; return S_OK; }
        else if (id == IID_IDispatch)           { AddRef(); *result = (IDispatch*) this; return S_OK; }
        else if (id == IID_IObjectWithSite)     { AddRef(); *result = (IObjectWithSite*) this; return S_OK; }
        else if (id == IID_IObjectSafety)       { AddRef(); *result = (IObjectSafety*) this; return S_OK; }
        else if (id == IID_IOleInPlaceObject)   { AddRef(); *result = (IOleInPlaceObject*) this; return S_OK; }
        else if (id == IID_IOleWindow)          { AddRef(); *result = (IOleWindow*) (IOleInPlaceObject*) this; return S_OK; }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

    HRESULT __stdcall GetTypeInfoCount (UINT* pctinfo)                              { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo (UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)     { return E_NOTIMPL; }

    HRESULT __stdcall GetIDsOfNames (REFIID riid, LPOLESTR* rgszNames, UINT cNames,
                                     LCID lcid, DISPID* rgDispId)
    {
        return iDispatchHelper.doGetIDsOfNames (rgszNames, cNames, rgDispId);
    }

    HRESULT __stdcall Invoke (DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                              DISPPARAMS* pDispParams, VARIANT* pVarResult,
                              EXCEPINFO* pExcepInfo, UINT* puArgErr)
    {
        if (holderComp == 0)
            return DISP_E_MEMBERNOTFOUND;

        return iDispatchHelper.doInvoke (holderComp->getObject(),
                                         dispIdMember, riid, lcid, wFlags, pDispParams,
                                         pVarResult, pExcepInfo, puArgErr);
    }

    HRESULT __stdcall SetSite (IUnknown* newSite)
    {
        if (newSite != site)
        {
            if (site != 0)
                site->Release();

            site = newSite;

            if (site != 0)
            {
                site->AddRef();

                IOleInPlaceSite* inPlaceSite = 0;
                site->QueryInterface (IID_IOleInPlaceSite, (void**) &inPlaceSite);

                if (inPlaceSite != 0)
                {
                    createHolderComp();

                    holderComp->setWindow (inPlaceSite);
                    inPlaceSite->Release();
                }
                else
                {
                    deleteHolderComp();
                }
            }
            else
            {
                deleteHolderComp();
            }
        }

        return S_OK;
    }

    void createHolderComp()
    {
        if (numActivePlugins++ == 0)
        {
            log ("initialiseJuce_GUI()");
            initialiseJuce_GUI();

            browserVersionDesc = "Internet Explorer " + getExeVersion (getExePath(), "FileVersion");
        }

        if (holderComp == 0)
            holderComp = new AXBrowserPluginHolderComponent();
    }

    void deleteHolderComp()
    {
        deleteAndZero (holderComp);

        if (--numActivePlugins == 0)
        {
            log ("shutdownJuce_GUI()");
            shutdownJuce_GUI();
        }
    }

    HRESULT __stdcall GetSite (REFIID riid, void **ppvSite)
    {
        *ppvSite = site;
        return S_OK;
    }

    //==============================================================================
    HRESULT __stdcall SetObjectRects (LPCRECT r, LPCRECT c)
    {
        if (holderComp != 0)
            holderComp->setBounds (r->left, r->top, r->right - r->left, r->bottom - r->top);

        return S_OK;
    }

    HRESULT __stdcall GetWindow (HWND* phwnd)
    {
        if (holderComp == 0)
            return E_NOTIMPL;

        *phwnd = (HWND) holderComp->getWindowHandle();
        return S_OK;
    }

    //==============================================================================
    HRESULT __stdcall ContextSensitiveHelp (BOOL fEnterMode)    { return E_NOTIMPL; }
    HRESULT __stdcall InPlaceDeactivate()                       { return E_NOTIMPL; }
    HRESULT __stdcall UIDeactivate()                            { return E_NOTIMPL; }
    HRESULT __stdcall ReactivateAndUndo()                       { return E_NOTIMPL; }

    //==============================================================================
    HRESULT __stdcall GetInterfaceSafetyOptions (REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
    {
        *pdwSupportedOptions = *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA;
        return S_OK;
    }

    HRESULT __stdcall SetInterfaceSafetyOptions (REFIID, DWORD, DWORD)      { return S_OK; }

private:
    IUnknown* site;
    int refCount;
    AXBrowserPluginHolderComponent* holderComp;
    IDispatchHelper iDispatchHelper;

    JuceActiveXObject (const JuceActiveXObject&);
    const JuceActiveXObject& operator= (const JuceActiveXObject&);
};

//==============================================================================
class JuceActiveXObjectFactory     : public IUnknown,
                                     public IClassFactory
{
public:
    JuceActiveXObjectFactory()  : refCount (0)   {}
    ~JuceActiveXObjectFactory()  {}

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown)             { AddRef(); *result = (IUnknown*) this; return S_OK; }
        else if (id == IID_IClassFactory)   { AddRef(); *result = (IClassFactory*) this; return S_OK; }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

    HRESULT __stdcall CreateInstance (IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
    {
        *ppvObject = 0;

        if (pUnkOuter != 0 && riid != IID_IUnknown)
            return CLASS_E_NOAGGREGATION;

        JuceActiveXObject* ax = new JuceActiveXObject();
        return ax->QueryInterface (riid, ppvObject);
    }

    HRESULT __stdcall LockServer (BOOL /*fLock*/)    { return S_OK; }

private:
    int refCount;

    JuceActiveXObjectFactory (const JuceActiveXObjectFactory&);
    const JuceActiveXObjectFactory& operator= (const JuceActiveXObjectFactory&);
};

//==============================================================================
const String getActiveXBrowserURL (const BrowserPluginComponent* comp)
{
    AXBrowserPluginHolderComponent* const ax = dynamic_cast <AXBrowserPluginHolderComponent*> (comp->getParentComponent());
    return ax != 0 ? ax->getBrowserURL() : String::empty;
}

//==============================================================================
extern "C" BOOL WINAPI DllMain (HANDLE instance, DWORD reason, LPVOID)
{
    #pragma EXPORTED_FUNCTION

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        log ("DLL_PROCESS_ATTACH");
        PlatformUtilities::setCurrentModuleInstanceHandle (instance);
        break;

    case DLL_PROCESS_DETACH:
        log ("DLL_PROCESS_DETACH");
        browserVersionDesc = String::empty;

        // IE has a tendency to leak our objects, so although none of this should be
        // necessary, it's best to make sure..
        jassert (numActivePlugins == 0);
        shutdownJuce_GUI();
        break;

    default:
        break;
    }

    return TRUE;
}

static const String CLSIDToJuceString (REFCLSID clsid)
{
    LPWSTR s = 0;
    StringFromIID (clsid, &s);

    if (s == 0)
        return String::empty;

    const String result (s);
    LPMALLOC malloc;
    CoGetMalloc (1, &malloc);
    if (malloc != 0)
    {
        malloc->Free (s);
        malloc->Release();
    }

    return result.removeCharacters (T("{}")).trim();
}

STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    #pragma EXPORTED_FUNCTION

    *ppv = 0;

    if (CLSIDToJuceString (rclsid).equalsIgnoreCase (String (JuceBrowserPlugin_ActiveXCLSID)))
    {
        JuceActiveXObjectFactory* afx = new JuceActiveXObjectFactory();
        if (afx->QueryInterface (riid, ppv) == S_OK)
            return S_OK;

        delete afx;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow()
{
    #pragma EXPORTED_FUNCTION
    return S_OK;
}

//==============================================================================
static const String makeLegalRegistryName (const String& s)
{
    return s.retainCharacters (T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_."));
}

static HRESULT doRegistration (const bool unregister)
{
    const String company (makeLegalRegistryName (JuceBrowserPlugin_Company));
    const String plugin (makeLegalRegistryName (JuceBrowserPlugin_Name));
    const String clsID ("{" + String (JuceBrowserPlugin_ActiveXCLSID).toUpperCase() + "}");
    const String root ("HKEY_CLASSES_ROOT\\");
    const String companyDotPlugin (company + "." + plugin);
    const String companyDotPluginCur (companyDotPlugin + ".1");
    const String clsIDRoot (root + "CLSID\\" + clsID + "\\");
    const String dllPath (File::getSpecialLocation (File::currentApplicationFile).getFullPathName());

    StringPairArray settings;
    settings.set (root + companyDotPluginCur + "\\", JuceBrowserPlugin_Name);
    settings.set (root + companyDotPluginCur + "\\CLSID\\", clsID);
    settings.set (root + companyDotPlugin + "\\", JuceBrowserPlugin_Name);
    settings.set (root + companyDotPlugin + "\\CLSID\\", clsID);
    settings.set (root + companyDotPlugin + "\\CurVer\\", companyDotPluginCur);
    settings.set (clsIDRoot, JuceBrowserPlugin_Name);
    settings.set (clsIDRoot + "Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}\\", String::empty);
    settings.set (clsIDRoot + "Implemented Categories\\{7DD95802-9882-11CF-9FA9-00AA006C42C4}\\", String::empty);
    settings.set (clsIDRoot + "ProgID\\", companyDotPluginCur);
    settings.set (clsIDRoot + "VersionIndependentProgID\\", companyDotPlugin);
    settings.set (clsIDRoot + "Programmable\\", String::empty);
    settings.set (clsIDRoot + "InProcServer32\\", dllPath);
    settings.set (clsIDRoot + "InProcServer32\\ThreadingModel", "Apartment");
    settings.set (clsIDRoot + "Control\\", String::empty);
    settings.set (clsIDRoot + "Insertable\\", String::empty);
    settings.set (clsIDRoot + "ToolboxBitmap32\\", dllPath + ", 101");
    settings.set (clsIDRoot + "TypeLib\\", "");
    settings.set (clsIDRoot + "Version\\", JuceBrowserPlugin_Version);

    if (unregister)
    {
        for (int i = 0; i < settings.getAllKeys().size(); ++i)
            PlatformUtilities::deleteRegistryValue (settings.getAllKeys()[i]);

        PlatformUtilities::deleteRegistryKey (root + companyDotPluginCur);
        PlatformUtilities::deleteRegistryKey (root + companyDotPlugin);
        PlatformUtilities::deleteRegistryKey (clsIDRoot);

        if (PlatformUtilities::registryValueExists (clsIDRoot + "InProcServer32"))
            return SELFREG_E_CLASS;
    }
    else
    {
        PlatformUtilities::deleteRegistryKey (clsIDRoot);

        for (int i = 0; i < settings.getAllKeys().size(); ++i)
            PlatformUtilities::setRegistryValue (settings.getAllKeys()[i],
                                                 settings [settings.getAllKeys()[i]]);

        // check whether the registration actually worked - if not, we probably don't have
        // enough privileges to write to the registry..
        if (PlatformUtilities::getRegistryValue (clsIDRoot + "InProcServer32\\") != dllPath)
            return SELFREG_E_CLASS;
    }

    return S_OK;
}

STDAPI DllRegisterServer()
{
    #pragma EXPORTED_FUNCTION
    return doRegistration (false);
}

STDAPI DllUnregisterServer()
{
    #pragma EXPORTED_FUNCTION
    return doRegistration (true);
}

#endif
