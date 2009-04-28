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
/*
    This file contains all the mess that creates an NPAPI interface, and connects
    that interface to your BrowserPluginComponent object.

*/
//==============================================================================
#if defined (__APPLE__) && ! JUCE_NPAPI_WRAPPED_IN_MM
 #error "On the Mac, you can't compile this .cpp file directly - use juce_NPAPI_GlueCode.mm instead"
#endif

#define XPCOM_GLUE

//==============================================================================
#if _MSC_VER
 #define XP_WIN
 #define _X86_
 #include <windows.h>
 #include <windowsx.h>
 #include "npapi/npupp.h"

 // Cunning trick used to add functions to export list and avoid messing about with .def files.
 // (can't add a declspec because the functions have already been pre-declared in the npapi headers).
 #define EXPORTED_FUNCTION comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

//==============================================================================
#elif defined (__APPLE__)
 #define XP_MACOSX
 #define OSCALL
 #include <WebKit/npapi.h>
 #include <WebKit/npfunctions.h>
 #include <WebKit/npruntime.h>

//==============================================================================
#else
 #define XP_UNIX
 #include "npapi.h"
 #include "npupp.h"
 #include "npruntime.h"

#endif

//==============================================================================
#include "../../../juce_amalgamated.h"
#include "juce_BrowserPluginComponent.h"
#include "juce_IncludeBrowserPluginInfo.h"

#if JUCE_MAC && JUCE_DEBUG && 0
static void log (const String& s)
{
    FILE* f = fopen ("/Users/jules/Desktop/log.txt", "a+");
    fprintf (f, (const char*) s);
    fprintf (f, "\n");
    fflush (f);
    fclose (f);
}
#else
#define log(a)
#endif

//==============================================================================
#if JUCE_MAC
static const String nsStringToJuce (NSString* s)    { return String::fromUTF8 ((juce::uint8*) [s UTF8String]); }
static NSString* juceStringToNS (const String& s)   { return [NSString stringWithUTF8String: (const char*) s.toUTF8()]; }

#pragma export on
extern "C"
{
    NPError NP_Initialize (NPNetscapeFuncs*);
    NPError NP_GetEntryPoints (NPPluginFuncs*);
    NPError NP_Shutdown();
}
#pragma export off
#endif

//==============================================================================
static NPNetscapeFuncs browser;

//==============================================================================
NPError NP_GetValue (void* future, NPPVariable variable, void* value)
{
    return NPP_GetValue ((NPP_t*) future, variable, value);
}

#if JUCE_WIN32 || JUCE_MAC
NPError OSCALL NP_GetEntryPoints (NPPluginFuncs* funcs)
{
#if JUCE_WIN32
  #pragma EXPORTED_FUNCTION
#endif

    log ("NP_GetEntryPoints");
    if (funcs == 0 || (funcs->size > 0 && funcs->size < sizeof (NPPluginFuncs)))
        return NPERR_INVALID_FUNCTABLE_ERROR;

    funcs->size          = sizeof (NPPluginFuncs);
    funcs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
    funcs->newp          = NPP_New;
    funcs->destroy       = NPP_Destroy;
    funcs->setwindow     = NPP_SetWindow;
    funcs->newstream     = NPP_NewStream;
    funcs->destroystream = NPP_DestroyStream;
    funcs->asfile        = NPP_StreamAsFile;
    funcs->writeready    = NPP_WriteReady;
#if JUCE_MAC
    funcs->write         = (NPP_WriteProcPtr) NPP_Write;
#else
    funcs->write         = NPP_Write;
#endif
    funcs->print         = NPP_Print;
    funcs->event         = NPP_HandleEvent;
    funcs->urlnotify     = NPP_URLNotify;
    funcs->getvalue      = NPP_GetValue;
    funcs->setvalue      = NPP_SetValue;
    funcs->javaClass     = 0;

    return NPERR_NO_ERROR;
}
#endif

