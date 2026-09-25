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

namespace XdgPortalHelpers
{
    static constexpr auto portalBusName        = "org.freedesktop.portal.Desktop";
    static constexpr auto portalPath           = "/org/freedesktop/portal/desktop";
    static constexpr auto fileChooserInterface = "org.freedesktop.portal.FileChooser";
    static constexpr auto requestInterface     = "org.freedesktop.portal.Request";

    static String makeRequestMatchRule (const String& requestPath)
    {
        return "type='signal',interface='org.freedesktop.portal.Request',member='Response',path='"
                + requestPath + "'";
    }

    static void appendString (DBusMessageIter& iter, const char* value)
    {
        DBusSymbols::getInstance()->dbusMessageIterAppendBasic (&iter, DBusConstants::typeString, &value);
    }

    static void appendDictString (DBusMessageIter& dict, const char* key, const char* value)
    {
        auto* dbus = DBusSymbols::getInstance();

        DBusMessageIter entry;
        dbus->dbusMessageIterOpenContainer (&dict, DBusConstants::typeDictEntry, nullptr, &entry);
        dbus->dbusMessageIterAppendBasic (&entry, DBusConstants::typeString, &key);

        DBusMessageIter variant;
        dbus->dbusMessageIterOpenContainer (&entry, DBusConstants::typeVariant, "s", &variant);
        dbus->dbusMessageIterAppendBasic (&variant, DBusConstants::typeString, &value);
        dbus->dbusMessageIterCloseContainer (&entry, &variant);

        dbus->dbusMessageIterCloseContainer (&dict, &entry);
    }

    static void appendDictBool (DBusMessageIter& dict, const char* key, bool value)
    {
        auto* dbus = DBusSymbols::getInstance();

        DBusMessageIter entry;
        dbus->dbusMessageIterOpenContainer (&dict, DBusConstants::typeDictEntry, nullptr, &entry);
        dbus->dbusMessageIterAppendBasic (&entry, DBusConstants::typeString, &key);

        DBusMessageIter variant;
        dbus->dbusMessageIterOpenContainer (&entry, DBusConstants::typeVariant, "b", &variant);
        const unsigned int boolValue = value ? 1u : 0u;
        dbus->dbusMessageIterAppendBasic (&variant, DBusConstants::typeBoolean, &boolValue);
        dbus->dbusMessageIterCloseContainer (&entry, &variant);

        dbus->dbusMessageIterCloseContainer (&dict, &entry);
    }

    static void appendCurrentFolder (DBusMessageIter& dict, const File& folder)
    {
        const auto path = folder.getFullPathName();

        if (path.isEmpty())
            return;

        auto* dbus = DBusSymbols::getInstance();
        const char* key = "current_folder";

        DBusMessageIter entry;
        dbus->dbusMessageIterOpenContainer (&dict, DBusConstants::typeDictEntry, nullptr, &entry);
        dbus->dbusMessageIterAppendBasic (&entry, DBusConstants::typeString, &key);

        DBusMessageIter variant;
        dbus->dbusMessageIterOpenContainer (&entry, DBusConstants::typeVariant, "ay", &variant);

        DBusMessageIter bytes;
        dbus->dbusMessageIterOpenContainer (&variant, DBusConstants::typeArray, "y", &bytes);

        // The portal expects the path as raw bytes including the trailing NUL.
        const auto* utf8 = path.toRawUTF8();
        const auto numBytes = (int) path.getNumBytesAsUTF8() + 1;

        for (int i = 0; i < numBytes; ++i)
        {
            const unsigned char byteValue = (unsigned char) utf8[i];
            dbus->dbusMessageIterAppendBasic (&bytes, DBusConstants::typeByte, &byteValue);
        }

        dbus->dbusMessageIterCloseContainer (&variant, &bytes);
        dbus->dbusMessageIterCloseContainer (&entry, &variant);
        dbus->dbusMessageIterCloseContainer (&dict, &entry);
    }

