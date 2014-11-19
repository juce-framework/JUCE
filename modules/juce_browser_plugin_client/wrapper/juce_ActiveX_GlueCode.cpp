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

//==============================================================================
/*
    This file contains all the gubbins to create an ActiveX browser plugin that
    wraps your BrowserPluginComponent object.

*/
//==============================================================================
#if _MSC_VER

//==============================================================================
#include <olectl.h>
#include <objsafe.h>
#include <exdisp.h>
#pragma warning (disable:4584)

#include "../juce_browser_plugin.h"
using namespace juce;

#include "juce_BrowserPluginComponent.h"

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
static var variantTojuceVar (const VARIANT& v);

//==============================================================================
// Takes care of the logic in invoking var methods from IDispatch callbacks.
class IDispatchHelper
{
public:
    IDispatchHelper() {}

    String getStringFromDISPID (const DISPID hash) const
    {
        return identifierNames [identifierIDs.indexOf (hash)];
    }

    DISPID getDISPIDForName (const String& name)
    {
        const int i = identifierNames.indexOf (String (name));

        if (i >= 0)
            return identifierIDs[i];

        const DISPID newID = (DISPID) name.hashCode64();
        identifierNames.add (name);
        identifierIDs.add (newID);
        return newID;
    }

    HRESULT doGetIDsOfNames (LPOLESTR* rgszNames, UINT cNames, DISPID* rgDispId)
    {
        for (unsigned int i = 0; i < cNames; ++i)
            rgDispId[i] = getDISPIDForName (rgszNames[i]);

        return S_OK;
    }

    HRESULT doInvoke (const var& v,
                      DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
                      VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
    {
        const Identifier memberId (getStringFromDISPID (dispIdMember));

        DynamicObject* const object = v.getDynamicObject();

        if (memberId.toString().isEmpty() || object == nullptr)
            return DISP_E_MEMBERNOTFOUND;

        if ((wFlags & DISPATCH_METHOD) != 0)
        {
            if (object->hasMethod (memberId))
            {
                const int numArgs = pDispParams == nullptr ? 0 : pDispParams->cArgs;
                var result;

                if (numArgs == 0)
                {
                    result = v.call (memberId);
                }
                else
                {
                    Array<var> args;
                    for (int j = numArgs; --j >= 0;)
                        args.add (variantTojuceVar (pDispParams->rgvarg[j]));

                    result = v.invoke (memberId, numArgs == 0 ? nullptr : args.getRawDataPointer(), numArgs);
                }

                if (pVarResult != nullptr)
                    juceVarToVariant (result, *pVarResult);

                return S_OK;
            }
        }
        else if ((wFlags & DISPATCH_PROPERTYGET) != 0)
        {
            if (object->hasProperty (memberId) && pVarResult != nullptr)
            {
                juceVarToVariant (object->getProperty (memberId), *pVarResult);
                return S_OK;
            }
        }
        else if ((wFlags & DISPATCH_PROPERTYPUT) != 0)
        {
            if (pDispParams != nullptr && pDispParams->cArgs > 0)
            {
                object->setProperty (memberId, variantTojuceVar (pDispParams->rgvarg[0]));
                return S_OK;
            }
        }

        return DISP_E_MEMBERNOTFOUND;
    }

private:
    Array<DISPID> identifierIDs;
    StringArray identifierNames;

    JUCE_DECLARE_NON_COPYABLE (IDispatchHelper)
};

//==============================================================================
// Makes a var look like an IDispatch
class IDispatchWrappingDynamicObject   : public IDispatch
{
public:
    IDispatchWrappingDynamicObject (const var& object_)
        : object (object_),
          refCount (1)
    {
        DBG ("num Juce wrapper objs: " + String (++numJuceSO));
    }

    virtual ~IDispatchWrappingDynamicObject()
    {
        DBG ("num Juce wrapper objs: " + String (--numJuceSO));
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown)    { AddRef(); *result = (IUnknown*)  this; return S_OK; }
        if (id == IID_IDispatch)   { AddRef(); *result = (IDispatch*) this; return S_OK; }

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

