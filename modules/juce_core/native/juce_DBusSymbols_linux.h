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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

#pragma once

// Opaque libdbus handle types
struct DBusConnection;
struct DBusMessage;
struct DBusPendingCall;
struct DBusTimeout;
struct DBusWatch;

// Hand-rolled mirrors of the fixed public libdbus ABI (dbus-errors.h, dbus-message.h)
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
    unsigned int dummy3;
    int dummy4, dummy5, dummy6, dummy7, dummy8, dummy9, dummy10, dummy11;
    int pad1;
    void* pad2;
    void* pad3;
};

namespace juce
{

// Values mirror dbus-shared.h / dbus-connection.h
namespace DBusConstants
{
    enum : int { busSession = 0 };

    enum : int
    {
        typeInvalid = 0,
        typeByte = 'y',
        typeBoolean = 'b',
        typeUInt32 = 'u',
        typeString = 's',
        typeObjectPath = 'o',
        typeUnixFd = 'h',
        typeArray = 'a',
        typeVariant = 'v',
        typeDictEntry = 'e',
        typeStruct = 'r'
    };

    enum : int { messageTypeMethodReturn = 2, messageTypeError = 3, messageTypeSignal = 4 };
    enum : unsigned int { watchReadable = 1, watchWritable = 2 };
    enum : int { dispatchDataRemains = 0, dispatchComplete = 1, dispatchNeedMemory = 2 };
    enum : int { handlerResultHandled = 0, handlerResultNotYetHandled = 1 };
    enum : int { timeoutUseDefault = -1 };
}

using DBusAddWatchFunction = unsigned int (*) (DBusWatch*, void*);
using DBusRemoveWatchFunction = void (*) (DBusWatch*, void*);
using DBusWatchToggledFunction = void (*) (DBusWatch*, void*);
using DBusFreeFunction = void (*) (void*);
using DBusHandleMessageFunction = int (*) (DBusConnection*, DBusMessage*, void*);
using DBusPendingCallNotifyFunction = void (*) (DBusPendingCall*, void*);
using DBusAddTimeoutFunction = unsigned int (*) (DBusTimeout*, void*);
using DBusRemoveTimeoutFunction = void (*) (DBusTimeout*, void*);
using DBusTimeoutToggledFunction = void (*) (DBusTimeout*, void*);

//==============================================================================
#define JUCE_GENERATE_DBUS_FUNCTION(functionName, objectName, args, returnType) \
    using functionName      = returnType (*) args; \
    functionName objectName = nullptr;

class JUCE_API  DBusSymbols
{
public:
    static DBusSymbols* getInstance();

    bool loadAllSymbols();