    static void appendFilters (DBusMessageIter& dict, const String& filterString)
    {
        if (filterString.isEmpty() || filterString == "*" || filterString == "*.*")
            return;

        StringArray patterns;
        patterns.addTokens (filterString, ";,|", "\"");
        patterns.removeEmptyStrings();

        if (patterns.isEmpty())
            return;

        auto* dbus = DBusSymbols::getInstance();
        const char* key = "filters";

        DBusMessageIter entry;
        dbus->dbusMessageIterOpenContainer (&dict, DBusConstants::typeDictEntry, nullptr, &entry);
        dbus->dbusMessageIterAppendBasic (&entry, DBusConstants::typeString, &key);

        DBusMessageIter variant;
        dbus->dbusMessageIterOpenContainer (&entry, DBusConstants::typeVariant, "a(sa(us))", &variant);

        DBusMessageIter filterArray;
        dbus->dbusMessageIterOpenContainer (&variant, DBusConstants::typeArray, "(sa(us))", &filterArray);

        DBusMessageIter filterStruct;
        dbus->dbusMessageIterOpenContainer (&filterArray, DBusConstants::typeStruct, nullptr, &filterStruct);

        const auto label = patterns.joinIntoString (" ");
        const auto* labelUtf8 = label.toRawUTF8();
        dbus->dbusMessageIterAppendBasic (&filterStruct, DBusConstants::typeString, &labelUtf8);

        DBusMessageIter patternArray;
        dbus->dbusMessageIterOpenContainer (&filterStruct, DBusConstants::typeArray, "(us)", &patternArray);

        for (const auto& pattern : patterns)
        {
            DBusMessageIter patternStruct;
            dbus->dbusMessageIterOpenContainer (&patternArray, DBusConstants::typeStruct, nullptr, &patternStruct);

            const unsigned int globType = 0;
            dbus->dbusMessageIterAppendBasic (&patternStruct, DBusConstants::typeUInt32, &globType);

            const auto* patternUtf8 = pattern.toRawUTF8();
            dbus->dbusMessageIterAppendBasic (&patternStruct, DBusConstants::typeString, &patternUtf8);

            dbus->dbusMessageIterCloseContainer (&patternArray, &patternStruct);
        }

        dbus->dbusMessageIterCloseContainer (&filterStruct, &patternArray);
        dbus->dbusMessageIterCloseContainer (&filterArray, &filterStruct);
        dbus->dbusMessageIterCloseContainer (&variant, &filterArray);
        dbus->dbusMessageIterCloseContainer (&entry, &variant);
        dbus->dbusMessageIterCloseContainer (&dict, &entry);
    }

    static URL uriToURL (const String& uri)
    {
        if (uri.startsWith ("file://"))
        {
            auto path = uri.substring (7);
            path = path.upToFirstOccurrenceOf ("?", false, false);
            path = path.upToFirstOccurrenceOf ("#", false, false);
            return URL (File (URL::removeEscapeChars (path)));
        }

        return URL (uri);
    }

    static Array<URL> parseResultsDict (DBusMessageIter dictIter)
    {
        auto* dbus = DBusSymbols::getInstance();
        Array<URL> urls;

        DBusMessageIter entries;
        dbus->dbusMessageIterRecurse (&dictIter, &entries);

        while (dbus->dbusMessageIterGetArgType (&entries) == DBusConstants::typeDictEntry)
        {
            DBusMessageIter entry;
            dbus->dbusMessageIterRecurse (&entries, &entry);

            const char* key = nullptr;

            if (dbus->dbusMessageIterGetArgType (&entry) == DBusConstants::typeString)
                dbus->dbusMessageIterGetBasic (&entry, &key);

            const String keyString (CharPointer_UTF8 (key != nullptr ? key : ""));
            dbus->dbusMessageIterNext (&entry);

            if (keyString == "uris" && dbus->dbusMessageIterGetArgType (&entry) == DBusConstants::typeVariant)
            {
                DBusMessageIter variant;
                dbus->dbusMessageIterRecurse (&entry, &variant);

                if (dbus->dbusMessageIterGetArgType (&variant) == DBusConstants::typeArray)
                {
                    DBusMessageIter uriArray;
                    dbus->dbusMessageIterRecurse (&variant, &uriArray);

                    while (dbus->dbusMessageIterGetArgType (&uriArray) == DBusConstants::typeString)
                    {
                        const char* uri = nullptr;
                        dbus->dbusMessageIterGetBasic (&uriArray, &uri);
                        urls.add (uriToURL (String (CharPointer_UTF8 (uri != nullptr ? uri : ""))));
                        dbus->dbusMessageIterNext (&uriArray);
                    }
                }
            }

            dbus->dbusMessageIterNext (&entries);
        }

        return urls;
    }

