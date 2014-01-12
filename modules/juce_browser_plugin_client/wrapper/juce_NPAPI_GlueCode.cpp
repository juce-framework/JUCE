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
    This file contains all the mess that creates an NPAPI interface, and connects
    that interface to your BrowserPluginComponent object.

*/
//==============================================================================
#if defined (__APPLE__) && ! JUCE_NPAPI_WRAPPED_IN_MM
 #error "You mustn't compile this cpp file directly - use juce_browser_plugin.cpp instead"
#endif

#define XPCOM_GLUE

//==============================================================================
#if _MSC_VER
 // Cunning trick used to add functions to export list and avoid messing about with .def files.
 // (can't add a declspec because the functions have already been pre-declared in the npapi headers).
 #define EXPORTED_FUNCTION comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

//==============================================================================
#elif defined (__APPLE__)
 #define XP_MACOSX
 #define OSCALL
 #define Point CarbonDummyPointName
 #define Component CarbonDummyCompName
 #include <WebKit/npapi.h>
 #include <WebKit/npfunctions.h>
 #include <WebKit/npruntime.h>
 #undef Point
 #undef Component

//==============================================================================
#else
 #define XP_UNIX
 #include "npapi.h"
 #include "npupp.h"
 #include "npruntime.h"

#endif

//==============================================================================
#if JUCE_MAC && JUCE_DEBUG && 0
 #include <fstream>
 static void log (const String& s)
 {
     std::ofstream file ("/Users/jules/Desktop/log.txt", std::ios::out | std::ios::app);
     file << s << std::endl;
 }
#else
 #define log(a)
#endif

//==============================================================================
#if JUCE_MAC
static String nsStringToJuce (NSString* s)          { return String::fromUTF8 ([s UTF8String]); }
static NSString* juceStringToNS (const String& s)   { return [NSString stringWithUTF8String: s.toUTF8()]; }

#pragma export on
extern "C"
{
    NPError NP_Initialize (NPNetscapeFuncs*);
    NPError NP_GetEntryPoints (NPPluginFuncs*);
    NPError NP_Shutdown();
}
#pragma export off

#ifndef NP_CLASS_STRUCT_VERSION_ENUM   // fill in some symbols that are missing from the OSX 10.4 SDK
  #define NPNVpluginDrawingModel 1000
  #define NPDrawingModelCoreGraphics 1

  typedef struct NP_CGContext
  {
    CGContextRef context;
    WindowRef window;
  } NP_CGContext;
#endif

#endif

//==============================================================================
static NPNetscapeFuncs browser;
String browserVersionDesc;

//==============================================================================
NPError NP_GetValue (void* future, NPPVariable variable, void* value)
{
    return NPP_GetValue ((NPP_t*) future, variable, value);
}

#if JUCE_WINDOWS || JUCE_MAC
NPError OSCALL NP_GetEntryPoints (NPPluginFuncs* funcs)
{
   #if JUCE_WINDOWS
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
    funcs->javaClass     = nullptr;

    return NPERR_NO_ERROR;
}
#endif

NPError OSCALL NP_Initialize (NPNetscapeFuncs* funcs
                              #ifdef XP_UNIX
                              , NPPluginFuncs* pluginFuncs
                              #endif
                              )
{
   #if JUCE_WINDOWS
    #pragma EXPORTED_FUNCTION
   #endif

    log ("NP_Initialize");
    if (funcs == 0)
        return NPERR_INVALID_FUNCTABLE_ERROR;

    if (((funcs->version >> 8) & 0xff) > NP_VERSION_MAJOR)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    zerostruct (browser);
    memcpy (&browser, funcs, jmin ((size_t) funcs->size, sizeof (browser)));

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
   #if JUCE_WINDOWS
    #pragma EXPORTED_FUNCTION
   #endif

    log ("NP_Shutdown");
    return NPERR_NO_ERROR;
}

