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

namespace
{
    constexpr auto textUtf8MimeType = "text/plain;charset=utf-8";
    constexpr auto textMimeType = "text/plain";
    constexpr auto utf8StringMimeType = "UTF8_STRING";
    constexpr auto uriListMimeType = "text/uri-list";
    constexpr size_t maximumTransferSize = 16 * 1024 * 1024;
    constexpr size_t transferBufferSize = 8192;
    constexpr int clipboardReadTimeoutMs = 1000;

    enum class ReadResult
    {
        complete,
        wouldBlock,
        failed
    };

    bool isTextMimeType (StringRef mimeType)
    {
        const String mime { mimeType };
        return mime == textUtf8MimeType || mime == textMimeType || mime == utf8StringMimeType;
    }

    bool isUriListMimeType (StringRef mimeType)
    {
        return String { mimeType } == uriListMimeType;
    }

    ReadResult readAvailableData (int fd, MemoryBlock& content)
    {
        std::array<char, transferBufferSize> buffer;

        for (;;)
        {
            const auto bytesRead = read (fd, buffer.data(), buffer.size());

            if (bytesRead > 0)
            {
                content.append (buffer.data(), (size_t) bytesRead);

                if (maximumTransferSize < content.getSize())
                    return ReadResult::failed;

                continue;
            }

            if (bytesRead == 0)
                return ReadResult::complete;

            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return ReadResult::wouldBlock;

            if (errno != EINTR)
                return ReadResult::failed;
        }
    }

    // A Wayland recipient may close its pipe while data is being sent. Block SIGPIPE only around
    // this write so cancellation cannot terminate the host or change its process-wide disposition.
    ssize_t writeWithoutSigpipe (int fd, const void* data, size_t size)
    {
        sigset_t sigpipeSignals;
        sigemptyset (&sigpipeSignals);
        sigaddset (&sigpipeSignals, SIGPIPE);

        sigset_t previousMask;
        pthread_sigmask (SIG_BLOCK, &sigpipeSignals, &previousMask);

        sigset_t pendingSignals;
        sigpending (&pendingSignals);
        const auto sigpipeWasPending = sigismember (&pendingSignals, SIGPIPE) == 1;
        const auto result = write (fd, data, size);
        const auto writeError = errno;

        if (result < 0 && writeError == EPIPE && ! sigpipeWasPending)
        {
            const timespec noWait {};

            while (sigtimedwait (&sigpipeSignals, nullptr, &noWait) < 0 && errno == EINTR)
            {}
        }

        pthread_sigmask (SIG_SETMASK, &previousMask, nullptr);
        errno = writeError;
        return result;
    }
}

//==============================================================================
WaylandDataDevice::FileDescriptor& WaylandDataDevice::FileDescriptor::operator= (FileDescriptor&& other) noexcept
{
    reset (other.release());
    return *this;
}

WaylandDataDevice::FileDescriptor::~FileDescriptor()
{
    reset();
}

void WaylandDataDevice::FileDescriptor::reset (int fdIn)
{
    if (fd >= 0)
        close (fd);

    fd = fdIn;
}

bool WaylandDataDevice::FileDescriptor::setCloseOnExec()
{
    const auto flags = fcntl (fd, F_GETFD);
    return flags >= 0 && fcntl (fd, F_SETFD, flags | FD_CLOEXEC) == 0;
}

