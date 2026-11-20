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

// ============================================================================
// Linux accessibility bridge (AT-SPI 2 over D-Bus).
//
// JUCE upstream provides no Linux accessibility implementation: the generic
// juce_Accessibility.cpp compiles an empty stub, so the tree that
// AccessibilityHandler already maintains is never exposed to assistive
// technologies or UI-automation tools. This file implements that native layer
// by exposing the handler tree as AT-SPI 2 objects on the accessibility D-Bus
// (direct D-Bus, like Qt; not ATK).
//
//  - libdbus is loaded lazily via dlopen (no build-time/link-time dependency);
//    everything we touch is hand-declared (its ABI is frozen).
//  - All bus work happens on the JUCE message thread, so handler-tree access
//    needs no locking: the bus socket fd is added to the JUCE event loop via
//    LinuxEventLoop::registerFdCallback (using libdbus's watch functions), so
//    the connection is pumped on read-readiness rather than polled.
//
// Object model: one fallback handler serves the whole /org/a11y/atspi/accessible
// subtree. The application node is at .../root; every AccessibilityHandler gets
// a numeric id and lives at .../<id>. Implemented interfaces: Accessible,
// Component, Application (root). Still TODO: Action, Value/Text/Table, the Cache
// object, and change/event signals.
// ============================================================================

#include <dlfcn.h>
#include <cstring>
#include <map>

namespace juce
{

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

#define JUCE_ATSPI_LOG(textExpr) Logger::writeToLog (String ("[AT-SPI] ") + (textExpr))

//==============================================================================
/*  The native peer for a single AccessibilityHandler: it carries the handler's
    AT-SPI object id (assigned by the bridge) so the bridge can map between
    object paths and handlers. Defined before the bridge, which reads the id.
*/
class AccessibilityNativeHandle
{
public:
    explicit AccessibilityNativeHandle (AccessibilityHandler& h) noexcept
        : handler (h)
    {
    }

    AccessibilityHandler& getHandler() noexcept  { return handler; }
    int  getId() const noexcept                  { return id; }
    void setId (int newId) noexcept              { id = newId; }

private:
    AccessibilityHandler& handler;
    int id = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeHandle)
};

namespace
{

//==============================================================================
// Minimal libdbus surface, hand-declared (frozen ABI; no dbus dev headers).
using dbus_bool_t   = uint32_t;
using dbus_uint32_t = uint32_t;

enum DBusBusType { DBUS_BUS_SESSION = 0, DBUS_BUS_SYSTEM = 1, DBUS_BUS_STARTER = 2 };

struct DBusError
{
    const char* name;
    const char* message;
    unsigned int dummy1 : 1, dummy2 : 1, dummy3 : 1, dummy4 : 1, dummy5 : 1;
    void* padding1;
};

struct DBusMessageIter
{
    void* dummy1;
    void* dummy2;
    dbus_uint32_t dummy3;
    int dummy4, dummy5, dummy6, dummy7, dummy8, dummy9, dummy10, dummy11;
    int pad1;
    void* pad2;
    void* pad3;
};

using DBusConnection = void;
using DBusMessage    = void;

constexpr int DBUS_TYPE_INT32       = 'i';
constexpr int DBUS_TYPE_UINT32      = 'u';
constexpr int DBUS_TYPE_STRING      = 's';
constexpr int DBUS_TYPE_OBJECT_PATH = 'o';
constexpr int DBUS_TYPE_ARRAY       = 'a';
constexpr int DBUS_TYPE_VARIANT     = 'v';
constexpr int DBUS_TYPE_STRUCT      = 'r';
constexpr int DBUS_TYPE_BOOLEAN     = 'b';
constexpr int DBUS_TYPE_DOUBLE      = 'd';
constexpr int DBUS_TYPE_DICT_ENTRY  = 'e';

constexpr int DBUS_HANDLER_RESULT_HANDLED         = 0;
constexpr int DBUS_HANDLER_RESULT_NOT_YET_HANDLED = 1;
constexpr int DBUS_HANDLER_RESULT_NEED_MEMORY     = 2;

using DBusObjectPathUnregisterFunction = void (*) (DBusConnection*, void*);
using DBusObjectPathMessageFunction    = int  (*) (DBusConnection*, DBusMessage*, void*);

struct DBusObjectPathVTable
{
    DBusObjectPathUnregisterFunction unregister_function;
    DBusObjectPathMessageFunction    message_function;
    void (*dbus_internal_pad1) (void*);
    void (*dbus_internal_pad2) (void*);
    void (*dbus_internal_pad3) (void*);
    void (*dbus_internal_pad4) (void*);
};

using DBusWatch                = void;
using DBusAddWatchFunction     = dbus_bool_t (*) (DBusWatch*, void*);
using DBusRemoveWatchFunction  = void        (*) (DBusWatch*, void*);
using DBusWatchToggledFunction = void        (*) (DBusWatch*, void*);
using DBusFreeFunction         = void        (*) (void*);

enum DBusDispatchStatus { DBUS_DISPATCH_DATA_REMAINS = 0, DBUS_DISPATCH_COMPLETE = 1, DBUS_DISPATCH_NEED_MEMORY = 2 };

//==============================================================================
struct DBusLibrary
{
    bool open()
    {
        if (handle != nullptr)
            return true;

        handle = dlopen ("libdbus-1.so.3", RTLD_NOW | RTLD_GLOBAL);

        if (handle == nullptr)
            return false;

        bool ok = true;
        ok &= load (threads_init_default,              "dbus_threads_init_default");
        ok &= load (error_init,                        "dbus_error_init");
        ok &= load (error_is_set,                      "dbus_error_is_set");
        ok &= load (error_free,                        "dbus_error_free");
        ok &= load (bus_get,                           "dbus_bus_get");
        ok &= load (connection_open_private,           "dbus_connection_open_private");
        ok &= load (bus_register,                      "dbus_bus_register");
        ok &= load (bus_get_unique_name,               "dbus_bus_get_unique_name");
        ok &= load (connection_set_exit_on_disconnect, "dbus_connection_set_exit_on_disconnect");
        ok &= load (connection_close,                  "dbus_connection_close");
        ok &= load (connection_unref,                  "dbus_connection_unref");
        ok &= load (connection_read_write_dispatch,    "dbus_connection_read_write_dispatch");
        ok &= load (connection_read_write,             "dbus_connection_read_write");
        ok &= load (connection_set_watch_functions,    "dbus_connection_set_watch_functions");
        ok &= load (connection_get_dispatch_status,    "dbus_connection_get_dispatch_status");
        ok &= load (connection_flush,                  "dbus_connection_flush");
        ok &= load (watch_get_unix_fd,                 "dbus_watch_get_unix_fd");
        ok &= load (connection_register_fallback,      "dbus_connection_register_fallback");
        ok &= load (connection_send,                   "dbus_connection_send");
        ok &= load (message_new_method_call,           "dbus_message_new_method_call");
        ok &= load (message_new_method_return,         "dbus_message_new_method_return");
        ok &= load (message_new_signal,                "dbus_message_new_signal");
        ok &= load (connection_send_with_reply_block,  "dbus_connection_send_with_reply_and_block");
        ok &= load (message_get_interface,             "dbus_message_get_interface");
        ok &= load (message_get_member,                "dbus_message_get_member");
        ok &= load (message_get_path,                  "dbus_message_get_path");
        ok &= load (message_iter_init,                 "dbus_message_iter_init");
        ok &= load (message_iter_init_append,          "dbus_message_iter_init_append");
        ok &= load (message_iter_next,                 "dbus_message_iter_next");
        ok &= load (message_iter_get_arg_type,         "dbus_message_iter_get_arg_type");
        ok &= load (message_iter_get_basic,            "dbus_message_iter_get_basic");
        ok &= load (message_iter_append_basic,         "dbus_message_iter_append_basic");
        ok &= load (message_iter_open_container,       "dbus_message_iter_open_container");
        ok &= load (message_iter_close_container,      "dbus_message_iter_close_container");
        ok &= load (message_iter_recurse,              "dbus_message_iter_recurse");
        ok &= load (message_unref,                     "dbus_message_unref");

        return ok;
    }

    dbus_bool_t    (*threads_init_default)();
    void           (*error_init) (DBusError*);
    dbus_bool_t    (*error_is_set) (const DBusError*);
    void           (*error_free) (DBusError*);
    DBusConnection* (*bus_get) (DBusBusType, DBusError*);
    DBusConnection* (*connection_open_private) (const char*, DBusError*);
    dbus_bool_t    (*bus_register) (DBusConnection*, DBusError*);
    const char*    (*bus_get_unique_name) (DBusConnection*);
    void           (*connection_set_exit_on_disconnect) (DBusConnection*, dbus_bool_t);
    void           (*connection_close) (DBusConnection*);
    void           (*connection_unref) (DBusConnection*);
    dbus_bool_t    (*connection_read_write_dispatch) (DBusConnection*, int);
    dbus_bool_t    (*connection_read_write) (DBusConnection*, int);
    dbus_bool_t    (*connection_set_watch_functions) (DBusConnection*, DBusAddWatchFunction, DBusRemoveWatchFunction, DBusWatchToggledFunction, void*, DBusFreeFunction);
    DBusDispatchStatus (*connection_get_dispatch_status) (DBusConnection*);
    void           (*connection_flush) (DBusConnection*);
    int            (*watch_get_unix_fd) (DBusWatch*);
    dbus_bool_t    (*connection_register_fallback) (DBusConnection*, const char*, const DBusObjectPathVTable*, void*);
    dbus_bool_t    (*connection_send) (DBusConnection*, DBusMessage*, dbus_uint32_t*);
    DBusMessage*   (*message_new_method_call) (const char*, const char*, const char*, const char*);
    DBusMessage*   (*message_new_method_return) (DBusMessage*);
    DBusMessage*   (*message_new_signal) (const char*, const char*, const char*);
    DBusMessage*   (*connection_send_with_reply_block) (DBusConnection*, DBusMessage*, int, DBusError*);
    const char*    (*message_get_interface) (DBusMessage*);
    const char*    (*message_get_member) (DBusMessage*);
    const char*    (*message_get_path) (DBusMessage*);
    dbus_bool_t    (*message_iter_init) (DBusMessage*, DBusMessageIter*);
    void           (*message_iter_init_append) (DBusMessage*, DBusMessageIter*);
    dbus_bool_t    (*message_iter_next) (DBusMessageIter*);
    int            (*message_iter_get_arg_type) (DBusMessageIter*);
    void           (*message_iter_get_basic) (DBusMessageIter*, void*);
    dbus_bool_t    (*message_iter_append_basic) (DBusMessageIter*, int, const void*);
    dbus_bool_t    (*message_iter_open_container) (DBusMessageIter*, int, const char*, DBusMessageIter*);
    dbus_bool_t    (*message_iter_close_container) (DBusMessageIter*, DBusMessageIter*);
    void           (*message_iter_recurse) (DBusMessageIter*, DBusMessageIter*);
    void           (*message_unref) (DBusMessage*);

private:
    template <typename Fn>
    bool load (Fn& fn, const char* symbol)
    {
        fn = reinterpret_cast<Fn> (dlsym (handle, symbol));
        return fn != nullptr;
    }

