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
struct WaylandDataDeviceDelegate
{
    virtual ~WaylandDataDeviceDelegate() = default;

    virtual void flush() = 0;
    virtual void dragSourceActionChanged (uint32_t action) = 0;
    virtual void externalDragEnded() = 0;
};

//==============================================================================
struct WaylandDataDeviceListener
{
    virtual ~WaylandDataDeviceListener() = default;

    virtual bool dataDragMoved (const ComponentPeer::DragInfo&) = 0;
    virtual void dataDragExited (const ComponentPeer::DragInfo&) = 0;
    virtual bool dataDropped (const ComponentPeer::DragInfo&) = 0;
};

//==============================================================================
class WaylandDataDevice final
{
public:
    explicit WaylandDataDevice (WaylandDataDeviceDelegate&);
    ~WaylandDataDevice();

    void bind (wl_data_device_manager*, wl_seat*);
    void unbind();

    void addListener (wl_surface*, WaylandDataDeviceListener&);
    void removeListener (WaylandDataDeviceListener&);

    void inputSerialReceived (uint32_t serial);
    void copyTextToClipboard (const String&);
    String getTextFromClipboard();

    bool externalDragFileInit (wl_surface* origin, uint32_t serial, const StringArray& files,
                               bool canMoveFiles, std::function<void()> callback);
    bool externalDragTextInit (wl_surface* origin, uint32_t serial, const String& text,
                               std::function<void()> callback);

private:
    class FileDescriptor final
    {
    public:
        FileDescriptor() = default;
        explicit FileDescriptor (int fdIn) : fd (fdIn) {}
        FileDescriptor (FileDescriptor&& other) noexcept : fd (std::exchange (other.fd, -1)) {}
        FileDescriptor& operator= (FileDescriptor&& other) noexcept;
        ~FileDescriptor();

        int get() const noexcept       { return fd; }
        int release() noexcept         { return std::exchange (fd, -1); }
        void reset (int fdIn = -1);
        [[nodiscard]] bool setCloseOnExec();
        [[nodiscard]] bool setNonBlocking();

    private:
        int fd = -1;

        JUCE_DECLARE_NON_COPYABLE (FileDescriptor)
    };

    using DataDeviceHandle = std::unique_ptr<wl_data_device,
                                             FunctionPointerDestructor<WaylandProtocol::destroyDataDevice>>;
    using DataSourceHandle = std::unique_ptr<wl_data_source,
                                             FunctionPointerDestructor<WaylandProtocol::wlDataSourceDestroy>>;
    using DataOfferHandle = std::unique_ptr<wl_data_offer,
                                            FunctionPointerDestructor<WaylandProtocol::wlDataOfferDestroy>>;

    enum class SourceKind
    {
        clipboardText,
        draggedText,
        draggedFiles
    };

    enum class NotifyListener
    {
        no,
        yes
    };

    struct Source
    {
        DataSourceHandle handle;
        SourceKind kind = SourceKind::clipboardText;
        bool usesActionNegotiation = false;
        MemoryBlock payload;
        std::function<void()> completion;
    };

    struct Offer
    {
        explicit Offer (wl_data_offer*);

        DataOfferHandle handle;
        std::vector<String> mimeTypes;
        uint32_t sourceActions = WaylandProtocol::wlDataDeviceManagerDndActionNone;
        uint32_t selectedAction = WaylandProtocol::wlDataDeviceManagerDndActionNone;

        JUCE_DECLARE_NON_COPYABLE (Offer)
    };

    struct SurfaceListener
    {
        wl_surface* surface = nullptr;
        WaylandDataDeviceListener* listener = nullptr;
    };

    struct PendingRead
    {
        FileDescriptor fd;
        MemoryBlock content;
    };

    struct PendingWrite
    {
        FileDescriptor fd;
        MemoryBlock content;
        size_t offset = 0;
    };

    struct DragState
    {
        std::unique_ptr<Offer> offer;
        WaylandDataDeviceListener* listener = nullptr;
        wl_surface* surface = nullptr;
        ComponentPeer::DragInfo info;
        String mimeType;
        uint32_t serial = 0;
        bool dataReady = false;
        bool targetNotified = false;
        std::optional<bool> targetAccepted;
        bool dropPending = false;
        bool usesActionNegotiation = false;
    };

    static std::optional<std::pair<FileDescriptor, FileDescriptor>> createPipe();
    static MemoryBlock toMemoryBlock (StringRef);
    static String createUriList (const StringArray&);
    static ComponentPeer::DragInfo makeDragInfo (StringRef mimeType, const MemoryBlock&);
    static String chooseTextMimeType (const std::vector<String>&);
    static String chooseDragMimeType (const std::vector<String>&);

    void publishClipboardIfPossible();
    std::optional<Source> createSource (SourceKind, StringRef payload, uint32_t actions,
                                        std::function<void()> completion = nullptr);
    bool startExternalDrag (wl_surface*, uint32_t serial, SourceKind, StringRef payload,
                            uint32_t actions, std::function<void()> callback);

    Source* findSource (wl_data_source*);
    void handleSourceTarget (wl_data_source*, const char* mimeType);
    void handleSourceSend (wl_data_source*, const char* mimeType, int fd);
    void handleSourceCancelled (wl_data_source*);
    void handleSourceDndDropPerformed (wl_data_source*);
    void handleSourceDndFinished (wl_data_source*);
    void handleSourceAction (wl_data_source*, uint32_t action);
    void endDragSource (wl_data_source*);

    void handleDataOffer (wl_data_offer*);
    void handleDragEnter (uint32_t serial, wl_surface*, int32_t x, int32_t y, wl_data_offer*);
    void handleDragLeave();
    void handleDragMotion (int32_t x, int32_t y);
    void handleDragDrop();
    void handleSelection (wl_data_offer*);

    std::unique_ptr<Offer> takePendingOffer (wl_data_offer*);
    WaylandDataDeviceListener* findListenerForSurface (wl_surface*) const;
    void beginDragRead();
    void handleReadReady (int fd);
    void completeDragRead (bool succeeded, MemoryBlock content);
    void cancelPendingRead();
    void updateDragTarget();
    void finishDrag();
    void clearDrag (NotifyListener);

    std::optional<MemoryBlock> readOffer (Offer&, StringRef mimeType);
    void beginWrite (FileDescriptor, const MemoryBlock&);
    void handleWriteReady (int fd);
    void cancelPendingWrites();

    static const wl_data_source_listener dataSourceListener;
    static const wl_data_device_listener dataDeviceListener;
    static const wl_data_offer_listener dataOfferListener;

    WaylandDataDeviceDelegate& delegate;
    std::shared_ptr<int> eventLoopLiveness = std::make_shared<int> (0);

    wl_data_device_manager* manager = nullptr;
    DataDeviceHandle device;
    std::vector<SurfaceListener> surfaceListeners;
    // The compositor announces an offer and its MIME types before identifying it as a drag or selection.
    std::vector<std::unique_ptr<Offer>> pendingOffers;
    std::unique_ptr<Offer> selectionOffer;
    std::optional<DragState> drag;
    std::optional<PendingRead> pendingRead;
    std::vector<PendingWrite> pendingWrites;
    std::optional<Source> clipboardSource;
    std::optional<Source> dragSource;

    String localClipboardText;
    std::optional<uint32_t> latestInputSerial;
    bool clipboardOwnedLocally = false;
    bool clipboardPublicationPending = false;

   #if JUCE_UNIT_TESTS
    friend class WaylandDataDeviceTests;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandDataDevice)
};

} // namespace juce