bool WaylandDataDevice::FileDescriptor::setNonBlocking()
{
    const auto flags = fcntl (fd, F_GETFL);
    return flags >= 0 && fcntl (fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

//==============================================================================
WaylandDataDevice::Offer::Offer (wl_data_offer* offer)
    : handle (offer)
{
}

WaylandDataDevice::WaylandDataDevice (WaylandDataDeviceDelegate& delegateIn)
    : delegate (delegateIn)
{
}

WaylandDataDevice::~WaylandDataDevice()
{
    unbind();
    eventLoopLiveness.reset();
}

void WaylandDataDevice::bind (wl_data_device_manager* managerIn, wl_seat* seat)
{
    if (device != nullptr || managerIn == nullptr || seat == nullptr)
        return;

    manager = managerIn;
    device.reset (WaylandProtocol::wlDataDeviceManagerGetDataDevice (manager, seat));

    if (device == nullptr)
    {
        manager = nullptr;
        return;
    }

    WaylandProtocol::wlDataDeviceAddListener (device.get(), &dataDeviceListener, this);
    publishClipboardIfPossible();
}

void WaylandDataDevice::unbind()
{
    const auto hadDragSource = dragSource.has_value();
    const auto completion = dragSource.has_value() ? std::move (dragSource->completion) : std::function<void()>{};

    cancelPendingRead();
    cancelPendingWrites();
    clearDrag (NotifyListener::no);
    pendingOffers.clear();
    selectionOffer.reset();
    dragSource.reset();
    clipboardSource.reset();
    device.reset();
    manager = nullptr;

    if (hadDragSource)
        delegate.externalDragEnded();

    NullCheckedInvocation::invoke (completion);
}

void WaylandDataDevice::addListener (wl_surface* surface, WaylandDataDeviceListener& listener)
{
    // A peer must not register the same surface twice.
    jassert (findListenerForSurface (surface) == nullptr);

    if (surface != nullptr && findListenerForSurface (surface) == nullptr)
        surfaceListeners.push_back ({ surface, &listener });
}

void WaylandDataDevice::removeListener (WaylandDataDeviceListener& listener)
{
    const auto matches = [&] (const SurfaceListener& item) { return item.listener == &listener; };
    const auto it = std::find_if (surfaceListeners.begin(), surfaceListeners.end(), matches);

    if (it == surfaceListeners.end())
        return;

    if (drag.has_value() && drag->listener == &listener)
        clearDrag (NotifyListener::no);

    surfaceListeners.erase (it);
}

void WaylandDataDevice::inputSerialReceived (uint32_t serial)
{
    latestInputSerial = serial;
    publishClipboardIfPossible();
}

void WaylandDataDevice::copyTextToClipboard (const String& text)
{
    localClipboardText = text;
    clipboardOwnedLocally = true;
    clipboardPublicationPending = true;
    clipboardSource.reset();
    publishClipboardIfPossible();
}

String WaylandDataDevice::getTextFromClipboard()
{
    if (clipboardOwnedLocally)
        return localClipboardText;

    if (selectionOffer == nullptr)
        return {};

    const auto mimeType = chooseTextMimeType (selectionOffer->mimeTypes);

    if (mimeType.isEmpty())
        return {};

    if (const auto content = readOffer (*selectionOffer, mimeType))
        return String::fromUTF8 (static_cast<const char*> (content->getData()), (int) content->getSize());

    return {};
}

bool WaylandDataDevice::externalDragFileInit (wl_surface* origin, uint32_t serial, const StringArray& files,
                                              bool canMoveFiles, std::function<void()> callback)
{
    const auto actions = WaylandProtocol::wlDataDeviceManagerDndActionCopy
                       | (canMoveFiles ? WaylandProtocol::wlDataDeviceManagerDndActionMove
                                       : WaylandProtocol::wlDataDeviceManagerDndActionNone);

    return startExternalDrag (origin, serial, SourceKind::draggedFiles, createUriList (files), actions,
                              std::move (callback));
}

bool WaylandDataDevice::externalDragTextInit (wl_surface* origin, uint32_t serial, const String& text,
                                              std::function<void()> callback)
{
    return startExternalDrag (origin, serial, SourceKind::draggedText, text,
                              WaylandProtocol::wlDataDeviceManagerDndActionCopy, std::move (callback));
}

//==============================================================================
std::optional<std::pair<WaylandDataDevice::FileDescriptor, WaylandDataDevice::FileDescriptor>> WaylandDataDevice::createPipe()
{
    int fds[2] { -1, -1 };

    if (pipe (fds) != 0)
        return std::nullopt;

    FileDescriptor readEnd { fds[0] };
    FileDescriptor writeEnd { fds[1] };

    if (! readEnd.setCloseOnExec()
        || ! writeEnd.setCloseOnExec()
        || ! readEnd.setNonBlocking())
        return std::nullopt;

    return std::make_pair (std::move (readEnd), std::move (writeEnd));
}

MemoryBlock WaylandDataDevice::toMemoryBlock (StringRef value)
{
    const String string { value };
    MemoryBlock result;
    result.append (string.toRawUTF8(), string.getNumBytesAsUTF8());
    return result;
}

String WaylandDataDevice::createUriList (const StringArray& files)
{
    String result;

    for (const auto& file : files)
    {
        const auto uri = file.matchesWildcard ("?*://*", false) ? file
                                                                : URL { File { file } }.toString (false);
        result << uri << "\r\n";
    }

    return result;
}

ComponentPeer::DragInfo WaylandDataDevice::makeDragInfo (StringRef mimeType, const MemoryBlock& content)
{
    ComponentPeer::DragInfo result;
    const auto text = String::fromUTF8 (static_cast<const char*> (content.getData()), (int) content.getSize());

    if (! isUriListMimeType (mimeType))
    {
        result.text = text;
        return result;
    }

    for (auto line : StringArray::fromLines (text))
    {
        line = line.trim();

        if (line.isEmpty() || line.startsWithChar ('#'))
            continue;

        // URL::getDomain() returns the first path segment of a file:/// URL, so the raw prefix
        // is the only reliable test for a local file against one on another host.
        if (line.startsWith ("file:///"))
            result.files.add (URL { line }.getLocalFile().getFullPathName());
    }

    return result;
}

String WaylandDataDevice::chooseTextMimeType (const std::vector<String>& mimeTypes)
{
    for (const auto* preferred : { textUtf8MimeType, textMimeType, utf8StringMimeType })
        if (std::find (mimeTypes.begin(), mimeTypes.end(), preferred) != mimeTypes.end())
            return preferred;

    return {};
}

String WaylandDataDevice::chooseDragMimeType (const std::vector<String>& mimeTypes)
{
    if (std::find (mimeTypes.begin(), mimeTypes.end(), uriListMimeType) != mimeTypes.end())
        return uriListMimeType;

    return chooseTextMimeType (mimeTypes);
}

//==============================================================================
void WaylandDataDevice::publishClipboardIfPossible()
{
    if (! clipboardPublicationPending || ! latestInputSerial.has_value() || device == nullptr)
        return;

    auto source = createSource (SourceKind::clipboardText, localClipboardText,
                                WaylandProtocol::wlDataDeviceManagerDndActionNone);

    if (! source.has_value())
        return;

    clipboardSource = std::move (source);
    WaylandProtocol::wlDataDeviceSetSelection (device.get(), clipboardSource->handle.get(), *latestInputSerial);
    clipboardPublicationPending = false;
    delegate.flush();
}

std::optional<WaylandDataDevice::Source> WaylandDataDevice::createSource (SourceKind kind, StringRef payload,
                                                                          uint32_t actions,
                                                                          std::function<void()> completion)
{
    if (manager == nullptr)
        return std::nullopt;

    DataSourceHandle handle { WaylandProtocol::wlDataDeviceManagerCreateDataSource (manager) };

    if (handle == nullptr)
        return std::nullopt;

    WaylandProtocol::wlDataSourceAddListener (handle.get(), &dataSourceListener, this);

    if (kind == SourceKind::draggedFiles)
    {
        WaylandProtocol::wlDataSourceOffer (handle.get(), uriListMimeType);
    }
    else
    {
        WaylandProtocol::wlDataSourceOffer (handle.get(), textUtf8MimeType);
        WaylandProtocol::wlDataSourceOffer (handle.get(), textMimeType);
        WaylandProtocol::wlDataSourceOffer (handle.get(), utf8StringMimeType);
    }

    const auto usesActionNegotiation = WaylandProtocol::getDataSourceVersion (handle.get())
                                    >= WaylandProtocol::dataDeviceActionNegotiationVersion;

    // wl_data_source.set_actions is only valid for drag-and-drop sources.
    if (kind != SourceKind::clipboardText && usesActionNegotiation)
        WaylandProtocol::wlDataSourceSetActions (handle.get(), actions);

    return Source { std::move (handle), kind, usesActionNegotiation,
                    toMemoryBlock (payload), std::move (completion) };
}

bool WaylandDataDevice::startExternalDrag (wl_surface* origin, uint32_t serial, SourceKind kind,
                                           StringRef payload, uint32_t actions,
                                           std::function<void()> callback)
{
    if (origin == nullptr || device == nullptr || dragSource.has_value())
        return false;

    auto source = createSource (kind, payload, actions, std::move (callback));

    if (! source.has_value())
        return false;

    dragSource = std::move (source);
    WaylandProtocol::wlDataDeviceStartDrag (device.get(), dragSource->handle.get(), origin, nullptr, serial);
    delegate.flush();
    return true;
}

WaylandDataDevice::Source* WaylandDataDevice::findSource (wl_data_source* source)
{
    if (clipboardSource.has_value() && clipboardSource->handle.get() == source)
        return &*clipboardSource;

    if (dragSource.has_value() && dragSource->handle.get() == source)
        return &*dragSource;

    return nullptr;
}

void WaylandDataDevice::handleSourceTarget (wl_data_source* source, const char* mimeType)
{
    if (! dragSource.has_value()
        || dragSource->handle.get() != source
        || dragSource->usesActionNegotiation)
        return;

    const auto action = mimeType != nullptr ? WaylandProtocol::wlDataDeviceManagerDndActionCopy
                                            : WaylandProtocol::wlDataDeviceManagerDndActionNone;
    delegate.dragSourceActionChanged (action);
}

void WaylandDataDevice::handleSourceSend (wl_data_source* sourceHandle, const char* mimeType, int fd)
{
    FileDescriptor descriptor { fd };
    auto* source = findSource (sourceHandle);
    const String requestedType { CharPointer_UTF8 (mimeType != nullptr ? mimeType : "") };

    if (source == nullptr)
        return;

    const auto supported = source->kind == SourceKind::draggedFiles ? isUriListMimeType (requestedType)
                                                                    : isTextMimeType (requestedType);

    if (! supported)
        return;

    beginWrite (std::move (descriptor), source->payload);
}

void WaylandDataDevice::handleSourceCancelled (wl_data_source* source)
{
    if (clipboardSource.has_value() && clipboardSource->handle.get() == source)
    {
        clipboardSource.reset();
        clipboardOwnedLocally = false;
        return;
    }

    endDragSource (source);
}

void WaylandDataDevice::handleSourceDndFinished (wl_data_source* source)
{
    endDragSource (source);
}

void WaylandDataDevice::handleSourceDndDropPerformed (wl_data_source* source)
{
    if (dragSource.has_value() && dragSource->handle.get() == source)
        delegate.externalDragEnded();
}

void WaylandDataDevice::handleSourceAction (wl_data_source* source, uint32_t action)
{
    if (! dragSource.has_value()
        || dragSource->handle.get() != source
        || ! dragSource->usesActionNegotiation)
        return;

    const auto recognisedAction = action == WaylandProtocol::wlDataDeviceManagerDndActionCopy
                               || action == WaylandProtocol::wlDataDeviceManagerDndActionMove;
    delegate.dragSourceActionChanged (recognisedAction ? action
                                                       : WaylandProtocol::wlDataDeviceManagerDndActionNone);
}

void WaylandDataDevice::endDragSource (wl_data_source* source)
{
    if (! dragSource.has_value() || dragSource->handle.get() != source)
        return;

    auto completion = std::move (dragSource->completion);
    dragSource.reset();
    delegate.externalDragEnded();
    NullCheckedInvocation::invoke (completion);
}

//==============================================================================
void WaylandDataDevice::handleDataOffer (wl_data_offer* offerHandle)
{
    if (offerHandle == nullptr)
        return;

    auto offer = std::make_unique<Offer> (offerHandle);
    WaylandProtocol::wlDataOfferAddListener (offer->handle.get(), &dataOfferListener, offer.get());
    pendingOffers.push_back (std::move (offer));
}

void WaylandDataDevice::handleDragEnter (uint32_t serial, wl_surface* surface, int32_t x, int32_t y,
                                         wl_data_offer* offerHandle)
{
    clearDrag (NotifyListener::yes);

    auto offer = takePendingOffer (offerHandle);

    if (offer == nullptr)
        return;

    DragState state;
    state.offer = std::move (offer);
    state.listener = findListenerForSurface (surface);
    state.surface = surface;
    state.info.position = WaylandProtocol::fixedToPoint (x, y).roundToInt();
    state.mimeType = chooseDragMimeType (state.offer->mimeTypes);
    state.serial = serial;
    state.usesActionNegotiation = WaylandProtocol::getDataOfferVersion (state.offer->handle.get())
                               >= WaylandProtocol::dataDeviceActionNegotiationVersion;
    drag = std::move (state);

    if (drag->listener == nullptr || drag->mimeType.isEmpty())
    {
        WaylandProtocol::wlDataOfferAccept (drag->offer->handle.get(), serial, nullptr);
        return;
    }

    beginDragRead();
}

void WaylandDataDevice::handleDragLeave()
{
    clearDrag (NotifyListener::yes);
}

void WaylandDataDevice::handleDragMotion (int32_t x, int32_t y)
{
    if (! drag.has_value())
        return;

    drag->info.position = WaylandProtocol::fixedToPoint (x, y).roundToInt();

    if (drag->dataReady)
        updateDragTarget();
}

void WaylandDataDevice::handleDragDrop()
{
    if (! drag.has_value())
        return;

    if (! drag->dataReady)
    {
        if (pendingRead.has_value())
            drag->dropPending = true;
        else
            clearDrag (NotifyListener::no);

        return;
    }

    finishDrag();
}

void WaylandDataDevice::handleSelection (wl_data_offer* offer)
{
    selectionOffer = takePendingOffer (offer);
}

std::unique_ptr<WaylandDataDevice::Offer> WaylandDataDevice::takePendingOffer (wl_data_offer* offer)
{
    if (offer == nullptr)
        return nullptr;

    const auto matches = [offer] (const std::unique_ptr<Offer>& candidate)
    {
        return candidate->handle.get() == offer;
    };
    const auto it = std::find_if (pendingOffers.begin(), pendingOffers.end(), matches);

    if (it == pendingOffers.end())
        return nullptr;

    auto result = std::move (*it);
    pendingOffers.erase (it);
    return result;
}

WaylandDataDeviceListener* WaylandDataDevice::findListenerForSurface (wl_surface* surface) const
{
    const auto matches = [surface] (const SurfaceListener& item) { return item.surface == surface; };

    if (const auto it = std::find_if (surfaceListeners.begin(), surfaceListeners.end(), matches);
        it != surfaceListeners.end())
    {
        return it->listener;
    }

    return nullptr;
}

void WaylandDataDevice::beginDragRead()
{
    if (! drag.has_value())
        return;

    auto pipeEnds = createPipe();

    if (! pipeEnds.has_value())
    {
        completeDragRead (false, {});
        return;
    }

    auto [readEnd, writeEnd] = std::move (*pipeEnds);
    WaylandProtocol::wlDataOfferReceive (drag->offer->handle.get(), drag->mimeType.toRawUTF8(), writeEnd.get());
    writeEnd.reset();
    delegate.flush();

    const auto fd = readEnd.get();
    pendingRead.emplace (PendingRead { std::move (readEnd), {} });
    LinuxEventLoop::registerFdCallback (fd,
                                        [this, weak = std::weak_ptr<int> { eventLoopLiveness }] (int readyFd)
                                        {
                                            if (! weak.expired())
                                                handleReadReady (readyFd);
                                        });
}

void WaylandDataDevice::handleReadReady (int fd)
{
    if (! pendingRead.has_value() || pendingRead->fd.get() != fd)
        return;

    switch (readAvailableData (fd, pendingRead->content))
    {
        case ReadResult::complete:
        {
            auto content = std::move (pendingRead->content);
            completeDragRead (true, std::move (content));
            break;
        }

        case ReadResult::wouldBlock:
            break;

        case ReadResult::failed:
            completeDragRead (false, {});
            break;
    }
}

void WaylandDataDevice::completeDragRead (bool succeeded, MemoryBlock content)
{
    cancelPendingRead();

    if (! drag.has_value())
        return;

    if (succeeded)
        drag->info = makeDragInfo (drag->mimeType, content);

    drag->dataReady = true;
    updateDragTarget();

    if (drag.has_value() && drag->dropPending)
        finishDrag();
}

void WaylandDataDevice::cancelPendingRead()
{
    if (! pendingRead.has_value())
        return;

    LinuxEventLoop::unregisterFdCallback (pendingRead->fd.get());
    pendingRead.reset();
}

void WaylandDataDevice::updateDragTarget()
{
    if (! drag.has_value() || drag->listener == nullptr)
        return;

    auto* listener = drag->listener;
    auto* offer = drag->offer->handle.get();
    const auto info = drag->info;
    const auto sourceOffersCopy = ! drag->usesActionNegotiation
                                || (drag->offer->sourceActions
                                    & WaylandProtocol::wlDataDeviceManagerDndActionCopy) != 0;
    bool accepted = false;

    if (! info.isEmpty() && sourceOffersCopy)
    {
        drag->targetNotified = true;
        accepted = listener->dataDragMoved (info);
    }

    // Listener callbacks can clear the current drag or replace it with another offer.
    if (! drag.has_value() || drag->offer->handle.get() != offer)
        return;

    if (drag->targetAccepted == accepted)
        return;

    drag->targetAccepted = accepted;
    WaylandProtocol::wlDataOfferAccept (offer, drag->serial, accepted ? drag->mimeType.toRawUTF8() : nullptr);

    if (drag->usesActionNegotiation)
        WaylandProtocol::wlDataOfferSetActions (offer,
                                                accepted ? WaylandProtocol::wlDataDeviceManagerDndActionCopy
                                                         : WaylandProtocol::wlDataDeviceManagerDndActionNone,
                                                accepted ? WaylandProtocol::wlDataDeviceManagerDndActionCopy
                                                         : WaylandProtocol::wlDataDeviceManagerDndActionNone);

    delegate.flush();
}

void WaylandDataDevice::finishDrag()
{
    if (! drag.has_value())
        return;

    updateDragTarget();

    if (! drag.has_value())
        return;

    auto* listener = drag->listener;
    auto* offer = drag->offer->handle.get();
    const auto info = drag->info;
    const auto targetAccepted = drag->targetAccepted.value_or (false);
    const auto selectedActionIsCopy = ! drag->usesActionNegotiation
                                   || drag->offer->selectedAction
                                          == WaylandProtocol::wlDataDeviceManagerDndActionCopy;
    const auto dropped = targetAccepted && selectedActionIsCopy
                      && listener != nullptr && listener->dataDropped (info);

    // Listener callbacks can clear the current drag or replace it with another offer.
    if (! drag.has_value() || drag->offer->handle.get() != offer)
        return;

    if (dropped)
        WaylandProtocol::wlDataOfferFinish (offer);
    else
        WaylandProtocol::wlDataOfferAccept (offer, drag->serial, nullptr);

    clearDrag (NotifyListener::no);
    delegate.flush();
}

void WaylandDataDevice::clearDrag (NotifyListener notifyListener)
{
    cancelPendingRead();

    if (! drag.has_value())
        return;

    auto* listener = drag->listener;
    const auto info = drag->info;
    const auto shouldNotify = notifyListener == NotifyListener::yes && drag->targetNotified && listener != nullptr;
    drag.reset();

    if (shouldNotify)
        listener->dataDragExited (info);
}

//==============================================================================
std::optional<MemoryBlock> WaylandDataDevice::readOffer (Offer& offer, StringRef mimeType)
{
    auto pipeEnds = createPipe();

    if (! pipeEnds.has_value())
        return std::nullopt;

    auto [readEnd, writeEnd] = std::move (*pipeEnds);
    const String mime { mimeType };
    WaylandProtocol::wlDataOfferReceive (offer.handle.get(), mime.toRawUTF8(), writeEnd.get());
    writeEnd.reset();
    delegate.flush();

    MemoryBlock content;
    const auto startTime = Time::getMillisecondCounter();

    for (;;)
    {
        const auto elapsed = Time::getMillisecondCounter() - startTime;

        if (clipboardReadTimeoutMs <= elapsed)
            return std::nullopt;

        pollfd descriptor { readEnd.get(), POLLIN, 0 };
        const auto pollResult = poll (&descriptor, 1, clipboardReadTimeoutMs - (int) elapsed);

        if (pollResult < 0)
        {
            if (errno == EINTR)
                continue;

            return std::nullopt;
        }

        if (pollResult == 0)
            return std::nullopt;

        switch (readAvailableData (readEnd.get(), content))
        {
            case ReadResult::complete:
                return content;

            case ReadResult::wouldBlock:
                break;

            case ReadResult::failed:
                return std::nullopt;
        }
    }
}

void WaylandDataDevice::beginWrite (FileDescriptor descriptor, const MemoryBlock& content)
{
    if (descriptor.get() < 0
        || content.isEmpty()
        || ! descriptor.setCloseOnExec()
        || ! descriptor.setNonBlocking())
        return;

    const auto fd = descriptor.get();
    pendingWrites.push_back ({ std::move (descriptor), content, 0 });
    LinuxEventLoop::registerFdCallback (fd,
                                        [this, weak = std::weak_ptr<int> { eventLoopLiveness }] (int readyFd)
                                        {
                                            if (! weak.expired())
                                                handleWriteReady (readyFd);
                                        },
                                        POLLOUT);
}

void WaylandDataDevice::handleWriteReady (int fd)
{
    const auto matches = [fd] (const PendingWrite& pending) { return pending.fd.get() == fd; };
    const auto it = std::find_if (pendingWrites.begin(), pendingWrites.end(), matches);

    if (it == pendingWrites.end())
        return;

    for (;;)
    {
        const auto* data = static_cast<const char*> (it->content.getData()) + it->offset;
        const auto remaining = it->content.getSize() - it->offset;
        const auto bytesWritten = writeWithoutSigpipe (fd, data, remaining);

        if (bytesWritten > 0)
        {
            it->offset += (size_t) bytesWritten;

            if (it->offset == it->content.getSize())
            {
                LinuxEventLoop::unregisterFdCallback (fd);
                pendingWrites.erase (it);
                return;
            }

            continue;
        }

        if (bytesWritten < 0 && errno == EINTR)
            continue;

        if (bytesWritten < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return;

        LinuxEventLoop::unregisterFdCallback (fd);
        pendingWrites.erase (it);
        return;
    }
}

void WaylandDataDevice::cancelPendingWrites()
{
    for (const auto& pending : pendingWrites)
        LinuxEventLoop::unregisterFdCallback (pending.fd.get());

    pendingWrites.clear();
}

//==============================================================================
const wl_data_source_listener WaylandDataDevice::dataSourceListener
{
    [] (void* data, wl_data_source* source, const char* mimeType)
    {
        static_cast<WaylandDataDevice*> (data)->handleSourceTarget (source, mimeType);
    },
    [] (void* data, wl_data_source* source, const char* mimeType, int32_t fd)
    {
        static_cast<WaylandDataDevice*> (data)->handleSourceSend (source, mimeType, fd);
    },
    [] (void* data, wl_data_source* source)
    {
        static_cast<WaylandDataDevice*> (data)->handleSourceCancelled (source);
    },
    [] (void* data, wl_data_source* source)
    {
        static_cast<WaylandDataDevice*> (data)->handleSourceDndDropPerformed (source);
    },
    [] (void* data, wl_data_source* source)
    {
        static_cast<WaylandDataDevice*> (data)->handleSourceDndFinished (source);
    },
    [] (void* data, wl_data_source* source, uint32_t action)
    {
        static_cast<WaylandDataDevice*> (data)->handleSourceAction (source, action);
    }
};

const wl_data_device_listener WaylandDataDevice::dataDeviceListener
{
    [] (void* data, wl_data_device*, wl_data_offer* offer)
    {
        static_cast<WaylandDataDevice*> (data)->handleDataOffer (offer);
    },
    [] (void* data, wl_data_device*, uint32_t serial, wl_surface* surface, int32_t x, int32_t y,
        wl_data_offer* offer)
    {
        static_cast<WaylandDataDevice*> (data)->handleDragEnter (serial, surface, x, y, offer);
    },
    [] (void* data, wl_data_device*)
    {
        static_cast<WaylandDataDevice*> (data)->handleDragLeave();
    },
    [] (void* data, wl_data_device*, uint32_t, int32_t x, int32_t y)
    {
        static_cast<WaylandDataDevice*> (data)->handleDragMotion (x, y);
    },
    [] (void* data, wl_data_device*)
    {
        static_cast<WaylandDataDevice*> (data)->handleDragDrop();
    },
    [] (void* data, wl_data_device*, wl_data_offer* offer)
    {
        static_cast<WaylandDataDevice*> (data)->handleSelection (offer);
    }
};

const wl_data_offer_listener WaylandDataDevice::dataOfferListener
{
    [] (void* data, wl_data_offer*, const char* mimeType)
    {
        if (mimeType != nullptr)
            static_cast<Offer*> (data)->mimeTypes.emplace_back (CharPointer_UTF8 (mimeType));
    },
    [] (void* data, wl_data_offer*, uint32_t actions)
    {
        static_cast<Offer*> (data)->sourceActions = actions;
    },
    [] (void* data, wl_data_offer*, uint32_t action)
    {
        static_cast<Offer*> (data)->selectedAction = action;
    }
};

#if JUCE_UNIT_TESTS

//==============================================================================
class WaylandDataDeviceTests final : public UnitTest
{
public:
    WaylandDataDeviceTests()
        : UnitTest ("WaylandDataDevice", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("UTF-8 text is preferred over other text formats", [&]
        {
            const std::vector<String> offered { utf8StringMimeType, textMimeType, textUtf8MimeType };
            expectEquals (WaylandDataDevice::chooseTextMimeType (offered), String { textUtf8MimeType });
        });

        testCase ("An offer without a supported text format is rejected", [&]
        {
            const std::vector<String> offered { uriListMimeType, "application/octet-stream" };
            expect (WaylandDataDevice::chooseTextMimeType (offered).isEmpty());
        });

        testCase ("A URI list is preferred for a drag that also offers text", [&]
        {
            const std::vector<String> offered { textUtf8MimeType, uriListMimeType };
            expectEquals (WaylandDataDevice::chooseDragMimeType (offered), String { uriListMimeType });
        });

        testCase ("Text MIME data is returned as drag text", [&]
        {
            const auto info = WaylandDataDevice::makeDragInfo (textUtf8MimeType,
                                                               WaylandDataDevice::toMemoryBlock ("dragged text"));

            expectEquals (info.text, String { "dragged text" });
            expect (info.files.isEmpty());
        });

        testCase ("URI lists ignore comments and decode local file names", [&]
        {
            const auto content = WaylandDataDevice::toMemoryBlock ("# source\r\nfile:///tmp/one%20two.wav\r\nhttps://example.com/file\r\n");
            const auto info = WaylandDataDevice::makeDragInfo (uriListMimeType, content);

            expectEquals (info.files.size(), 1);
            expectEquals (info.files[0], String { "/tmp/one two.wav" });
            expect (info.text.isEmpty());
        });

        testCase ("URI lists exclude file URLs on another host", [&]
        {
            const auto content = WaylandDataDevice::toMemoryBlock ("file://other-machine/shared/song.wav\r\n");
            const auto info = WaylandDataDevice::makeDragInfo (uriListMimeType, content);

            expect (info.files.isEmpty());
            expect (info.text.isEmpty());
        });

        testCase ("Outbound file drags use CRLF-separated file URLs", [&]
        {
            const StringArray files { "/tmp/one two.wav", "https://example.com/file" };
            expectEquals (WaylandDataDevice::createUriList (files),
                          String { "file:///tmp/one%20two.wav\r\nhttps://example.com/file\r\n" });
        });
    }
};

static WaylandDataDeviceTests waylandDataDeviceTests;

#endif

} // namespace juce