NPError OSCALL NP_Initialize (NPNetscapeFuncs* funcs
                              #ifdef XP_UNIX
                              , NPPluginFuncs* pluginFuncs
                              #endif
                              )
{
#if JUCE_WIN32
  #pragma EXPORTED_FUNCTION
#endif

    log ("NP_Initialize");
    if (funcs == 0)
        return NPERR_INVALID_FUNCTABLE_ERROR;

    if (((funcs->version >> 8) & 0xff) > NP_VERSION_MAJOR)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    if (funcs->size < sizeof (NPNetscapeFuncs))
        return NPERR_INVALID_FUNCTABLE_ERROR;

    browser.size                   = funcs->size;
    browser.version                = funcs->version;
    browser.geturlnotify           = funcs->geturlnotify;
    browser.geturl                 = funcs->geturl;
    browser.posturlnotify          = funcs->posturlnotify;
    browser.posturl                = funcs->posturl;
    browser.requestread            = funcs->requestread;
    browser.newstream              = funcs->newstream;
    browser.write                  = funcs->write;
    browser.destroystream          = funcs->destroystream;
    browser.status                 = funcs->status;
    browser.uagent                 = funcs->uagent;
    browser.memalloc               = funcs->memalloc;
    browser.memfree                = funcs->memfree;
    browser.memflush               = funcs->memflush;
    browser.reloadplugins          = funcs->reloadplugins;
    browser.getJavaEnv             = funcs->getJavaEnv;
    browser.getJavaPeer            = funcs->getJavaPeer;
    browser.getvalue               = funcs->getvalue;
    browser.setvalue               = funcs->setvalue;
    browser.invalidaterect         = funcs->invalidaterect;
    browser.invalidateregion       = funcs->invalidateregion;
    browser.forceredraw            = funcs->forceredraw;
    browser.getstringidentifier    = funcs->getstringidentifier;
    browser.getstringidentifiers   = funcs->getstringidentifiers;
    browser.getintidentifier       = funcs->getintidentifier;
    browser.identifierisstring     = funcs->identifierisstring;
    browser.utf8fromidentifier     = funcs->utf8fromidentifier;
    browser.intfromidentifier      = funcs->intfromidentifier;
    browser.createobject           = funcs->createobject;
    browser.retainobject           = funcs->retainobject;
    browser.releaseobject          = funcs->releaseobject;
    browser.invoke                 = funcs->invoke;
    browser.invokeDefault          = funcs->invokeDefault;
    browser.evaluate               = funcs->evaluate;
    browser.getproperty            = funcs->getproperty;
    browser.setproperty            = funcs->setproperty;
    browser.removeproperty         = funcs->removeproperty;
    browser.hasproperty            = funcs->hasproperty;
    browser.hasmethod              = funcs->hasmethod;
    browser.releasevariantvalue    = funcs->releasevariantvalue;
    browser.setexception           = funcs->setexception;

#ifdef XP_UNIX
    pluginFuncs->version            = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
    pluginFuncs->size               = sizeof (NPPluginFuncs);
    pluginFuncs->newp               = NewNPP_NewProc (NPP_New);
    pluginFuncs->destroy            = NewNPP_DestroyProc (NPP_Destroy);
    pluginFuncs->setwindow          = NewNPP_SetWindowProc (NPP_SetWindow);
    pluginFuncs->newstream          = NewNPP_NewStreamProc (NPP_NewStream);
    pluginFuncs->destroystream      = NewNPP_DestroyStreamProc (NPP_DestroyStream);
    pluginFuncs->asfile             = NewNPP_StreamAsFileProc (NPP_StreamAsFile);
    pluginFuncs->writeready         = NewNPP_WriteReadyProc (NPP_WriteReady);
    pluginFuncs->write              = NewNPP_WriteProc (NPP_Write);
    pluginFuncs->print              = NewNPP_PrintProc (NPP_Print);
    pluginFuncs->urlnotify          = NewNPP_URLNotifyProc (NPP_URLNotify);
    pluginFuncs->event              = 0;
    pluginFuncs->getvalue           = NewNPP_GetValueProc (NPP_GetValue);
  #ifdef OJI
    pluginFuncs->javaClass          = NPP_GetJavaClass();
  #endif
#endif

    return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown()
{
#if JUCE_WIN32
  #pragma EXPORTED_FUNCTION
#endif

    log ("NP_Shutdown");
    return NPERR_NO_ERROR;
}

char* NP_GetMIMEDescription()
{
    log ("NP_GetMIMEDescription");
    static String mimeDesc;

    mimeDesc = String (T(JuceBrowserPlugin_MimeType))
                + T(":") + String (T(JuceBrowserPlugin_FileSuffix))
                + T(":") + String (T(JuceBrowserPlugin_Name));

    return (char*) (const char*) mimeDesc.toUTF8();
}

//==============================================================================
/*
NPError NPN_GetURLNotify (NPP instance, const char *url, const char *target, void* notifyData)
{
    return (browser.version & 0xFF) >= NPVERS_HAS_NOTIFICATION
              ? browser.geturlnotify (instance, url, target, notifyData);
              : NPERR_INCOMPATIBLE_VERSION_ERROR;
}

NPError NPN_PostURLNotify (NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData)
{
    return (browser.version & 0xFF) >= NPVERS_HAS_NOTIFICATION
                ? browser.posturlnotify (instance, url, window, len, buf, file, notifyData)
                : NPERR_INCOMPATIBLE_VERSION_ERROR;
}

NPError NPN_NewStream (NPP instance, NPMIMEType type, const char* target, NPStream** stream)
{
    return (browser.version & 0xFF) >= NPVERS_HAS_STREAMOUTPUT
                ? browser.newstream (instance, type, target, stream)
                : NPERR_INCOMPATIBLE_VERSION_ERROR;
}

int32 NPN_Write (NPP instance, NPStream *stream, int32 len, void *buffer)
{
    return (browser.version & 0xFF) >= NPVERS_HAS_STREAMOUTPUT
                ? browser.write (instance, stream, len, buffer)
                : -1;
}

NPError NPN_DestroyStream (NPP instance, NPStream* stream, NPError reason)
{
    return (browser.version & 0xFF) >= NPVERS_HAS_STREAMOUTPUT
                ? browser.destroystream (instance, stream, reason)
                : NPERR_INCOMPATIBLE_VERSION_ERROR;
}
*/

//==============================================================================
class BrowserPluginHolderComponent    : public Component
{
public:
    //==============================================================================
    BrowserPluginHolderComponent (NPP npp_)
        : npp (npp_),
          child (0)
    {
        log ("BrowserPluginHolderComponent created");
#if JUCE_WIN32
        parentHWND = 0;
        oldWinProc = 0;
#else
        currentParentView = 0;
#endif
        setOpaque (true);
        setWantsKeyboardFocus (false);

        addAndMakeVisible (child = createBrowserPlugin());
        jassert (child != 0);   // You have to create one of these!
    }

    ~BrowserPluginHolderComponent()
    {
        log ("BrowserPluginHolderComponent deleted");
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

    const var getObject()
    {
        return child->getJavascriptObject();
    }
    
    //==============================================================================
    NPP npp;
    BrowserPluginComponent* child;

private:

    //==============================================================================
#if JUCE_WIN32
    HWND parentHWND;
    WNDPROC oldWinProc;

    void resizeToParentWindow()
    {
        if (IsWindow (parentHWND))
        {
            RECT r;
            GetWindowRect (parentHWND, &r);
            setBounds (0, 0, r.right - r.left, r.bottom - r.top);
        }
    }

    static LRESULT CALLBACK interceptingWinProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint (hWnd, &ps);
                EndPaint (hWnd, &ps);
            }
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_WINDOWPOSCHANGING:
            if ((((WINDOWPOS*) lParam)->flags & SWP_NOSIZE) == 0)
            {
                BrowserPluginHolderComponent* const comp = (BrowserPluginHolderComponent*) GetWindowLong (hWnd, GWL_USERDATA);
                comp->resizeToParentWindow();
            }
            break;

        default:
            break;
        }

        return DefWindowProc (hWnd, msg, wParam, lParam);
    }

