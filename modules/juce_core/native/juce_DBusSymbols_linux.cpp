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

namespace juce
{

//==============================================================================
DBusSymbols* DBusSymbols::getInstance()
{
    static DBusSymbols instance;
    return &instance;
}

bool DBusSymbols::loadAllSymbols()
{
    std::call_once (loadFlag, [this]
    {
        loaded = dbus.loadInto (dbusThreadsInitDefault,              "dbus_threads_init_default")
              && dbus.loadInto (dbusBusGetPrivate,                   "dbus_bus_get_private")
              && dbus.loadInto (dbusConnectionSetExitOnDisconnect,   "dbus_connection_set_exit_on_disconnect")
              && dbus.loadInto (dbusConnectionClose,                 "dbus_connection_close")
              && dbus.loadInto (dbusConnectionUnref,                 "dbus_connection_unref")
              && dbus.loadInto (dbusConnectionFlush,                 "dbus_connection_flush")
              && dbus.loadInto (dbusConnectionSend,                  "dbus_connection_send")
              && dbus.loadInto (dbusConnectionSendWithReply,         "dbus_connection_send_with_reply")
              && dbus.loadInto (dbusConnectionSendWithReplyAndBlock, "dbus_connection_send_with_reply_and_block")
              && dbus.loadInto (dbusConnectionDispatch,              "dbus_connection_dispatch")
              && dbus.loadInto (dbusConnectionGetDispatchStatus,     "dbus_connection_get_dispatch_status")
              && dbus.loadInto (dbusConnectionSetWatchFunctions,     "dbus_connection_set_watch_functions")
              && dbus.loadInto (dbusConnectionSetTimeoutFunctions,   "dbus_connection_set_timeout_functions")
              && dbus.loadInto (dbusConnectionAddFilter,             "dbus_connection_add_filter")
              && dbus.loadInto (dbusConnectionRemoveFilter,          "dbus_connection_remove_filter")
              && dbus.loadInto (dbusBusGetUniqueName,                "dbus_bus_get_unique_name")
              && dbus.loadInto (dbusBusAddMatch,                     "dbus_bus_add_match")
              && dbus.loadInto (dbusBusRemoveMatch,                  "dbus_bus_remove_match")
              && dbus.loadInto (dbusWatchGetUnixFd,                  "dbus_watch_get_unix_fd")
              && dbus.loadInto (dbusWatchGetFlags,                   "dbus_watch_get_flags")
              && dbus.loadInto (dbusWatchGetEnabled,                 "dbus_watch_get_enabled")
              && dbus.loadInto (dbusWatchHandle,                     "dbus_watch_handle")
              && dbus.loadInto (dbusMessageNewMethodCall,            "dbus_message_new_method_call")
              && dbus.loadInto (dbusMessageUnref,                    "dbus_message_unref")
              && dbus.loadInto (dbusMessageGetType,                  "dbus_message_get_type")
              && dbus.loadInto (dbusMessageIsSignal,                 "dbus_message_is_signal")
              && dbus.loadInto (dbusMessageGetPath,                  "dbus_message_get_path")
              && dbus.loadInto (dbusMessageIterInit,                 "dbus_message_iter_init")
              && dbus.loadInto (dbusMessageIterInitAppend,           "dbus_message_iter_init_append")
              && dbus.loadInto (dbusMessageIterAppendBasic,          "dbus_message_iter_append_basic")
              && dbus.loadInto (dbusMessageIterOpenContainer,        "dbus_message_iter_open_container")
              && dbus.loadInto (dbusMessageIterCloseContainer,       "dbus_message_iter_close_container")
              && dbus.loadInto (dbusMessageIterGetArgType,           "dbus_message_iter_get_arg_type")
              && dbus.loadInto (dbusMessageIterGetBasic,             "dbus_message_iter_get_basic")
              && dbus.loadInto (dbusMessageIterRecurse,              "dbus_message_iter_recurse")
              && dbus.loadInto (dbusMessageIterNext,                 "dbus_message_iter_next")
              && dbus.loadInto (dbusPendingCallSetNotify,            "dbus_pending_call_set_notify")
              && dbus.loadInto (dbusPendingCallStealReply,           "dbus_pending_call_steal_reply")
              && dbus.loadInto (dbusPendingCallCancel,               "dbus_pending_call_cancel")
              && dbus.loadInto (dbusPendingCallUnref,                "dbus_pending_call_unref")
              && dbus.loadInto (dbusTimeoutGetInterval,              "dbus_timeout_get_interval")
              && dbus.loadInto (dbusTimeoutGetEnabled,               "dbus_timeout_get_enabled")
              && dbus.loadInto (dbusTimeoutHandle,                   "dbus_timeout_handle")
              && dbus.loadInto (dbusErrorInit,                       "dbus_error_init")
              && dbus.loadInto (dbusErrorFree,                       "dbus_error_free")
              && dbus.loadInto (dbusErrorIsSet,                      "dbus_error_is_set")
              && dbusThreadsInitDefault() != 0;
    });

    return loaded;
}

//==============================================================================
void DBusConnectionCloser::operator() (DBusConnection* connection) const
{
    if (connection == nullptr)
        return;

    auto* symbols = DBusSymbols::getInstance();
    symbols->dbusConnectionClose (connection);
    symbols->dbusConnectionUnref (connection);
}

//==============================================================================
void DBusMessageUnrefer::operator() (DBusMessage* message) const
{
    if (message != nullptr)
        DBusSymbols::getInstance()->dbusMessageUnref (message);
}

} // namespace juce