    void* handle = nullptr;
};

//==============================================================================
// AtspiRole numeric value for a JUCE role (from atspi-constants.h).
uint32_t atspiRole (AccessibilityRole role)
{
    switch (role)
    {
        case AccessibilityRole::button:        return 43; // PUSH_BUTTON
        case AccessibilityRole::toggleButton:  return 62; // TOGGLE_BUTTON
        case AccessibilityRole::radioButton:   return 44; // RADIO_BUTTON
        case AccessibilityRole::comboBox:      return 11; // COMBO_BOX
        case AccessibilityRole::image:         return 27; // IMAGE
        case AccessibilityRole::slider:        return 51; // SLIDER
        case AccessibilityRole::label:         return 29; // LABEL
        case AccessibilityRole::staticText:    return 29; // LABEL
        case AccessibilityRole::editableText:  return 79; // ENTRY
        case AccessibilityRole::menuItem:      return 35; // MENU_ITEM
        case AccessibilityRole::menuBar:       return 34; // MENU_BAR
        case AccessibilityRole::popupMenu:     return 33; // MENU
        case AccessibilityRole::table:         return 55; // TABLE
        case AccessibilityRole::tableHeader:   return 57; // TABLE_COLUMN_HEADER
        case AccessibilityRole::column:        return 57; // TABLE_COLUMN_HEADER
        case AccessibilityRole::row:           return 90; // TABLE_ROW
        case AccessibilityRole::cell:          return 56; // TABLE_CELL
        case AccessibilityRole::hyperlink:     return 88; // LINK
        case AccessibilityRole::list:          return 31; // LIST
        case AccessibilityRole::listItem:      return 32; // LIST_ITEM
        case AccessibilityRole::tree:          return 65; // TREE
        case AccessibilityRole::treeItem:      return 91; // TREE_ITEM
        case AccessibilityRole::progressBar:   return 42; // PROGRESS_BAR
        case AccessibilityRole::group:         return 39; // PANEL
        case AccessibilityRole::dialogWindow:  return 16; // DIALOG
        case AccessibilityRole::window:        return 23; // FRAME
        case AccessibilityRole::scrollBar:     return 48; // SCROLL_BAR
        case AccessibilityRole::tooltip:       return 64; // TOOL_TIP
        case AccessibilityRole::splashScreen:  return 23; // FRAME
        case AccessibilityRole::ignored:
        case AccessibilityRole::unspecified:
        default:                               return 20; // FILLER
    }
}

// The canonical AT-SPI name for a JUCE role (matching atspi_role_get_name()
// for the values returned by atspiRole() above).
const char* atspiRoleName (AccessibilityRole role)
{
    switch (role)
    {
        case AccessibilityRole::button:        return "push button";
        case AccessibilityRole::toggleButton:  return "toggle button";
        case AccessibilityRole::radioButton:   return "radio button";
        case AccessibilityRole::comboBox:      return "combo box";
        case AccessibilityRole::image:         return "image";
        case AccessibilityRole::slider:        return "slider";
        case AccessibilityRole::label:         return "label";
        case AccessibilityRole::staticText:    return "label";
        case AccessibilityRole::editableText:  return "entry";
        case AccessibilityRole::menuItem:      return "menu item";
        case AccessibilityRole::menuBar:       return "menu bar";
        case AccessibilityRole::popupMenu:     return "menu";
        case AccessibilityRole::table:         return "table";
        case AccessibilityRole::tableHeader:   return "table column header";
        case AccessibilityRole::column:        return "table column header";
        case AccessibilityRole::row:           return "table row";
        case AccessibilityRole::cell:          return "table cell";
        case AccessibilityRole::hyperlink:     return "link";
        case AccessibilityRole::list:          return "list";
        case AccessibilityRole::listItem:      return "list item";
        case AccessibilityRole::tree:          return "tree";
        case AccessibilityRole::treeItem:      return "tree item";
        case AccessibilityRole::progressBar:   return "progress bar";
        case AccessibilityRole::group:         return "panel";
        case AccessibilityRole::dialogWindow:  return "dialog";
        case AccessibilityRole::window:        return "frame";
        case AccessibilityRole::scrollBar:     return "scroll bar";
        case AccessibilityRole::tooltip:       return "tool tip";
        case AccessibilityRole::splashScreen:  return "frame";
        case AccessibilityRole::ignored:
        case AccessibilityRole::unspecified:
        default:                               return "filler";
    }
}

// Pack a JUCE AccessibleState into the 64 AT-SPI state bits
// (wire format: array of two uint32, low word first).
uint64_t atspiStateBits (const AccessibleState& s)
{
    uint64_t bits = 0;
    auto set = [&bits] (int bit) { bits |= (1ull << bit); };

    if (! s.isIgnored())
    {
        set (8);   // ENABLED
        set (24);  // SENSITIVE
        if (! s.isAccessibleOffscreen())
        {
            set (25);  // SHOWING
            set (30);  // VISIBLE
        }
    }

    if (s.isFocusable())  set (11); // FOCUSABLE
    if (s.isFocused())    set (12); // FOCUSED
    if (s.isCheckable())  set (41); // CHECKABLE
    if (s.isChecked())    set (4);  // CHECKED
    if (s.isExpandable()) set (9);  // EXPANDABLE
    if (s.isExpanded())   set (10); // EXPANDED
    if (s.isSelectable())      set (22); // SELECTABLE
    if (s.isSelected())        set (23); // SELECTED
    if (s.isMultiSelectable()) set (18); // MULTISELECTABLE

    return bits;
}

// Visibility walk equivalent to Component::isShowing() minus the final
// peer->isMinimised() query. That query takes the X display lock, and another
// thread (e.g. a GL render thread inside glXMakeCurrent/DRI3) can hold that
// lock for an unbounded time - taking it from the D-Bus dispatch deadlocks
// the message thread. Skipping it only costs the minimised case, which AT-SPI
// models on the frame (ICONIFIED), not on the content.
static bool isShowingWithoutXQuery (const Component& component)
{
    for (auto* c = &component;; )
    {
        if (! c->isVisible())
            return false;

        if (auto* parent = c->getParentComponent())
            c = parent;
        else
            return c->getPeer() != nullptr;
    }
}

// State bits for a handler, with SHOWING/VISIBLE reflecting the component's
// actual visibility. The tree-navigation paths filter hidden elements while
// enumerating children, but state queries and the Cache snapshot reach
// handlers directly (e.g. the content of a hidden tab), so the bits must tell
// the truth on their own. The walk is cheap (per-element parent chain);
// AccessibilityHandler::isVisibleWithinParent() is not - it runs the focus
// traverser per element, which is quadratic over a Cache snapshot.
uint64_t atspiStateBits (AccessibilityHandler& handler)
{
    auto bits = atspiStateBits (handler.getCurrentState());

    if (! isShowingWithoutXQuery (handler.getComponent()))
        bits &= ~((1ull << 25) | (1ull << 30)); // SHOWING | VISIBLE

    return bits;
}

//==============================================================================
/*  Process-wide owner of the AT-SPI bus connection and the handler<->object-path
    registry. Lazily brought up the first time an AccessibilityHandler exists.
*/
class AtSpiBridge final
{
public:
    static AtSpiBridge& getInstance()
    {
        // Intentionally leaked process-wide service (avoids static-destruction
        // ordering issues between the Timer and the MessageManager).
        static AtSpiBridge* instance = new AtSpiBridge();
        return *instance;
    }

    void ensureStarted()
    {
        if (! MessageManager::getInstance()->isThisTheMessageThread())
        {
            MessageManager::callAsync ([this] { ensureStarted(); });
            return;
        }

        if (! started)
        {
            started = true;
            connect();
        }
    }

    // Assign a stable id to a handler and remember it for path<->handler lookup.
    int registerHandler (AccessibilityHandler& h)
    {
        // the registry is read from the message thread's D-Bus dispatch, so
        // components (and their handlers) must live on the message thread
        JUCE_ASSERT_MESSAGE_THREAD

        const int id = nextId++;
        handlers[id] = &h;
        return id;
    }

    void unregisterHandler (int id)
    {
        JUCE_ASSERT_MESSAGE_THREAD

        handlers.erase (id);
    }

    //==============================================================================
    // Translate JUCE accessibility notifications into AT-SPI signals.
    void onAccessibilityEvent (const AccessibilityHandler& h, AccessibilityEvent e)
    {
        if (a11yConnection == nullptr)
            return;


        auto* handler = const_cast<AccessibilityHandler*> (&h);
        const auto path = pathForHandler (handler);

        switch (e)
        {
            case AccessibilityEvent::titleChanged:         emitEvent (path, ifEventObject, "PropertyChange", "accessible-name", 0, 0, nullptr); break;
            case AccessibilityEvent::valueChanged:         emitEvent (path, ifEventObject, "PropertyChange", "accessible-value", 0, 0, nullptr); break;
            case AccessibilityEvent::textChanged:          emitEvent (path, ifEventObject, "TextChanged", "", 0, 0, nullptr); break;
            case AccessibilityEvent::textSelectionChanged: emitEvent (path, ifEventObject, "TextSelectionChanged", "", 0, 0, nullptr); break;
            case AccessibilityEvent::rowSelectionChanged:  emitEvent (path, ifEventObject, "SelectionChanged", "", 0, 0, nullptr); break;
            case AccessibilityEvent::structureChanged:     break; // no precise AT-SPI equivalent
            default: break;
        }
    }