    static std::unique_ptr<WaylandWindowSystem::ExportedSurfaceHandle> exportWaylandSurfaceForPeer (ComponentPeer& peer)
    {
        if (auto* waylandPeer = dynamic_cast<WaylandComponentPeer*> (&peer))
            return WaylandWindowSystem::getInstance()->exportSurfaceForExternalParenting (static_cast<wl_surface*> (waylandPeer->getNativeHandle()));

        return nullptr;
    }
} // namespace XdgPortalHelpers

//==============================================================================
XdgDesktopPortal::ParentWindow::ParentWindow (Component* preferredParent)
{
    auto* peer = preferredParent != nullptr ? preferredParent->getPeer() : nullptr;

    if (peer == nullptr)
    {
        if (auto* topLevel = TopLevelWindow::getActiveTopLevelWindow())
            peer = topLevel->getPeer();
    }

    if (peer == nullptr)
        return;

    exportedWaylandSurface = XdgPortalHelpers::exportWaylandSurfaceForPeer (*peer);

    if (exportedWaylandSurface != nullptr)
    {
        handle = "wayland:" + exportedWaylandSurface->handle;
        return;
    }

    if (dynamic_cast<X11ComponentPeer*> (peer) != nullptr)
        handle = "x11:" + String::toHexString ((pointer_sized_int) peer->getNativeHandle());
}

//==============================================================================
XdgDesktopPortal::~XdgDesktopPortal()
{
    // Any live Request still points back at this singleton, so dialogs must not outlive it.
    jassert (requests.empty());

    closeConnection();
    clearSingletonInstance();
}

void XdgDesktopPortal::closeConnection()
{
    stopTimer();

    for (const auto fd : registeredFds)
        LinuxEventLoop::unregisterFdCallback (fd);

    registeredFds.clear();
    watches.clear();
    timeouts.clear();

    if (connection != nullptr && filterInstalled)
        DBusSymbols::getInstance()->dbusConnectionRemoveFilter (connection.get(), handleMessage, this);

    filterInstalled = false;
    connection.reset();
}

// A transient bus failure is remembered for retryDelayMs and then retried, rather than
// disabling the portal for the rest of the process. Inside a sandbox the built-in dialog
// can't browse the host filesystem, so a one-off timeout must not be permanent.
bool XdgDesktopPortal::withinRetryDelay() const
{
    return lastFailureTimeMs.has_value()
        && Time::getMillisecondCounter() - *lastFailureTimeMs < retryDelayMs;
}

bool XdgDesktopPortal::ensureConnection()
{
    if (connection != nullptr)
        return true;

    if (withinRetryDelay())
        return false;

    auto* dbus = DBusSymbols::getInstance();

    if (! dbus->loadAllSymbols())
        return false;

    ScopedDBusError error;
    auto* raw = dbus->dbusBusGetPrivate (DBusConstants::busSession, error.get());

    if (raw == nullptr || error.isSet())
    {
        lastFailureTimeMs = Time::getMillisecondCounter();
        return false;
    }

    dbus->dbusConnectionSetExitOnDisconnect (raw, 0);
    connection = DBusConnectionHandle (raw);

    const auto watchFunctionsInstalled = dbus->dbusConnectionSetWatchFunctions (connection.get(), addWatch, removeWatch,
                                                                                toggledWatch, this, nullptr) != 0;
    const auto timeoutFunctionsInstalled = watchFunctionsInstalled
                                        && dbus->dbusConnectionSetTimeoutFunctions (connection.get(), addTimeout, removeTimeout,
                                                                                    toggledTimeout, this, nullptr) != 0;
    filterInstalled = timeoutFunctionsInstalled
                   && dbus->dbusConnectionAddFilter (connection.get(), handleMessage, this, nullptr) != 0;

    if (! filterInstalled)
    {
        closeConnection();
        lastFailureTimeMs = Time::getMillisecondCounter();
        return false;
    }

    lastFailureTimeMs.reset();
    return true;
}

