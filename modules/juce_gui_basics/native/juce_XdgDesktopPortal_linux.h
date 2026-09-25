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
struct DBusPendingCallCanceller
{
    void operator() (DBusPendingCall*) const;
};

using DBusPendingCallHandle = std::unique_ptr<DBusPendingCall, DBusPendingCallCanceller>;

//==============================================================================
/*
    Speaks the org.freedesktop.portal.Desktop D-Bus API directly so that file dialogs
    keep working inside Flatpak and Snap sandboxes and on native Wayland sessions, where
    the zenity/kdialog shell-out path cannot be reached or parented.

    The connection to the session bus is made lazily on first use and shared across
    requests. Everything here runs on the message thread only (no internal locking):
    the public methods, the completion callbacks, and the Request lifetime.

    See FileChooser.
*/
class XdgDesktopPortal final : public DeletedAtShutdown,
                               private Timer
{
public:
    // True when org.freedesktop.portal.Desktop serves a usable FileChooser interface.
    bool isFileChooserAvailable();

    // The FileChooser portal interface version, or 0 when unavailable.
    uint32_t getFileChooserVersion();

    //==============================================================================
    // Owns a portal parent-window identifier and any resource that keeps it valid.
    class ParentWindow final
    {
    public:
        explicit ParentWindow (Component* preferredParent);

        const String& getHandle() const noexcept { return handle; }

    private:
        String handle;

        std::unique_ptr<WaylandWindowSystem::ExportedSurfaceHandle> exportedWaylandSurface;

        JUCE_DECLARE_NON_COPYABLE (ParentWindow)
    };

    //==============================================================================
    struct FileChooserOptions
    {
        String title;
        String filters;              ///< JUCE wildcard list, e.g. "*.wav,*.aiff"
        File startingFile;
        bool isSave = false;
        bool isDirectory = false;
        bool selectMultiple = false;
    };

    //==============================================================================
    /*
        A single in-progress portal file dialog.

        Destroying it dismisses the dialog (sends Request.Close) when it has not already
        completed, removes its signal subscription, and cancels any outstanding method call.
    */
    class Request final
    {
    public:
        ~Request();

    private:
        friend class XdgDesktopPortal;

        Request (XdgDesktopPortal& portalIn, String requestPathIn, String matchRuleIn,
                 std::function<void (Array<URL>)> onFinishedIn);

        void handleResponse (DBusMessage*);
        void onMethodReply (DBusPendingCall*);
        void complete (Array<URL>);
        void sendClose();

        static void pendingCallNotify (DBusPendingCall*, void*);

        XdgDesktopPortal& portal;
        String requestPath;
        String matchRule;
        std::function<void (Array<URL>)> onFinished;
        DBusPendingCallHandle pendingCall;
        bool completed = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Request)
    };

    //==============================================================================
    /*  Starts a portal file dialog. onFinished runs once on the message thread with the
        selection (empty on cancel or error). The parent window must outlive the returned
        Request. Returns nullptr if the request couldn't be sent.
    */
    std::unique_ptr<Request> openFileChooser (const ParentWindow&,
                                              const FileChooserOptions&,
                                              std::function<void (Array<URL>)> onFinished);

    JUCE_DECLARE_SINGLETON_INLINE (XdgDesktopPortal, false)

private:
    //==============================================================================
    XdgDesktopPortal() = default;
    ~XdgDesktopPortal() override;

    struct WatchEntry
    {
        DBusWatch* watch = nullptr;
        int fd = -1;
    };

    struct TimeoutEntry
    {
        DBusTimeout* timeout = nullptr;
        double nextCallMs = 0.0;
    };

    bool ensureConnection();
    void closeConnection();
    bool withinRetryDelay() const;
    std::optional<uint32_t> probeFileChooserVersion();

    unsigned int handleAddWatch (DBusWatch*);
    void handleRemoveWatch (DBusWatch*);
    void handleToggledWatch (DBusWatch*);
    void updateWatchRegistration (int fd);
    void handleFdReadable (int fd);

    unsigned int handleAddTimeout (DBusTimeout*);
    void handleRemoveTimeout (DBusTimeout*);
    void handleToggledTimeout (DBusTimeout*);
    void updateTimeoutTimer();
    void dispatchPendingMessages();
    void timerCallback() override;

    int handleFilterMessage (DBusMessage*);
    void rekeyRequest (Request&, const String& newPath);

    static unsigned int addWatch (DBusWatch*, void*);
    static void removeWatch (DBusWatch*, void*);
    static void toggledWatch (DBusWatch*, void*);
    static unsigned int addTimeout (DBusTimeout*, void*);
    static void removeTimeout (DBusTimeout*, void*);
    static void toggledTimeout (DBusTimeout*, void*);
    static int handleMessage (DBusConnection*, DBusMessage*, void*);

    static constexpr uint32_t retryDelayMs = 10000;

    DBusConnectionHandle connection;
    std::optional<uint32_t> cachedVersion;
    std::optional<uint32_t> lastFailureTimeMs;
    std::vector<WatchEntry> watches;
    std::vector<TimeoutEntry> timeouts;
    std::vector<int> registeredFds;
    std::map<String, Request*> requests;
    bool filterInstalled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XdgDesktopPortal)
};

} // namespace juce