    JUCE_DECLARE_NON_COPYABLE (IDispatchWrappingDynamicObject)
};


//==============================================================================
// Makes an IDispatch look like a var
class DynamicObjectWrappingIDispatch   : public DynamicObject
{
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

    var getProperty (const Identifier& propertyName) const override
    {
        const String nameCopy (propertyName.toString());
        LPCOLESTR name = nameCopy.toUTF16();
        DISPID id = 0;
        if (source->GetIDsOfNames (IID_NULL, (LPOLESTR*) &name, 1, 0, &id) == S_OK)
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

    bool hasProperty (const Identifier& propertyName) const override
    {
        const String nameCopy (propertyName.toString());
        LPCOLESTR name = nameCopy.toUTF16();
        DISPID id = 0;
        return source->GetIDsOfNames (IID_NULL, (LPOLESTR*) &name, 1, 0, &id) == S_OK;
    }

    void setProperty (const Identifier& propertyName, const var& newValue) override
    {
        const String nameCopy (propertyName.toString());
        LPCOLESTR name = nameCopy.toUTF16();
        DISPID id = 0;
        if (source->GetIDsOfNames (IID_NULL, (LPOLESTR*) &name, 1, 0, &id) == S_OK)
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

    void removeProperty (const Identifier& propertyName) override
    {
        setProperty (propertyName, var());
    }

    bool hasMethod (const Identifier& methodName) const override
    {
        const String nameCopy (methodName.toString());
        LPCOLESTR name = nameCopy.toUTF16();
        DISPID id = 0;
        return source->GetIDsOfNames (IID_NULL, (LPOLESTR*) &name, 1, 0, &id) == S_OK;
    }

    var invokeMethod (Identifier methodName, const var::NativeFunctionArgs& args) override
    {
        var returnValue;
        const String nameCopy (methodName.toString());
        LPCOLESTR name = nameCopy.toUTF16();
        DISPID id = 0;
        if (source->GetIDsOfNames (IID_NULL, (LPOLESTR*) &name, 1, 0, &id) == S_OK)
        {
            HeapBlock <VARIANT> params;
            params.calloc (args.numArguments + 1);

            for (int i = 0; i < args.numArguments; ++i)
                juceVarToVariant (args.arguments[(args.numArguments - 1) - i], params[i]);

            DISPPARAMS dispParams;
            zerostruct (dispParams);
            dispParams.cArgs = args.numArguments;
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
        }

        return returnValue;
    }

private:
    IDispatch* const source;

    JUCE_DECLARE_NON_COPYABLE (DynamicObjectWrappingIDispatch)
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
        dest.bstrVal = SysAllocString (v.toString().toUTF16());
    }
    else if (v.getDynamicObject() != nullptr)
    {
        dest.vt = VT_DISPATCH;
        dest.pdispVal = new IDispatchWrappingDynamicObject (v);
    }
    else if (v.isMethod())
    {
        dest.vt = VT_EMPTY;
    }
}

var variantTojuceVar (const VARIANT& v)
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
            case VT_BSTR:       return var (String (v.bstrVal));
            case VT_BOOL:       return var (v.boolVal ? true : false);
            case VT_DISPATCH:   return var (new DynamicObjectWrappingIDispatch (v.pdispVal));
            default:            break;
        }
    }

    return var();
}

//==============================================================================
// This acts as the embedded HWND
class AXBrowserPluginHolderComponent    : public Component
{
public:
    AXBrowserPluginHolderComponent()
        : parentHWND (0),
          browser (nullptr)
    {
        setOpaque (true);
        setWantsKeyboardFocus (false);

        addAndMakeVisible (child = createBrowserPlugin());
        jassert (child != nullptr);   // You have to create one of these!
    }

    ~AXBrowserPluginHolderComponent()
    {
        setWindow (nullptr);
        child = nullptr;
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        if (child == nullptr || ! child->isOpaque())
            g.fillAll (Colours::white);
    }

    void resized() override
    {
        if (child != nullptr)
            child->setBounds (getLocalBounds());
    }

    var getObject()   { return child->getJavascriptObject(); }