//==============================================================================
bool XdgDesktopPortal::isFileChooserAvailable()
{
    return getFileChooserVersion() >= 1;
}

uint32_t XdgDesktopPortal::getFileChooserVersion()
{
    // Only a successful probe is cached. A failed one returns 0 for now and is retried later.
    if (! cachedVersion.has_value())
        cachedVersion = probeFileChooserVersion();

    return cachedVersion.value_or (0);
}

std::optional<uint32_t> XdgDesktopPortal::probeFileChooserVersion()
{
    using namespace XdgPortalHelpers;

    if (! ensureConnection() || withinRetryDelay())
        return {};

    auto* dbus = DBusSymbols::getInstance();

    const DBusMessageHandle msg { dbus->dbusMessageNewMethodCall (portalBusName, portalPath,
                                                                  "org.freedesktop.DBus.Properties", "Get") };

    if (msg == nullptr)
        return {};

    DBusMessageIter args;
    dbus->dbusMessageIterInitAppend (msg.get(), &args);
    appendString (args, fileChooserInterface);
    appendString (args, "version");

    ScopedDBusError error;
    const DBusMessageHandle reply { dbus->dbusConnectionSendWithReplyAndBlock (connection.get(), msg.get(), 3000, error.get()) };

    if (reply == nullptr || error.isSet())
    {
        lastFailureTimeMs = Time::getMillisecondCounter();
        return {};
    }

    DBusMessageIter iter;

    if (dbus->dbusMessageIterInit (reply.get(), &iter) == 0
        || dbus->dbusMessageIterGetArgType (&iter) != DBusConstants::typeVariant)
        return {};

    DBusMessageIter variant;
    dbus->dbusMessageIterRecurse (&iter, &variant);

    if (dbus->dbusMessageIterGetArgType (&variant) != DBusConstants::typeUInt32)
        return {};

    uint32_t version = 0;
    dbus->dbusMessageIterGetBasic (&variant, &version);
    lastFailureTimeMs.reset();
    return version;
}