public:
    void setWindow (NPWindow* window)
    {
        HWND newHWND = (window != 0 ? ((HWND) window->window) : 0);

        if (parentHWND != newHWND)
        {
            removeFromDesktop();
            setVisible (false);

            if (IsWindow (parentHWND))
            {
                SubclassWindow (parentHWND, oldWinProc);  // restore the old winproc..
                oldWinProc = 0;
            }

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

                oldWinProc = SubclassWindow (parentHWND, (WNDPROC) interceptingWinProc);
                SetWindowLong (parentHWND, GWL_USERDATA, (LONG) this);

                resizeToParentWindow();
            }
        }
    }

    //==============================================================================
#else
    NSView* currentParentView;

    NSView* findViewAt (NSView* parent, float x, float y) const
    {
        NSRect r = [parent frame];
        x -= r.origin.x;
        y -= r.origin.y;

        if (x >= 0 && x < r.size.width && y >= 0 && y < r.size.height)
        {
            for (int i = [[parent subviews] count]; --i >= 0;)
            {
                NSView* v = (NSView*) [[parent subviews] objectAtIndex: i];

                if (v != (NSView*) getWindowHandle())
                {
                    NSView* found = findViewAt (v, x, y);

                    if (found != 0)
                        return found;
                }
            }

            return parent;
        }

        return 0;
    }