char* NP_GetMIMEDescription()
{
    log ("NP_GetMIMEDescription");
    static String mimeDesc (String (JuceBrowserPlugin_MimeType)
                                + ":" + String (JuceBrowserPlugin_FileSuffix)
                                + ":" + String (JuceBrowserPlugin_Name));

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
          isFirefox4 (false)
    {
        log ("BrowserPluginHolderComponent created");
       #if JUCE_WINDOWS
        parentHWND = 0;
        oldWinProc = 0;
       #else
        currentParentView = nil;
       #endif

        setOpaque (true);
        setWantsKeyboardFocus (false);

        addAndMakeVisible (child = createBrowserPlugin());
        jassert (child != nullptr);   // You have to create one of these!

       #if JUCE_MAC
        const String browserVersion (child->getBrowserVersion());
        isFirefox4 = browserVersion.containsIgnoreCase("firefox/")
                        && browserVersion.fromFirstOccurrenceOf ("firefox/", false, true).getIntValue() >= 4;
       #endif
    }

    ~BrowserPluginHolderComponent()
    {
        log ("BrowserPluginHolderComponent deleted");
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
        if (child != nullptr && ! isFirefox4)
            child->setBounds (getLocalBounds());
    }

    var getObject()
    {
        return child->getJavascriptObject();
    }

    //==============================================================================
    NPP npp;
    ScopedPointer<BrowserPluginComponent> child;

private:
    bool isFirefox4;

    //==============================================================================
#if JUCE_WINDOWS
    HWND parentHWND;
    WNDPROC oldWinProc;

    void resizeToParentWindow (const int requestedWidth = 0, const int requestedHeight = 0)
    {
        if (IsWindow (parentHWND))
        {
            RECT r;
            GetWindowRect (parentHWND, &r);

            int w = r.right - r.left;
            int h = r.bottom - r.top;

            if (w == 0 || h == 0)
            {
                w = requestedWidth;  // On Safari, the HWND can have a zero-size, so we might need to
                h = requestedHeight; // force it to the size that the NPAPI call asked for..
                MoveWindow (parentHWND, r.left, r.top, w, h, TRUE);
            }

            setBounds (0, 0, w, h);
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
        case WM_WINDOWPOSCHANGED:
            //if ((((WINDOWPOS*) lParam)->flags & SWP_NOSIZE) == 0)
            {
                BrowserPluginHolderComponent* const comp = (BrowserPluginHolderComponent*) GetWindowLongPtr (hWnd, GWLP_USERDATA);
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
        HWND newHWND = (window != nullptr ? ((HWND) window->window) : 0);

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
                addToDesktop (0, parentHWND);
                setVisible (true);

                oldWinProc = SubclassWindow (parentHWND, (WNDPROC) interceptingWinProc);

                jassert (GetWindowLongPtr (parentHWND, GWLP_USERDATA) == 0);
                SetWindowLongPtr (parentHWND, GWLP_USERDATA, (LONG_PTR) this);

                resizeToParentWindow (window->width, window->height);
            }
        }
    }

    //==============================================================================
#else
    NSView* currentParentView;

    NSView* findViewAt (NSView* parent, float x, float y) const
    {
        NSRect frame = [parent frame];
        NSRect bounds = [parent bounds];
        x -= frame.origin.x;
        y -= frame.origin.y;

        Rectangle<int> rr (frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
        Rectangle<int> rr2 (bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
        log (String ((int) x) + ", " + String ((int) y) + " - " + nsStringToJuce ([parent description]) + "   " + rr.toString() + "   " + rr2.toString());

        if (x >= 0 && x < frame.size.width && y >= 0 && y < frame.size.height)
        {
            x += bounds.origin.x; // adjust for scrolling panels
            y += bounds.origin.y;

            for (int i = [[parent subviews] count]; --i >= 0;)
            {
                NSView* v = (NSView*) [[parent subviews] objectAtIndex: i];

                if (v != (NSView*) getWindowHandle() && ! [v isHidden])
                {
                    NSView* found = findViewAt (v, x, y);

                    if (found != nil)
                        return found;
                }
            }

            if (isBrowserContentView (parent))
                return parent;
        }

        return nil;
    }

public:
    static bool isBrowserContentView (NSView* v)
    {
        return [[v className] isEqualToString: @"WebNetscapePluginDocumentView"]
            || [[v className] isEqualToString: @"WebPluginDocumentView"]
            || ([[v className] isEqualToString: @"ChildView"] && ([v frame].origin.x != 0 && [v frame].origin.y != 0));
    }

    static bool contains (NSView* parent, NSView* child)
    {
        if (parent == child)
            return true;

        for (int i = [[parent subviews] count]; --i >= 0;)
            if (contains ((NSView*) [[parent subviews] objectAtIndex: i], child))
                return true;

        return false;
    }

    void setWindow (NPWindow* window)
    {
        JUCE_AUTORELEASEPOOL
        {
            log ("setWindow");

            NSView* parentView = nil;
            NP_CGContext* const cgContext = (window != nullptr) ? (NP_CGContext*) window->window : nullptr;
            log ("NP_CGContext: " + String::toHexString ((pointer_sized_int) cgContext));

           #if JUCE_USE_NPAPI_CARBON_UI
            if (WindowRef windowRef = cgContext != nullptr ? (WindowRef) cgContext->window : 0)
            {
                NSWindow* win = [[[NSWindow alloc] initWithWindowRef: windowRef] autorelease];
           #else
            if (NSWindow* win = cgContext != nullptr ? (NSWindow*) cgContext->window : nil)
            {
           #endif
                log ("window: " + nsStringToJuce ([win description]));

                const Rectangle<int> clip (window->clipRect.left, window->clipRect.top,
                                           window->clipRect.right - window->clipRect.left,
                                           window->clipRect.bottom - window->clipRect.top);
                const Rectangle<int> target ((int) window->x, (int) window->y, (int) window->width, (int) window->height);
                const Rectangle<int> intersection (clip.getIntersection (target));

                // in firefox the clip rect is usually out of step with the target rect, but in safari it matches
                log ("plugin window clip: " + clip.toString());
                log ("plugin window target: " + target.toString());
                log ("plugin window intersection: " + intersection.toString());

                NSView* content = [win contentView];

                if (! intersection.isEmpty())
                {
                    log ("content: " + nsStringToJuce ([content description]));

                    float wx = (float) intersection.getCentreX();
                    float wy = (float) intersection.getCentreY();

                    NSRect v = [content convertRect: [content frame] toView: nil];
                    NSRect w = [win frame];

                    log ("content: " + Rectangle<int> (v.origin.x, v.origin.y, v.size.width, v.size.height).toString()
                         + "   frame: " + Rectangle<int> (w.origin.x, w.origin.y, w.size.width, w.size.height).toString());

                    // adjust the requested window pos to deal with the content view's origin within the window
                    wy -= w.size.height - (v.origin.y + v.size.height);

                    parentView = findViewAt (content, wx, wy);

                    if (! isBrowserContentView (parentView))
                        parentView = currentParentView;
                }
                else if (currentParentView != nil && ! target.isEmpty())
                {
                    // Firefox can send lots of spurious resize messages when updating its pages, so this is a
                    // bodge to avoid flickering caused by repeatedly removing and re-adding the view..

                    if (content != nil && contains (content, currentParentView))
                        parentView = currentParentView;
                }
                log ("parent: " + nsStringToJuce ([parentView description]));
            }

            if (parentView != currentParentView)
            {
                log ("new view: " + nsStringToJuce ([parentView description]));

                removeFromDesktop();
                setVisible (false);

                currentParentView = parentView;

                if (parentView != nil)
                {
                    setSize (window->width, window->height);
                    addToDesktop (0, parentView);
                    setVisible (true);
                }
            }

            if (window != nullptr)
            {
                if (isFirefox4)
                {
                    // This voodoo is required to keep the plugin clipped to the correct region and
                    // to stop it overwriting the toolbars in FF4.
                    const Rectangle<int> clip (window->clipRect.left, window->clipRect.top,
                                               window->clipRect.right - window->clipRect.left,
                                               window->clipRect.bottom - window->clipRect.top);
                    const Rectangle<int> target ((int) window->x, (int) window->y, (int) window->width, (int) window->height);

                    Point<int> clipToTargetOffset;

                    {
                        NSView* contentView = [[parentView window] contentView];
                        NSRect v = [contentView convertRect: [contentView frame] toView: nil];
                        NSRect w = [[parentView window] frame];

                        clipToTargetOffset.setX ((int) v.origin.x);
                        clipToTargetOffset.setY ((int) (w.size.height - (v.origin.y + v.size.height)));
                    }

                    if (child != nullptr)
                        child->setBounds (target - clip.getPosition() - clipToTargetOffset);

                    NSRect parentFrame = [parentView frame];

                    [(NSView*) getWindowHandle() setFrame: NSMakeRect (clip.getX() - parentFrame.origin.x,
                                                                       clip.getY() - parentFrame.origin.y,
                                                                       clip.getWidth(), clip.getHeight())];
                }
                else
                {
                    setSize (window->width, window->height);
                }
            }
        }
    }
#endif
};

//==============================================================================
static NPIdentifier getIdentifierFromString (const var::identifier& s) noexcept
{
    return browser.getstringidentifier (s.toString().toUTF8());
}

static var createValueFromNPVariant (NPP npp, const NPVariant& v);
static void createNPVariantFromValue (NPP npp, NPVariant& out, const var& v);

#if JUCE_DEBUG
 static int numDOWNP = 0, numJuceWDO = 0;
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

    var getProperty (const var::identifier& propertyName) const override
    {
        NPVariant result;
        VOID_TO_NPVARIANT (result);
        browser.getproperty (npp, source, getIdentifierFromString (propertyName), &result);
        const var v (createValueFromNPVariant (npp, result));
        browser.releasevariantvalue (&result);
        return v;
    }

    bool hasProperty (const var::identifier& propertyName) const override
    {
        NPVariant result;
        VOID_TO_NPVARIANT (result);
        const bool hasProp = browser.getproperty (npp, source, getIdentifierFromString (propertyName), &result);
        browser.releasevariantvalue (&result);
        return hasProp;
    }

    void setProperty (const var::identifier& propertyName, const var& newValue) override
    {
        NPVariant value;
        createNPVariantFromValue (npp, value, newValue);

        browser.setproperty (npp, source, getIdentifierFromString (propertyName), &value);
        browser.releasevariantvalue (&value);
    }

    void removeProperty (const var::identifier& propertyName) override
    {
        browser.removeproperty (npp, source, getIdentifierFromString (propertyName));
    }

    bool hasMethod (const var::identifier& methodName) const override
    {
        return browser.hasmethod (npp, source, getIdentifierFromString (methodName));
    }

    var invokeMethod (Identifier methodName, const var::NativeFunctionArgs& args) override
    {
        var returnVal;

        NPVariant result;
        VOID_TO_NPVARIANT (result);

        if (args.numArguments > 0)
        {
            HeapBlock<NPVariant> params (args.numArguments);

            for (int i = 0; i < args.numArguments; ++i)
                createNPVariantFromValue (npp, params[i], args.arguments[i]);

            if (browser.invoke (npp, source, getIdentifierFromString (methodName),
                                params, args.numArguments, &result))
            {
                returnVal = createValueFromNPVariant (npp, result);
                browser.releasevariantvalue (&result);
            }

            for (int i = 0; i < args.numArguments; ++i)
                browser.releasevariantvalue (&params[i]);
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
        DBG ("num Juce wrapper objs: " + String (--numJuceWDO));
    }

private:
    NPObjectWrappingDynamicObject (NPP n)  : npp (n)
    {
        DBG ("num Juce wrapper objs: " + String (++numJuceWDO));
    }

    //==============================================================================
    bool construct (const NPVariant *args, uint32_t argCount, NPVariant *result);
    void invalidate() {}

    bool hasMethod (NPIdentifier name)
    {
        DynamicObject* const o = object.getDynamicObject();
        return o != nullptr && o->hasMethod (identifierToString (name));
    }

    bool invoke (NPIdentifier name, const NPVariant* args, uint32_t argCount, NPVariant* out)
    {
        DynamicObject* const o = object.getDynamicObject();
        const var::identifier methodName (identifierToString (name));

        if (o == nullptr || ! o->hasMethod (methodName))
            return false;

        struct ParamHolder
        {
            ParamHolder (uint32_t num)  { params = new var [num]; }
            ~ParamHolder()              { delete[] params; }

            var* params;
        };

        ParamHolder params (argCount);

        for (uint32_t i = 0; i < argCount; ++i)
            params.params[i] = createValueFromNPVariant (npp, args[i]);

        const var result (o->invokeMethod (methodName, var::NativeFunctionArgs (object, params.params, (int) argCount)));

        if (out != nullptr)
            createNPVariantFromValue (npp, *out, result);

        return true;
    }

    bool invokeDefault (const NPVariant* args, uint32_t argCount, NPVariant* result)
    {
        return false;
    }

    bool hasProperty (NPIdentifier name)
    {
        DynamicObject* const o = object.getDynamicObject();
        return o != nullptr && o->hasProperty (identifierToString (name));
    }

    bool getProperty (NPIdentifier name, NPVariant* out)
    {
        DynamicObject* const o = object.getDynamicObject();
        const var::identifier propName (identifierToString (name));

        if (o == nullptr || ! o->hasProperty (propName))
            return false;

        const var result (o->getProperty (propName));

        if (out != nullptr)
            createNPVariantFromValue (npp, *out, result);

        return true;
    }

    bool setProperty (NPIdentifier name, const NPVariant* value)
    {
        DynamicObject* const o = object.getDynamicObject();

        if (value == nullptr || o == nullptr)
            return false;

        o->setProperty (identifierToString (name), createValueFromNPVariant (npp, *value));
        return true;
    }

    bool removeProperty (NPIdentifier name)
    {
        DynamicObject* const o = object.getDynamicObject();
        const var::identifier propName (identifierToString (name));

        if (o == nullptr || ! o->hasProperty (propName))
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

    static var::identifier identifierToString (NPIdentifier id)
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
   #ifndef NP_CLASS_STRUCT_VERSION_ENUM
    NP_CLASS_STRUCT_VERSION, NPObjectWrappingDynamicObject::createInstance,
    NPObjectWrappingDynamicObject::class_deallocate, NPObjectWrappingDynamicObject::class_invalidate,
    NPObjectWrappingDynamicObject::class_hasMethod, NPObjectWrappingDynamicObject::class_invoke,
    NPObjectWrappingDynamicObject::class_invokeDefault, NPObjectWrappingDynamicObject::class_hasProperty,
    NPObjectWrappingDynamicObject::class_getProperty, NPObjectWrappingDynamicObject::class_setProperty,
    NPObjectWrappingDynamicObject::class_removeProperty
   #else
    NP_CLASS_STRUCT_VERSION_ENUM, NPObjectWrappingDynamicObject::createInstance,
    NPObjectWrappingDynamicObject::class_deallocate, NPObjectWrappingDynamicObject::class_invalidate,
    NPObjectWrappingDynamicObject::class_hasMethod, NPObjectWrappingDynamicObject::class_invoke,
    NPObjectWrappingDynamicObject::class_invokeDefault, NPObjectWrappingDynamicObject::class_hasProperty,
    NPObjectWrappingDynamicObject::class_getProperty, NPObjectWrappingDynamicObject::class_setProperty,
    NPObjectWrappingDynamicObject::class_removeProperty, NPObjectWrappingDynamicObject::class_enumerate
   #endif
};

bool NPObjectWrappingDynamicObject::construct (const NPVariant* args, uint32_t argCount, NPVariant* result)
{
    NPObject* const newObj = browser.createobject (npp, &sNPObjectWrappingDynamicObject_NPClass);

    if (newObj == nullptr)
        return false;

    OBJECT_TO_NPVARIANT (newObj, *result);
    return true;
}

NPObject* NPObjectWrappingDynamicObject::create (NPP npp, const var& objectToWrap)
{
    jassert (objectToWrap.getDynamicObject() != nullptr);

    NPObject* const nppObject = browser.createobject (npp, &sNPObjectWrappingDynamicObject_NPClass);

    if (nppObject != nullptr)
        ((NPObjectWrappingDynamicObject*) nppObject)->object = objectToWrap;

    return nppObject;
}


//==============================================================================
static var createValueFromNPVariant (NPP npp, const NPVariant& v)
{
    if (NPVARIANT_IS_BOOLEAN (v))
        return var (NPVARIANT_TO_BOOLEAN (v));
    else if (NPVARIANT_IS_INT32 (v))
        return var (NPVARIANT_TO_INT32 (v));
    else if (NPVARIANT_IS_DOUBLE (v))
        return var (NPVARIANT_TO_DOUBLE (v));
    else if (NPVARIANT_IS_STRING (v))
    {
        return var (String::fromUTF8 ((const char*)
           #if JUCE_MAC
            (NPVARIANT_TO_STRING (v).UTF8Characters), (int) NPVARIANT_TO_STRING (v).UTF8Length));
           #else
            (NPVARIANT_TO_STRING (v).utf8characters), (int) NPVARIANT_TO_STRING (v).utf8length));
           #endif
    }
    else if (NPVARIANT_IS_OBJECT (v) && npp != nullptr)
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
    {
        const String s (v.toString());
        const char* const utf8 = s.toUTF8();
        const int utf8Len = strlen (utf8) + 1;
        char* const stringCopy = (char*) browser.memalloc (utf8Len);
        memcpy (stringCopy, utf8, utf8Len);
        STRINGZ_TO_NPVARIANT (stringCopy, out);
    }
    else if (v.getDynamicObject() != nullptr && npp != nullptr)
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
          scriptObject (nullptr)
    {
    }

    bool setWindow (NPWindow* window)
    {
        if (window != nullptr)
        {
            if (holderComp == nullptr)
                holderComp = new BrowserPluginHolderComponent (npp);

            holderComp->setWindow (window);
        }
        else
        {
            holderComp = nullptr;
            scriptObject = nullptr;
        }

        return true;
    }

    NPObject* getScriptableObject()
    {
        if (scriptObject == nullptr)
            scriptObject = NPObjectWrappingDynamicObject::create (npp, holderComp->getObject());

        if (scriptObject != nullptr && shouldRetainBrowserObject())
            browser.retainobject (scriptObject);

        return scriptObject;
    }

    //==============================================================================
    NPP npp;
    ScopedPointer<BrowserPluginHolderComponent> holderComp;
    NPObject* scriptObject;

private:
    bool shouldRetainBrowserObject() const
    {
        const String version (browser.uagent (npp));

        if (! version.containsIgnoreCase (" AppleWebKit/"))
            return true;

        int versionNum = version.fromFirstOccurrenceOf (" AppleWebKit/", false, true).getIntValue();

        return versionNum == 0 || versionNum >= 420;
    }
};

//==============================================================================
static NPP currentlyInitialisingNPP = nullptr;
static int numPluginInstances = 0;

NPError NPP_New (NPMIMEType pluginType, NPP npp, ::uint16 mode, ::int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
    log ("NPP_New");
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

  #if JUCE_MAC
    browser.setvalue (npp, (NPPVariable) NPNVpluginDrawingModel, (void*) NPDrawingModelCoreGraphics);

   #if JUCE_USE_NPAPI_CARBON_UI
    browser.setvalue (npp, (NPPVariable) 1001 /*NPPVpluginEventModel*/, 0 /*NPEventModelCarbon*/);
   #else
    browser.setvalue (npp, (NPPVariable) 1001 /*NPPVpluginEventModel*/, (void*) 1 /*NPEventModelCocoa*/);
   #endif
  #endif

    if (numPluginInstances++ == 0)
    {
        log ("initialiseJuce_GUI()");
        initialiseJuce_GUI();
    }

    currentlyInitialisingNPP = npp;
    JucePluginInstance* p = new JucePluginInstance (npp);
    currentlyInitialisingNPP = nullptr;

    npp->pdata = (void*) p;
    return NPERR_NO_ERROR;
}

NPError NPP_Destroy (NPP npp, NPSavedData** save)
{
    log ("NPP_Destroy");
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;

    if (p != nullptr)
    {
        delete p;

        if (--numPluginInstances == 0)
        {
            log ("shutdownJuce_GUI()");
            shutdownJuce_GUI();
            browserVersionDesc.clear();
        }
    }

    return NPERR_NO_ERROR;
}

NPError NPP_SetWindow (NPP npp, NPWindow* pNPWindow)
{
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    if (pNPWindow == nullptr)
        return NPERR_GENERIC_ERROR;

    JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;

    if (p == nullptr)
        return NPERR_GENERIC_ERROR;

    currentlyInitialisingNPP = npp;
    NPError result = p->setWindow (pNPWindow) ? NPERR_NO_ERROR
                                              : NPERR_MODULE_LOAD_FAILED_ERROR;
    currentlyInitialisingNPP = nullptr;
    return result;
}

//==============================================================================
NPError NPP_GetValue (NPP npp, NPPVariable variable, void* value)
{
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;

    if (p == nullptr)
        return NPERR_GENERIC_ERROR;

    switch (variable)
    {
        case NPPVpluginNameString:          *((const char**) value) = JuceBrowserPlugin_Name; break;
        case NPPVpluginDescriptionString:   *((const char**) value) = JuceBrowserPlugin_Desc; break;
        case NPPVpluginScriptableNPObject:  *((NPObject**) value) = p->getScriptableObject(); break;
        default:                            return NPERR_GENERIC_ERROR;
    }

    return NPERR_NO_ERROR;
}

NPError NPP_NewStream (NPP npp, NPMIMEType type, NPStream* stream, NPBool seekable, ::uint16* stype)
{
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    return NPERR_NO_ERROR;
}

::int32 NPP_WriteReady (NPP npp, NPStream *stream)
{
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    return 0x0fffffff;
}

::int32 NPP_Write (NPP npp, NPStream *stream, ::int32 offset, ::int32 len, void *buffer)
{
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    return len;
}

NPError NPP_DestroyStream (NPP npp, NPStream *stream, NPError reason)
{
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    return NPERR_NO_ERROR;
}

void NPP_StreamAsFile (NPP npp, NPStream* stream, const char* fname)
{
    if (npp == nullptr)
        return;
}

void NPP_Print (NPP npp, NPPrint* printInfo)
{
    if (npp == nullptr)
        return;
}

void NPP_URLNotify (NPP npp, const char* url, NPReason reason, void* notifyData)
{
    if (npp == nullptr)
        return;
}

NPError NPP_SetValue (NPP npp, NPNVariable variable, void* value)
{
    if (npp == nullptr)
        return NPERR_INVALID_INSTANCE_ERROR;

    return NPERR_NO_ERROR;
}

::int16 NPP_HandleEvent (NPP npp, void* ev)
{
    if (npp != nullptr)
    {
        //JucePluginInstance* const p = (JucePluginInstance*) npp->pdata;
    }

    return 0;
}


//==============================================================================
static NPP getInstance (const BrowserPluginComponent* bpc)
{
    BrowserPluginHolderComponent* holder = dynamic_cast <BrowserPluginHolderComponent*> (bpc->getParentComponent());

    if (holder != nullptr)
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

String BrowserPluginComponent::getBrowserVersion() const
{
    if (browserVersionDesc.isEmpty())
    {
        if (getInstance (this) != nullptr)
            browserVersionDesc << browser.uagent (getInstance (this));
        else
            browserVersionDesc << "Netscape Plugin V" << (int) ((browser.version >> 8) & 0xff)
                               << "." << (int) (browser.version & 0xff);
    }

    return browserVersionDesc;
}

//==============================================================================
#if JUCE_WINDOWS
extern String getActiveXBrowserURL (const BrowserPluginComponent* comp);
#endif

String BrowserPluginComponent::getBrowserURL() const
{
    String result;

   #if JUCE_WINDOWS
    result = getActiveXBrowserURL (this);

    if (result.isNotEmpty())
        return result;
   #endif

    // (FireFox doesn't seem happy if you call this from a background thread..)
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    NPP npp = getInstance (this);
    if (npp != nullptr)
    {
        NPObject* windowObj = nullptr;
        browser.getvalue (npp, NPNVWindowNPObject, &windowObj);

        if (windowObj != nullptr)
        {
            NPVariant location;
            bool ok = browser.getproperty (npp, windowObj,
                                           browser.getstringidentifier ("location"), &location);
            browser.releaseobject (windowObj);

            jassert (ok);
            if (ok)
            {
                NPVariant href;
                ok = browser.getproperty (npp, location.value.objectValue,
                                          browser.getstringidentifier ("href"), &href);
                browser.releasevariantvalue (&location);

                jassert (ok);
                if (ok)
                {
                    result = URL::removeEscapeChars (createValueFromNPVariant (npp, href).toString());
                    browser.releasevariantvalue (&href);
                }
            }
        }
    }

    return result;
}

#undef log