//==============================================================================
std::unique_ptr<XdgDesktopPortal::Request> XdgDesktopPortal::openFileChooser (const ParentWindow& parentWindow,
                                                                              const FileChooserOptions& options,
                                                                              std::function<void (Array<URL>)> onFinished)
{
    using namespace XdgPortalHelpers;

    if (! ensureConnection())
        return nullptr;

    const auto version = getFileChooserVersion();

    if (version < 1)
        return nullptr;

    auto* dbus = DBusSymbols::getInstance();

    const auto token = "juce" + String::toHexString (Random::getSystemRandom().nextInt64());

    const auto* uniqueNameRaw = dbus->dbusBusGetUniqueName (connection.get());

    if (uniqueNameRaw == nullptr)
        return nullptr;

    const String uniqueName { CharPointer_UTF8 (uniqueNameRaw) };
    auto sender = uniqueName.startsWith (":") ? uniqueName.substring (1) : uniqueName;
    sender = sender.replaceCharacter ('.', '_');

    const auto requestPath = String ("/org/freedesktop/portal/desktop/request/") + sender + "/" + token;
    const auto matchRule = makeRequestMatchRule (requestPath);

    // Subscribe before sending so a fast reply can't arrive before we are listening.
    {
        ScopedDBusError error;
        dbus->dbusBusAddMatch (connection.get(), matchRule.toRawUTF8(), error.get());

        if (error.isSet())
            return nullptr;
    }

    ErasedScopeGuard removeMatchOnFailure { [this, dbus, &matchRule]
    {
        ScopedDBusError error;
        dbus->dbusBusRemoveMatch (connection.get(), matchRule.toRawUTF8(), error.get());
    } };

    const DBusMessageHandle msg { dbus->dbusMessageNewMethodCall (portalBusName, portalPath, fileChooserInterface,
                                                                  options.isSave ? "SaveFile" : "OpenFile") };

    if (msg == nullptr)
        return nullptr;

    DBusMessageIter args;
    dbus->dbusMessageIterInitAppend (msg.get(), &args);

    appendString (args, parentWindow.getHandle().toRawUTF8());
    appendString (args, options.title.toRawUTF8());

    DBusMessageIter optionsIter;
    dbus->dbusMessageIterOpenContainer (&args, DBusConstants::typeArray, "{sv}", &optionsIter);

    appendDictString (optionsIter, "handle_token", token.toRawUTF8());

    if (! options.isSave && options.selectMultiple)
        appendDictBool (optionsIter, "multiple", true);

    if (options.isDirectory && version >= 3)
        appendDictBool (optionsIter, "directory", true);

    appendFilters (optionsIter, options.filters);

    if (options.isSave)
    {
        if (options.startingFile.isDirectory())
        {
            appendCurrentFolder (optionsIter, options.startingFile);
        }
        else
        {
            const auto fileName = options.startingFile.getFileName();

            if (fileName.isNotEmpty())
                appendDictString (optionsIter, "current_name", fileName.toRawUTF8());

            appendCurrentFolder (optionsIter, options.startingFile.getParentDirectory());
        }
    }
    else
    {
        appendCurrentFolder (optionsIter, options.startingFile.isDirectory() ? options.startingFile
                                                                             : options.startingFile.getParentDirectory());
    }

    dbus->dbusMessageIterCloseContainer (&args, &optionsIter);

    DBusPendingCall* pending = nullptr;

    if (dbus->dbusConnectionSendWithReply (connection.get(), msg.get(), &pending, DBusConstants::timeoutUseDefault) == 0
        || pending == nullptr)
        return nullptr;

    auto request = std::unique_ptr<Request> (new Request (*this, requestPath, matchRule, std::move (onFinished)));
    request->pendingCall.reset (pending);
    requests[requestPath] = request.get();

    // From here the Request owns teardown of the match rule and the pending call.
    removeMatchOnFailure.release();

    if (dbus->dbusPendingCallSetNotify (pending, Request::pendingCallNotify, request.get(), nullptr) == 0)
        return nullptr;   // the Request destructor removes the match, unrefs the pending call, erases the map entry

    dbus->dbusConnectionFlush (connection.get());
    return request;
}

//==============================================================================
void XdgDesktopPortal::rekeyRequest (Request& request, const String& newPath)
{
    using namespace XdgPortalHelpers;

    auto* dbus = DBusSymbols::getInstance();

    requests.erase (request.requestPath);

    {
        ScopedDBusError error;
        dbus->dbusBusRemoveMatch (connection.get(), request.matchRule.toRawUTF8(), error.get());
    }

    request.requestPath = newPath;
    request.matchRule = makeRequestMatchRule (newPath);

    {
        ScopedDBusError error;
        dbus->dbusBusAddMatch (connection.get(), request.matchRule.toRawUTF8(), error.get());
    }

    requests[newPath] = &request;
}

int XdgDesktopPortal::handleFilterMessage (DBusMessage* msg)
{
    auto* dbus = DBusSymbols::getInstance();

    if (dbus->dbusMessageIsSignal (msg, XdgPortalHelpers::requestInterface, "Response") == 0)
        return DBusConstants::handlerResultNotYetHandled;

    const auto* path = dbus->dbusMessageGetPath (msg);

    if (path == nullptr)
        return DBusConstants::handlerResultNotYetHandled;

    const auto it = requests.find (String (CharPointer_UTF8 (path)));

    if (it == requests.end())
        return DBusConstants::handlerResultNotYetHandled;

    // handleResponse may destroy the Request, so this is the iterator's last use.
    it->second->handleResponse (msg);
    return DBusConstants::handlerResultHandled;
}