public:
    static bool isBrowserContentView (NSView* v)
    {
        return [[v className] isEqualToString: @"WebNetscapePluginDocumentView"]
            || [[v className] isEqualToString: @"WebPluginDocumentView"]
            || [[v className] isEqualToString: @"ChildView"];
    }

    void setWindow (NPWindow* window)
    {
        const ScopedAutoReleasePool pool;

        NSView* parentView = 0;
        WindowRef windowRef = window != 0 ? ((NP_CGContext*) window->window)->window : 0;

        if (windowRef != 0)
        {
            NSWindow* win = [[[NSWindow alloc] initWithWindowRef: windowRef] autorelease];
            parentView = findViewAt ([win contentView], window->x + 0.5f, window->y + 0.5f);

            log (nsStringToJuce ([parentView description]));

            if (! isBrowserContentView (parentView))
                parentView = currentParentView;
        }

        if (parentView != currentParentView)
        {
            //log ("new view: " + nsStringToJuce ([parentView description]));

            removeFromDesktop();
            setVisible (false);

            currentParentView = parentView;

            if (parentView != 0)
            {
                setSize (window->width, window->height);
                addToDesktop (0, parentView);
                setVisible (true);
            }
        }

        if (window != 0)
            setSize (window->width, window->height);
    }
#endif
};

//==============================================================================
static NPIdentifier getIdentifierFromString (const var::identifier& s) throw()
{
    return browser.getstringidentifier (s.name.toUTF8());
}

static const var createValueFromNPVariant (NPP npp, const NPVariant& v);
static void createNPVariantFromValue (NPP npp, NPVariant& out, const var& v);

#if JUCE_DEBUG
 static int numDOWNP = 0, numJuceSO = 0;
#endif

//==============================================================================
class DynamicObjectWrappingNPObject   : public DynamicObject
{
    NPP npp;
    NPObject* const source;

public:
    DynamicObjectWrappingNPObject (NPP npp_, NPObject* const source_)
        : npp (npp_),
          source (browser.retainobject (source_))
    {
        DBG ("num NP wrapper objs: " + String (++numDOWNP));
    }

    ~DynamicObjectWrappingNPObject()
    {
        browser.releaseobject (source);
        DBG ("num NP wrapper objs: " + String (--numDOWNP));
    }

    const var getProperty (const var::identifier& propertyName) const
    {
        NPVariant result;
        VOID_TO_NPVARIANT (result);
        browser.getproperty (npp, source, getIdentifierFromString (propertyName), &result);
        const var v (createValueFromNPVariant (npp, result));
        browser.releasevariantvalue (&result);
        return v;
    }

    bool hasProperty (const var::identifier& propertyName) const
    {
        NPVariant result;
        VOID_TO_NPVARIANT (result);
        const bool hasProp = browser.getproperty (npp, source, getIdentifierFromString (propertyName), &result);
        browser.releasevariantvalue (&result);
        return hasProp;
    }