    void onAccessibilityEvent (const AccessibilityHandler& h, detail::AccessibilityHelpers::Event e)
    {
        if (a11yConnection == nullptr)
            return;


        using Ev = detail::AccessibilityHelpers::Event;
        auto* handler = const_cast<AccessibilityHandler*> (&h);

        switch (e)
        {
            case Ev::elementCreated:
                // NB: don't compute the child index here — that would enumerate
                // the parent's children, which creates sibling handlers and fires
                // their elementCreated, recursing during tree construction. The
                // child reference is carried in any_data; clients re-query.
                emitEvent (pathForHandler (handler->getParent()), ifEventObject, "ChildrenChanged", "add", 0, 0, handler);
                break;
            case Ev::elementDestroyed:
                emitEvent (pathForHandler (handler->getParent()), ifEventObject, "ChildrenChanged", "remove", 0, 0, handler);
                break;
            case Ev::elementMovedOrResized:
                emitEvent (pathForHandler (handler), ifEventObject, "BoundsChanged", "", 0, 0, nullptr);
                break;
            case Ev::focusChanged:
                emitEvent (pathForHandler (handler), ifEventObject, "StateChanged", "focused", 1, 0, nullptr);
                break;
            case Ev::windowOpened:
                emitEvent (pathForHandler (handler), ifEventWindow, "Create", "", 0, 0, nullptr);
                emitEvent (pathForHandler (handler), ifEventWindow, "Activate", "", 0, 0, nullptr);
                break;
            case Ev::windowClosed:
                emitEvent (pathForHandler (handler), ifEventWindow, "Deactivate", "", 0, 0, nullptr);
                emitEvent (pathForHandler (handler), ifEventWindow, "Destroy", "", 0, 0, nullptr);
                break;
        }
    }

    // Screen-reader announcement (an AT-SPI live-region event; at-spi2-core >= 2.46).
    // Announcements are application-wide, so the signal is emitted from the root.
    void announce (const String& text, int32_t politeness)
    {
        if (a11yConnection == nullptr)
            return;

        auto* sig = dbus.message_new_signal (rootPath, ifEventObject, "Announcement");
        if (sig == nullptr)
            return;

        DBusMessageIter it;
        dbus.message_iter_init_append (sig, &it);
        const char* detail = "";
        int32_t zero = 0;
        dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &detail);
        dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &politeness);
        dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &zero);
        appendVariantString (it, text);

        DBusMessageIter props;
        dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "{sv}", &props);
        dbus.message_iter_close_container (&it, &props);

        dbus.connection_send (a11yConnection, sig, nullptr);
        dbus.message_unref (sig);

        // flush() can block if the socket buffer to the registry is full
        // (i.e. the peer has been wedged for a long while). Accepted: the
        // alternative is write-watch plumbing, and ATs depend on prompt
        // event delivery.
        dbus.connection_flush (a11yConnection);
    }