//==============================================================================
unsigned int XdgDesktopPortal::handleAddWatch (DBusWatch* watch)
{
    const auto fd = DBusSymbols::getInstance()->dbusWatchGetUnixFd (watch);
    watches.push_back ({ watch, fd });
    updateWatchRegistration (fd);
    return 1;
}

void XdgDesktopPortal::handleRemoveWatch (DBusWatch* watch)
{
    int fd = -1;

    for (const auto& entry : watches)
        if (entry.watch == watch)
            fd = entry.fd;

    watches.erase (std::remove_if (watches.begin(), watches.end(),
                                   [watch] (const WatchEntry& entry) { return entry.watch == watch; }),
                   watches.end());

    if (fd >= 0)
        updateWatchRegistration (fd);
}

void XdgDesktopPortal::handleToggledWatch (DBusWatch* watch)
{
    updateWatchRegistration (DBusSymbols::getInstance()->dbusWatchGetUnixFd (watch));
}

void XdgDesktopPortal::updateWatchRegistration (int fd)
{
    auto* dbus = DBusSymbols::getInstance();

    // Only readable watches drive the event loop. Outgoing writes are flushed synchronously.
    const auto hasEnabledReadableWatch = std::invoke ([this, dbus, fd]
    {
        for (const auto& entry : watches)
            if (entry.fd == fd
                && dbus->dbusWatchGetEnabled (entry.watch) != 0
                && (dbus->dbusWatchGetFlags (entry.watch) & DBusConstants::watchReadable) != 0)
                return true;

        return false;
    });

    const auto alreadyRegistered = std::find (registeredFds.begin(), registeredFds.end(), fd) != registeredFds.end();

    if (hasEnabledReadableWatch && ! alreadyRegistered)
    {
        LinuxEventLoop::registerFdCallback (fd, [this] (int readyFd) { handleFdReadable (readyFd); });
        registeredFds.push_back (fd);
    }
    else if (! hasEnabledReadableWatch && alreadyRegistered)
    {
        LinuxEventLoop::unregisterFdCallback (fd);
        registeredFds.erase (std::remove (registeredFds.begin(), registeredFds.end(), fd), registeredFds.end());
    }
}

void XdgDesktopPortal::handleFdReadable (int fd)
{
    auto* dbus = DBusSymbols::getInstance();

    // Snapshot the watches first: handling one may add or remove entries.
    std::vector<DBusWatch*> readable;

    for (const auto& entry : watches)
        if (entry.fd == fd
            && dbus->dbusWatchGetEnabled (entry.watch) != 0
            && (dbus->dbusWatchGetFlags (entry.watch) & DBusConstants::watchReadable) != 0)
            readable.push_back (entry.watch);

    for (auto* watch : readable)
    {
        const auto isStillRegistered = std::any_of (watches.begin(), watches.end(),
                                                    [watch] (const WatchEntry& entry) { return entry.watch == watch; });

        if (isStillRegistered)
            dbus->dbusWatchHandle (watch, DBusConstants::watchReadable);
    }

    dispatchPendingMessages();
}

//==============================================================================
unsigned int XdgDesktopPortal::handleAddTimeout (DBusTimeout* timeout)
{
    auto* dbus = DBusSymbols::getInstance();
    const auto interval = jmax (1, dbus->dbusTimeoutGetInterval (timeout));
    timeouts.push_back ({ timeout, Time::getMillisecondCounterHiRes() + interval });
    updateTimeoutTimer();
    return 1;
}

void XdgDesktopPortal::handleRemoveTimeout (DBusTimeout* timeout)
{
    timeouts.erase (std::remove_if (timeouts.begin(), timeouts.end(),
                                    [timeout] (const TimeoutEntry& entry) { return entry.timeout == timeout; }),
                    timeouts.end());
    updateTimeoutTimer();
}