    void setProperty (const var::identifier& propertyName, const var& newValue)
    {
        NPVariant value;
        createNPVariantFromValue (npp, value, newValue);
        
        browser.setproperty (npp, source, getIdentifierFromString (propertyName), &value);
        browser.releasevariantvalue (&value);
    }

    void removeProperty (const var::identifier& propertyName)
    {
        browser.removeproperty (npp, source, getIdentifierFromString (propertyName));
    }

    bool hasMethod (const var::identifier& methodName) const
    {
        return browser.hasmethod (npp, source, getIdentifierFromString (methodName));
    }

    const var invokeMethod (const var::identifier& methodName,
                            const var* parameters,
                            int numParameters)
    {
        var returnVal;
        
        NPVariant result;
        VOID_TO_NPVARIANT (result);

        if (numParameters > 0)
        {
            NPVariant* params = (NPVariant*) juce_malloc (sizeof (NPVariant*) * numParameters);

            int i;
            for (i = 0; i < numParameters; ++i)
                createNPVariantFromValue (npp, params[i], parameters[i]);

            if (browser.invoke (npp, source, getIdentifierFromString (methodName),
                                params, numParameters, &result))
            {
                returnVal = createValueFromNPVariant (npp, result);
                browser.releasevariantvalue (&result);
            }

            for (i = 0; i < numParameters; ++i)
                browser.releasevariantvalue (&params[i]);
            
            juce_free (params);
        }
        else
        {
            if (browser.invoke (npp, source, getIdentifierFromString (methodName), 0, 0, &result))
            {
                returnVal = createValueFromNPVariant (npp, result);
                browser.releasevariantvalue (&result);
            }
        }
        
        return returnVal;
    }
};

//==============================================================================
class NPObjectWrappingDynamicObject : public NPObject
{
public:
    static NPObject* create (NPP npp, const var& objectToWrap);

    virtual ~NPObjectWrappingDynamicObject()
    {
        DBG ("num Juce wrapper objs: " + String (--numJuceSO));
    }

private:
    NPObjectWrappingDynamicObject (NPP npp_)
        : npp (npp_)
    {
        DBG ("num Juce wrapper objs: " + String (++numJuceSO));
    }

    //==============================================================================
    bool construct (const NPVariant *args, uint32_t argCount, NPVariant *result);
    void invalidate()    {}

    bool hasMethod (NPIdentifier name)
    {
        DynamicObject* const o = object.getObject();
        return o != 0 && o->hasMethod (identifierToString (name));
    }

    bool invoke (NPIdentifier name, const NPVariant* args, uint32_t argCount, NPVariant* out)
    {
        DynamicObject* const o = object.getObject();
        const var::identifier methodName (identifierToString (name));

        if (o == 0 || ! o->hasMethod (methodName))
            return false;

        var* params = (var*) juce_calloc (sizeof (var) * argCount);
        for (uint32_t i = 0; i < argCount; ++i)
            params[i] = createValueFromNPVariant (npp, args[i]);

        const var result (o->invokeMethod (methodName, params, argCount));

        for (int i = argCount; --i >= 0;)
            params[i] = var();

        juce_free (params);

        if (out != 0)
            createNPVariantFromValue (npp, *out, result);

        return true;
    }

    bool invokeDefault (const NPVariant* args, uint32_t argCount, NPVariant* result)
    {
        return false;
    }

    bool hasProperty (NPIdentifier name)
    {
        DynamicObject* const o = object.getObject();
        return o != 0 && o->hasProperty (identifierToString (name));
    }

    bool getProperty (NPIdentifier name, NPVariant* out)
    {
        DynamicObject* const o = object.getObject();
        const var::identifier propName (identifierToString (name));

        if (o == 0 || ! o->hasProperty (propName))
            return false;

        const var result (o->getProperty (propName));
        
        if (out != 0)
            createNPVariantFromValue (npp, *out, result);

        return true;
    }

    bool setProperty (NPIdentifier name, const NPVariant* value)
    {
        DynamicObject* const o = object.getObject();

        if (value == 0 || o == 0)
            return false;

        o->setProperty (identifierToString (name), createValueFromNPVariant (npp, *value));
        return true;
    }