    void setWindow (IOleInPlaceSite* site)
    {
        if (browser != nullptr)
        {
            browser->Release();
            browser = nullptr;
        }

        HWND newHWND = 0;

        if (site != nullptr)
        {
            site->GetWindow (&newHWND);

            IServiceProvider* sp = nullptr;
            site->QueryInterface (IID_IServiceProvider, (void**) &sp);

            if (sp != nullptr)
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

        if (site != nullptr)
            site->OnInPlaceActivate();
    }

    String getBrowserURL() const
    {
        if (browser == nullptr)
            return String::empty;

        BSTR url = nullptr;
        browser->get_LocationURL (&url);
        return URL::removeEscapeChars (url);
    }

private:
    //==============================================================================
    ScopedPointer<BrowserPluginComponent> child;
    HWND parentHWND;
    IWebBrowser2* browser;

    JUCE_DECLARE_NON_COPYABLE (AXBrowserPluginHolderComponent)
};

//==============================================================================
extern String browserVersionDesc;

static String getExePath()
{
    TCHAR moduleFile [2048] = { 0 };
    GetModuleFileName (0, moduleFile, 2048);
    return moduleFile;
}

static String getExeVersion (const String& exeFileName, const String& fieldName)
{
    DWORD pointlessWin32Variable;
    DWORD size = GetFileVersionInfoSize (exeFileName.toUTF16(), &pointlessWin32Variable);

    if (size > 0)
    {
        HeapBlock <char> exeInfo;
        exeInfo.calloc (size);

        if (GetFileVersionInfo (exeFileName.toUTF16(), 0, size, exeInfo))
        {
            TCHAR* result = nullptr;
            unsigned int resultLen = 0;

            // try the 1200 codepage (Unicode)
            String queryStr ("\\StringFileInfo\\040904B0\\" + fieldName);

            if (! VerQueryValue (exeInfo, (LPTSTR) queryStr.toUTF16().getAddress(), (void**) &result, &resultLen))
            {
                // try the 1252 codepage (Windows Multilingual)
                queryStr = "\\StringFileInfo\\040904E4\\" + fieldName;
                VerQueryValue (exeInfo, (LPTSTR) queryStr.toUTF16().getAddress(), (void**) &result, &resultLen);
            }

            return String (result, resultLen);
        }
    }

    return String::empty;
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
        : site (nullptr), refCount (0)
    {
        log ("JuceActiveXObject");
    }

    ~JuceActiveXObject()
    {
        log ("~JuceActiveXObject");
        holderComp = nullptr;
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown)            { AddRef(); *result = (IUnknown*) this; return S_OK; }
        if (id == IID_IDispatch)           { AddRef(); *result = (IDispatch*) this; return S_OK; }
        if (id == IID_IObjectWithSite)     { AddRef(); *result = (IObjectWithSite*) this; return S_OK; }
        if (id == IID_IObjectSafety)       { AddRef(); *result = (IObjectSafety*) this; return S_OK; }
        if (id == IID_IOleInPlaceObject)   { AddRef(); *result = (IOleInPlaceObject*) this; return S_OK; }
        if (id == IID_IOleWindow)          { AddRef(); *result = (IOleWindow*) (IOleInPlaceObject*) this; return S_OK; }

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
        if (holderComp == nullptr)
            return DISP_E_MEMBERNOTFOUND;

        return iDispatchHelper.doInvoke (holderComp->getObject(),
                                         dispIdMember, riid, lcid, wFlags, pDispParams,
                                         pVarResult, pExcepInfo, puArgErr);
    }