void XdgDesktopPortal::handleToggledTimeout (DBusTimeout* timeout)
{
    const auto it = std::find_if (timeouts.begin(), timeouts.end(),
                                  [timeout] (const TimeoutEntry& entry) { return entry.timeout == timeout; });

    if (it != timeouts.end())
    {
        const auto interval = jmax (1, DBusSymbols::getInstance()->dbusTimeoutGetInterval (timeout));
        it->nextCallMs = Time::getMillisecondCounterHiRes() + interval;
    }

    updateTimeoutTimer();
}

void XdgDesktopPortal::updateTimeoutTimer()
{
    auto* dbus = DBusSymbols::getInstance();
    std::optional<double> nextCall;

    for (const auto& entry : timeouts)
    {
        if (dbus->dbusTimeoutGetEnabled (entry.timeout) == 0)
            continue;

        if (! nextCall.has_value() || entry.nextCallMs < *nextCall)
            nextCall = entry.nextCallMs;
    }

    if (! nextCall.has_value())
    {
        stopTimer();
        return;
    }

    const auto delay = *nextCall - Time::getMillisecondCounterHiRes();
    startTimer (jmax (1, (int) std::ceil (delay)));
}

void XdgDesktopPortal::dispatchPendingMessages()
{
    auto* dbus = DBusSymbols::getInstance();

    while (connection != nullptr
           && dbus->dbusConnectionGetDispatchStatus (connection.get()) == DBusConstants::dispatchDataRemains)
        dbus->dbusConnectionDispatch (connection.get());
}

void XdgDesktopPortal::timerCallback()
{
    auto* dbus = DBusSymbols::getInstance();

    for (;;)
    {
        const auto now = Time::getMillisecondCounterHiRes();
        const auto it = std::find_if (timeouts.begin(), timeouts.end(), [dbus, now] (const TimeoutEntry& entry)
        {
            return dbus->dbusTimeoutGetEnabled (entry.timeout) != 0 && entry.nextCallMs <= now;
        });

        if (it == timeouts.end())
            break;

        auto* timeout = it->timeout;
        it->nextCallMs = now + jmax (1, dbus->dbusTimeoutGetInterval (timeout));
        dbus->dbusTimeoutHandle (timeout);
    }

    dispatchPendingMessages();
    updateTimeoutTimer();
}

//==============================================================================
unsigned int XdgDesktopPortal::addWatch (DBusWatch* watch, void* data)
{
    return static_cast<XdgDesktopPortal*> (data)->handleAddWatch (watch);
}

void XdgDesktopPortal::removeWatch (DBusWatch* watch, void* data)
{
    static_cast<XdgDesktopPortal*> (data)->handleRemoveWatch (watch);
}

void XdgDesktopPortal::toggledWatch (DBusWatch* watch, void* data)
{
    static_cast<XdgDesktopPortal*> (data)->handleToggledWatch (watch);
}

unsigned int XdgDesktopPortal::addTimeout (DBusTimeout* timeout, void* data)
{
    return static_cast<XdgDesktopPortal*> (data)->handleAddTimeout (timeout);
}

void XdgDesktopPortal::removeTimeout (DBusTimeout* timeout, void* data)
{
    static_cast<XdgDesktopPortal*> (data)->handleRemoveTimeout (timeout);
}

void XdgDesktopPortal::toggledTimeout (DBusTimeout* timeout, void* data)
{
    static_cast<XdgDesktopPortal*> (data)->handleToggledTimeout (timeout);
}

int XdgDesktopPortal::handleMessage (DBusConnection*, DBusMessage* msg, void* data)
{
    return static_cast<XdgDesktopPortal*> (data)->handleFilterMessage (msg);
}

//==============================================================================
void DBusPendingCallCanceller::operator() (DBusPendingCall* call) const
{
    auto* dbus = DBusSymbols::getInstance();
    dbus->dbusPendingCallCancel (call);
    dbus->dbusPendingCallUnref (call);
}