    bool removeProperty (NPIdentifier name)
    {
        DynamicObject* const o = object.getObject();
        const var::identifier propName (identifierToString (name));

        if (o == 0 || ! o->hasProperty (propName))
            return false;

        o->removeProperty (propName);
        return true;
    }

    bool enumerate (NPIdentifier** identifier, uint32_t* count)
    {
        return false;
    }

    //==============================================================================
    NPP npp;
    var object;

    static const var::identifier identifierToString (NPIdentifier id)
    {
        NPUTF8* const name = browser.utf8fromidentifier (id);
        const var::identifier result ((const char*) name);
        browser.memfree (name);
        return result;
    }

public:
    //==============================================================================
    static NPObject* createInstance (NPP npp, NPClass* aClass)  { return new NPObjectWrappingDynamicObject (npp); }
    static void class_deallocate (NPObject* npobj)  { delete (NPObjectWrappingDynamicObject*) npobj; }
    static void class_invalidate (NPObject* npobj)  { ((NPObjectWrappingDynamicObject*) npobj)->invalidate(); }
    static bool class_hasMethod (NPObject* npobj, NPIdentifier name)    { return ((NPObjectWrappingDynamicObject*) npobj)->hasMethod (name); }
    static bool class_invoke (NPObject* npobj, NPIdentifier name, const NPVariant* args, uint32_t argCount, NPVariant* result)  { return ((NPObjectWrappingDynamicObject*) npobj)->invoke (name, args, argCount, result); }
    static bool class_invokeDefault (NPObject* npobj, const NPVariant* args, uint32_t argCount, NPVariant* result)  { return ((NPObjectWrappingDynamicObject*) npobj)->invokeDefault (args, argCount, result); }
    static bool class_hasProperty (NPObject* npobj, NPIdentifier name)  { return ((NPObjectWrappingDynamicObject*) npobj)->hasProperty (name); }
    static bool class_getProperty (NPObject* npobj, NPIdentifier name, NPVariant* result)   { return ((NPObjectWrappingDynamicObject*) npobj)->getProperty (name, result); }
    static bool class_setProperty (NPObject* npobj, NPIdentifier name, const NPVariant* value)  { return ((NPObjectWrappingDynamicObject*) npobj)->setProperty (name, value); }
    static bool class_removeProperty (NPObject* npobj, NPIdentifier name)   { return ((NPObjectWrappingDynamicObject*) npobj)->removeProperty (name); }
    static bool class_enumerate (NPObject* npobj, NPIdentifier** identifier, uint32_t* count) { return ((NPObjectWrappingDynamicObject*) npobj)->enumerate (identifier, count); }
    static bool class_construct (NPObject* npobj, const NPVariant* args, uint32_t argCount, NPVariant* result)  { return ((NPObjectWrappingDynamicObject*) npobj)->construct (args, argCount, result); }
};

static NPClass sNPObjectWrappingDynamicObject_NPClass =
{
    NP_CLASS_STRUCT_VERSION_ENUM, NPObjectWrappingDynamicObject::createInstance,
    NPObjectWrappingDynamicObject::class_deallocate, NPObjectWrappingDynamicObject::class_invalidate,
    NPObjectWrappingDynamicObject::class_hasMethod, NPObjectWrappingDynamicObject::class_invoke,
    NPObjectWrappingDynamicObject::class_invokeDefault, NPObjectWrappingDynamicObject::class_hasProperty,
    NPObjectWrappingDynamicObject::class_getProperty, NPObjectWrappingDynamicObject::class_setProperty,
    NPObjectWrappingDynamicObject::class_removeProperty, NPObjectWrappingDynamicObject::class_enumerate
};

bool NPObjectWrappingDynamicObject::construct (const NPVariant* args, uint32_t argCount, NPVariant* result)
{
    NPObject* const newObj = browser.createobject (npp, &sNPObjectWrappingDynamicObject_NPClass);

    if (newObj == 0)
        return false;

    OBJECT_TO_NPVARIANT (newObj, *result);
    return true;
}