private:
    AtSpiBridge() = default;

    //==============================================================================
    void connect()
    {
        if (! dbus.open())
            return (void) JUCE_ATSPI_LOG ("libdbus not available; accessibility disabled");

        dbus.threads_init_default();

        DBusError err;
        dbus.error_init (&err);

        auto* session = dbus.bus_get (DBUS_BUS_SESSION, &err);

        if (session == nullptr || dbus.error_is_set (&err))
            return failConnect ("no session bus", err);

        const auto address = getA11yBusAddress (session, err);
        dbus.connection_unref (session); // shared connection: unref, never close

        if (address.isEmpty())
            return failConnect ("could not get a11y bus address", err);

        a11yConnection = dbus.connection_open_private (address.toRawUTF8(), &err);

        if (a11yConnection == nullptr || dbus.error_is_set (&err))
            return failConnect ("could not open a11y bus", err);

        dbus.connection_set_exit_on_disconnect (a11yConnection, false);

        if (! dbus.bus_register (a11yConnection, &err) || dbus.error_is_set (&err))
            return failConnect ("could not register on a11y bus", err);

        if (auto* unique = dbus.bus_get_unique_name (a11yConnection))
            myName = String::fromUTF8 (unique);

        JUCE_ATSPI_LOG (String ("connected to a11y bus: ") + address);

        registerSubtree();
        embed();
        setupDispatch();
    }

    //==============================================================================
    // Drive the bus from the JUCE message thread with no polling: libdbus's watch
    // functions tell us the connection's socket fd, which we add to the event loop;
    // when it becomes readable we read, dispatch and flush replies.
    void setupDispatch()
    {
        dbus.connection_set_watch_functions (a11yConnection,
                                             &AtSpiBridge::addWatchThunk,
                                             &AtSpiBridge::removeWatchThunk,
                                             &AtSpiBridge::toggledWatchThunk,
                                             this, nullptr);
        drain(); // anything already buffered
    }

    void drain()
    {
        if (a11yConnection == nullptr)
            return;

        // libdbus's dispatch lock is not recursive: if a dispatched method
        // (e.g. an action that opens a modal loop) pumps the event loop and
        // the bus fd fires again, a nested dispatch would deadlock on the
        // same thread. Read pending data off the socket (keeping the fd
        // quiet) but leave dispatching to the outer call's loop.
        if (draining)
        {
            dbus.connection_read_write (a11yConnection, 0);
            return;
        }

        const ScopedValueSetter<bool> scope (draining, true);

        while (dbus.connection_read_write_dispatch (a11yConnection, 0)
               && dbus.connection_get_dispatch_status (a11yConnection) == DBUS_DISPATCH_DATA_REMAINS)
        {}

        dbus.connection_flush (a11yConnection);
    }

    static dbus_bool_t addWatchThunk     (DBusWatch* w, void* data) { return static_cast<AtSpiBridge*> (data)->onAddWatch (w); }
    static void        removeWatchThunk  (DBusWatch* w, void* data) { static_cast<AtSpiBridge*> (data)->onRemoveWatch (w); }
    static void        toggledWatchThunk (DBusWatch*,   void*)      {}

    dbus_bool_t onAddWatch (DBusWatch* w)
    {
        const int fd = dbus.watch_get_unix_fd (w);

        // A connection's read and write watches share the same socket fd; we
        // watch it for readability (read_write_dispatch handles writes), so we
        // ref-count and register each unique fd once.
        if (fd >= 0 && ++watchedFds[fd] == 1)
            LinuxEventLoop::registerFdCallback (fd, [this] (int) { drain(); });

        return true;
    }

    void onRemoveWatch (DBusWatch* w)
    {
        const auto it = watchedFds.find (dbus.watch_get_unix_fd (w));

        if (it != watchedFds.end() && --(it->second) <= 0)
        {
            const int fd = it->first;
            watchedFds.erase (it);
            LinuxEventLoop::unregisterFdCallback (fd);
        }
    }

    void registerSubtree()
    {
        DBusObjectPathVTable vtable {};
        vtable.message_function = &AtSpiBridge::messageThunk;

        // Register one level up so the same handler serves both the accessible
        // subtree (.../accessible/*) and the cache object (.../cache).
        if (! dbus.connection_register_fallback (a11yConnection, atspiPrefix, &vtable, this))
            JUCE_ATSPI_LOG ("failed to register the accessibility object tree");
    }

    void embed()
    {
        const char* uniqueName = myName.toRawUTF8();

        auto* call = dbus.message_new_method_call ("org.a11y.atspi.Registry", rootPath,
                                                   "org.a11y.atspi.Socket", "Embed");
        if (call == nullptr)
            return;

        DBusMessageIter args, plug;
        dbus.message_iter_init_append (call, &args);
        dbus.message_iter_open_container (&args, DBUS_TYPE_STRUCT, nullptr, &plug);
        dbus.message_iter_append_basic (&plug, DBUS_TYPE_STRING, &uniqueName);
        dbus.message_iter_append_basic (&plug, DBUS_TYPE_OBJECT_PATH, &rootPath);
        dbus.message_iter_close_container (&args, &plug);

        DBusError err;
        dbus.error_init (&err);
        auto* reply = dbus.connection_send_with_reply_block (a11yConnection, call, 2000, &err);
        dbus.message_unref (call);

        if (reply != nullptr)
        {
            // reply is the registry's (so) reference, which becomes our parent.
            DBusMessageIter it, st;
            if (dbus.message_iter_init (reply, &it) && dbus.message_iter_get_arg_type (&it) == DBUS_TYPE_STRUCT)
            {
                dbus.message_iter_recurse (&it, &st);
                const char* n = nullptr; const char* p = nullptr;
                dbus.message_iter_get_basic (&st, &n);
                if (dbus.message_iter_next (&st)) dbus.message_iter_get_basic (&st, &p);
                if (n != nullptr) registryName = String::fromUTF8 (n);
                if (p != nullptr) registryPath = String::fromUTF8 (p);
            }
            dbus.message_unref (reply);
            JUCE_ATSPI_LOG ("registered application root with the AT-SPI registry");
        }
        else
        {
            JUCE_ATSPI_LOG (String ("Embed failed")
                            + (err.message != nullptr ? String (" (") + err.message + ")" : String()));
            dbus.error_free (&err);
        }
    }

    String getA11yBusAddress (DBusConnection* session, DBusError& err)
    {
        auto* call = dbus.message_new_method_call ("org.a11y.Bus", "/org/a11y/bus", "org.a11y.Bus", "GetAddress");
        if (call == nullptr)
            return {};

        auto* reply = dbus.connection_send_with_reply_block (session, call, 2000, &err);
        dbus.message_unref (call);

        if (reply == nullptr || dbus.error_is_set (&err))
            return {};

        String result;
        DBusMessageIter iter;
        if (dbus.message_iter_init (reply, &iter) && dbus.message_iter_get_arg_type (&iter) == DBUS_TYPE_STRING)
        {
            const char* address = nullptr;
            dbus.message_iter_get_basic (&iter, &address);
            if (address != nullptr)
                result = String::fromUTF8 (address);
        }

        dbus.message_unref (reply);
        return result;
    }

    void failConnect (const char* what, DBusError& err)
    {
        JUCE_ATSPI_LOG (String ("accessibility disabled: ") + what
                        + (err.message != nullptr ? String (" (") + err.message + ")" : String()));
        dbus.error_free (&err);

        if (a11yConnection != nullptr)
        {
            dbus.connection_close (a11yConnection);
            dbus.connection_unref (a11yConnection);
            a11yConnection = nullptr;
        }
    }

    //==============================================================================
    // Path helpers. The application node is at .../root; a handler is at .../<id>.
    String pathForHandler (const AccessibilityHandler* h) const
    {
        if (h == nullptr)
            return "/org/a11y/atspi/null";

        if (auto* nativeHandle = h->getNativeImplementation())
            return String (accessiblePrefix) + "/" + String (nativeHandle->getId());

        return "/org/a11y/atspi/null";
    }

    AccessibilityHandler* handlerForPath (const String& path) const
    {
        const String prefix = String (accessiblePrefix) + "/";
        if (! path.startsWith (prefix))
            return nullptr;

        const auto id = path.substring (prefix.length()).getIntValue();
        const auto it = handlers.find (id);
        return it != handlers.end() ? it->second : nullptr;
    }

    //==============================================================================
    static int messageThunk (DBusConnection* conn, DBusMessage* msg, void* userData)
    {
        return static_cast<AtSpiBridge*> (userData)->handleMessage (conn, msg);
    }

    int handleMessage (DBusConnection* conn, DBusMessage* msg)
    {
        const char* path = dbus.message_get_path (msg);
        if (path == nullptr)
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

        const String p = String::fromUTF8 (path);

        if (p == cachePath)
            return handleCache (conn, msg);

        if (p == rootPath)
            return handleObject (conn, msg, nullptr); // the application node

        if (auto* h = handlerForPath (p))
            return handleObject (conn, msg, h);

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    // Serves one node. handler == nullptr is the application root node.
    int handleObject (DBusConnection* conn, DBusMessage* msg, AccessibilityHandler* handler)
    {
        const char* iface  = dbus.message_get_interface (msg);
        const char* member = dbus.message_get_member (msg);
        if (iface == nullptr || member == nullptr)
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

        DBusMessage* reply = nullptr;
        DBusMessageIter it {};
        bool outOfMemory = false;
        const bool isRoot = (handler == nullptr);

        auto begin = [&]
        {
            reply = dbus.message_new_method_return (msg);

            if (reply != nullptr)
                dbus.message_iter_init_append (reply, &it);
            else
                outOfMemory = true;
        };

        if (equals (iface, ifProperties) && equals (member, "Get"))
        {
            const char* prop = readSecondString (msg);
            auto* value = handler != nullptr ? handler->getValueInterface() : nullptr;
            auto* table = handler != nullptr ? handler->getTableInterface() : nullptr;
            auto* cell  = handler != nullptr ? handler->getCellInterface() : nullptr;
            auto* text  = handler != nullptr ? handler->getTextInterface() : nullptr;
            begin();

            if (equals (prop, "Name"))
                appendVariantString (it, isRoot ? getAppName() : handler->getTitle());
            else if (equals (prop, "Description"))
                appendVariantString (it, isRoot ? String() : handler->getDescription());
            else if (equals (prop, "ToolkitName"))
                appendVariantString (it, "JUCE");
            else if (equals (prop, "AtspiVersion"))
                appendVariantString (it, "2.1");
            else if (equals (prop, "ChildCount"))
                appendVariantInt32 (it, childCount (handler));
            else if (equals (prop, "NActions"))
                appendVariantInt32 (it, actionCount (handler));
            else if (equals (prop, "Parent"))
                appendVariantParent (it, handler);
            else if (value != nullptr && equals (prop, "CurrentValue"))
                appendVariantDouble (it, value->getCurrentValue());
            else if (value != nullptr && equals (prop, "MinimumValue"))
                appendVariantDouble (it, value->getRange().getMinimumValue());
            else if (value != nullptr && equals (prop, "MaximumValue"))
                appendVariantDouble (it, value->getRange().getMaximumValue());
            else if (value != nullptr && equals (prop, "MinimumIncrement"))
                appendVariantDouble (it, value->getRange().getInterval());
            else if (table != nullptr && equals (prop, "NRows"))
                appendVariantInt32 (it, table->getNumRows());
            else if (table != nullptr && equals (prop, "NColumns"))
                appendVariantInt32 (it, table->getNumColumns());
            else if (table != nullptr && (equals (prop, "NSelectedRows") || equals (prop, "NSelectedColumns")))
                appendVariantInt32 (it, 0);
            else if (table != nullptr && (equals (prop, "Caption") || equals (prop, "Summary")))
                appendVariantObjectRef (it, pathForHandler (nullptr));
            else if (cell != nullptr && equals (prop, "RowSpan"))
                appendVariantInt32 (it, cellPosition (handler).rowSpan);
            else if (cell != nullptr && equals (prop, "ColumnSpan"))
                appendVariantInt32 (it, cellPosition (handler).colSpan);
            else if (cell != nullptr && equals (prop, "Position"))
            {
                const auto p = cellPosition (handler);
                appendVariantPosition (it, p.row, p.col);
            }
            else if (cell != nullptr && equals (prop, "Table"))
                appendVariantObjectRef (it, pathForHandler (cellPosition (handler).table));
            else if (text != nullptr && equals (prop, "CharacterCount"))
                appendVariantInt32 (it, text->getTotalNumCharacters());
            else if (text != nullptr && equals (prop, "CaretOffset"))
                appendVariantInt32 (it, text->getTextInsertionOffset());
            else if (equals (prop, "NSelectedChildren"))
                appendVariantInt32 (it, selectedChildCount (handler));
            else
                appendVariantString (it, String());
        }
        else if (equals (iface, ifProperties) && equals (member, "Set"))
        {
            // org.freedesktop.DBus.Properties.Set (s interface, s property, v value).
            // The only writable property we expose is Value.CurrentValue.
            DBusMessageIter in;
            const char* setIface = ""; const char* prop = "";
            if (dbus.message_iter_init (msg, &in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_STRING)
            {
                dbus.message_iter_get_basic (&in, &setIface);
                if (dbus.message_iter_next (&in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_STRING)
                {
                    dbus.message_iter_get_basic (&in, &prop);
                    if (dbus.message_iter_next (&in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_VARIANT)
                    {
                        DBusMessageIter var;
                        dbus.message_iter_recurse (&in, &var);

                        if (handler != nullptr && equals (setIface, ifValue) && equals (prop, "CurrentValue")
                            && dbus.message_iter_get_arg_type (&var) == DBUS_TYPE_DOUBLE)
                        {
                            double newValue = 0.0;
                            dbus.message_iter_get_basic (&var, &newValue);
                            if (auto* value = handler->getValueInterface(); value != nullptr && ! value->isReadOnly())
                                value->setValue (newValue);
                        }
                    }
                }
            }
            begin(); // empty reply
        }
        else if (equals (iface, ifProperties) && equals (member, "GetAll"))
        {
            const char* reqIface = "";
            { DBusMessageIter in; if (dbus.message_iter_init (msg, &in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_STRING) dbus.message_iter_get_basic (&in, &reqIface); }
            begin();
            DBusMessageIter dict;
            dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "{sv}", &dict);

            if (equals (reqIface, ifAccessible))
            {
                appendStringEntry (dict, "Name", isRoot ? getAppName() : handler->getTitle());
                appendStringEntry (dict, "Description", isRoot ? String() : handler->getDescription());
                appendInt32Entry  (dict, "ChildCount", childCount (handler));
                appendParentEntry (dict, handler);
            }
            else if (equals (reqIface, ifApplication))
            {
                appendStringEntry (dict, "ToolkitName", "JUCE");
                appendStringEntry (dict, "AtspiVersion", "2.1");
            }

            dbus.message_iter_close_container (&it, &dict);
        }
        else if (equals (iface, ifIntrospectable) && equals (member, "Introspect"))
        {
            begin();
            // Minimal valid introspection. AT clients use the typed AT-SPI protocol
            // rather than introspection, so a node that just advertises the
            // standard interfaces is sufficient and keeps tools like busctl happy.
            const char* xml =
                "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
                "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
                "<node>\n"
                "  <interface name=\"org.freedesktop.DBus.Introspectable\"/>\n"
                "  <interface name=\"org.freedesktop.DBus.Properties\"/>\n"
                "  <interface name=\"org.a11y.atspi.Accessible\"/>\n"
                "  <interface name=\"org.a11y.atspi.Component\"/>\n"
                "</node>\n";
            dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &xml);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetRole"))
        {
            begin();
            uint32_t role = isRoot ? 75u : atspiRole (handler->getRole()); // 75 = APPLICATION
            dbus.message_iter_append_basic (&it, DBUS_TYPE_UINT32, &role);
        }
        else if (equals (iface, ifAccessible) && (equals (member, "GetRoleName") || equals (member, "GetLocalizedRoleName")))
        {
            begin();
            const char* roleName = isRoot ? "application" : atspiRoleName (handler->getRole());
            dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &roleName);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetState"))
        {
            begin();
            DBusMessageIter arr;
            dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "u", &arr);
            const uint64_t bits = isRoot ? ((1u << 8) | (1u << 25) | (1u << 30))    // ENABLED|SHOWING|VISIBLE
                                         : atspiStateBits (*handler);
            uint32_t lo = (uint32_t) bits, hi = (uint32_t) (bits >> 32);
            dbus.message_iter_append_basic (&arr, DBUS_TYPE_UINT32, &lo);
            dbus.message_iter_append_basic (&arr, DBUS_TYPE_UINT32, &hi);
            dbus.message_iter_close_container (&it, &arr);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetInterfaces"))
        {
            begin();
            DBusMessageIter arr;
            dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "s", &arr);
            appendInterfaceNames (arr, handler);
            dbus.message_iter_close_container (&it, &arr);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetApplication"))
        {
            begin();
            appendObjectRef (it, myName, rootPath);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetChildren"))
        {
            begin();
            DBusMessageIter arr;
            dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "(so)", &arr);
            forEachChild (handler, [&] (AccessibilityHandler* child)
            {
                appendObjectRef (arr, myName, pathForHandler (child));
            });
            dbus.message_iter_close_container (&it, &arr);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetChildAtIndex"))
        {
            const int index = readFirstInt32 (msg);
            begin();
            appendObjectRef (it, myName, pathForHandler (childAtIndex (handler, index)));
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetIndexInParent"))
        {
            begin();
            int32_t index = indexInParent (handler);
            dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &index);
        }
        else if (equals (iface, ifApplication) && equals (member, "GetLocale"))
        {
            begin();
            const char* loc = "C";
            dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &loc);
        }
        else if (equals (iface, ifComponent) && equals (member, "GetExtents"))
        {
            const auto b = isRoot ? Rectangle<int>() : componentBounds (handler, readCoordType (msg));
            begin();
            DBusMessageIter st;
            dbus.message_iter_open_container (&it, DBUS_TYPE_STRUCT, nullptr, &st);
            int32_t v[] = { b.getX(), b.getY(), b.getWidth(), b.getHeight() };
            for (auto& comp : v)
                dbus.message_iter_append_basic (&st, DBUS_TYPE_INT32, &comp);
            dbus.message_iter_close_container (&it, &st);
        }
        else if (equals (iface, ifComponent) && equals (member, "GetPosition"))
        {
            const auto b = isRoot ? Rectangle<int>() : componentBounds (handler, readCoordType (msg));
            begin();
            int32_t x = b.getX(), y = b.getY();
            dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &x);
            dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &y);
        }
        else if (equals (iface, ifComponent) && equals (member, "GetSize"))
        {
            begin();
            const auto b = isRoot ? Rectangle<int>() : handler->getComponent().getScreenBounds();
            int32_t w = b.getWidth(), h = b.getHeight();
            dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &w);
            dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &h);
        }
        else if (equals (iface, ifComponent) && equals (member, "GetAccessibleAtPoint"))
        {
            int32_t x = 0, y = 0, coord = 0;
            readThreeInt32 (msg, x, y, coord);

            AccessibilityHandler* found = nullptr;
            if (! isRoot)
            {
                // Convert window/parent-relative coordinates to the screen space
                // getChildAt() expects (the inverse of componentBounds()).
                auto point = Point<int> (x, y);
                if (coord != 0) // not ATSPI_COORD_TYPE_SCREEN
                    if (auto* top = handler->getComponent().getTopLevelComponent())
                        point += top->getScreenPosition();

                found = handler->getChildAt (point);
            }

            begin();
            appendObjectRef (it, myName, pathForHandler (found));
        }
        else if (equals (iface, ifComponent) && equals (member, "GrabFocus"))
        {
            // Bring the containing window to the front as well: assistive
            // tech (and tests driving the app through it) use GrabFocus to
            // foreground the application, and JUCE dismisses popups when the
            // process is not in the foreground.
            dbus_bool_t ok = false;
            if (! isRoot)
            {
                if (auto* peer = handler->getComponent().getPeer())
                {
                    // grab at the peer rather than through the component:
                    // Component::grabKeyboardFocus() is a no-op when the
                    // component doesn't want keyboard focus
                    peer->toFront (true);
                    peer->grabFocus();
                    ok = true;
                }

                handler->grabFocus();
            }
            begin();
            dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &ok);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetAttributes"))
        {
            // No element attributes (yet); an empty dict keeps clients happy.
            begin();
            DBusMessageIter dict;
            dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "{ss}", &dict);
            dbus.message_iter_close_container (&it, &dict);
        }
        else if (equals (iface, ifAccessible) && equals (member, "GetRelationSet"))
        {
            // JUCE has no notion of accessible relations; an empty set is valid.
            begin();
            DBusMessageIter arr;
            dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "(ua(so))", &arr);
            dbus.message_iter_close_container (&it, &arr);
        }
        else if (equals (iface, ifSelection) && equals (member, "GetSelectedChild"))
        {
            auto* child = selectedChildAt (handler, readFirstInt32 (msg));
            begin();
            appendObjectRef (it, myName, pathForHandler (child));
        }
        else if (equals (iface, ifSelection) && equals (member, "IsChildSelected"))
        {
            auto* child = childAtIndex (handler, readFirstInt32 (msg));
            begin();
            uint32_t selected = (child != nullptr && child->getCurrentState().isSelected()) ? 1u : 0u;
            dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &selected);
        }
        else if (equals (iface, ifSelection) && equals (member, "SelectChild"))
        {
            // Selecting maps to pressing the child, which is how all JUCE list
            // rows and selectable cells take selection.
            auto* child = childAtIndex (handler, readFirstInt32 (msg));
            const bool ok = child != nullptr && child->getActions().invoke (AccessibilityActionType::press);
            begin();
            uint32_t result = ok ? 1u : 0u;
            dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &result);
        }
        else if (equals (iface, ifSelection)
                 && (equals (member, "DeselectSelectedChild") || equals (member, "DeselectChild")
                     || equals (member, "SelectAll") || equals (member, "ClearSelection")))
        {
            // JUCE's accessibility model has no deselect/select-all primitives.
            begin();
            uint32_t result = 0u;
            dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &result);
        }
        else if (equals (iface, ifAction) && equals (member, "DoAction"))
        {
            const bool ok = doAction (handler, readFirstInt32 (msg));
            begin();
            uint32_t result = ok ? 1u : 0u;
            dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &result);
        }
        else if (equals (iface, ifAction)
                 && (equals (member, "GetName") || equals (member, "GetLocalizedName")))
        {
            const auto name = actionName (handler, readFirstInt32 (msg));
            begin();
            const char* raw = name.toRawUTF8();
            dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &raw);
        }
        else if (equals (iface, ifAction)
                 && (equals (member, "GetDescription") || equals (member, "GetKeyBinding")))
        {
            begin();
            const char* empty = "";
            dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &empty);
        }
        else if (equals (iface, ifAction) && equals (member, "GetActions"))
        {
            begin();
            DBusMessageIter arr;
            dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "(sss)", &arr);
            forEachAction (handler, [&] (int, const char* name, AccessibilityActionType)
            {
                DBusMessageIter st;
                dbus.message_iter_open_container (&arr, DBUS_TYPE_STRUCT, nullptr, &st);
                const char* empty = "";
                dbus.message_iter_append_basic (&st, DBUS_TYPE_STRING, &name);
                dbus.message_iter_append_basic (&st, DBUS_TYPE_STRING, &empty);  // description
                dbus.message_iter_append_basic (&st, DBUS_TYPE_STRING, &empty);  // key binding
                dbus.message_iter_close_container (&arr, &st);
            });
            dbus.message_iter_close_container (&it, &arr);
        }
        else if (equals (iface, ifTable) && handler != nullptr && handler->getTableInterface() != nullptr)
        {
            auto* t = handler->getTableInterface();
            const int cols = t->getNumColumns();

            if (equals (member, "GetAccessibleAt"))
            {
                int32_t row = 0, col = 0; readTwoInt32 (msg, row, col);
                begin();
                appendObjectRef (it, myName, pathForHandler (t->getCellHandler (row, col)));
            }
            else if (equals (member, "GetIndexAt"))
            {
                int32_t row = 0, col = 0; readTwoInt32 (msg, row, col);
                begin();
                int32_t idx = cols > 0 ? row * cols + col : -1;
                dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &idx);
            }
            else if (equals (member, "GetRowAtIndex"))
            {
                int32_t idx = readFirstInt32 (msg);
                begin();
                int32_t r = cols > 0 ? idx / cols : -1;
                dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &r);
            }
            else if (equals (member, "GetColumnAtIndex"))
            {
                int32_t idx = readFirstInt32 (msg);
                begin();
                int32_t c = cols > 0 ? idx % cols : -1;
                dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &c);
            }
            else if (equals (member, "GetColumnHeader") || equals (member, "GetRowHeader"))
            {
                begin();
                appendObjectRef (it, myName, pathForHandler (t->getHeaderHandler()));
            }
            else if (equals (member, "GetRowDescription") || equals (member, "GetColumnDescription"))
            {
                begin();
                const char* empty = "";
                dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &empty);
            }
            else if (equals (member, "IsRowSelected") || equals (member, "IsColumnSelected") || equals (member, "IsSelected"))
            {
                begin();
                uint32_t f = 0;
                dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &f);
            }
            else if (equals (member, "GetSelectedRows") || equals (member, "GetSelectedColumns"))
            {
                begin();
                DBusMessageIter arr;
                dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "i", &arr);
                dbus.message_iter_close_container (&it, &arr);
            }
        }
        else if (equals (iface, ifTableCell) && handler != nullptr && handler->getCellInterface() != nullptr)
        {
            if (equals (member, "GetRowColumnSpan"))
            {
                const auto p = cellPosition (handler);
                begin();
                uint32_t valid = p.valid ? 1u : 0u;
                int32_t row = p.row, col = p.col, re = p.rowSpan, ce = p.colSpan;
                dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &valid);
                dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &row);
                dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &col);
                dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &re);
                dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &ce);
            }
            else if (equals (member, "GetColumnHeaderCells") || equals (member, "GetRowHeaderCells"))
            {
                begin();
                DBusMessageIter arr;
                dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "(so)", &arr);
                dbus.message_iter_close_container (&it, &arr);
            }
        }
        else if (equals (iface, ifText) && handler != nullptr && handler->getTextInterface() != nullptr)
        {
            auto* tx = handler->getTextInterface();
            const int total = tx->getTotalNumCharacters();
            auto appendBool = [&] (bool b) { uint32_t v = b ? 1u : 0u; dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &v); };
            auto appendInt  = [&] (int32_t v) { dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &v); };
            auto appendStr  = [&] (const String& s) { const char* p = s.toRawUTF8(); dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &p); };

            if (equals (member, "GetText"))
            {
                int32_t s = 0, e = 0; readTwoInt32 (msg, s, e);
                begin();
                appendStr (tx->getText (Range<int> (s, e)));
            }
            else if (equals (member, "GetCharacterCount"))
            {
                begin();
                appendInt (total);
            }
            else if (equals (member, "GetTextAtOffset") || equals (member, "GetStringAtOffset"))
            {
                // arg0 = offset, arg1 = boundary/granularity type. We handle the
                // character boundary (0) precisely and otherwise return the whole
                // text, which is correct for single-line controls.
                int32_t offset = 0, type = 0; readTwoInt32 (msg, offset, type);
                int32_t s = 0, e = total;
                if (type == 0) { s = offset; e = jmin (offset + 1, total); }
                begin();
                appendStr (tx->getText (Range<int> (s, e)));
                appendInt (s);
                appendInt (e);
            }
            else if (equals (member, "GetCharacterAtOffset"))
            {
                int32_t offset = readFirstInt32 (msg);
                const auto s = tx->getText (Range<int> (offset, offset + 1));
                begin();
                appendInt (s.isNotEmpty() ? (int32_t) s[0] : 0);
            }
            else if (equals (member, "SetCaretOffset"))
            {
                int32_t offset = readFirstInt32 (msg);
                tx->setSelection (Range<int> (offset, offset));
                begin();
                appendBool (true);
            }
            else if (equals (member, "GetNSelections"))
            {
                begin();
                appendInt (tx->getSelection().isEmpty() ? 0 : 1);
            }
            else if (equals (member, "GetSelection"))
            {
                const auto sel = tx->getSelection();
                begin();
                appendInt (sel.getStart());
                appendInt (sel.getEnd());
            }
            else if (equals (member, "SetSelection") || equals (member, "AddSelection"))
            {
                int32_t a = 0, b = 0, c = 0;
                if (equals (member, "SetSelection")) { readThreeInt32 (msg, a, b, c); }    // (num, start, end)
                else                                 { readTwoInt32 (msg, b, c); }          // (start, end)
                tx->setSelection (Range<int> (b, c));
                begin();
                appendBool (true);
            }
            else if (equals (member, "RemoveSelection"))
            {
                const auto caret = tx->getTextInsertionOffset();
                tx->setSelection (Range<int> (caret, caret));
                begin();
                appendBool (true);
            }
            else if (equals (member, "GetCharacterExtents"))
            {
                int32_t offset = 0, coord = 0; readTwoInt32 (msg, offset, coord);
                const auto b = tx->getTextBounds (Range<int> (offset, offset + 1)).getBounds();
                begin();
                appendInt (b.getX()); appendInt (b.getY()); appendInt (b.getWidth()); appendInt (b.getHeight());
            }
            else if (equals (member, "GetRangeExtents"))
            {
                int32_t s = 0, e = 0, coord = 0; readThreeInt32 (msg, s, e, coord);
                const auto b = tx->getTextBounds (Range<int> (s, e)).getBounds();
                begin();
                appendInt (b.getX()); appendInt (b.getY()); appendInt (b.getWidth()); appendInt (b.getHeight());
            }
            else if (equals (member, "GetOffsetAtPoint"))
            {
                int32_t x = 0, y = 0, coord = 0; readThreeInt32 (msg, x, y, coord);
                begin();
                appendInt (tx->getOffsetAtPoint (Point<int> (x, y)));
            }
        }
        else if (equals (iface, ifEditableText) && handler != nullptr && handler->getTextInterface() != nullptr)
        {
            auto* tx = handler->getTextInterface();
            const auto whole = tx->getText (Range<int> (0, tx->getTotalNumCharacters()));
            bool ok = false;

            if (equals (member, "SetTextContents"))
            {
                DBusMessageIter in; const char* s = "";
                if (dbus.message_iter_init (msg, &in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_STRING)
                    dbus.message_iter_get_basic (&in, &s);
                if (! tx->isReadOnly()) { tx->setText (String::fromUTF8 (s)); ok = true; }
            }
            else if (equals (member, "InsertText"))
            {
                // (position i, text s, length i)
                DBusMessageIter in; int32_t pos = 0; const char* s = "";
                if (dbus.message_iter_init (msg, &in) && readInt32Arg (in, pos))
                {
                    if (dbus.message_iter_next (&in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_STRING)
                        dbus.message_iter_get_basic (&in, &s);
                }
                if (! tx->isReadOnly())
                {
                    const int p = jlimit (0, whole.length(), pos);
                    tx->setText (whole.substring (0, p) + String::fromUTF8 (s) + whole.substring (p));
                    ok = true;
                }
            }
            else if (equals (member, "DeleteText") || equals (member, "CutText"))
            {
                int32_t s = 0, e = 0; readTwoInt32 (msg, s, e);
                if (! tx->isReadOnly())
                {
                    const int a = jlimit (0, whole.length(), s), b = jlimit (0, whole.length(), e);
                    tx->setText (whole.substring (0, jmin (a, b)) + whole.substring (jmax (a, b)));
                    ok = true;
                }
            }
            // PasteText/CopyText: no clipboard access through this interface.

            begin();
            if (! equals (member, "CopyText"))
            {
                uint32_t v = ok ? 1u : 0u;
                dbus.message_iter_append_basic (&it, DBUS_TYPE_BOOLEAN, &v);
            }
        }

        if (outOfMemory)
            return DBUS_HANDLER_RESULT_NEED_MEMORY; // libdbus redelivers the message

        if (reply != nullptr)
        {
            dbus.connection_send (conn, reply, nullptr);
            dbus.message_unref (reply);
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    //==============================================================================
    // org.a11y.atspi.Cache @ /org/a11y/atspi/cache. GetItems returns a snapshot of
    // the currently-known accessible objects so a client (e.g. a screen reader)
    // can populate its cache in one round-trip.
    int handleCache (DBusConnection* conn, DBusMessage* msg)
    {
        if (! equals (dbus.message_get_interface (msg), ifCache) || ! equals (dbus.message_get_member (msg), "GetItems"))
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

        auto* reply = dbus.message_new_method_return (msg);

        if (reply == nullptr)
            return DBUS_HANDLER_RESULT_NEED_MEMORY; // libdbus redelivers the message

        DBusMessageIter it, arr;
        dbus.message_iter_init_append (reply, &it);
        dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "((so)(so)(so)iiassusau)", &arr);

        appendCacheItem (arr, nullptr); // the application node

        // Snapshot the handler ids first: appending an item enumerates
        // children, which can create (and register) new handlers, mutating
        // the map - and can also destroy handlers (getAccessibilityHandler()
        // recreates a handler whose type no longer matches), so re-find each
        // id at use rather than holding pointers across iterations.
        std::vector<int> snapshot;
        snapshot.reserve (handlers.size());
        for (auto& entry : handlers)
            snapshot.push_back (entry.first);

        for (auto id : snapshot)
            if (const auto it = handlers.find (id); it != handlers.end())
                appendCacheItem (arr, it->second);

        dbus.message_iter_close_container (&it, &arr);
        dbus.connection_send (conn, reply, nullptr);
        dbus.message_unref (reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    // One cache tuple: ((so)obj (so)app (so)parent i:index i:childCount as:ifaces
    //                   s:name u:role s:description au:state)
    void appendCacheItem (DBusMessageIter& arr, AccessibilityHandler* handler)
    {
        const bool isRoot = (handler == nullptr);
        DBusMessageIter item;
        dbus.message_iter_open_container (&arr, DBUS_TYPE_STRUCT, nullptr, &item);

        appendObjectRef (item, myName, isRoot ? String (rootPath) : pathForHandler (handler));     // object
        appendObjectRef (item, myName, rootPath);                                                   // application
        appendObjectRef (item, myName, isRoot ? String (registryPath.isNotEmpty() ? registryPath : String (rootPath))
                                              : (handler->getParent() != nullptr ? pathForHandler (handler->getParent())
                                                                                 : String (rootPath))); // parent

        int32_t index = isRoot ? 0 : indexInParent (handler);
        int32_t nChildren = childCount (handler);
        dbus.message_iter_append_basic (&item, DBUS_TYPE_INT32, &index);
        dbus.message_iter_append_basic (&item, DBUS_TYPE_INT32, &nChildren);

        // interfaces (as)
        DBusMessageIter ifaces;
        dbus.message_iter_open_container (&item, DBUS_TYPE_ARRAY, "s", &ifaces);
        appendInterfaceNames (ifaces, handler);
        dbus.message_iter_close_container (&item, &ifaces);

        // name (s)
        const auto nameStr = isRoot ? getAppName() : handler->getTitle();
        const char* name = nameStr.toRawUTF8();
        dbus.message_iter_append_basic (&item, DBUS_TYPE_STRING, &name);

        // role (u)
        uint32_t role = isRoot ? 75u : atspiRole (handler->getRole());
        dbus.message_iter_append_basic (&item, DBUS_TYPE_UINT32, &role);

        // description (s)
        const auto descStr = isRoot ? String() : handler->getDescription();
        const char* desc = descStr.toRawUTF8();
        dbus.message_iter_append_basic (&item, DBUS_TYPE_STRING, &desc);

        // state (au)
        DBusMessageIter st;
        dbus.message_iter_open_container (&item, DBUS_TYPE_ARRAY, "u", &st);
        const uint64_t bits = isRoot ? ((1u << 8) | (1u << 25) | (1u << 30)) : atspiStateBits (*handler);
        uint32_t lo = (uint32_t) bits, hi = (uint32_t) (bits >> 32);
        dbus.message_iter_append_basic (&st, DBUS_TYPE_UINT32, &lo);
        dbus.message_iter_append_basic (&st, DBUS_TYPE_UINT32, &hi);
        dbus.message_iter_close_container (&item, &st);

        dbus.message_iter_close_container (&arr, &item);
    }

    // Append the interface-name strings a handler supports into an already-open
    // ARRAY-of-string container (shared by GetInterfaces and the cache).
    void appendInterfaceNames (DBusMessageIter& arr, AccessibilityHandler* handler)
    {
        const char* base[] = { ifAccessible, ifComponent };
        for (auto* supported : base)
            dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &supported);

        if (handler == nullptr)
        {
            dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifApplication);
            return;
        }

        if (actionCount (handler) > 0)               dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifAction);
        if (handler->getValueInterface() != nullptr) dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifValue);
        if (handler->getTableInterface() != nullptr) dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifTable);
        if (handler->getCellInterface() != nullptr)  dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifTableCell);
        if (auto* tx = handler->getTextInterface())
        {
            dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifText);
            if (! tx->isReadOnly())
                dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifEditableText);
        }
        if (hasSelectableChild (handler))                dbus.message_iter_append_basic (&arr, DBUS_TYPE_STRING, &ifSelection);
    }

    //==============================================================================
    // Tree navigation. The application node's children are the top-level windows.
    template <typename Fn>
    void forEachChild (AccessibilityHandler* handler, Fn&& fn)
    {
        if (handler == nullptr)
        {
            auto& desktop = Desktop::getInstance();
            for (int i = 0; i < desktop.getNumComponents(); ++i)
                if (auto* comp = desktop.getComponent (i))
                    if (auto* h = comp->getAccessibilityHandler())
                        fn (h);
        }
        else
        {
            for (auto* child : handler->getChildren())
                fn (child);
        }
    }

    int childCount (AccessibilityHandler* handler)
    {
        int n = 0;
        forEachChild (handler, [&n] (AccessibilityHandler*) { ++n; });
        return n;
    }

    //==============================================================================
    // org.a11y.atspi.Selection, derived from the children's selected/selectable
    // states (the same model the UIA SelectionProvider uses on Windows).
    template <typename Fn>
    void forEachSelectedChild (AccessibilityHandler* handler, Fn&& fn)
    {
        forEachChild (handler, [&fn] (AccessibilityHandler* child)
        {
            if (child->getCurrentState().isSelected())
                fn (child);
        });
    }

    int selectedChildCount (AccessibilityHandler* handler)
    {
        int n = 0;
        forEachSelectedChild (handler, [&n] (AccessibilityHandler*) { ++n; });
        return n;
    }

    AccessibilityHandler* selectedChildAt (AccessibilityHandler* handler, int index)
    {
        AccessibilityHandler* found = nullptr;
        int i = 0;
        forEachSelectedChild (handler, [&found, &i, index] (AccessibilityHandler* child) { if (i++ == index) found = child; });
        return found;
    }

    bool hasSelectableChild (AccessibilityHandler* handler)
    {
        bool any = false;
        forEachChild (handler, [&any] (AccessibilityHandler* child)
        {
            if (child->getCurrentState().isSelectable())
                any = true;
        });
        return any;
    }

    AccessibilityHandler* childAtIndex (AccessibilityHandler* handler, int index)
    {
        AccessibilityHandler* found = nullptr;
        int i = 0;
        forEachChild (handler, [&] (AccessibilityHandler* child) { if (i++ == index) found = child; });
        return found;
    }

    int indexInParent (AccessibilityHandler* handler)
    {
        if (handler == nullptr)
            return 0;

        auto* parent = handler->getParent();
        int i = 0, result = -1;
        forEachChild (parent, [&] (AccessibilityHandler* child) { if (child == handler) result = i; ++i; });
        return result;
    }

    //==============================================================================
    // org.a11y.atspi.Action, backed by JUCE's AccessibilityActions. The order
    // here defines the action index seen over the bus.
    template <typename Fn>
    void forEachFixedAction (AccessibilityHandler* handler, Fn&& fn) // fn (int index, const char* name, AccessibilityActionType)
    {
        if (handler == nullptr)
            return;

        static constexpr struct { AccessibilityActionType type; const char* name; } table[] =
        {
            { AccessibilityActionType::press,    "click" },
            { AccessibilityActionType::toggle,   "toggle" },
            { AccessibilityActionType::showMenu, "menu" },
        };

        const auto& actions = handler->getActions();
        int index = 0;

        for (const auto& a : table)
            if (actions.contains (a.type))
                fn (index++, a.name, a.type);
    }

    // Top-level windows expose a "close" action: windows with a native title
    // bar have no JUCE close button in the tree, and AT-SPI has no standard
    // window-close call - without this they cannot be closed at all.
    static ComponentPeer* closeablePeerOf (AccessibilityHandler* handler)
    {
        if (handler == nullptr)
            return nullptr;

        auto& component = handler->getComponent();
        if (component.getParentComponent() != nullptr)
            return nullptr;

        return component.getPeer();
    }

    template <typename Fn>
    void forEachAction (AccessibilityHandler* handler, Fn&& fn) // fn (int index, const char* name, AccessibilityActionType)
    {
        int index = 0;
        forEachFixedAction (handler, [&] (int, const char* name, AccessibilityActionType type) { fn (index++, name, type); });

        if (closeablePeerOf (handler) != nullptr)
            fn (index++, "close", AccessibilityActionType::press);
    }

    int actionCount (AccessibilityHandler* handler)
    {
        int n = 0;
        forEachAction (handler, [&n] (int, const char*, AccessibilityActionType) { ++n; });
        return n;
    }

    String actionName (AccessibilityHandler* handler, int index)
    {
        String result;
        forEachAction (handler, [&] (int i, const char* name, AccessibilityActionType) { if (i == index) result = name; });
        return result;
    }

    bool doAction (AccessibilityHandler* handler, int index)
    {
        // Find the action first and invoke only after the iteration: the
        // invoked user code may destroy the component (and with it the
        // handler and its actions map the iteration is reading).
        std::optional<AccessibilityActionType> foundType;
        int fixedCount = 0;
        forEachFixedAction (handler, [&] (int i, const char*, AccessibilityActionType type)
        {
            ++fixedCount;
            if (i == index)
                foundType = type;
        });

        if (foundType.has_value())
            return handler->getActions().invoke (*foundType);

        if (auto* peer = closeablePeerOf (handler))
            if (index == fixedCount)
            {
                peer->handleUserClosingWindow();
                return true;
            }

        return false;
    }

    //==============================================================================
    // Marshalling helpers.
    void appendObjectRef (DBusMessageIter& it, const String& busName, const String& path)
    {
        DBusMessageIter st;
        dbus.message_iter_open_container (&it, DBUS_TYPE_STRUCT, nullptr, &st);
        const char* n = busName.toRawUTF8();
        const char* p = path.toRawUTF8();
        dbus.message_iter_append_basic (&st, DBUS_TYPE_STRING, &n);
        dbus.message_iter_append_basic (&st, DBUS_TYPE_OBJECT_PATH, &p);
        dbus.message_iter_close_container (&it, &st);
    }

    // Emit an AT-SPI event signal: body is the standard (s detail, i, i, v any_data, a{sv}).
    void emitEvent (const String& path, const char* iface, const char* signal,
                    const char* detail, int32_t detail1, int32_t detail2, AccessibilityHandler* anyDataRef)
    {
        auto* sig = dbus.message_new_signal (path.toRawUTF8(), iface, signal);
        if (sig == nullptr)
            return;

        DBusMessageIter it;
        dbus.message_iter_init_append (sig, &it);
        dbus.message_iter_append_basic (&it, DBUS_TYPE_STRING, &detail);
        dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &detail1);
        dbus.message_iter_append_basic (&it, DBUS_TYPE_INT32, &detail2);

        DBusMessageIter var;
        if (anyDataRef != nullptr)
        {
            dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "(so)", &var);
            appendObjectRef (var, myName, pathForHandler (anyDataRef));
            dbus.message_iter_close_container (&it, &var);
        }
        else
        {
            dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "i", &var);
            int32_t zero = 0;
            dbus.message_iter_append_basic (&var, DBUS_TYPE_INT32, &zero);
            dbus.message_iter_close_container (&it, &var);
        }

        DBusMessageIter props;
        dbus.message_iter_open_container (&it, DBUS_TYPE_ARRAY, "{sv}", &props);
        dbus.message_iter_close_container (&it, &props);

        dbus.connection_send (a11yConnection, sig, nullptr);
        dbus.message_unref (sig);

        // flush() can block if the socket buffer to the registry is full
        // (i.e. the peer has been wedged for a long while). Accepted: the
        // alternative is write-watch plumbing, and ATs depend on prompt
        // event delivery.
        dbus.connection_flush (a11yConnection);
    }

    void appendVariantParent (DBusMessageIter& it, AccessibilityHandler* handler)
    {
        // Parent of the root is the registry; parent of a top-level handler is
        // the root; otherwise the handler's parent handler.
        String busName = myName, path;

        if (handler == nullptr)        { busName = registryName.isNotEmpty() ? registryName : myName;
                                         path = registryPath.isNotEmpty() ? registryPath : String (rootPath); }
        else if (auto* p = handler->getParent()) path = pathForHandler (p);
        else                                     path = rootPath;

        DBusMessageIter v;
        dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "(so)", &v);
        appendObjectRef (v, busName, path);
        dbus.message_iter_close_container (&it, &v);
    }

    void appendVariantString (DBusMessageIter& it, const String& value)
    {
        DBusMessageIter v;
        dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "s", &v);
        const char* s = value.toRawUTF8();
        dbus.message_iter_append_basic (&v, DBUS_TYPE_STRING, &s);
        dbus.message_iter_close_container (&it, &v);
    }

    void appendVariantInt32 (DBusMessageIter& it, int32_t value)
    {
        DBusMessageIter v;
        dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "i", &v);
        dbus.message_iter_append_basic (&v, DBUS_TYPE_INT32, &value);
        dbus.message_iter_close_container (&it, &v);
    }

    void appendVariantDouble (DBusMessageIter& it, double value)
    {
        DBusMessageIter v;
        dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "d", &v);
        dbus.message_iter_append_basic (&v, DBUS_TYPE_DOUBLE, &value);
        dbus.message_iter_close_container (&it, &v);
    }

    // a{sv} dict-entry helpers for Properties.GetAll.
    void appendStringEntry (DBusMessageIter& dict, const char* key, const String& value)
    {
        DBusMessageIter e;
        dbus.message_iter_open_container (&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &e);
        dbus.message_iter_append_basic (&e, DBUS_TYPE_STRING, &key);
        appendVariantString (e, value);
        dbus.message_iter_close_container (&dict, &e);
    }

    void appendInt32Entry (DBusMessageIter& dict, const char* key, int32_t value)
    {
        DBusMessageIter e;
        dbus.message_iter_open_container (&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &e);
        dbus.message_iter_append_basic (&e, DBUS_TYPE_STRING, &key);
        appendVariantInt32 (e, value);
        dbus.message_iter_close_container (&dict, &e);
    }

    void appendParentEntry (DBusMessageIter& dict, AccessibilityHandler* handler)
    {
        DBusMessageIter e;
        dbus.message_iter_open_container (&dict, DBUS_TYPE_DICT_ENTRY, nullptr, &e);
        const char* key = "Parent";
        dbus.message_iter_append_basic (&e, DBUS_TYPE_STRING, &key);
        appendVariantParent (e, handler);
        dbus.message_iter_close_container (&dict, &e);
    }

    void appendVariantObjectRef (DBusMessageIter& it, const String& path)
    {
        DBusMessageIter v;
        dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "(so)", &v);
        appendObjectRef (v, myName, path);
        dbus.message_iter_close_container (&it, &v);
    }

    void appendVariantPosition (DBusMessageIter& it, int32_t row, int32_t col)
    {
        DBusMessageIter v, st;
        dbus.message_iter_open_container (&it, DBUS_TYPE_VARIANT, "(ii)", &v);
        dbus.message_iter_open_container (&v, DBUS_TYPE_STRUCT, nullptr, &st);
        dbus.message_iter_append_basic (&st, DBUS_TYPE_INT32, &row);
        dbus.message_iter_append_basic (&st, DBUS_TYPE_INT32, &col);
        dbus.message_iter_close_container (&v, &st);
        dbus.message_iter_close_container (&it, &v);
    }

    // AT-SPI signatures mix i and u args (e.g. (i offset, u granularity),
    // (i x, i y, u coord_type)); both are 32-bit on the wire, so the readers
    // accept either type per argument.
    bool readInt32Arg (DBusMessageIter& in, int32_t& out)
    {
        const auto type = dbus.message_iter_get_arg_type (&in);

        if (type != DBUS_TYPE_INT32 && type != DBUS_TYPE_UINT32)
            return false;

        dbus.message_iter_get_basic (&in, &out);
        return true;
    }

    void readTwoInt32 (DBusMessage* msg, int32_t& a, int32_t& b)
    {
        a = 0; b = 0;
        DBusMessageIter in;
        if (dbus.message_iter_init (msg, &in) && readInt32Arg (in, a) && dbus.message_iter_next (&in))
            readInt32Arg (in, b);
    }

    uint32_t readCoordType (DBusMessage* msg)
    {
        DBusMessageIter in; uint32_t v = 0;
        if (dbus.message_iter_init (msg, &in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_UINT32)
            dbus.message_iter_get_basic (&in, &v);
        return v;
    }

    // Component bounds in the requested AT-SPI coord space: 0 = screen, 1 = window,
    // 2 = parent. We map window/parent to top-level-relative (a close approximation
    // for parent), which is what hit-testing clients need.
    Rectangle<int> componentBounds (AccessibilityHandler* h, uint32_t coordType)
    {
        auto& comp = h->getComponent();
        const auto screen = comp.getScreenBounds();

        if (coordType == 0) // ATSPI_COORD_TYPE_SCREEN
            return screen;

        if (auto* top = comp.getTopLevelComponent())
        {
            const auto origin = top->getScreenPosition();
            return screen.translated (-origin.getX(), -origin.getY());
        }

        return screen;
    }

    // Reads up to three leading 32-bit args (e.g. (num, start, end) or (x, y, coordType)).
    void readThreeInt32 (DBusMessage* msg, int32_t& a, int32_t& b, int32_t& c)
    {
        a = 0; b = 0; c = 0;
        DBusMessageIter in;
        if (! dbus.message_iter_init (msg, &in) || ! readInt32Arg (in, a)) return;
        if (! dbus.message_iter_next (&in)      || ! readInt32Arg (in, b)) return;
        if (dbus.message_iter_next (&in))
            readInt32Arg (in, c);
    }

    // Resolve a cell's tabular position from its containing table (JUCE keeps cell
    // location on the table, not the cell).
    struct CellPos { bool valid = false; int row = 0, col = 0, rowSpan = 1, colSpan = 1; const AccessibilityHandler* table = nullptr; };

    CellPos cellPosition (AccessibilityHandler* cell)
    {
        CellPos p;

        if (cell == nullptr)
            return p;

        auto* ci = cell->getCellInterface();
        if (ci == nullptr)
            return p;

        p.table = ci->getTableHandler();
        if (p.table == nullptr)
            return p;

        if (auto* ti = const_cast<AccessibilityHandler*> (p.table)->getTableInterface())
        {
            const auto rs = ti->getRowSpan (*cell);
            const auto cs = ti->getColumnSpan (*cell);

            if (rs.hasValue()) { p.row = rs->begin; p.rowSpan = rs->num; }
            if (cs.hasValue()) { p.col = cs->begin; p.colSpan = cs->num; }
            p.valid = rs.hasValue() && cs.hasValue();
        }

        return p;
    }

    const char* readSecondString (DBusMessage* msg)
    {
        // type-check each arg: get_basic trusts the message's own signature,
        // so reading a non-string arg as char* yields a garbage pointer
        DBusMessageIter in;
        const char* second = "";
        if (dbus.message_iter_init (msg, &in)
            && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_STRING
            && dbus.message_iter_next (&in)
            && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_STRING)
            dbus.message_iter_get_basic (&in, &second);
        return second;
    }

    int readFirstInt32 (DBusMessage* msg)
    {
        DBusMessageIter in;
        int32_t value = 0;
        if (dbus.message_iter_init (msg, &in) && dbus.message_iter_get_arg_type (&in) == DBUS_TYPE_INT32)
            dbus.message_iter_get_basic (&in, &value);
        return value;
    }

    static bool equals (const char* a, const char* b)
    {
        return a != nullptr && b != nullptr && std::strcmp (a, b) == 0;
    }

    static String getAppName()
    {
        if (auto* app = JUCEApplicationBase::getInstance())
            return app->getApplicationName();
        return "JUCE Application";
    }

    //==============================================================================
    static constexpr const char* atspiPrefix      = "/org/a11y/atspi";
    static constexpr const char* accessiblePrefix = "/org/a11y/atspi/accessible";
    static constexpr const char* rootPath         = "/org/a11y/atspi/accessible/root";
    static constexpr const char* cachePath        = "/org/a11y/atspi/cache";
    static constexpr const char* ifCache          = "org.a11y.atspi.Cache";
    static constexpr const char* ifAccessible     = "org.a11y.atspi.Accessible";
    static constexpr const char* ifComponent      = "org.a11y.atspi.Component";
    static constexpr const char* ifApplication    = "org.a11y.atspi.Application";
    static constexpr const char* ifAction         = "org.a11y.atspi.Action";
    static constexpr const char* ifValue          = "org.a11y.atspi.Value";
    static constexpr const char* ifTable          = "org.a11y.atspi.Table";
    static constexpr const char* ifTableCell      = "org.a11y.atspi.TableCell";
    static constexpr const char* ifText           = "org.a11y.atspi.Text";
    static constexpr const char* ifEditableText   = "org.a11y.atspi.EditableText";
    static constexpr const char* ifSelection      = "org.a11y.atspi.Selection";
    static constexpr const char* ifProperties     = "org.freedesktop.DBus.Properties";
    static constexpr const char* ifIntrospectable = "org.freedesktop.DBus.Introspectable";
    static constexpr const char* ifEventObject    = "org.a11y.atspi.Event.Object";
    static constexpr const char* ifEventWindow    = "org.a11y.atspi.Event.Window";

    DBusLibrary dbus;
    DBusConnection* a11yConnection = nullptr;
    bool started = false;
    bool draining = false;
    int nextId = 1;
    std::map<int, AccessibilityHandler*> handlers;
    std::map<int, int> watchedFds;
    String myName, registryName, registryPath;

    JUCE_DECLARE_NON_COPYABLE (AtSpiBridge)
};

} // namespace

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& owner)
        : nativeHandle (owner)
    {
        auto& bridge = AtSpiBridge::getInstance();
        bridge.ensureStarted();
        nativeHandle.setId (bridge.registerHandler (owner));
    }

    ~AccessibilityNativeImpl()
    {
        AtSpiBridge::getInstance().unregisterHandler (nativeHandle.getId());
    }

    AccessibilityNativeHandle* getNativeHandle() noexcept  { return &nativeHandle; }

private:
    AccessibilityNativeHandle nativeHandle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeImpl)
};

//==============================================================================
AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const
{
    return nativeImpl != nullptr ? nativeImpl->getNativeHandle() : nullptr;
}

void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    AtSpiBridge::getInstance().onAccessibilityEvent (*this, eventType);
}

void AccessibilityHandler::postAnnouncement (const String& announcementString, AnnouncementPriority priority)
{
    // AT-SPI live-region politeness: 1 = polite, 2 = assertive.
    const int32_t politeness = priority == AnnouncementPriority::high ? 2 : 1;
    AtSpiBridge::getInstance().announce (announcementString, politeness);
}

} // namespace juce

namespace juce::detail
{

void AccessibilityHelpers::notifyAccessibilityEvent (const AccessibilityHandler& handler, Event eventType)
{
    AtSpiBridge::getInstance().onAccessibilityEvent (handler, eventType);
}

} // namespace juce::detail

#undef JUCE_ATSPI_LOG