XdgDesktopPortal::Request::Request (XdgDesktopPortal& portalIn, String requestPathIn, String matchRuleIn,
                                    std::function<void (Array<URL>)> onFinishedIn)
    : portal (portalIn),
      requestPath (std::move (requestPathIn)),
      matchRule (std::move (matchRuleIn)),
      onFinished (std::move (onFinishedIn))
{
}

XdgDesktopPortal::Request::~Request()
{
    auto* dbus = DBusSymbols::getInstance();

    portal.requests.erase (requestPath);
    pendingCall.reset();

    if (! completed)
        sendClose();

    if (matchRule.isNotEmpty() && portal.connection != nullptr)
    {
        ScopedDBusError error;
        dbus->dbusBusRemoveMatch (portal.connection.get(), matchRule.toRawUTF8(), error.get());
    }
}

void XdgDesktopPortal::Request::sendClose()
{
    if (portal.connection == nullptr)
        return;

    auto* dbus = DBusSymbols::getInstance();

    const DBusMessageHandle msg { dbus->dbusMessageNewMethodCall (XdgPortalHelpers::portalBusName, requestPath.toRawUTF8(),
                                                                  XdgPortalHelpers::requestInterface, "Close") };

    if (msg == nullptr)
        return;

    dbus->dbusConnectionSend (portal.connection.get(), msg.get(), nullptr);
    dbus->dbusConnectionFlush (portal.connection.get());
}

void XdgDesktopPortal::Request::handleResponse (DBusMessage* msg)
{
    auto* dbus = DBusSymbols::getInstance();

    DBusMessageIter iter;

    if (dbus->dbusMessageIterInit (msg, &iter) == 0
        || dbus->dbusMessageIterGetArgType (&iter) != DBusConstants::typeUInt32)
    {
        complete ({});
        return;
    }

    uint32_t responseCode = 0;
    dbus->dbusMessageIterGetBasic (&iter, &responseCode);

    if (responseCode != 0)
    {
        complete ({});
        return;
    }

    Array<URL> urls;

    if (dbus->dbusMessageIterNext (&iter) != 0
        && dbus->dbusMessageIterGetArgType (&iter) == DBusConstants::typeArray)
        urls = XdgPortalHelpers::parseResultsDict (iter);

    complete (std::move (urls));
}

void XdgDesktopPortal::Request::onMethodReply (DBusPendingCall* pending)
{
    auto* dbus = DBusSymbols::getInstance();

    const DBusMessageHandle reply { dbus->dbusPendingCallStealReply (pending) };

    // Cancelling a completed call is a no-op, so the shared deleter path is fine here.
    pendingCall.reset();

    if (reply == nullptr || dbus->dbusMessageGetType (reply.get()) == DBusConstants::messageTypeError)
    {
        complete ({});   // may destroy this
        return;
    }

    DBusMessageIter iter;

    if (dbus->dbusMessageIterInit (reply.get(), &iter) != 0
        && dbus->dbusMessageIterGetArgType (&iter) == DBusConstants::typeObjectPath)
    {
        const char* returnedPath = nullptr;
        dbus->dbusMessageIterGetBasic (&iter, &returnedPath);
        const String actualPath (CharPointer_UTF8 (returnedPath != nullptr ? returnedPath : ""));

        // Ancient portals may return a request path we didn't predict. Subscribe to the returned path.
        if (actualPath.isNotEmpty() && actualPath != requestPath)
            portal.rekeyRequest (*this, actualPath);
    }
}

void XdgDesktopPortal::Request::complete (Array<URL> urls)
{
    portal.requests.erase (requestPath);
    completed = true;

    // The callback may destroy this Request, so move it out of the member before calling it.
    auto localCallback = std::move (onFinished);
    onFinished = nullptr;

    if (localCallback != nullptr)
        localCallback (std::move (urls));
}

void XdgDesktopPortal::Request::pendingCallNotify (DBusPendingCall* pending, void* data)
{
    static_cast<Request*> (data)->onMethodReply (pending);
}

} // namespace juce