NPObject* NPObjectWrappingDynamicObject::create (NPP npp, const var& objectToWrap)
{
    jassert (objectToWrap.getObject() != 0);

    NPObject* const nppObject = browser.createobject (npp, &sNPObjectWrappingDynamicObject_NPClass);

    if (nppObject != 0)
        ((NPObjectWrappingDynamicObject*) nppObject)->object = objectToWrap;
    
    return nppObject;
}


//==============================================================================
static const var createValueFromNPVariant (NPP npp, const NPVariant& v)
{
    if (NPVARIANT_IS_BOOLEAN (v))
        return var (NPVARIANT_TO_BOOLEAN (v));
    else if (NPVARIANT_IS_INT32 (v))
        return var (NPVARIANT_TO_INT32 (v));
    else if (NPVARIANT_IS_DOUBLE (v))
        return var (NPVARIANT_TO_DOUBLE (v));
    else if (NPVARIANT_IS_STRING (v))
#if JUCE_MAC
        return var (String::fromUTF8 ((const juce::uint8*) (NPVARIANT_TO_STRING (v).UTF8Characters), 
                                      (int) NPVARIANT_TO_STRING (v).UTF8Length));
#else
        return var (String::fromUTF8 ((const juce::uint8*) (NPVARIANT_TO_STRING (v).utf8characters), 
                                      (int) NPVARIANT_TO_STRING (v).utf8length));
#endif
    else if (NPVARIANT_IS_OBJECT (v))
        return var (new DynamicObjectWrappingNPObject (npp, NPVARIANT_TO_OBJECT (v)));

    return var();
}

static void createNPVariantFromValue (NPP npp, NPVariant& out, const var& v)
{
    if (v.isInt())
        INT32_TO_NPVARIANT ((int) v, out);
    else if (v.isBool())
        BOOLEAN_TO_NPVARIANT ((bool) v, out); 
    else if (v.isDouble())
        DOUBLE_TO_NPVARIANT ((double) v, out); 
    else if (v.isString())
#if JUCE_MAC
        STRINGZ_TO_NPVARIANT (strdup (v.toString().toUTF8()), out);
#else
        STRINGZ_TO_NPVARIANT (_strdup (v.toString().toUTF8()), out);
#endif
    else if (v.isObject())
        OBJECT_TO_NPVARIANT (NPObjectWrappingDynamicObject::create (npp, v), out);
    else
        VOID_TO_NPVARIANT (out);
}

//==============================================================================
class JucePluginInstance
{
public:
    //==============================================================================
    JucePluginInstance (NPP npp_)
        : npp (npp_),
          holderComp (0),
          scriptObject (0)
    {
    }

    ~JucePluginInstance()
    {
        setWindow (0);
    }

    bool setWindow (NPWindow* window)
    {
        if (window != 0)
        {
            if (holderComp == 0)
                holderComp = new BrowserPluginHolderComponent (npp);

            holderComp->setWindow (window);
        }
        else
        {
            deleteAndZero (holderComp);
            scriptObject = 0;
        }
        
        return true;
    }

    NPObject* getScriptableObject()
    {
        if (scriptObject == 0)
            scriptObject = NPObjectWrappingDynamicObject::create (npp, holderComp->getObject());

        if (scriptObject != 0 && shouldRetainBrowserObject())
            browser.retainobject (scriptObject);

        return scriptObject;
    }

    //==============================================================================
    NPP npp;
    BrowserPluginHolderComponent* holderComp;
    NPObject* scriptObject;

private:
    bool shouldRetainBrowserObject() const
    {
#if JUCE_MAC
        const String version (browser.uagent (npp));

        if (! version.containsIgnoreCase (T(" AppleWebKit/")))
            return true;

        int versionNum = version.fromFirstOccurrenceOf (T(" AppleWebKit/"), false, true).getIntValue();

        return versionNum == 0 || versionNum >= 420;
#else
        return true;
#endif
    }
};

//==============================================================================
static NPP currentlyInitialisingNPP = 0;
static int numPluginInstances = 0;