    HRESULT __stdcall SetSite (IUnknown* newSite)
    {
        if (newSite != site)
        {
            if (site != nullptr)
                site->Release();

            site = newSite;

            if (site != nullptr)
            {
                site->AddRef();

                IOleInPlaceSite* inPlaceSite = nullptr;
                site->QueryInterface (IID_IOleInPlaceSite, (void**) &inPlaceSite);

                if (inPlaceSite != nullptr)
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
        if (holderComp == nullptr)
        {
            if (numActivePlugins++ == 0)
            {
                log ("initialiseJuce_GUI()");
                initialiseJuce_GUI();

                browserVersionDesc = "Internet Explorer " + getExeVersion (getExePath(), "FileVersion");
            }

            holderComp = new AXBrowserPluginHolderComponent();
        }
    }

    void deleteHolderComp()
    {
        if (holderComp != nullptr)
        {
            holderComp = nullptr;

            if (--numActivePlugins == 0)
            {
                log ("shutdownJuce_GUI()");
                shutdownJuce_GUI();
            }
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
        if (holderComp != nullptr)
            holderComp->setBounds (r->left, r->top, r->right - r->left, r->bottom - r->top);

        return S_OK;
    }

    HRESULT __stdcall GetWindow (HWND* phwnd)
    {
        if (holderComp == nullptr)
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
    ScopedPointer<AXBrowserPluginHolderComponent> holderComp;
    IDispatchHelper iDispatchHelper;

    JUCE_DECLARE_NON_COPYABLE (JuceActiveXObject)
};

//==============================================================================
class JuceActiveXObjectFactory     : public IUnknown,
                                     public IClassFactory
{
public:
    JuceActiveXObjectFactory()  : refCount (0)   {}

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown)        { AddRef(); *result = (IUnknown*) this; return S_OK; }
        if (id == IID_IClassFactory)   { AddRef(); *result = (IClassFactory*) this; return S_OK; }

        *result = nullptr;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

    HRESULT __stdcall CreateInstance (IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
    {
        *ppvObject = nullptr;

        if (pUnkOuter != nullptr && riid != IID_IUnknown)
            return CLASS_E_NOAGGREGATION;

        JuceActiveXObject* ax = new JuceActiveXObject();
        return ax->QueryInterface (riid, ppvObject);
    }

    HRESULT __stdcall LockServer (BOOL /*fLock*/)    { return S_OK; }

private:
    int refCount;

    JUCE_DECLARE_NON_COPYABLE (JuceActiveXObjectFactory)
};

//==============================================================================
String getActiveXBrowserURL (const BrowserPluginComponent* comp)
{
    if (AXBrowserPluginHolderComponent* ax = dynamic_cast<AXBrowserPluginHolderComponent*> (comp->getParentComponent()))
        return ax->getBrowserURL();

    return String();
}

//==============================================================================
extern "C" BOOL WINAPI DllMain (HANDLE instance, DWORD reason, LPVOID)
{
    #pragma EXPORTED_FUNCTION

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        log ("DLL_PROCESS_ATTACH");
        Process::setCurrentModuleInstanceHandle (instance);
        break;

    case DLL_PROCESS_DETACH:
        log ("DLL_PROCESS_DETACH");
        browserVersionDesc.clear();

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

static String CLSIDToJuceString (REFCLSID clsid)
{
    LPWSTR s = nullptr;
    StringFromIID (clsid, &s);

    if (s == nullptr)
        return String::empty;

    const String result (s);
    LPMALLOC malloc;
    CoGetMalloc (1, &malloc);
    if (malloc != nullptr)
    {
        malloc->Free (s);
        malloc->Release();
    }

    return result.removeCharacters ("{}").trim();
}

STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    #pragma EXPORTED_FUNCTION

    *ppv = nullptr;

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
static String makeLegalRegistryName (const String& s)
{
    return s.retainCharacters ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.");
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
            WindowsRegistry::deleteValue (settings.getAllKeys()[i]);

        WindowsRegistry::deleteKey (root + companyDotPluginCur);
        WindowsRegistry::deleteKey (root + companyDotPlugin);
        WindowsRegistry::deleteKey (clsIDRoot);

        if (WindowsRegistry::valueExists (clsIDRoot + "InProcServer32"))
            return SELFREG_E_CLASS;
    }
    else
    {
        WindowsRegistry::deleteKey (clsIDRoot);

        for (int i = 0; i < settings.getAllKeys().size(); ++i)
            WindowsRegistry::setValue (settings.getAllKeys()[i],
                                       settings [settings.getAllKeys()[i]]);

        // check whether the registration actually worked - if not, we probably don't have
        // enough privileges to write to the registry..
        if (WindowsRegistry::getValue (clsIDRoot + "InProcServer32\\") != dllPath)
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