    JUCE_GENERATE_DBUS_FUNCTION (DbusThreadsInitDefault, dbusThreadsInitDefault,
                                 (),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusBusGetPrivate, dbusBusGetPrivate,
                                 (int, DBusError*),
                                 DBusConnection*)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionSetExitOnDisconnect, dbusConnectionSetExitOnDisconnect,
                                 (DBusConnection*, unsigned int),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionClose, dbusConnectionClose,
                                 (DBusConnection*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionUnref, dbusConnectionUnref,
                                 (DBusConnection*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionFlush, dbusConnectionFlush,
                                 (DBusConnection*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionSend, dbusConnectionSend,
                                 (DBusConnection*, DBusMessage*, unsigned int*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionSendWithReply, dbusConnectionSendWithReply,
                                 (DBusConnection*, DBusMessage*, DBusPendingCall**, int),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionSendWithReplyAndBlock, dbusConnectionSendWithReplyAndBlock,
                                 (DBusConnection*, DBusMessage*, int, DBusError*),
                                 DBusMessage*)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionDispatch, dbusConnectionDispatch,
                                 (DBusConnection*),
                                 int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionGetDispatchStatus, dbusConnectionGetDispatchStatus,
                                 (DBusConnection*),
                                 int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionSetWatchFunctions, dbusConnectionSetWatchFunctions,
                                 (DBusConnection*, DBusAddWatchFunction, DBusRemoveWatchFunction, DBusWatchToggledFunction, void*, DBusFreeFunction),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionSetTimeoutFunctions, dbusConnectionSetTimeoutFunctions,
                                 (DBusConnection*, DBusAddTimeoutFunction, DBusRemoveTimeoutFunction, DBusTimeoutToggledFunction, void*, DBusFreeFunction),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionAddFilter, dbusConnectionAddFilter,
                                 (DBusConnection*, DBusHandleMessageFunction, void*, DBusFreeFunction),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusConnectionRemoveFilter, dbusConnectionRemoveFilter,
                                 (DBusConnection*, DBusHandleMessageFunction, void*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusBusGetUniqueName, dbusBusGetUniqueName,
                                 (DBusConnection*),
                                 const char*)

    JUCE_GENERATE_DBUS_FUNCTION (DbusBusAddMatch, dbusBusAddMatch,
                                 (DBusConnection*, const char*, DBusError*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusBusRemoveMatch, dbusBusRemoveMatch,
                                 (DBusConnection*, const char*, DBusError*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusWatchGetUnixFd, dbusWatchGetUnixFd,
                                 (DBusWatch*),
                                 int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusWatchGetFlags, dbusWatchGetFlags,
                                 (DBusWatch*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusWatchGetEnabled, dbusWatchGetEnabled,
                                 (DBusWatch*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusWatchHandle, dbusWatchHandle,
                                 (DBusWatch*, unsigned int),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageNewMethodCall, dbusMessageNewMethodCall,
                                 (const char*, const char*, const char*, const char*),
                                 DBusMessage*)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageUnref, dbusMessageUnref,
                                 (DBusMessage*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageGetType, dbusMessageGetType,
                                 (DBusMessage*),
                                 int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIsSignal, dbusMessageIsSignal,
                                 (DBusMessage*, const char*, const char*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageGetPath, dbusMessageGetPath,
                                 (DBusMessage*),
                                 const char*)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterInit, dbusMessageIterInit,
                                 (DBusMessage*, DBusMessageIter*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterInitAppend, dbusMessageIterInitAppend,
                                 (DBusMessage*, DBusMessageIter*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterAppendBasic, dbusMessageIterAppendBasic,
                                 (DBusMessageIter*, int, const void*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterOpenContainer, dbusMessageIterOpenContainer,
                                 (DBusMessageIter*, int, const char*, DBusMessageIter*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterCloseContainer, dbusMessageIterCloseContainer,
                                 (DBusMessageIter*, DBusMessageIter*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterGetArgType, dbusMessageIterGetArgType,
                                 (DBusMessageIter*),
                                 int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterGetBasic, dbusMessageIterGetBasic,
                                 (DBusMessageIter*, void*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterRecurse, dbusMessageIterRecurse,
                                 (DBusMessageIter*, DBusMessageIter*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusMessageIterNext, dbusMessageIterNext,
                                 (DBusMessageIter*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusPendingCallSetNotify, dbusPendingCallSetNotify,
                                 (DBusPendingCall*, DBusPendingCallNotifyFunction, void*, DBusFreeFunction),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusPendingCallStealReply, dbusPendingCallStealReply,
                                 (DBusPendingCall*),
                                 DBusMessage*)

    JUCE_GENERATE_DBUS_FUNCTION (DbusPendingCallCancel, dbusPendingCallCancel,
                                 (DBusPendingCall*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusPendingCallUnref, dbusPendingCallUnref,
                                 (DBusPendingCall*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusTimeoutGetInterval, dbusTimeoutGetInterval,
                                 (DBusTimeout*),
                                 int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusTimeoutGetEnabled, dbusTimeoutGetEnabled,
                                 (DBusTimeout*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusTimeoutHandle, dbusTimeoutHandle,
                                 (DBusTimeout*),
                                 unsigned int)

    JUCE_GENERATE_DBUS_FUNCTION (DbusErrorInit, dbusErrorInit,
                                 (DBusError*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusErrorFree, dbusErrorFree,
                                 (DBusError*),
                                 void)

    JUCE_GENERATE_DBUS_FUNCTION (DbusErrorIsSet, dbusErrorIsSet,
                                 (const DBusError*),
                                 unsigned int)

private:
    DBusSymbols() = default;
    ~DBusSymbols() = default;

    std::once_flag loadFlag;
    bool loaded = false;
    DynamicLibrary dbus { "libdbus-1.so.3" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DBusSymbols)
};

//==============================================================================
class ScopedDBusError
{
public:
    ScopedDBusError()  { DBusSymbols::getInstance()->dbusErrorInit (&error); }
    ~ScopedDBusError() { DBusSymbols::getInstance()->dbusErrorFree (&error); }

    bool isSet() const { return DBusSymbols::getInstance()->dbusErrorIsSet (&error) != 0; }
    DBusError* get()   { return &error; }

private:
    DBusError error;

    JUCE_DECLARE_NON_COPYABLE (ScopedDBusError)
};

struct DBusConnectionCloser
{
    void operator() (DBusConnection*) const;
};

using DBusConnectionHandle = std::unique_ptr<DBusConnection, DBusConnectionCloser>;

struct DBusMessageUnrefer
{
    void operator() (DBusMessage*) const;
};

using DBusMessageHandle = std::unique_ptr<DBusMessage, DBusMessageUnrefer>;

#undef JUCE_GENERATE_DBUS_FUNCTION

} // namespace juce