NPError NPP_New (NPMIMEType pluginType, NPP npp, ::uint16 mode, ::int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
    log ("NPP_New");
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

#if JUCE_MAC
    browser.setvalue (npp, (NPPVariable) NPNVpluginDrawingModel, (void*) NPDrawingModelCoreGraphics);
#endif

    if (numPluginInstances++ == 0)
    {
        initialiseJuce_GUI();
        log ("initialiseJuce_GUI()");
    }

    currentlyInitialisingNPP = npp;
    JucePluginInstance* p = new JucePluginInstance (npp);
    currentlyInitialisingNPP = 0;

    npp->pdata = (void*) p;
    return NPERR_NO_ERROR;
}

NPError NPP_Destroy (NPP npp, NPSavedData** save)
{
    log ("NPP_Destroy");
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;

    if (p != 0)
    {
        delete p;

        if (--numPluginInstances == 0)
        {
            shutdownJuce_GUI();
            log ("shutdownJuce_GUI()");
        }
    }

    return NPERR_NO_ERROR;
}

NPError NPP_SetWindow (NPP npp, NPWindow* pNPWindow)
{
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    if (pNPWindow == 0)
        return NPERR_GENERIC_ERROR;

    JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;

    if (p == 0) 
        return NPERR_GENERIC_ERROR;

    currentlyInitialisingNPP = npp;
    NPError result = p->setWindow (pNPWindow) ? NPERR_NO_ERROR 
                                              : NPERR_MODULE_LOAD_FAILED_ERROR;
    currentlyInitialisingNPP = 0;
    return result;
}

//==============================================================================
NPError	NPP_GetValue (NPP npp, NPPVariable variable, void* value)
{
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;

    if (p == 0)
        return NPERR_GENERIC_ERROR;

    switch (variable) 
    {
    case NPPVpluginNameString:
        *((char**) value) = JuceBrowserPlugin_Name;
        break;
    case NPPVpluginDescriptionString:
        *((char**) value) = JuceBrowserPlugin_Desc;
        break;
    case NPPVpluginScriptableNPObject:
        *((NPObject**) value) = p->getScriptableObject();
        break;

    default:
        return NPERR_GENERIC_ERROR;
    }

    return NPERR_NO_ERROR;
}

NPError NPP_NewStream (NPP npp,
                       NPMIMEType type,
                       NPStream* stream, 
                       NPBool seekable,
                       ::uint16* stype)
{
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    return NPERR_NO_ERROR;
}

::int32 NPP_WriteReady (NPP npp, NPStream *stream)
{
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    return 0x0fffffff;
}

::int32 NPP_Write (NPP npp, NPStream *stream, ::int32 offset, ::int32 len, void *buffer)
{
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    return len;
}

NPError NPP_DestroyStream (NPP npp, NPStream *stream, NPError reason)
{
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    return NPERR_NO_ERROR;
}

void NPP_StreamAsFile (NPP npp, NPStream* stream, const char* fname)
{
    if (npp == 0)
        return;
}

void NPP_Print (NPP npp, NPPrint* printInfo)
{
    if (npp == 0)
        return;
}

void NPP_URLNotify (NPP npp, const char* url, NPReason reason, void* notifyData)
{
    if (npp == 0)
        return;
}

NPError NPP_SetValue (NPP npp, NPNVariable variable, void* value)
{
    if (npp == 0)
        return NPERR_INVALID_INSTANCE_ERROR;

    return NPERR_NO_ERROR;
}

::int16 NPP_HandleEvent (NPP npp, void* ev)
{
    if (npp != 0)
    {
        //JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;
    }

    return 0;
}


//==============================================================================
static NPP getInstance (const BrowserPluginComponent* bpc)
{
    BrowserPluginHolderComponent* holder = dynamic_cast <BrowserPluginHolderComponent*> (bpc->getParentComponent());

    if (holder != 0)
        return holder->npp;

    return currentlyInitialisingNPP;
}

//==============================================================================
BrowserPluginComponent::BrowserPluginComponent()
{
}

BrowserPluginComponent::~BrowserPluginComponent()
{
}
const String BrowserPluginComponent::getBrowserVersion() const
{
    String s;

    if (getInstance (this) != 0)
        s << browser.uagent (getInstance (this));
    else
        s << "Netscape Plugin V" << (int) ((browser.version >> 8) & 0xff)
          << "." << (int) (browser.version & 0xff);

    return s;
}
