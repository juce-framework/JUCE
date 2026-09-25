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
    // Wayland's xkb_v1 keymap format requires this offset when converting event keycodes to XKB.
    static constexpr uint32_t evdevToXkbKeycode (uint32_t keycode)
    {
        return keycode + 8;
    }

    // Linux evdev button codes (linux/input-event-codes.h).
    enum : uint32_t
    {
        btnLeft   = 0x110,
        btnRight  = 0x111,
        btnMiddle = 0x112,
        btnSide   = 0x113,
        btnExtra  = 0x114
    };

    int mouseButtonFlagFor (uint32_t button)
    {
        switch (button)
        {
            case btnLeft:   return ModifierKeys::leftButtonModifier;
            case btnRight:  return ModifierKeys::rightButtonModifier;
            case btnMiddle: return ModifierKeys::middleButtonModifier;
            case btnSide:   return ModifierKeys::backButtonModifier;
            case btnExtra:  return ModifierKeys::forwardButtonModifier;
            default:        return 0;
        }
    }

    // Use the same timestamp rebasing as the X11 backend's getEventTime().
    int64 getEventTimeMs (uint32_t eventTimeMs)
    {
        static int64 eventTimeOffset = 0x12345678;
        const auto thisMessageTime = (int64) eventTimeMs;

        // wl_pointer.enter has no timestamp, so treat 0 as the current time without
        // changing the offset used by later events.
        if (thisMessageTime == 0)
            return Time::currentTimeMillis();

        if (eventTimeOffset == 0x12345678)
            eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;

        return eventTimeOffset + thisMessageTime;
    }

    // Wayland may focus a popup that JUCE has marked as ignoring key presses. Find the
    // nearest parent that accepts key presses instead. Stop after checking every registered
    // listener so an invalid fallback chain cannot loop indefinitely.
    template <typename Listener, typename GetParentListener>
    Listener* resolveKeyEventTarget (Listener* target, size_t maxListenersToCheck, GetParentListener&& getParentListener)
    {
        for (auto remaining = maxListenersToCheck; target != nullptr && remaining > 0; --remaining)
        {
            if (target->canReceiveKeyboardFocus())
                return target;

            target = getParentListener (*target);
        }

        return nullptr;
    }

    bool isModifierKeysym (uint32_t sym)
    {
        switch (sym)
        {
            case KeySymTranslation::keyShiftL:   case KeySymTranslation::keyShiftR:
            case KeySymTranslation::keyControlL: case KeySymTranslation::keyControlR:
            case KeySymTranslation::keyAltL:     case KeySymTranslation::keyAltR:
            case KeySymTranslation::keyNumLock:  case KeySymTranslation::keyCapsLock: case KeySymTranslation::keyScrollLock:
                return true;

            default:
                return false;
        }
    }

    // Values mirror <xkbcommon/xkbcommon.h>.
    enum : int { xkbContextNoFlags = 0 };
    enum : int { xkbKeymapFormatTextV1 = 1 };
    enum : int { xkbKeymapCompileNoFlags = 0 };
    enum : int { xkbStateModsEffective = 1 << 3 };

}

//==============================================================================
bool WaylandInputHandler::XkbSupport::load()
{
    if (! (lib.loadInto (contextNew,           "xkb_context_new")
        && lib.loadInto (contextUnref,         "xkb_context_unref")
        && lib.loadInto (keymapNewFromString,  "xkb_keymap_new_from_string")
        && lib.loadInto (keymapUnref,          "xkb_keymap_unref")
        && lib.loadInto (stateNew,             "xkb_state_new")
        && lib.loadInto (stateUnref,           "xkb_state_unref")
        && lib.loadInto (stateUpdateMask,      "xkb_state_update_mask")
        && lib.loadInto (stateKeyGetOneSym,    "xkb_state_key_get_one_sym")
        && lib.loadInto (stateKeyGetUtf32,     "xkb_state_key_get_utf32")
        && lib.loadInto (keymapKeyRepeatsFn,   "xkb_keymap_key_repeats")
        && lib.loadInto (modGetIndex,          "xkb_keymap_mod_get_index")
        && lib.loadInto (stateModIndexIsActive, "xkb_state_mod_index_is_active")
        && lib.loadInto (stateKeyGetLayout,    "xkb_state_key_get_layout")
        && lib.loadInto (keymapKeyGetSymsByLevel, "xkb_keymap_key_get_syms_by_level")))
    {
        return false;
    }

    context = { contextNew (xkbContextNoFlags), contextUnref };
    return context != nullptr;
}

void WaylandInputHandler::XkbSupport::buildKeymap (const char* keymapString)
{
    resetKeymap();

    keymap = { keymapNewFromString (context.get(), keymapString, xkbKeymapFormatTextV1, xkbKeymapCompileNoFlags),
               keymapUnref };

    if (keymap == nullptr)
        return;

    state = { stateNew (keymap.get()), stateUnref };

    if (state == nullptr)
    {
        resetKeymap();
        return;
    }

    shiftIndex = modGetIndex (keymap.get(), "Shift");
    ctrlIndex  = modGetIndex (keymap.get(), "Control");
    altIndex   = modGetIndex (keymap.get(), "Mod1");
}

void WaylandInputHandler::XkbSupport::resetKeymap()
{
    state.reset();
    keymap.reset();
    shiftIndex = ctrlIndex = altIndex = xkbModInvalid;
}

void WaylandInputHandler::XkbSupport::updateMask (uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group)
{
    if (state != nullptr)
        stateUpdateMask (state.get(), depressed, latched, locked, 0, 0, group);
}

uint32_t WaylandInputHandler::XkbSupport::keyGetBaseSym (uint32_t keycode) const
{
    if (state == nullptr)
        return 0;

    const uint32_t* syms = nullptr;
    const auto layout = stateKeyGetLayout (state.get(), keycode);

    if (keymapKeyGetSymsByLevel (keymap.get(), keycode, layout, 0, &syms) < 1 || syms == nullptr)
        return 0;

    return syms[0];
}

int WaylandInputHandler::XkbSupport::currentModifierFlags() const
{
    int flags = 0;

    if (modActive (shiftIndex)) flags |= ModifierKeys::shiftModifier;
    if (modActive (ctrlIndex))  flags |= ModifierKeys::ctrlModifier;
    if (modActive (altIndex))   flags |= ModifierKeys::altModifier;

    return flags;
}

bool WaylandInputHandler::XkbSupport::modActive (uint32_t index) const
{
    return state != nullptr && index != xkbModInvalid
        && stateModIndexIsActive (state.get(), index, xkbStateModsEffective) > 0;
}

//==============================================================================
void WaylandTouchQueue::touchDown (uint32_t time, wl_surface* surface, int32_t id, int32_t x, int32_t y,
                                   std::optional<uint32_t> popupGrabSerial)
{
    openFrameTouches.push_back (PendingTouchDown { time, surface, id, x, y, popupGrabSerial });
    ++queuedTouchPoints;
}

void WaylandTouchQueue::touchMotion (uint32_t time, int32_t id, int32_t x, int32_t y)
{
    openFrameTouches.push_back (PendingTouchMotion { time, id, x, y });
}

void WaylandTouchQueue::touchUp (uint32_t time, int32_t id)
{
    openFrameTouches.push_back (PendingTouchUp { time, id });

    queuedTouchPoints = jmax (0, queuedTouchPoints - 1);

    // Some compositors omit wl_touch.frame after the final wl_touch.up.
    if (queuedTouchPoints == 0)
        frameComplete();
}

// Events from a later frame remain in openFrameTouches so a partial frame cannot be
// dispatched with an earlier completed frame.
void WaylandTouchQueue::frameComplete()
{
    completedTouches.insert (completedTouches.end(), openFrameTouches.begin(), openFrameTouches.end());
    openFrameTouches.clear();
    dispatchCompletedFrames();
}

void WaylandTouchQueue::cancel()
{
    openFrameTouches.clear();
    completedTouches.clear();
    queuedTouchPoints = 0;
    releaseActiveTouches();
}

void WaylandTouchQueue::surfaceRemoved (wl_surface* surface)
{
    for (auto& slot : touchSlots)
        if (slot.active && slot.surface == surface)
            slot = {};
}

// A nested invocation only queues and returns, so the outermost call processes completed
// frames in arrival order.
void WaylandTouchQueue::dispatchCompletedFrames()
{
    if (isDispatching)
        return;

    const ScopedValueSetter scope { isDispatching, true };

    while (! completedTouches.empty())
    {
        const auto pending = completedTouches.front();
        completedTouches.erase (completedTouches.begin());

        if (const auto* down = std::get_if<PendingTouchDown> (&pending))
            applyTouchDown (*down);
        else if (const auto* motion = std::get_if<PendingTouchMotion> (&pending))
            applyTouchMotion (*motion);
        else if (const auto* up = std::get_if<PendingTouchUp> (&pending))
            applyTouchUp (*up);
    }
}

void WaylandTouchQueue::applyTouchDown (const PendingTouchDown& pending)
{
    if (! dispatcher.hasTouchTarget (pending.surface))
        return;

    const auto index = std::invoke ([&]
    {
        for (const auto [i, slot] : enumerate (touchSlots, int{}))
            if (! slot.active)
                return i;

        touchSlots.push_back ({});
        return (int) touchSlots.size() - 1;
    });

    const auto pos = WaylandProtocol::fixedToPoint (pending.x, pending.y);
    auto& slot = touchSlots[(size_t) index];
    slot.active = true;
    slot.id = pending.id;
    slot.pos = pos;
    slot.surface = pending.surface;
    lastTouchTimeMs = pending.time;

    // An initial no-button event forces mouse-enter, matching the X11 touch path.
    const auto generationBefore = generation;
    dispatcher.dispatchTouchEvent (pending.surface, index, pos, ModifierKeys{}, pending.time, std::nullopt);

    // Stop if a nested cancel removed this touch.
    if (generationBefore != generation)
        return;

    dispatcher.dispatchTouchEvent (pending.surface, index, pos,
                                   ModifierKeys{}.withFlags (ModifierKeys::leftButtonModifier),
                                   pending.time,
                                   pending.popupGrabSerial);
}

void WaylandTouchQueue::applyTouchMotion (const PendingTouchMotion& pending)
{
    const auto index = findActiveTouchIndex (pending.id);

    if (index < 0)
        return;

    // Dispatch can reallocate touchSlots via a nested event loop, so copy what it needs
    // rather than holding a slot reference across the call.
    auto& slot = touchSlots[(size_t) index];
    slot.pos = WaylandProtocol::fixedToPoint (pending.x, pending.y);
    const auto surface = slot.surface;
    const auto pos = slot.pos;
    lastTouchTimeMs = pending.time;

    dispatcher.dispatchTouchEvent (surface, index, pos, ModifierKeys{}.withFlags (ModifierKeys::leftButtonModifier), pending.time, std::nullopt);
}

void WaylandTouchQueue::applyTouchUp (const PendingTouchUp& pending)
{
    const auto index = findActiveTouchIndex (pending.id);

    if (index < 0)
        return;

    auto& slot = touchSlots[(size_t) index];
    const auto surface = slot.surface;
    const auto pos = slot.pos;
    slot = {};
    lastTouchTimeMs = pending.time;

    // A release at the last position ends the drag (and lets a tap behave as click).
    dispatcher.dispatchTouchEvent (surface, index, pos, ModifierKeys{}, pending.time, std::nullopt);

    // An offscreen move clears the hover.
    dispatcher.dispatchTouchEvent (surface, index, MouseInputSource::offscreenMousePos, ModifierKeys{}, pending.time, std::nullopt);
}

// No further events arrive for cancelled touches
// Deliver a mouseUp to end any drag at an offscreen position so it's not a click.
void WaylandTouchQueue::releaseActiveTouches()
{
    ++generation;

    // The slots are taken before dispatching so touches from a new gesture arriving via a
    // nested event loop are placed in the fresh vector and remain outside this cancellation.
    const auto taken = std::exchange (touchSlots, {});

    {
        // Do not dispatch a new gesture until the cancellation releases have been sent,
        // because it may reuse the same JUCE touch indices.
        const ScopedValueSetter scope { isDispatching, true };

        for (const auto [index, slot] : enumerate (taken, int{}))
            if (slot.active)
                dispatcher.dispatchTouchEvent (slot.surface, index, MouseInputSource::offscreenMousePos, ModifierKeys{}, lastTouchTimeMs, std::nullopt);
    }

    dispatchCompletedFrames();
}

int WaylandTouchQueue::findActiveTouchIndex (int32_t id) const
{
    for (const auto [index, slot] : enumerate (touchSlots, int{}))
        if (slot.active && slot.id == id)
            return index;

    return -1;
}

bool WaylandTouchQueue::hasTouchesDown() const noexcept
{
    return std::any_of (touchSlots.begin(), touchSlots.end(), [] (const TouchSlot& slot) { return slot.active; });
}

//==============================================================================
void WaylandPointerAxisState::bindPointer (uint32_t version)
{
    *this = {};
    pointerVersion = version;
}

WaylandPointerAxisState::FrameCompletion WaylandPointerAxisState::addAxisValue (uint32_t axisIndex, int32_t value)
{
    if (std::size (axes) <= axisIndex)
        return FrameCompletion::notRequired;

    axes[axisIndex].value = axes[axisIndex].value.value_or (0.0f) + (float) (value / 256.0);

    return pointerVersion < 5 ? FrameCompletion::required : FrameCompletion::notRequired;
}

// wl_pointer.axis_source arrives at most once per frame and applies to both axes.
void WaylandPointerAxisState::setAxisSource (uint32_t source)
{
    frameAxisSource = source;
}

void WaylandPointerAxisState::addDiscreteAxisSteps (uint32_t axisIndex, int32_t steps)
{
    if (std::size (axes) <= axisIndex)
        return;

    axes[axisIndex].discrete = axes[axisIndex].discrete.value_or (0) + steps;
}

void WaylandPointerAxisState::addAxisValue120 (uint32_t axisIndex, int32_t value)
{
    if (std::size (axes) <= axisIndex)
        return;

    axes[axisIndex].value120 = axes[axisIndex].value120.value_or (0) + value;
}

void WaylandPointerAxisState::setAxisRelativeDirection (uint32_t axisIndex, uint32_t direction)
{
    if (std::size (axes) <= axisIndex)
        return;

    axes[axisIndex].inverted = direction == WaylandProtocol::wlPointerAxisRelativeDirectionInverted;
}

std::array<std::optional<MouseWheelDetails>, 2> WaylandPointerAxisState::commitFrame()
{
    std::array<std::optional<MouseWheelDetails>, 2> wheelDetails;

    for (const auto [axisIndex, accumulated] : enumerate (axes))
    {
        if (! (accumulated.value || accumulated.discrete || accumulated.value120))
            continue;

        // One detent is 120 value120 units, a discrete step, or about 15 units of
        // continuous axis value. At v8+ the compositor sends value120 instead of discrete.
        const auto detents = std::invoke ([&axis = accumulated]
        {
            if (axis.value120.has_value())
                return (float) *axis.value120 / 120.0f;

            if (axis.discrete.has_value())
                return (float) *axis.discrete;

            return *axis.value / 15.0f;
        });
        const auto amount = -detents * (50.0f / 256.0f);

        MouseWheelDetails wheel;
        wheel.deltaX = (axisIndex == (int) WaylandProtocol::wlPointerAxisHorizontalScroll) ? amount : 0.0f;
        wheel.deltaY = (axisIndex == (int) WaylandProtocol::wlPointerAxisVerticalScroll)   ? amount : 0.0f;
        wheel.isReversed = accumulated.inverted;
        wheel.isSmooth = frameAxisSource == WaylandProtocol::wlPointerAxisSourceFinger
                      || frameAxisSource == WaylandProtocol::wlPointerAxisSourceContinuous;
        wheel.isInertial = false;

        wheelDetails[(size_t) axisIndex] = wheel;
    }

    axes[0] = {};
    axes[1] = {};
    frameAxisSource.reset();

    return wheelDetails;
}

//==============================================================================
WaylandInputHandler::WaylandInputHandler (WaylandInputHandlerDelegate& delegateIn)
    : delegate (delegateIn)
{
    xkb.load();
}

WaylandInputHandler::~WaylandInputHandler()
{
    unbindAllDevices();
}

void WaylandInputHandler::unbindAllDevices()
{
    stopKeyRepeat();
    unbindPointer();
    unbindKeyboard();
    unbindTouch();
}

void WaylandInputHandler::addListener (wl_surface* surface, WaylandInputHandlerListener& listener)
{
    // Each surface can only have one listener, its peer
    jassert (findListenerForSurface (surface) == nullptr);

    surfaceListeners.push_back ({ surface, &listener });
}

void WaylandInputHandler::removeListener (WaylandInputHandlerListener& listener)
{
    const auto matches = [&] (const SurfaceListener& surfaceListener) { return surfaceListener.listener == &listener; };
    const auto it = std::find_if (surfaceListeners.begin(), surfaceListeners.end(), matches);

    if (it == surfaceListeners.end())
        return;

    touchQueue.surfaceRemoved (it->surface);

    if (pointerFocus.has_value() && &pointerFocus->listener == &listener)
    {
        pointerFocus.reset();
        LinuxInputState::get().pointerLeft (LinuxInputBackend::wayland);
    }

    const auto removedNativeFocus = nativeKeyboardFocus.has_value() && &nativeKeyboardFocus->listener == &listener;
    const auto removedPeerFocus = peerFocusListener == &listener;

    if (removedNativeFocus)
    {
        nativeKeyboardFocus.reset();
        LinuxInputState::get().keyboardLeft (LinuxInputBackend::wayland);
    }

    if (removedPeerFocus)
        peerFocusListener = nullptr;

    // The current event needs no matching cleanup because it only lives inside a dispatch
    // scope, and its surfaces are only used as lookup keys.
    if (retainedPress.has_value() && retainedPress->trigger.surface == it->surface)
        retainedPress.reset();

    if (latestInputSerial.has_value() && latestInputSerial->sourceSurface == it->surface)
        latestInputSerial.reset();

    const auto dragSurface = std::invoke ([&] () -> wl_surface*
    {
        if (const auto* candidate = std::get_if<DragCandidate> (&dragState))
            return candidate->surface;

        if (const auto* externalDrag = std::get_if<ExternalDrag> (&dragState))
            return externalDrag->surface;

        return nullptr;
    });

    if (dragSurface == it->surface)
        dragState = std::monostate{};

    for (auto& entry : surfaceListeners)
        if (entry.rememberedFocus == &listener)
            entry.rememberedFocus = nullptr;

    surfaceListeners.erase (it);

    // Removing the peer focus target may leave native focus unchanged, so the compositor
    // may not send a keyboard focus event. Schedule an update to select another peer.
    if (removedPeerFocus && ! removedNativeFocus)
        scheduleFocusUpdate();
}

void WaylandInputHandler::seatCapabilitiesChanged (wl_seat* seat, uint32_t capabilities)
{
    if (seat == nullptr)
        return;

    const auto updateBinding = [&] (uint32_t capability, const auto& proxy, auto bind, auto unbind)
    {
        const bool wanted = (capabilities & capability) != 0;

        if (wanted && proxy == nullptr)
            (this->*bind) (seat);
        else if (! wanted && proxy != nullptr)
            (this->*unbind)();
    };

    updateBinding (WaylandProtocol::wlSeatCapabilityPointer,  pointer,  &WaylandInputHandler::bindPointer,  &WaylandInputHandler::unbindPointer);
    updateBinding (WaylandProtocol::wlSeatCapabilityKeyboard, keyboard, &WaylandInputHandler::bindKeyboard, &WaylandInputHandler::unbindKeyboard);
    updateBinding (WaylandProtocol::wlSeatCapabilityTouch,    touch,    &WaylandInputHandler::bindTouch,    &WaylandInputHandler::unbindTouch);
}

bool WaylandInputHandler::isKeyCurrentlyDown (int keyCode) const
{
    // Match the applied or the unshifted sym, case-folded: X11 answers by physical key, so
    // shift+5 queried as '5' and a numlocked keypad digit must both still count as down.
    const auto target = CharacterFunctions::toLowerCase ((juce_wchar) KeySymTranslation::keySymForKeyPressCode (keyCode));

    const auto matchesTarget = [target] (uint32_t sym)
    {
        return CharacterFunctions::toLowerCase ((juce_wchar) sym) == target;
    };

    for (const auto& pressed : pressedKeys)
        if (matchesTarget (pressed.sym) || matchesTarget (pressed.baseSym))
            return true;

    return false;
}

std::optional<WaylandCursor::PointerTarget> WaylandInputHandler::getCursorTarget (wl_surface* surface) const
{
    if (pointer == nullptr
        || getExternalDragCursorTarget().has_value()
        || ! pointerFocus.has_value()
        || pointerFocus->surface != surface)
        return std::nullopt;

    return WaylandCursor::PointerTarget { pointer.get(), pointerFocus->serial, pointerFocus->listener.getPointerCursorScale() };
}

std::optional<uint32_t> WaylandInputHandler::getHeldPressSerial (wl_surface* surface) const
{
    if (const auto* candidate = std::get_if<DragCandidate> (&dragState);
        candidate != nullptr && candidate->surface == surface)
        return candidate->serial;

    return std::nullopt;
}

void WaylandInputHandler::externalDragStarted (wl_surface* surface)
{
    const auto* candidate = std::get_if<DragCandidate> (&dragState);

    if (candidate == nullptr || candidate->surface != surface)
        return;

    const auto externalDrag = ExternalDrag { candidate->surface, candidate->mouseButtonFlag,
                                             candidate->cursorTarget, lastPointerPos };
    dragState = externalDrag;
}

std::optional<WaylandCursor::PointerTarget> WaylandInputHandler::getExternalDragCursorTarget() const
{
    if (const auto* drag = std::get_if<ExternalDrag> (&dragState))
        return drag->cursorTarget;

    return std::nullopt;
}

void WaylandInputHandler::externalDragEnded()
{
    const auto* drag = std::get_if<ExternalDrag> (&dragState);

    if (drag == nullptr)
        return;

    const auto endedDrag = *drag;
    dragState = std::monostate{};

    if (endedDrag.unreleasedButtonFlag == 0)
        return;

    // The compositor owns the grab during a native drag, so wl_pointer may not report the release.
    const auto modifiers = ModifierKeys::getCurrentModifiers();
    ModifierKeys::currentModifiers = modifiers.withoutFlags (endedDrag.unreleasedButtonFlag);

    if (auto* listener = findListenerForSurface (endedDrag.surface))
        listener->pointerButton (endedDrag.releasePosition, false, Time::currentTimeMillis());
}

void WaylandInputHandler::dragHandedToCompositor (wl_surface* surface)
{
    const auto* candidate = std::get_if<DragCandidate> (&dragState);

    if (candidate == nullptr || candidate->surface != surface)
        return;

    const auto buttonFlag = candidate->mouseButtonFlag;
    dragState = std::monostate{};

    // The touch queue ends the press when the compositor cancels the gesture.
    if (buttonFlag == 0)
        return;

    // The current mouse event already carries its modifiers by value. Clearing the global state
    // makes the next pointer event end the JUCE drag without re-entering this callback.
    ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutFlags (buttonFlag);
}

std::optional<WaylandPopupTrigger> WaylandInputHandler::getAvailablePopupTrigger()
{
    if (currentInputEvent.trigger.has_value() && currentInputEvent.trigger->grabSerial->isAvailable())
        return currentInputEvent.trigger;

    if (retainedPress.has_value() && retainedPress->trigger.grabSerial->isAvailable())
        return retainedPress->trigger;

    retainedPress.reset();

    return std::nullopt;
}

WaylandPopupContext WaylandInputHandler::getPopupContext()
{
    WaylandPopupContext result;
    result.surfaces.reserve (surfaceListeners.size());

    for (const auto& entry : surfaceListeners)
        result.surfaces.push_back ({ entry.surface, entry.listener });

    result.trigger = getAvailablePopupTrigger();
    result.pointerSurface = pointerFocus.has_value() ? pointerFocus->surface : nullptr;
    result.inputSurface = std::invoke ([&]() -> wl_surface*
    {
        if (currentInputEvent.targetSurface != nullptr)
            return currentInputEvent.targetSurface;

        if (currentInputEvent.trigger.has_value())
            return currentInputEvent.trigger->surface;

        if (result.trigger.has_value())
            return result.trigger->surface;

        if (nativeKeyboardFocus.has_value())
            return nativeKeyboardFocus->surface;

        if (pointerFocus.has_value())
            return pointerFocus->surface;

        return nullptr;
    });
    return result;
}

void WaylandInputHandler::popupGrabStarted (wl_surface* toplevelSurface, Component& componentToDismiss)
{
    ++activePopupGrabs;

    if (auto* listener = findListenerForSurface (toplevelSurface))
        listener->popupGrabStarted (componentToDismiss);
}

void WaylandInputHandler::popupGrabEnded (wl_surface* toplevelSurface)
{
    activePopupGrabs = jmax (0, activePopupGrabs - 1);

    if (auto* listener = findListenerForSurface (toplevelSurface))
        listener->popupGrabEnded();
}

std::optional<WaylandPopupTrigger> WaylandInputHandler::makePopupTrigger (wl_surface* surface,
                                                                        std::optional<uint32_t> serial)
{
    if (surface == nullptr || ! serial.has_value())
        return std::nullopt;

    return WaylandPopupTrigger { surface, std::make_shared<WaylandPopupGrabSerial> (*serial) };
}

void WaylandInputHandler::retainCurrentPressForPopup (PressDevice device)
{
    if (currentInputEvent.trigger.has_value())
        retainedPress = RetainedPress { *currentInputEvent.trigger, device };
}

// A retained press cannot authorise a popup grab after the seat loses the corresponding input capability.
void WaylandInputHandler::dropRetainedPress (PressDevice device)
{
    if (retainedPress.has_value() && retainedPress->device == device)
    {
        // The current input event or a popup context may still share this serial.
        retainedPress->trigger.grabSerial->claim();
        retainedPress.reset();
    }
}

void WaylandInputHandler::reportInputSerial (wl_surface* sourceSurface, uint32_t serial)
{
    latestInputSerial = InputEventSerial { sourceSurface, serial };
    delegate.inputSerialReceived (serial);
}

//==============================================================================
void WaylandInputHandler::bindPointer (wl_seat* seat)
{
    pointer.reset (WaylandProtocol::wlSeatGetPointer (seat));

    if (pointer == nullptr)
        return;

    axisState.bindPointer (WaylandClientSymbols::getInstance()->wl_proxy_get_version (reinterpret_cast<wl_proxy*> (pointer.get())));
    WaylandProtocol::wlPointerAddListener (pointer.get(), &pointerListener, this);
}

void WaylandInputHandler::bindKeyboard (wl_seat* seat)
{
    // No keyboard without xkbcommon, which we need to turn keycodes into characters.
    if (! xkb.isAvailable())
        return;

    keyboard.reset (WaylandProtocol::wlSeatGetKeyboard (seat));

    if (keyboard == nullptr)
        return;

    WaylandProtocol::wlKeyboardAddListener (keyboard.get(), &keyboardListener, this);
}

void WaylandInputHandler::bindTouch (wl_seat* seat)
{
    touch.reset (WaylandProtocol::wlSeatGetTouch (seat));

    if (touch == nullptr)
        return;

    WaylandProtocol::wlTouchAddListener (touch.get(), &touchListener, this);
}

void WaylandInputHandler::unbindPointer()
{
    dropRetainedPress (PressDevice::pointer);
    externalDragEnded();
    pointer.reset();
    pointerFocus.reset();
    dragState = std::monostate{};
    LinuxInputState::get().pointerLeft (LinuxInputBackend::wayland);
}

void WaylandInputHandler::unbindKeyboard()
{
    dropRetainedPress (PressDevice::keyboard);
    stopKeyRepeat();
    pressedKeys.clear();
    keyboard.reset();

    // Reset first so a focus query made from the callback sees focus as already gone.
    nativeKeyboardFocus.reset();
    focusLossSync.reset();

    if (auto* lost = std::exchange (peerFocusListener, nullptr))
        lost->keyboardFocusLost();

    xkb.resetKeymap();
    LinuxInputState::get().keyboardLeft (LinuxInputBackend::wayland);
}

void WaylandInputHandler::unbindTouch()
{
    dropRetainedPress (PressDevice::touch);
    touch.reset();
    touchQueue.cancel();
}

void WaylandInputHandler::surfaceUnmapped (wl_surface* surface)
{
    // Subsurfaces also unmap when their toplevel unmaps, without submitting their own commit.
    const auto unmapped = [&] (wl_surface* candidate)
    {
        if (candidate == surface)
            return true;

        return std::any_of (surfaceListeners.begin(), surfaceListeners.end(), [&] (const auto& entry)
        {
            return entry.surface == candidate && entry.listener->getToplevelSurface() == surface;
        });
    };

    if (pointerFocus.has_value() && unmapped (pointerFocus->surface))
    {
        pointerFocus.reset();
        LinuxInputState::get().pointerLeft (LinuxInputBackend::wayland);
    }

    if (retainedPress.has_value() && unmapped (retainedPress->trigger.surface))
        dropRetainedPress (PressDevice::pointer);

    // The compositor may not deliver the release for a button held on an unmapped surface.
    // External drags release their held button separately when they finish.
    if (const auto* candidate = std::get_if<DragCandidate> (&dragState); candidate != nullptr && unmapped (candidate->surface))
    {
        dragState = std::monostate{};

        if (LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::wayland))
            ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons();
    }
}

//==============================================================================
void WaylandInputHandler::onPointerEnter (uint32_t serial, wl_surface* surface, int32_t surfaceX, int32_t surfaceY)
{
    LinuxInputState::get().pointerEntered (LinuxInputBackend::wayland);
    pointerFocus.reset();

    if (auto* listener = findListenerForSurface (surface))
        pointerFocus.emplace (surface, *listener, serial);

    lastPointerPos = WaylandProtocol::fixedToPoint (surfaceX, surfaceY);
    dispatchPointerMotion();

    if (pointerFocus.has_value())
        pointerFocus->listener.pointerEntered();
}

void WaylandInputHandler::onPointerLeave()
{
    // A leave during a held press means the compositor moved the grab elsewhere, so the release
    // may never arrive. Popup grabs and external drags release their held button themselves.
    if (activePopupGrabs == 0 && ! std::holds_alternative<ExternalDrag> (dragState)
        && ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown()
        && LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::wayland))
    {
        dragState = std::monostate{};
        ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons();
    }

    // Mirror X11 LeaveNotify: a final move at the last position lets JUCE update hover.
    dispatchPointerMotion();
    pointerFocus.reset();
}

void WaylandInputHandler::onPointerMotion (uint32_t time, int32_t surfaceX, int32_t surfaceY)
{
    lastPointerTimeMs = time;
    lastPointerPos = WaylandProtocol::fixedToPoint (surfaceX, surfaceY);
    dispatchPointerMotion();
}

void WaylandInputHandler::onPointerButton (uint32_t serial, uint32_t time, uint32_t button, uint32_t buttonState)
{
    if (! pointerFocus.has_value())
        return;

    const auto flag = mouseButtonFlagFor (button);

    if (flag == 0)
        return;

    lastPointerTimeMs = time;
    const bool pressed = buttonState == WaylandProtocol::wlPointerButtonStatePressed;
    const auto previousModifiers = ModifierKeys::getCurrentModifiers();

    ModifierKeys::currentModifiers = pressed ? previousModifiers.withFlags (flag)
                                             : previousModifiers.withoutFlags (flag);

    if (pressed)
    {
        reportInputSerial (pointerFocus->surface, serial);

        if (! previousModifiers.isAnyMouseButtonDown())
            dragState = DragCandidate { pointerFocus->surface,
                                     serial,
                                     flag,
                                     WaylandCursor::PointerTarget { pointer.get(), pointerFocus->serial,
                                                                    pointerFocus->listener.getPointerCursorScale() } };
    }
    else if (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
    {
        if (auto* drag = std::get_if<ExternalDrag> (&dragState))
            drag->unreleasedButtonFlag = 0;
        else
            dragState = std::monostate{};
    }

    const ScopedValueSetter eventScope { currentInputEvent, InputEvent {
        pointerFocus->surface,
        makePopupTrigger (pointerFocus->surface, pressed ? std::optional { serial } : std::nullopt)
    } };

    if (pressed)
        retainCurrentPressForPopup (PressDevice::pointer);

    pointerFocus->listener.pointerButton (lastPointerPos, pressed, getEventTimeMs (time));
}

void WaylandInputHandler::onPointerAxis (uint32_t time, uint32_t axis, int32_t value)
{
    lastPointerTimeMs = time;

    if (axisState.addAxisValue (axis, value) == WaylandPointerAxisState::FrameCompletion::required)
        onPointerFrame();
}

void WaylandInputHandler::onPointerFrame()
{
    const auto wheelDetails = axisState.commitFrame();

    // Recheck the listener for each axis because application code run by pointerWheel()
    // may remove the peer.
    for (const auto& wheel : wheelDetails)
        if (wheel.has_value() && pointerFocus.has_value())
        {
            const ScopedValueSetter eventScope { currentInputEvent, InputEvent { pointerFocus->surface, std::nullopt } };
            pointerFocus->listener.pointerWheel (lastPointerPos, *wheel, getEventTimeMs (lastPointerTimeMs));
        }
}

void WaylandInputHandler::dispatchPointerMotion()
{
    if (pointerFocus.has_value())
    {
        const ScopedValueSetter eventScope { currentInputEvent, InputEvent { pointerFocus->surface, std::nullopt } };
        lastPointerGlobalPos = pointerFocus->listener.convertPointerPositionToGlobal (lastPointerPos);
        pointerFocus->listener.pointerMoved (lastPointerPos, getEventTimeMs (lastPointerTimeMs));
    }
}

//==============================================================================
void WaylandInputHandler::onKeymap (uint32_t format, int fd, uint32_t size)
{
    const ScopeGuard closeFd { [fd] { close (fd); } };

    if (format != WaylandProtocol::wlKeyboardKeymapFormatXkbV1)
        return;

    auto* mapped = mmap (nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (mapped == MAP_FAILED)
        return;

    const ScopeGuard unmapKeymap { [mapped, size] { munmap (mapped, size); } };

    xkb.buildKeymap (static_cast<const char*> (mapped));
    stopKeyRepeat();
    pressedKeys.clear();
}

void WaylandInputHandler::onKeyboardEnter (wl_surface* surface, Span<const uint32_t> keys)
{
    LinuxInputState::get().keyboardEntered (LinuxInputBackend::wayland);
    nativeKeyboardFocus.reset();
    stopKeyRepeat();
    pressedKeys.clear();

    if (xkb.hasState())
        for (const auto key : keys)
        {
            const auto xkbKeycode = evdevToXkbKeycode (key);
            updatePressedKeys (key, xkb.keyGetOneSym (xkbKeycode), xkb.keyGetBaseSym (xkbKeycode), true);
        }

    if (auto* listener = findListenerForSurface (surface))
    {
        nativeKeyboardFocus.emplace (surface, *listener);
        focusLossSync.reset();
        updatePeerFocus();
    }
}

void WaylandInputHandler::onKeyboardLeave (wl_surface* surface)
{
    if (surface != nullptr && (! nativeKeyboardFocus.has_value() || nativeKeyboardFocus->surface != surface))
        return;

    stopKeyRepeat();

    // No further key or modifiers events arrive once focus is gone, so held state would stick.
    // An X11 window in this process may already hold the keyboard, and then the flags belong to it.
    pressedKeys.clear();
    const auto oldMods = ModifierKeys::getCurrentModifiers();

    if (LinuxInputState::get().mayUpdateKeyboardState (LinuxInputBackend::wayland))
        ModifierKeys::currentModifiers = oldMods.withOnlyMouseButtons();

    auto* modifierTarget = oldMods != ModifierKeys::getCurrentModifiers() ? findKeyEventTarget() : nullptr;

    // Clear native focus before application code can deliver the next enter in a nested loop.
    nativeKeyboardFocus.reset();

    if (modifierTarget != nullptr)
        modifierTarget->modifierKeysChanged();

    if (peerFocusListener != nullptr && ! nativeKeyboardFocus.has_value())
        scheduleFocusUpdate();
}

void WaylandInputHandler::scheduleFocusUpdate()
{
    if (auto* display = delegate.getDisplay())
    {
        focusLossSync.reset (WaylandProtocol::wlDisplaySync (display));

        if (focusLossSync != nullptr)
            WaylandProtocol::wlCallbackAddListener (focusLossSync.get(), &focusLossSyncListener, this);
    }
}

void WaylandInputHandler::handleFocusLossSyncDone (wl_callback* callback)
{
    if (callback != focusLossSync.get())
        return;

    focusLossSync.reset();
    updatePeerFocus();
}

void WaylandInputHandler::updatePeerFocus()
{
    setPeerFocusListener (findKeyEventTarget());
}

void WaylandInputHandler::rememberKeyboardFocus (WaylandInputHandlerListener& listener)
{
    for (auto& entry : surfaceListeners)
        if (entry.surface == listener.getToplevelSurface())
            entry.rememberedFocus = &listener;
}

void WaylandInputHandler::setPeerFocusListener (WaylandInputHandlerListener* listener)
{
    if (peerFocusListener == listener)
        return;

    if (listener != nullptr)
        rememberKeyboardFocus (*listener);

    if (auto* previous = std::exchange (peerFocusListener, listener))
        previous->keyboardFocusLost();

    // The loss callback may destroy the previous or requested peer,
    // unmap the owning toplevel, or move focus elsewhere.
    if (listener != nullptr && peerFocusListener == listener)
    {
        if (listener->canReceiveKeyboardFocus() && sharesKeyboardFocus (*listener))
            listener->keyboardFocusGained();
        else
            peerFocusListener = nullptr;
    }
}

bool WaylandInputHandler::sharesKeyboardFocus (const WaylandInputHandlerListener& listener) const
{
    auto* toplevel = listener.getToplevelSurface();
    return nativeKeyboardFocus.has_value() && toplevel != nullptr
        && toplevel == nativeKeyboardFocus->listener.getToplevelSurface();
}

bool WaylandInputHandler::requestKeyboardFocus (wl_surface* surface)
{
    auto* listener = findListenerForSurface (surface);

    if (listener == nullptr || ! listener->canReceiveKeyboardFocus())
        return false;

    // A request for an inactive window takes effect when the compositor activates its toplevel.
    rememberKeyboardFocus (*listener);

    if (! sharesKeyboardFocus (*listener))
        return false;

    setPeerFocusListener (listener);
    return true;
}

bool WaylandInputHandler::hasKeyboardFocus (wl_surface* surface) const
{
    return peerFocusListener != nullptr && findListenerForSurface (surface) == peerFocusListener;
}

void WaylandInputHandler::releaseKeyboardFocus (wl_surface* surface)
{
    auto* listener = findListenerForSurface (surface);

    if (listener == nullptr)
        return;

    for (auto& entry : surfaceListeners)
        if (entry.surface == surface || entry.rememberedFocus == listener)
            entry.rememberedFocus = nullptr;

    if (peerFocusListener == nullptr
        || (peerFocusListener != listener && peerFocusListener->getToplevelSurface() != surface))
        return;

    setPeerFocusListener (std::invoke ([&]() -> WaylandInputHandlerListener*
    {
        auto* toplevel = findListenerForSurface (listener->getToplevelSurface());

        if (toplevel != nullptr && toplevel != listener
            && toplevel->canReceiveKeyboardFocus() && sharesKeyboardFocus (*toplevel))
            return toplevel;

        return nullptr;
    }));
}

void WaylandInputHandler::updatePressedKeys (uint32_t evdevKey, uint32_t sym, uint32_t baseSym, bool down)
{
    const auto sameKey = [evdevKey] (const PressedKey& pressed) { return pressed.evdevKey == evdevKey; };

    if (! down)
    {
        pressedKeys.erase (std::remove_if (pressedKeys.begin(), pressedKeys.end(), sameKey), pressedKeys.end());
        return;
    }

    if (std::none_of (pressedKeys.begin(), pressedKeys.end(), sameKey))
        pressedKeys.push_back ({ evdevKey, sym, baseSym });
}

void WaylandInputHandler::onKey (uint32_t serial, uint32_t, uint32_t key, uint32_t keyState)
{
    if (! xkb.hasState())
        return;

    const auto xkbKeycode = evdevToXkbKeycode (key);
    const auto sym = xkb.keyGetOneSym (xkbKeycode);

    const bool repeated = keyState == WaylandProtocol::wlKeyboardKeyStateRepeated;
    const bool down = keyState == WaylandProtocol::wlKeyboardKeyStatePressed || repeated;

    if (down)
        reportInputSerial (nativeKeyboardFocus.has_value() ? nativeKeyboardFocus->surface : nullptr, serial);

    const ScopedValueSetter eventScope { currentInputEvent, InputEvent {
        findSurfaceForListener (findKeyEventTarget()),
        makePopupTrigger (nativeKeyboardFocus.has_value() ? nativeKeyboardFocus->surface : nullptr,
                          down ? std::optional { serial } : std::nullopt)
    } };

    if (down && ! repeated)
        retainCurrentPressForPopup (PressDevice::keyboard);

    // Key state is tracked for every event so the global query stays correct for ignore-key peers.
    updatePressedKeys (key, sym, xkb.keyGetBaseSym (xkbKeycode), down);

    auto* keyEventTarget = findKeyEventTarget();

    if (keyEventTarget == nullptr)
        return;

    // Modifier presses are reflected by the modifiers event, not a key-state change. Repeats
    // deliver key presses only, matching the local repeat timer.
    if (! isModifierKeysym (sym) && ! repeated)
        keyEventTarget->keyStateChanged (down);

    if (down)
    {
        const auto utf32 = xkb.keyGetUtf32 (xkbKeycode);
        const auto unicodeChar = (juce_wchar) utf32;
        auto keyCode = (int) unicodeChar;

        if (keyCode < 0x20)
            keyCode = (int) sym;

        const auto translation = KeySymTranslation::translateKeySymToKeyPress (sym, keyCode, utf32 != 0);

        // Application code run by keyStateChanged() may remove the key-event target.
        if (translation.keyPressed)
            if (auto* target = findKeyEventTarget())
                target->keyPressed (translation.keyCode, unicodeChar);

        // Stop if a compositor sends repeated states after announcing a repeat rate of 0.
        if (repeated)
            stopKeyRepeat();
        else
            startKeyRepeat (key, translation.keyCode, unicodeChar, translation.keyPressed);
    }
    else if (repeatKey.has_value() && key == repeatKey->evdevKey)
    {
        stopKeyRepeat();
    }
}

void WaylandInputHandler::onModifiers (uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group)
{
    if (! xkb.hasState())
        return;

    xkb.updateMask (depressed, latched, locked, group);

    for (auto& key : pressedKeys)
    {
        const auto xkbKeycode = evdevToXkbKeycode (key.evdevKey);
        key.sym = xkb.keyGetOneSym (xkbKeycode);
        key.baseSym = xkb.keyGetBaseSym (xkbKeycode);
    }

    const auto oldMods = ModifierKeys::getCurrentModifiers();
    ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withOnlyMouseButtons()
                                                                        .withFlags (xkb.currentModifierFlags());

    if (oldMods != ModifierKeys::getCurrentModifiers())
        if (auto* target = findKeyEventTarget())
            target->modifierKeysChanged();
}

void WaylandInputHandler::onRepeatInfo (int rate, int delay)
{
    keyRepeatRate = rate;
    keyRepeatDelayMs = delay;

    // A rate of 0 disables repeating and can arrive while a repeat is running.
    if (rate <= 0)
        stopKeyRepeat();
}

//==============================================================================
void WaylandInputHandler::startKeyRepeat (uint32_t evdevKey, int keyCode, juce_wchar unicodeChar, bool keyPressed)
{
    // A press that produced no KeyPress leaves any active repeat running, matching X11.
    if (! keyPressed)
        return;

    stopKeyRepeat();

    // A rate of 0 disables repeat, and only keymap-repeatable keys auto-repeat.
    if (keyRepeatRate <= 0 || ! xkb.keymapKeyRepeats (evdevToXkbKeycode (evdevKey)))
        return;

    repeatKey = RepeatKey { evdevKey, keyCode, unicodeChar };
    repeatIntervalMs = jmax (1, 1000 / keyRepeatRate);
    startTimer (jmax (1, keyRepeatDelayMs));
}

void WaylandInputHandler::stopKeyRepeat()
{
    stopTimer();
    repeatKey.reset();
}

void WaylandInputHandler::timerCallback()
{
    startTimer (repeatIntervalMs);

    if (auto* target = findKeyEventTarget(); target != nullptr && repeatKey.has_value())
        target->keyPressed (repeatKey->keyCode, repeatKey->character);
    else
        stopKeyRepeat();
}

//==============================================================================
WaylandInputHandlerListener* WaylandInputHandler::findListenerForSurface (wl_surface* surface) const
{
    if (surface == nullptr)
        return nullptr;

    for (const auto& surfaceListener : surfaceListeners)
        if (surfaceListener.surface == surface)
            return surfaceListener.listener;

    return nullptr;
}

wl_surface* WaylandInputHandler::findSurfaceForListener (const WaylandInputHandlerListener* listener) const
{
    for (const auto& entry : surfaceListeners)
        if (entry.listener == listener)
            return entry.surface;

    return nullptr;
}

WaylandInputHandlerListener* WaylandInputHandler::findKeyEventTarget() const
{
    auto* target = nativeKeyboardFocus.has_value() ? &nativeKeyboardFocus->listener : nullptr;

    if (target != nullptr)
        for (const auto& entry : surfaceListeners)
            if (entry.surface == target->getToplevelSurface()
                && entry.rememberedFocus != nullptr
                && entry.rememberedFocus->canReceiveKeyboardFocus()
                && sharesKeyboardFocus (*entry.rememberedFocus))
                return entry.rememberedFocus;

    return resolveNativeKeyEventTarget();
}

WaylandInputHandlerListener* WaylandInputHandler::resolveNativeKeyEventTarget() const
{
    return resolveKeyEventTarget (nativeKeyboardFocus.has_value() ? &nativeKeyboardFocus->listener : nullptr,
                                  surfaceListeners.size(),
                                  [this] (WaylandInputHandlerListener& listener)
                                  {
                                      return findListenerForSurface (listener.getKeyEventFallbackSurface());
                                  });
}

bool WaylandInputHandler::hasTouchTarget (wl_surface* surface)
{
    return findListenerForSurface (surface) != nullptr;
}

// Resolving per event keeps a queued frame safe when a nested event loop destroys its
// peer: the peer unregisters, so the frame's remaining events fall through here.
void WaylandInputHandler::dispatchTouchEvent (wl_surface* surface, int touchIndex, Point<float> position,
                                              ModifierKeys mods, uint32_t time,
                                              std::optional<uint32_t> popupGrabSerial)
{
    if (auto* listener = findListenerForSurface (surface))
    {
        if (popupGrabSerial.has_value())
        {
            dragState = DragCandidate { surface, *popupGrabSerial, {}, {} };
            reportInputSerial (surface, *popupGrabSerial);
        }
        else if (! touchQueue.hasTouchesDown())
        {
            if (const auto* candidate = std::get_if<DragCandidate> (&dragState);
                candidate != nullptr && candidate->surface == surface)
                dragState = std::monostate{};
        }

        const ScopedValueSetter eventScope { currentInputEvent, InputEvent {
            surface, makePopupTrigger (surface, popupGrabSerial)
        } };

        if (popupGrabSerial.has_value())
            retainCurrentPressForPopup (PressDevice::touch);

        LinuxInputState::get().pointerEntered (LinuxInputBackend::wayland);
        ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons()
                                                                            .withFlags (touchQueue.hasTouchesDown() ? ModifierKeys::leftButtonModifier : 0);

        listener->touchEvent (touchIndex, position, mods, getEventTimeMs (time));
    }
}

//==============================================================================
const wl_pointer_listener WaylandInputHandler::pointerListener
{
    [] (void* data, wl_pointer*, uint32_t serial, wl_surface* surface, int32_t surfaceX, int32_t surfaceY)
    {
        static_cast<WaylandInputHandler*> (data)->onPointerEnter (serial, surface, surfaceX, surfaceY);
    },
    [] (void* data, wl_pointer*, uint32_t, wl_surface*)
    {
        static_cast<WaylandInputHandler*> (data)->onPointerLeave();
    },
    [] (void* data, wl_pointer*, uint32_t time, int32_t surfaceX, int32_t surfaceY)
    {
        static_cast<WaylandInputHandler*> (data)->onPointerMotion (time, surfaceX, surfaceY);
    },
    [] (void* data, wl_pointer*, uint32_t serial, uint32_t time, uint32_t button, uint32_t buttonState)
    {
        static_cast<WaylandInputHandler*> (data)->onPointerButton (serial, time, button, buttonState);
    },
    [] (void* data, wl_pointer*, uint32_t time, uint32_t axis, int32_t value)
    {
        static_cast<WaylandInputHandler*> (data)->onPointerAxis (time, axis, value);
    },
    [] (void* data, wl_pointer*)
    {
        static_cast<WaylandInputHandler*> (data)->onPointerFrame();
    },
    [] (void* data, wl_pointer*, uint32_t source)
    {
        static_cast<WaylandInputHandler*> (data)->axisState.setAxisSource (source);
    },
    [] (void*, wl_pointer*, uint32_t, uint32_t) {},
    [] (void* data, wl_pointer*, uint32_t axis, int32_t discrete)
    {
        static_cast<WaylandInputHandler*> (data)->axisState.addDiscreteAxisSteps (axis, discrete);
    },
    [] (void* data, wl_pointer*, uint32_t axis, int32_t value120)
    {
        static_cast<WaylandInputHandler*> (data)->axisState.addAxisValue120 (axis, value120);
    },
    [] (void* data, wl_pointer*, uint32_t axis, uint32_t direction)
    {
        static_cast<WaylandInputHandler*> (data)->axisState.setAxisRelativeDirection (axis, direction);
    }
};

const wl_callback_listener WaylandInputHandler::focusLossSyncListener
{
    [] (void* data, wl_callback* callback, uint32_t)
    {
        static_cast<WaylandInputHandler*> (data)->handleFocusLossSyncDone (callback);
    }
};

const wl_keyboard_listener WaylandInputHandler::keyboardListener
{
    [] (void* data, wl_keyboard*, uint32_t format, int32_t fd, uint32_t size)
    {
        static_cast<WaylandInputHandler*> (data)->onKeymap (format, fd, size);
    },
    [] (void* data, wl_keyboard*, uint32_t, wl_surface* surface, wl_array* keys)
    {
        const Span heldKeys { static_cast<const uint32_t*> (keys->data), keys->size / sizeof (uint32_t) };
        static_cast<WaylandInputHandler*> (data)->onKeyboardEnter (surface, heldKeys);
    },
    [] (void* data, wl_keyboard*, uint32_t, wl_surface* surface)
    {
        static_cast<WaylandInputHandler*> (data)->onKeyboardLeave (surface);
    },
    [] (void* data, wl_keyboard*, uint32_t serial, uint32_t time, uint32_t key, uint32_t keyState)
    {
        static_cast<WaylandInputHandler*> (data)->onKey (serial, time, key, keyState);
    },
    [] (void* data, wl_keyboard*, uint32_t, uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group)
    {
        static_cast<WaylandInputHandler*> (data)->onModifiers (depressed, latched, locked, group);
    },
    [] (void* data, wl_keyboard*, int32_t rate, int32_t delay)
    {
        static_cast<WaylandInputHandler*> (data)->onRepeatInfo (rate, delay);
    }
};

const wl_touch_listener WaylandInputHandler::touchListener
{
    [] (void* data, wl_touch*, uint32_t serial, uint32_t time, wl_surface* surface, int32_t id, int32_t x, int32_t y)
    {
        static_cast<WaylandInputHandler*> (data)->touchQueue.touchDown (time, surface, id, x, y, serial);
    },
    [] (void* data, wl_touch*, uint32_t, uint32_t time, int32_t id)
    {
        static_cast<WaylandInputHandler*> (data)->touchQueue.touchUp (time, id);
    },
    [] (void* data, wl_touch*, uint32_t time, int32_t id, int32_t x, int32_t y)
    {
        static_cast<WaylandInputHandler*> (data)->touchQueue.touchMotion (time, id, x, y);
    },
    [] (void* data, wl_touch*)
    {
        static_cast<WaylandInputHandler*> (data)->touchQueue.frameComplete();
    },
    [] (void* data, wl_touch*)
    {
        // The compositor has taken over the touch session (e.g. recognised a gesture).
        static_cast<WaylandInputHandler*> (data)->touchQueue.cancel();
    },
    // JUCE backend doesn't consume shape/orientation geometry.
    [] (void*, wl_touch*, int32_t, int32_t, int32_t) {},
    [] (void*, wl_touch*, int32_t, int32_t) {}
};

//==============================================================================
#if JUCE_UNIT_TESTS

class ResolveKeyEventTargetTests final : public UnitTest
{
public:
    ResolveKeyEventTargetTests()
        : UnitTest ("resolveKeyEventTarget", UnitTestCategories::gui) {}

    void runTest() override
    {
        struct Listener
        {
            bool canReceiveKeyboardFocus() const { return ! ignoresKeys; }

            Listener* parent = nullptr;
            bool ignoresKeys = false;
        };

        const auto getParentListener = [] (Listener& listener) { return listener.parent; };

        testCase ("Keys for a popup that ignores key presses are routed to its parent", [&]
        {
            Listener parent;
            Listener popup { &parent, true };

            expect (resolveKeyEventTarget (&popup, 2, getParentListener) == &parent);
        });

        testCase ("Nested popups that ignore key presses route keys to the first parent that accepts them", [&]
        {
            Listener parent;
            Listener popup { &parent, true };
            Listener child { &popup, true };

            expect (resolveKeyEventTarget (&child, 3, getParentListener) == &parent);
        });

        testCase ("Keys are discarded when no window in the parent chain accepts them", [&]
        {
            Listener popup { nullptr, true };

            expect (resolveKeyEventTarget (&popup, 1, getParentListener) == nullptr);
        });

        testCase ("A cycle in the parent chain causes the key event to be discarded", [&]
        {
            Listener popup { nullptr, true };
            popup.parent = &popup;

            expect (resolveKeyEventTarget (&popup, 1, getParentListener) == nullptr);
        });
    }
};

static ResolveKeyEventTargetTests resolveKeyEventTargetTests;

//==============================================================================
class WaylandPopupGrabSerialTests final : public UnitTest
{
public:
    WaylandPopupGrabSerialTests()
        : UnitTest ("WaylandPopupGrabSerial", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("A popup grab serial can only be claimed once", [&]
        {
            WaylandPopupGrabSerial serial { 42 };

            expect (serial.isAvailable());
            expect (serial.claim() == std::optional<uint32_t> { 42 });
            expect (! serial.isAvailable());
            expect (serial.claim() == std::nullopt);
        });
    }
};

static WaylandPopupGrabSerialTests waylandPopupGrabSerialTests;

struct TestInputDelegate final : WaylandInputHandlerDelegate
{
    void inputSerialReceived (uint32_t serial) override { latestInputSerial = serial; }
    wl_display* getDisplay() const override { return nullptr; }

    std::optional<uint32_t> latestInputSerial;
};

struct TestInputListener final : WaylandInputHandlerListener
{
    void keyboardFocusGained() override { ++focusGains; }
    void keyboardFocusLost() override
    {
        ++focusLosses;
        NullCheckedInvocation::invoke (onFocusLost);
    }
    void modifierKeysChanged() override { NullCheckedInvocation::invoke (onModifierKeysChanged); }
    void pointerEntered() override {}
    void pointerMoved (Point<float>, int64) override { NullCheckedInvocation::invoke (onPointerMoved); }
    void pointerButton (Point<float>, bool pressed, int64) override
    {
        ++pointerButtonCalls;
        lastPointerButtonWasPressed = pressed;
        NullCheckedInvocation::invoke (onPointerButton);
    }
    void pointerWheel (Point<float>, const MouseWheelDetails&, int64) override { NullCheckedInvocation::invoke (onPointerWheel); }
    Point<float> convertPointerPositionToGlobal (Point<float> position) override { return position; }
    int getPointerCursorScale() const override { return 1; }
    void touchEvent (int, Point<float>, ModifierKeys, int64) override {}
    bool canReceiveKeyboardFocus() const override { return visible && ! ignoresKeys; }
    wl_surface* getKeyEventFallbackSurface() const override { return keyEventFallbackSurface; }
    void keyStateChanged (bool) override { ++keyStateChanges; }
    void keyPressed (int, juce_wchar) override { ++keyPresses; }
    wl_surface* getToplevelSurface() const override { return toplevelSurface; }
    std::optional<WaylandPopupParentCandidate> getPopupParentCandidate() const override { return candidate; }
    void popupGrabStarted (Component&) override {}
    void popupGrabEnded() override {}
    void dismissPopupForAncestorUnmap() override {}

    wl_surface* toplevelSurface = nullptr;
    wl_surface* keyEventFallbackSurface = nullptr;
    bool visible = true, ignoresKeys = false;
    int focusGains = 0, focusLosses = 0;
    std::function<void()> onFocusLost, onModifierKeysChanged;
    std::optional<WaylandPopupParentCandidate> candidate;
    std::function<void()> onPointerButton, onPointerMoved, onPointerWheel;
    int keyStateChanges = 0, keyPresses = 0;
    int pointerButtonCalls = 0;
    bool lastPointerButtonWasPressed = false;
};

namespace
{
    template <typename Type>
    Type* fakeHandle (uintptr_t value)
    {
        return reinterpret_cast<Type*> (value);
    }
}

//==============================================================================
class WaylandInputHandlerDragTests final : public UnitTest
{
public:
    WaylandInputHandlerDragTests()
        : UnitTest ("WaylandInputHandler drag", UnitTestCategories::gui) {}

    void runTest() override
    {
        ScopedJuceInitialiser_GUI libraryInitialiser;

        testCase ("Updating X11 modifiers does not interrupt a Wayland mouse drag", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onKeyboardEnter (surface, {});
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);

            expect (! LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::x11));
            expect (getLinuxRealtimeModifiers (LinuxInputBackend::wayland).isLeftButtonDown());

            auto motionReceived = false;
            listener.onPointerMoved = [&]
            {
                motionReceived = true;
                expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());
            };
            handler.onPointerMotion (20, 256, 256);
            expect (motionReceived);

            handler.onPointerButton (3, 30, btnLeft, WaylandProtocol::wlPointerButtonStateReleased);
            expect (! getLinuxRealtimeModifiers (LinuxInputBackend::wayland).isAnyMouseButtonDown());
            expectEquals (listener.pointerButtonCalls, 2);
        });

        testCase ("Unmapping a registered surface after pointer leave keeps X11 input enabled", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.onPointerLeave();
            handler.surfaceUnmapped (surface);

            expect (handler.findListenerForSurface (surface) == &listener);
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
            expect (! handler.getHeldPressSerial (surface).has_value());

            LinuxInputState::get().pointerEntered (LinuxInputBackend::x11);
            expect (LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::x11));
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
        });

        testCase ("A subsurface's held mouse button is cleared only when its own toplevel unmaps", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            auto* toplevel = fakeHandle<wl_surface> (2);
            auto* otherToplevel = fakeHandle<wl_surface> (3);
            listener.toplevelSurface = toplevel;
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);

            handler.surfaceUnmapped (otherToplevel);
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());
            expect (! LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::x11));

            handler.surfaceUnmapped (toplevel);
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
            expect (LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::x11));
        });

        testCase ("Leaving a Wayland surface without a popup or external drag clears its held mouse button", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.onPointerLeave();

            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
        });

        testCase ("A pointer leave during a popup grab preserves the held mouse button", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            Component dismissTarget;
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.popupGrabStarted (surface, dismissTarget);
            handler.onPointerLeave();

            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());
            expect (! LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::x11));
            handler.popupGrabEnded (surface);
        });

        testCase ("A pointer leave during an external drag preserves its cursor target and held mouse button", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.externalDragStarted (surface);
            handler.onPointerLeave();

            expect (handler.getExternalDragCursorTarget().has_value());
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());
        });

        testCase ("A touch contact holds the shared mouse button state for realtime queries", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);

            handler.touchQueue.touchDown (1, surface, 0, 0, 0);
            handler.touchQueue.frameComplete();
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());
            expect (getLinuxRealtimeModifiers (LinuxInputBackend::wayland).isLeftButtonDown());
            expect (! LinuxInputState::get().mayUpdatePointerState (LinuxInputBackend::x11));

            handler.touchQueue.touchMotion (2, 0, 256, 256);
            handler.touchQueue.frameComplete();
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());

            handler.touchQueue.touchUp (3, 0);
            handler.touchQueue.frameComplete();
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
            expect (! getLinuxRealtimeModifiers (LinuxInputBackend::wayland).isAnyMouseButtonDown());
        });

        testCase ("The shared mouse button state is held until the last touch contact lifts", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);

            handler.touchQueue.touchDown (1, surface, 0, 0, 0);
            handler.touchQueue.touchDown (1, surface, 1, 256, 256);
            handler.touchQueue.frameComplete();
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());

            handler.touchQueue.touchUp (2, 0);
            handler.touchQueue.frameComplete();
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());

            // A down and an up in the same frame leave the other contact's button state alone.
            handler.touchQueue.touchDown (3, surface, 2, 0, 0);
            handler.touchQueue.touchUp (3, 2);
            handler.touchQueue.frameComplete();
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());

            handler.touchQueue.touchUp (4, 1);
            handler.touchQueue.frameComplete();
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
        });

        testCase ("The latest input serial keeps its source surface", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };

            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);

            const auto latestInputSerial = handler.getLatestInputSerial();
            expect (latestInputSerial.has_value()
                    && latestInputSerial->sourceSurface == surface
                    && latestInputSerial->value == 2);
            expect (delegate.latestInputSerial == std::optional<uint32_t> { 2 });

            handler.removeListener (listener);
            expect (! handler.getLatestInputSerial().has_value());
        });

        testCase ("Ending an external drag sends a release if the compositor did not deliver one", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };

            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);

            expectEquals (listener.pointerButtonCalls, 1);
            expect (listener.lastPointerButtonWasPressed);
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());

            handler.externalDragStarted (surface);

            // The compositor's drag grab may prevent wl_pointer from reporting the physical release.
            handler.externalDragEnded();

            expectEquals (listener.pointerButtonCalls, 2);
            expect (! listener.lastPointerButtonWasPressed);
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());

            // A successful drop may report both drop_performed and dnd_finished.
            handler.externalDragEnded();
            expectEquals (listener.pointerButtonCalls, 2);
        });

        testCase ("Ending an external drag does not repeat a release delivered by the compositor", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };

            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.externalDragStarted (surface);
            handler.onPointerButton (3, 20, btnLeft, WaylandProtocol::wlPointerButtonStateReleased);

            expectEquals (listener.pointerButtonCalls, 2);
            expect (! listener.lastPointerButtonWasPressed);

            const auto pointerButtonCallsBeforeDragEnd = listener.pointerButtonCalls;
            handler.externalDragEnded();
            expectEquals (listener.pointerButtonCalls, pointerButtonCallsBeforeDragEnd);
        });

        testCase ("A reused surface does not receive the previous listener's drag release", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };

            TestInputDelegate delegate;
            TestInputListener previousListener;
            TestInputListener replacementListener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, previousListener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.externalDragStarted (surface);

            handler.removeListener (previousListener);
            handler.addListener (surface, replacementListener);
            handler.externalDragEnded();

            expectEquals (replacementListener.pointerButtonCalls, 0);
        });

        testCase ("Handing a pointer drag to the compositor clears the held press", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };

            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            expect (handler.getHeldPressSerial (surface) == std::optional<uint32_t> { 2 });

            handler.dragHandedToCompositor (surface);

            expectEquals (listener.pointerButtonCalls, 1);
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
            expect (handler.getHeldPressSerial (surface) == std::nullopt);

            handler.onPointerLeave();
            handler.onPointerEnter (3, surface, 0, 0);
            handler.onPointerButton (4, 20, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            expect (handler.getHeldPressSerial (surface) == std::optional<uint32_t> { 4 });
        });
    }
};

static WaylandInputHandlerDragTests waylandInputHandlerDragTests;

//==============================================================================
class WaylandInputHandlerFocusTests final : public UnitTest
{
public:
    WaylandInputHandlerFocusTests()
        : UnitTest ("Wayland keyboard focus", UnitTestCategories::gui) {}

    void runTest() override
    {
        struct TestFocusContext
        {
            TestFocusContext()
            {
                popup.ignoresKeys = true;
                popup.keyEventFallbackSurface = windowSurface;
                window.toplevelSurface = popup.toplevelSurface = temporary.toplevelSurface = windowSurface;
                otherWindow.toplevelSurface = otherTemporary.toplevelSurface = otherWindowSurface;
                handler.addListener (windowSurface, window);
                handler.addListener (popupSurface, popup);
                handler.addListener (otherWindowSurface, otherWindow);
                handler.addListener (temporarySurface, temporary);
                handler.addListener (otherTemporarySurface, otherTemporary);
            }

            wl_surface* windowSurface = fakeHandle<wl_surface> (1);
            wl_surface* popupSurface = fakeHandle<wl_surface> (2);
            wl_surface* otherWindowSurface = fakeHandle<wl_surface> (3);
            wl_surface* temporarySurface = fakeHandle<wl_surface> (4);
            wl_surface* otherTemporarySurface = fakeHandle<wl_surface> (5);
            TestInputDelegate delegate;
            TestInputListener window, popup, otherWindow, temporary, otherTemporary;
            WaylandInputHandler handler { delegate };
        };

        ScopedJuceInitialiser_GUI guiInitialiser;

        // These tests call updatePeerFocus() directly because the delegate has no compositor connection.
        testCase ("A menu preserves a temporary window's focus even when its toplevel ignores keys", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.window.ignoresKeys = true;

            context.handler.onKeyboardEnter (context.temporarySurface, {});
            context.handler.onKeyboardLeave (context.temporarySurface);
            context.handler.onKeyboardEnter (context.popupSurface, {});

            expect (context.handler.hasKeyboardFocus (context.temporarySurface));
            expect (context.handler.findKeyEventTarget() == &context.temporary);
            expectEquals (context.temporary.focusGains, 1);
            expectEquals (context.temporary.focusLosses, 0);
            expectEquals (context.popup.focusGains, 0);
        });

        testCase ("Removing the focused listener clears peer focus without calling that listener", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.handler.removeListener (context.window);
            context.handler.updatePeerFocus();
            expectEquals (context.window.focusLosses, 0);
            expect (! context.handler.hasKeyboardFocus());
        });

        testCase ("A temporary window keeps keyboard focus while native focus moves to a menu and back", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});

            expect (context.handler.requestKeyboardFocus (context.temporarySurface));
            expect (context.handler.hasKeyboardFocus (context.temporarySurface));
            expect (context.handler.findKeyEventTarget() == &context.temporary);
            expectEquals (context.window.focusLosses, 1);
            expectEquals (context.temporary.focusGains, 1);

            context.handler.onKeyboardLeave (context.windowSurface);
            context.handler.onKeyboardEnter (context.popupSurface, {});
            context.handler.updatePeerFocus();
            expect (context.handler.findKeyEventTarget() == &context.temporary);
            expectEquals (context.temporary.focusLosses, 0);
            expectEquals (context.popup.focusGains, 0);

            context.handler.onKeyboardLeave (context.popupSurface);
            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.handler.updatePeerFocus();
            expect (context.handler.findKeyEventTarget() == &context.temporary);
            expectEquals (context.temporary.focusGains, 1);
            expectEquals (context.temporary.focusLosses, 0);

            context.handler.releaseKeyboardFocus (context.temporarySurface);
            expect (context.handler.findKeyEventTarget() == &context.window);
            expectEquals (context.temporary.focusLosses, 1);
        });

        testCase ("A focus request for a window that ignores keys leaves the current key target unchanged", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (! context.handler.requestKeyboardFocus (context.popupSurface));
            expect (context.handler.findKeyEventTarget() == &context.window);
        });

        testCase ("Updating focus after removing the selected temporary window restores its toplevel", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.requestKeyboardFocus (context.temporarySurface));
            context.handler.removeListener (context.temporary);
            expectEquals (context.temporary.focusLosses, 0);
            context.handler.updatePeerFocus();
            expect (context.handler.hasKeyboardFocus (context.windowSurface));
            expectEquals (context.window.focusGains, 2);
        });

        testCase ("A keyboard leave for a destroyed menu reports peer focus loss", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.handler.onKeyboardLeave (context.windowSurface);
            context.handler.onKeyboardEnter (context.popupSurface, {});
            context.handler.removeListener (context.popup);
            context.handler.onKeyboardLeave (nullptr);
            context.handler.updatePeerFocus();
            expectEquals (context.window.focusLosses, 1);
            expect (! context.handler.hasKeyboardFocus (context.windowSurface));
        });

        testCase ("Leaving and returning to a toplevel restores its temporary window's keyboard focus", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.requestKeyboardFocus (context.temporarySurface));
            context.handler.onKeyboardLeave (context.windowSurface);
            expectEquals (context.temporary.focusLosses, 0);
            expect (context.handler.hasKeyboardFocus (context.temporarySurface));

            context.handler.updatePeerFocus();
            expectEquals (context.temporary.focusLosses, 1);
            expect (! context.handler.hasKeyboardFocus());
            expect (context.handler.findKeyEventTarget() == nullptr);
            context.handler.updatePeerFocus();

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.hasKeyboardFocus (context.temporarySurface));
            expectEquals (context.temporary.focusGains, 2);
            expectEquals (context.temporary.focusLosses, 1);
            expectEquals (context.window.focusGains, 1);
        });

        testCase ("Each toplevel restores its own selected temporary window", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.requestKeyboardFocus (context.temporarySurface));
            context.handler.onKeyboardLeave (context.windowSurface);
            context.handler.onKeyboardEnter (context.otherWindowSurface, {});
            expectEquals (context.temporary.focusLosses, 1);
            expectEquals (context.otherWindow.focusGains, 1);
            expect (context.handler.requestKeyboardFocus (context.otherTemporarySurface));
            context.handler.onKeyboardLeave (context.otherWindowSurface);
            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.hasKeyboardFocus (context.temporarySurface));
            context.handler.onKeyboardLeave (context.windowSurface);
            context.handler.onKeyboardEnter (context.otherWindowSurface, {});
            expect (context.handler.hasKeyboardFocus (context.otherTemporarySurface));
            expectEquals (context.temporary.focusGains, 2);
            expectEquals (context.otherTemporary.focusGains, 2);
        });

        testCase ("A focus request for an inactive temporary window takes effect when its toplevel activates", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.otherWindowSurface, {});
            expect (! context.handler.requestKeyboardFocus (context.temporarySurface));
            expect (context.handler.hasKeyboardFocus (context.otherWindowSurface));
            expectEquals (context.temporary.focusGains, 0);
            context.handler.onKeyboardLeave (context.otherWindowSurface);
            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.hasKeyboardFocus (context.temporarySurface));
            expectEquals (context.temporary.focusGains, 1);
        });

        testCase ("Removing an inactive temporary window clears its remembered focus", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.requestKeyboardFocus (context.temporarySurface));
            context.handler.onKeyboardLeave (context.windowSurface);
            context.handler.updatePeerFocus();
            context.handler.removeListener (context.temporary);

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.hasKeyboardFocus (context.windowSurface));
            expectEquals (context.temporary.focusGains, 1);
        });

        testCase ("Releasing focus from an inactive temporary window clears its remembered focus", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.requestKeyboardFocus (context.temporarySurface));
            context.handler.onKeyboardLeave (context.windowSurface);
            context.handler.updatePeerFocus();
            context.handler.releaseKeyboardFocus (context.temporarySurface);

            context.handler.onKeyboardEnter (context.windowSurface, {});
            expect (context.handler.hasKeyboardFocus (context.windowSurface));
            expectEquals (context.temporary.focusGains, 1);
        });

        testCase ("A hidden temporary window cannot regain focus while the compositor still focuses its surface", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.temporary.keyEventFallbackSurface = context.windowSurface;
            context.handler.onKeyboardEnter (context.temporarySurface, {});
            context.temporary.visible = false;
            context.handler.releaseKeyboardFocus (context.temporarySurface);
            context.handler.updatePeerFocus();
            expect (context.handler.hasKeyboardFocus (context.windowSurface));
            expectEquals (context.temporary.focusGains, 1);
            expectEquals (context.temporary.focusLosses, 1);
        });

        testCase ("Removing the requested peer during focus loss prevents its gain callback", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.window.onFocusLost = [&] { context.handler.removeListener (context.temporary); };
            context.handler.requestKeyboardFocus (context.temporarySurface);
            expectEquals (context.temporary.focusGains, 0);
            context.handler.updatePeerFocus();
            expect (context.handler.hasKeyboardFocus (context.windowSurface));
        });

        testCase ("Removing the owning window during focus loss prevents the temporary window's gain callback", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.window.onFocusLost = [&] { context.handler.removeListener (context.window); };
            context.handler.requestKeyboardFocus (context.temporarySurface);
            expectEquals (context.temporary.focusGains, 0);
            expect (! context.handler.hasKeyboardFocus());
        });

        testCase ("A nested focus request supersedes the target of the outer request", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.window.onFocusLost = [&] { context.handler.requestKeyboardFocus (context.windowSurface); };
            context.handler.requestKeyboardFocus (context.temporarySurface);
            expect (context.handler.hasKeyboardFocus (context.windowSurface));
            expectEquals (context.temporary.focusGains, 0);
        });

        testCase ("A keyboard enter during the modifier callback preserves peer focus", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys { ModifierKeys::shiftModifier } };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.window.onModifierKeysChanged = [&] { context.handler.onKeyboardEnter (context.popupSurface, {}); };

            context.handler.onKeyboardLeave (context.windowSurface);
            context.handler.updatePeerFocus();

            expect (context.handler.hasKeyboardFocus());
            expectEquals (context.window.focusLosses, 0);
            expectEquals (context.popup.focusGains, 0);
        });

        testCase ("A keyboard leave for a surface other than the focused one changes nothing", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestFocusContext context;

            context.handler.onKeyboardEnter (context.windowSurface, {});
            context.handler.onKeyboardLeave (context.otherWindowSurface);
            context.handler.updatePeerFocus();

            expect (context.handler.hasKeyboardFocus());
            expectEquals (context.window.focusLosses, 0);
        });
    }
};

static WaylandInputHandlerFocusTests waylandInputHandlerFocusTests;

//==============================================================================
class WaylandHeldKeyTests final : public UnitTest
{
public:
    WaylandHeldKeyTests()
        : UnitTest ("Wayland held keys", UnitTestCategories::gui) {}

    void runTest() override
    {
        ScopedJuceInitialiser_GUI guiInitialiser;

        auto* surface = fakeHandle<wl_surface> (1);
        constexpr uint32_t evdevKeyA = 30;
        const uint32_t heldKeys[] { evdevKeyA };
        const auto* keymap = R"(
            xkb_keymap {
                xkb_keycodes { <AC01> = 38; };
                xkb_types {};
                xkb_compatibility {};
                xkb_symbols { key <AC01> { [ a ], [ b ] }; };
            };
        )";

        const auto testKeyboard = [&] (const String& testName, auto&& test)
        {
            testCase (testName, [&]
            {
                const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
                const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
                TestInputDelegate delegate;
                TestInputListener listener;
                WaylandInputHandler handler { delegate };
                listener.toplevelSurface = surface;
                handler.addListener (surface, listener);
                handler.xkb.buildKeymap (keymap);
                expect (handler.xkb.hasState());
                test (handler, listener);
            });
        };

        testKeyboard ("Keyboard enter restores held keys without generating key events or starting repeat", [&] (auto& handler, auto& listener)
        {
            handler.onKeyboardEnter (surface, heldKeys);
            expect (handler.isKeyCurrentlyDown ('A'));
            expectEquals (listener.keyPresses, 0);
            expectEquals (listener.keyStateChanges, 0);
            expect (! handler.repeatKey.has_value());
        });

        testKeyboard ("The modifiers after keyboard enter update held keys to the active layout", [&] (auto& handler, auto& listener)
        {
            handler.onKeyboardEnter (surface, heldKeys);
            expect (handler.isKeyCurrentlyDown ('A'));
            handler.onModifiers (0, 0, 0, 1);
            expect (handler.isKeyCurrentlyDown ('B'));
            expect (! handler.isKeyCurrentlyDown ('A'));
            expectEquals (listener.keyPresses, 0);
            expectEquals (listener.keyStateChanges, 0);
        });

        testKeyboard ("A layout change updates keys held after an ordinary key press", [&] (auto& handler, auto& listener)
        {
            handler.onKeyboardEnter (surface, {});
            handler.onModifiers (0, 0, 0, 0);
            handler.onKey (1, 0, evdevKeyA, WaylandProtocol::wlKeyboardKeyStatePressed);
            expect (handler.isKeyCurrentlyDown ('A'));
            expectEquals (listener.keyPresses, 1);
            expectEquals (listener.keyStateChanges, 1);

            handler.onModifiers (0, 0, 0, 1);
            expect (handler.isKeyCurrentlyDown ('B'));
            expect (! handler.isKeyCurrentlyDown ('A'));
            expectEquals (listener.keyPresses, 1);
            expectEquals (listener.keyStateChanges, 1);
        });

        testKeyboard ("Keyboard enter restores held keys when the surface has no registered listener", [&] (auto& handler, auto& listener)
        {
            handler.removeListener (listener);
            handler.onKeyboardEnter (surface, heldKeys);
            expect (handler.isKeyCurrentlyDown ('A'));
            expect (! handler.hasKeyboardFocus());
            expectEquals (listener.keyPresses, 0);
            expectEquals (listener.keyStateChanges, 0);

            handler.onKey (1, 0, evdevKeyA, WaylandProtocol::wlKeyboardKeyStateReleased);
            expect (! handler.isKeyCurrentlyDown ('A'));
        });

        testKeyboard ("Releasing a key restored by keyboard enter removes it from the held keys", [&] (auto& handler, auto& listener)
        {
            handler.onKeyboardEnter (surface, heldKeys);
            expect (handler.isKeyCurrentlyDown ('A'));
            handler.onKey (1, 0, heldKeys[0], WaylandProtocol::wlKeyboardKeyStateReleased);
            expect (! handler.isKeyCurrentlyDown ('A'));
            expectEquals (listener.keyStateChanges, 1);
            expectEquals (listener.keyPresses, 0);
        });

        testKeyboard ("Keyboard leave clears held keys and a later enter restores only the keys still held", [&] (auto& handler, auto&)
        {
            handler.onKeyboardEnter (surface, heldKeys);
            expect (handler.isKeyCurrentlyDown ('A'));
            handler.onKeyboardLeave (nullptr);
            expect (! handler.isKeyCurrentlyDown ('A'));
            handler.onKeyboardEnter (surface, {});
            expect (! handler.isKeyCurrentlyDown ('A'));
            handler.onKeyboardLeave (nullptr);
            handler.onKeyboardEnter (surface, heldKeys);
            expect (handler.isKeyCurrentlyDown ('A'));
        });

        testKeyboard ("Keyboard leave keeps the modifier flags of an X11 window that took the keyboard", [&] (auto& handler, auto&)
        {
            handler.onKeyboardEnter (surface, {});
            ModifierKeys::currentModifiers = ModifierKeys { ModifierKeys::shiftModifier | ModifierKeys::leftButtonModifier };

            // The X11 focus can be processed before this leave, as the two arrive on separate connections.
            LinuxInputState::get().keyboardEntered (LinuxInputBackend::x11);
            handler.onKeyboardLeave (nullptr);
            expect (ModifierKeys::getCurrentModifiers().isShiftDown());

            handler.onKeyboardEnter (surface, {});
            handler.onKeyboardLeave (nullptr);
            expect (! ModifierKeys::getCurrentModifiers().isShiftDown());
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());
        });
    }
};

static WaylandHeldKeyTests waylandHeldKeyTests;

//==============================================================================
class WaylandPopupContextTests final : public UnitTest
{
public:
    WaylandPopupContextTests()
        : UnitTest ("WaylandPopupContext", UnitTestCategories::gui) {}

    void runTest() override
    {
        ScopedJuceInitialiser_GUI libraryInitialiser;

        testCase ("Removing the listener that holds a device forgets the device", [&]
        {
            TestInputDelegate delegate;
            TestInputListener window, other;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            auto* otherSurface = fakeHandle<wl_surface> (2);
            handler.addListener (surface, window);
            handler.addListener (otherSurface, other);
            handler.onKeyboardEnter (surface, {});
            handler.onPointerEnter (1, surface, 0, 0);

            auto& state = LinuxInputState::get();
            handler.removeListener (other);
            expect (state.getPointerBackend() == LinuxInputBackend::wayland);
            expect (state.getKeyboardBackend() == LinuxInputBackend::wayland);

            handler.removeListener (window);
            expect (! state.getPointerBackend().has_value());
            expect (! state.getKeyboardBackend().has_value());
        });

        testCase ("Keyboard focus is preferred to pointer focus when no input event or grab trigger is available", [&]
        {
            TestInputDelegate delegate;
            TestInputListener keyboardWindow, pointerWindow;
            WaylandInputHandler handler { delegate };
            auto* keyboardSurface = fakeHandle<wl_surface> (1);
            auto* pointerSurface = fakeHandle<wl_surface> (2);
            handler.addListener (keyboardSurface, keyboardWindow);
            handler.addListener (pointerSurface, pointerWindow);
            handler.onKeyboardEnter (keyboardSurface, {});
            handler.onPointerEnter (1, pointerSurface, 0, 0);
            const auto context = handler.getPopupContext();
            expect (context.pointerSurface == pointerSurface);
            expect (context.inputSurface == keyboardSurface);
            expect (! context.trigger.has_value());

            handler.onKeyboardLeave (nullptr);
            expect (handler.getPopupContext().inputSurface == pointerSurface);
        });

        testCase ("A pointer callback keeps its input surface after its grab serial is claimed", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener keyboardWindow, pointerWindow;
            WaylandInputHandler handler { delegate };
            auto* keyboardSurface = fakeHandle<wl_surface> (1);
            auto* pointerSurface = fakeHandle<wl_surface> (2);
            handler.addListener (keyboardSurface, keyboardWindow);
            handler.addListener (pointerSurface, pointerWindow);
            handler.onKeyboardEnter (keyboardSurface, {});
            handler.onPointerEnter (1, pointerSurface, 0, 0);
            pointerWindow.onPointerButton = [&]
            {
                const auto beforeClaim = handler.getPopupContext();
                expect (beforeClaim.trigger.has_value() && beforeClaim.trigger->surface == pointerSurface);

                if (beforeClaim.trigger.has_value())
                    expect (beforeClaim.trigger->grabSerial->claim() == std::optional<uint32_t> { 42 });

                const auto afterClaim = handler.getPopupContext();
                expect (afterClaim.inputSurface == pointerSurface);
                expect (! afterClaim.trigger.has_value());
            };
            handler.onPointerButton (42, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            expectEquals (pointerWindow.pointerButtonCalls, 1);
            expect (handler.getPopupContext().inputSurface == keyboardSurface);
        });

        testCase ("Pointer motion selects the pointer surface while another window has keyboard focus", [&]
        {
            TestInputDelegate delegate;
            TestInputListener keyboardWindow, pointerWindow;
            WaylandInputHandler handler { delegate };
            auto* keyboardSurface = fakeHandle<wl_surface> (1);
            auto* pointerSurface = fakeHandle<wl_surface> (2);
            handler.addListener (keyboardSurface, keyboardWindow);
            handler.addListener (pointerSurface, pointerWindow);
            handler.onKeyboardEnter (keyboardSurface, {});
            handler.onPointerEnter (1, pointerSurface, 0, 0);
            auto* inputSurface = keyboardSurface;
            pointerWindow.onPointerMoved = [&] { inputSurface = handler.getPopupContext().inputSurface; };
            handler.onPointerMotion (10, 0, 0);
            expect (inputSurface == pointerSurface);
            expect (handler.getPopupContext().inputSurface == keyboardSurface);
        });

        testCase ("A wheel callback selects the pointer surface while another window has keyboard focus", [&]
        {
            TestInputDelegate delegate;
            TestInputListener keyboardWindow, pointerWindow;
            WaylandInputHandler handler { delegate };
            auto* keyboardSurface = fakeHandle<wl_surface> (1);
            auto* pointerSurface = fakeHandle<wl_surface> (2);
            handler.addListener (keyboardSurface, keyboardWindow);
            handler.addListener (pointerSurface, pointerWindow);
            handler.onKeyboardEnter (keyboardSurface, {});
            handler.onPointerEnter (1, pointerSurface, 0, 0);
            auto* inputSurface = keyboardSurface;
            pointerWindow.onPointerWheel = [&] { inputSurface = handler.getPopupContext().inputSurface; };
            handler.onPointerAxis (10, WaylandProtocol::wlPointerAxisVerticalScroll, 256);
            handler.onPointerFrame();
            expect (inputSurface == pointerSurface);
            expect (handler.getPopupContext().inputSurface == keyboardSurface);
        });

        testCase ("A nested pointer callback restores the outer callback's input surface", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener outerWindow, nestedWindow;
            WaylandInputHandler handler { delegate };
            auto* outerSurface = fakeHandle<wl_surface> (1);
            auto* nestedSurface = fakeHandle<wl_surface> (2);
            handler.addListener (outerSurface, outerWindow);
            handler.addListener (nestedSurface, nestedWindow);
            handler.onPointerEnter (1, outerSurface, 0, 0);
            auto* nestedInput = outerSurface;
            nestedWindow.onPointerMoved = [&] { nestedInput = handler.getPopupContext().inputSurface; };
            outerWindow.onPointerButton = [&]
            {
                handler.onPointerEnter (2, nestedSurface, 0, 0);
                expect (handler.getPopupContext().inputSurface == outerSurface);
            };
            handler.onPointerButton (42, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            expectEquals (outerWindow.pointerButtonCalls, 1);
            expect (nestedInput == nestedSurface);
        });

        testCase ("Losing pointer capability invalidates a retained popup grab serial", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.onPointerButton (3, 20, btnLeft, WaylandProtocol::wlPointerButtonStateReleased);
            const auto trigger = handler.getPopupContext().trigger;
            expect (trigger.has_value());

            handler.unbindPointer();
            expect (! handler.getPopupContext().trigger.has_value());

            if (trigger.has_value())
                expect (! trigger->grabSerial->isAvailable());
        });

        testCase ("Losing pointer capability clears the popup grab serial before reporting a drag release", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);
            handler.externalDragStarted (surface);
            listener.onPointerButton = [&] { expect (! handler.getPopupContext().trigger.has_value()); };

            handler.unbindPointer();
            expectEquals (listener.pointerButtonCalls, 2);
        });

        testCase ("Losing keyboard capability preserves a pointer serial for a popup grab", [&]
        {
            const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
            const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
            TestInputDelegate delegate;
            TestInputListener listener;
            WaylandInputHandler handler { delegate };
            auto* surface = fakeHandle<wl_surface> (1);
            handler.addListener (surface, listener);
            handler.onPointerEnter (1, surface, 0, 0);
            handler.onPointerButton (2, 10, btnLeft, WaylandProtocol::wlPointerButtonStatePressed);

            handler.unbindKeyboard();
            const auto trigger = handler.getPopupContext().trigger;
            expect (trigger.has_value() && trigger->grabSerial->claim() == std::optional<uint32_t> { 2 });
        });
    }
};

static WaylandPopupContextTests waylandPopupContextTests;

//==============================================================================
class WaylandTouchQueueTests final : public UnitTest
{
public:
    WaylandTouchQueueTests()
        : UnitTest ("WaylandTouchQueue", UnitTestCategories::gui) {}

    void runTest() override
    {
        // The queue never dereferences surfaces.
        auto* surfaceA = fakeSurface (1);
        auto* surfaceB = fakeSurface (2);

        testCase ("Touch events wait for the frame event that completes their group", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            // Queue calls use wl_touch arguments. Down events include a surface, and
            // coordinates use the protocol's 24.8 fixed-point representation.
            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (20.0f));
            expect (dispatcher.events.empty());

            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 20.0f }, Pressed::no,  1 },
                                        { surfaceA, 0, { 10.0f, 20.0f }, Pressed::yes, 1 } });

            queue.touchMotion (2, 0, fixed (15.0f), fixed (25.0f));
            expect (dispatcher.events.empty());

            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 15.0f, 25.0f }, Pressed::yes, 2 } });
        });

        testCase ("Releasing the final touch point does not wait for a frame event", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (20.0f));
            queue.frameComplete();
            dispatcher.events.clear();

            queue.touchUp (2, 0);
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 20.0f }, Pressed::no,  2 },
                                        { surfaceA, 0, MouseInputSource::offscreenMousePos, Pressed::no,  2 } });

            // A later frame must not dispatch the release again.
            queue.frameComplete();
            expect (dispatcher.events.empty());
        });

        testCase ("A serial accompanies a touch down", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (20.0f), 42);
            queue.frameComplete();

            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 20.0f }, Pressed::no,  1 },
                                        { surfaceA, 0, { 10.0f, 20.0f }, Pressed::yes, 1, 42 } });
        });

        testCase ("Releasing one of two touch points waits for the next frame event", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));
            queue.touchDown (1, surfaceA, 1, fixed (20.0f), fixed (20.0f));
            queue.frameComplete();
            dispatcher.events.clear();

            queue.touchUp (2, 0);
            expect (dispatcher.events.empty());

            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  2 },
                                        { surfaceA, 0, MouseInputSource::offscreenMousePos, Pressed::no,  2 } });

            queue.touchUp (3, 1);
            expectEvents (dispatcher, { { surfaceA, 1, { 20.0f, 20.0f }, Pressed::no,  3 },
                                        { surfaceA, 1, MouseInputSource::offscreenMousePos, Pressed::no,  3 } });
        });

        testCase ("Releasing both touch points without a frame event dispatches both releases", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));
            queue.touchDown (1, surfaceA, 1, fixed (20.0f), fixed (20.0f));
            queue.frameComplete();
            dispatcher.events.clear();

            queue.touchUp (2, 0);
            expect (dispatcher.events.empty());

            queue.touchUp (2, 1);
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  2 },
                                        { surfaceA, 0, MouseInputSource::offscreenMousePos, Pressed::no,  2 },
                                        { surfaceA, 1, { 20.0f, 20.0f }, Pressed::no,  2 },
                                        { surfaceA, 1, MouseInputSource::offscreenMousePos, Pressed::no,  2 } });
        });

        testCase ("Touch down and up without an intervening frame event dispatch a complete press and release", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));
            expect (dispatcher.events.empty());

            queue.touchUp (2, 0);
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  1 },
                                        { surfaceA, 0, { 10.0f, 10.0f }, Pressed::yes, 1 },
                                        { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  2 },
                                        { surfaceA, 0, MouseInputSource::offscreenMousePos, Pressed::no,  2 } });
        });

        testCase ("Releasing the final touch point during dispatch completes a separate frame", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));

            bool injected = false;
            dispatcher.onDispatch = [&]
            {
                // A nested event loop reads the final up during dispatch.
                if (! std::exchange (injected, true))
                    queue.touchUp (2, 0);
            };

            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  1 },
                                        { surfaceA, 0, { 10.0f, 10.0f }, Pressed::yes, 1 },
                                        { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  2 },
                                        { surfaceA, 0, MouseInputSource::offscreenMousePos, Pressed::no,  2 } });
        });

        testCase ("A released touch point's slot is reused", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (1.0f), fixed (1.0f));
            queue.touchDown (1, surfaceA, 1, fixed (2.0f), fixed (2.0f));
            queue.frameComplete();
            queue.touchUp (2, 0);
            queue.frameComplete();
            dispatcher.events.clear();

            queue.touchDown (3, surfaceA, 5, fixed (3.0f), fixed (3.0f));
            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 3.0f, 3.0f }, Pressed::no,  3 },
                                        { surfaceA, 0, { 3.0f, 3.0f }, Pressed::yes, 3 } });
        });

        testCase ("Touch frames received during dispatch are applied in arrival order", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            // Frame [down A, down B], with B's up arriving mid-dispatch the way a nested
            // event loop would deliver it.
            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));
            queue.touchDown (1, surfaceA, 1, fixed (50.0f), fixed (50.0f));

            bool injected = false;
            dispatcher.onDispatch = [&]
            {
                if (! std::exchange (injected, true))
                {
                    queue.touchUp (2, 1);
                    queue.frameComplete();
                }
            };

            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  1 },
                                        { surfaceA, 0, { 10.0f, 10.0f }, Pressed::yes, 1 },
                                        { surfaceA, 1, { 50.0f, 50.0f }, Pressed::no,  1 },
                                        { surfaceA, 1, { 50.0f, 50.0f }, Pressed::yes, 1 },
                                        { surfaceA, 1, { 50.0f, 50.0f }, Pressed::no,  2 },
                                        { surfaceA, 1, MouseInputSource::offscreenMousePos, Pressed::no,  2 } });

            // Touch A remains active and touch B has been released.
            queue.touchMotion (3, 0, fixed (11.0f), fixed (11.0f));
            queue.touchMotion (3, 1, fixed (51.0f), fixed (51.0f));
            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 11.0f, 11.0f }, Pressed::yes, 3 } });
        });

        testCase ("A touch event received during dispatch waits for its own frame event", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));

            bool injected = false;
            dispatcher.onDispatch = [&]
            {
                // A nested event loop reads the motion before its frame.
                if (! std::exchange (injected, true))
                    queue.touchMotion (2, 0, fixed (12.0f), fixed (12.0f));
            };

            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  1 },
                                        { surfaceA, 0, { 10.0f, 10.0f }, Pressed::yes, 1 } });

            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 12.0f, 12.0f }, Pressed::yes, 2 } });
        });

        testCase ("A nested cancel discards queued frames and a pending press", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));

            bool injected = false;
            dispatcher.onDispatch = [&]
            {
                if (! std::exchange (injected, true))
                {
                    // A second gesture is already queued when the compositor cancels.
                    queue.touchDown (2, surfaceA, 1, fixed (50.0f), fixed (50.0f));
                    queue.frameComplete();
                    queue.cancel();
                }
            };

            queue.frameComplete();

            // The enter dispatches first, then the cancellation release.
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  1 },
                                        { surfaceA, 0, MouseInputSource::offscreenMousePos, Pressed::no,  1 } });

            // The cancelled touch no longer accepts motion events.
            queue.touchMotion (3, 0, fixed (11.0f), fixed (11.0f));
            queue.frameComplete();
            expect (dispatcher.events.empty());

            queue.touchDown (4, surfaceA, 2, fixed (60.0f), fixed (60.0f));
            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 60.0f, 60.0f }, Pressed::no,  4 },
                                        { surfaceA, 0, { 60.0f, 60.0f }, Pressed::yes, 4 } });
        });

        testCase ("Cancel does not consume a new gesture arriving during its dispatch", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            // two fingers down
            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));
            queue.touchDown (1, surfaceA, 1, fixed (20.0f), fixed (20.0f));
            queue.frameComplete();
            dispatcher.events.clear();

            bool injected = false;
            dispatcher.onDispatch = [&]
            {
                // Two new fingers are going to land during the first two's release.
                // They will reuse slot 0/1
                if (! std::exchange (injected, true))
                {
                    queue.touchDown (2, surfaceA, 5, fixed (30.0f), fixed (30.0f));
                    queue.touchDown (2, surfaceA, 6, fixed (40.0f), fixed (40.0f));
                    queue.frameComplete();
                }
            };

            // compositor recognises touches as its gesture or unbindTouch() called
            queue.cancel();

            // Old touches release first, then the new gesture applies untouched by
            // the cancellation, even though it reuses indices 0 and 1.
            expectEvents (dispatcher, { { surfaceA, 0, MouseInputSource::offscreenMousePos, Pressed::no,  1 },
                                        { surfaceA, 1, MouseInputSource::offscreenMousePos, Pressed::no,  1 },
                                        { surfaceA, 0, { 30.0f, 30.0f }, Pressed::no,  2 },
                                        { surfaceA, 0, { 30.0f, 30.0f }, Pressed::yes, 2 },
                                        { surfaceA, 1, { 40.0f, 40.0f }, Pressed::no,  2 },
                                        { surfaceA, 1, { 40.0f, 40.0f }, Pressed::yes, 2 } });

            // New touches survive cancellation.
            queue.touchMotion (3, 5, fixed (31.0f), fixed (31.0f));
            queue.touchMotion (3, 6, fixed (41.0f), fixed (41.0f));
            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceA, 0, { 31.0f, 31.0f }, Pressed::yes, 3 },
                                        { surfaceA, 1, { 41.0f, 41.0f }, Pressed::yes, 3 } });
        });

        testCase ("Removing a surface clears its touches and rejects queued and future down events", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA, surfaceB } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));
            queue.touchDown (1, surfaceB, 1, fixed (20.0f), fixed (20.0f));
            queue.frameComplete();
            dispatcher.events.clear();

            queue.touchDown (2, surfaceA, 2, fixed (12.0f), fixed (12.0f));
            queue.surfaceRemoved (surfaceA);
            dispatcher.removeTarget (surfaceA);
            queue.frameComplete();
            expect (dispatcher.events.empty());

            // The removed surface's touch is gone, the other one still drags.
            queue.touchMotion (3, 0, fixed (11.0f), fixed (11.0f));
            queue.touchMotion (3, 1, fixed (21.0f), fixed (21.0f));
            queue.frameComplete();
            expectEvents (dispatcher, { { surfaceB, 1, { 21.0f, 21.0f }, Pressed::yes, 3 } });

            // Later touches for the removed surface are also rejected.
            queue.touchDown (4, surfaceA, 3, fixed (13.0f), fixed (13.0f));
            queue.frameComplete();
            expect (dispatcher.events.empty());
        });

        testCase ("A surface removed during dispatch receives no later events from that frame", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));

            bool injected = false;
            dispatcher.onDispatch = [&]
            {
                // Application code run by the enter event destroys the peer.
                if (! std::exchange (injected, true))
                {
                    queue.surfaceRemoved (surfaceA);
                    dispatcher.removeTarget (surfaceA);
                }
            };

            queue.frameComplete();

            // Only the enter event was sent before the peer disappeared. The press was discarded.
            expectEvents (dispatcher, { { surfaceA, 0, { 10.0f, 10.0f }, Pressed::no,  1 } });
        });

        testCase ("Releasing the final touch point for a removed surface dispatches nothing", [&]
        {
            RecordingDispatcher dispatcher { { surfaceA } };
            WaylandTouchQueue queue { dispatcher };

            queue.touchDown (1, surfaceA, 0, fixed (10.0f), fixed (10.0f));
            queue.frameComplete();
            queue.surfaceRemoved (surfaceA);
            dispatcher.removeTarget (surfaceA);
            dispatcher.events.clear();

            queue.touchUp (2, 0);
            expect (dispatcher.events.empty());
        });
    }

private:
    enum class Pressed { no, yes };

    struct TouchEvent
    {
        TouchEvent (wl_surface* surfaceIn,
                    int indexIn,
                    Point<float> positionIn,
                    Pressed pressedIn,
                    uint32_t timeIn,
                    std::optional<uint32_t> grabSerialIn = {})
            : surface (surfaceIn),
              index (indexIn),
              position (positionIn),
              pressed (pressedIn),
              time (timeIn),
              popupGrabSerial (grabSerialIn)
        {}

        wl_surface* surface = nullptr;
        int index = 0;
        Point<float> position;
        Pressed pressed = Pressed::no;
        uint32_t time = 0;
        std::optional<uint32_t> popupGrabSerial;

        auto tie() const
        {
            return std::tuple (surface, index, position, pressed, time, popupGrabSerial);
        }

        bool operator== (const TouchEvent& other) const
        {
            return tie() == other.tie();
        }

        bool operator!= (const TouchEvent& other) const
        {
            return ! operator== (other);
        }
    };

    struct RecordingDispatcher final : public WaylandTouchQueue::Dispatcher
    {
        explicit RecordingDispatcher (std::vector<wl_surface*> targets)
            : liveTargets (std::move (targets)) {}

        bool hasTouchTarget (wl_surface* surface) override
        {
            return std::find (liveTargets.begin(), liveTargets.end(), surface) != liveTargets.end();
        }

        void dispatchTouchEvent (wl_surface* surface, int touchIndex, Point<float> position,
                                 ModifierKeys mods, uint32_t time,
                                 std::optional<uint32_t> popupGrabSerial) override
        {
            // Mirror the real dispatcher: resolution happens per event, so a surface removed
            // during a frame silently discards its remaining events without running application code.
            if (! hasTouchTarget (surface))
                return;

            events.push_back ({ surface, touchIndex, position,
                                mods.isLeftButtonDown() ? Pressed::yes : Pressed::no,
                                time,
                                popupGrabSerial });

            if (onDispatch != nullptr)
                onDispatch();
        }

        void removeTarget (wl_surface* surface)
        {
            liveTargets.erase (std::remove (liveTargets.begin(), liveTargets.end(), surface), liveTargets.end());
        }

        std::vector<wl_surface*> liveTargets;
        std::vector<TouchEvent> events;
        std::function<void()> onDispatch;
    };

    // Asserts the recorded events match, then clears them for the next step.
    void expectEvents (RecordingDispatcher& dispatcher, const std::vector<TouchEvent>& expected)
    {
        const auto& actual = dispatcher.events;
        const auto matches = std::equal (actual.begin(), actual.end(), expected.begin(), expected.end());

        if (! matches)
            logMessage ("expected:" + newLine + toString (expected) + "actual:" + newLine + toString (actual));

        expect (matches);

        dispatcher.events.clear();
    }

    static String toString (const std::vector<TouchEvent>& events)
    {
        String result;

        for (const auto [index, event] : enumerate (events, int{}))
            result << "    [" << index << "] " << toString (event) << newLine;

        return result;
    }

    static String toString (const TouchEvent& event)
    {
        return "surface " + String::toHexString ((pointer_sized_int) event.surface)
             + ", index " + String (event.index)
             + ", position (" + event.position.toString() + ")"
             + ", " + (event.pressed == Pressed::yes ? "pressed" : "released")
             + ", time " + String (event.time);
    }

    static wl_surface* fakeSurface (uintptr_t value)
    {
        return reinterpret_cast<wl_surface*> (value);
    }

    // wl_touch coordinates are 24.8 fixed point.
    static int32_t fixed (float value)
    {
        return (int32_t) (value * 256.0f);
    }
};

static WaylandTouchQueueTests waylandTouchQueueTests;

//==============================================================================
class WaylandPointerAxisStateTests final : public UnitTest
{
public:
    WaylandPointerAxisStateTests()
        : UnitTest ("WaylandPointerAxisState", UnitTestCategories::gui) {}

    void runTest() override
    {
        using FrameCompletion = WaylandPointerAxisState::FrameCompletion;

        constexpr auto vertical   = (uint32_t) WaylandProtocol::wlPointerAxisVerticalScroll;
        constexpr auto horizontal = (uint32_t) WaylandProtocol::wlPointerAxisHorizontalScroll;

        // A scroll arrives as wl_pointer.axis (about 15 units per wheel detent),
        // axis_discrete (one step per detent) or axis_value120 (120 per detent).
        // Each detent maps to a wheel delta of 50/256, matching the X11 backend.
        constexpr auto deltaPerDetent = 50.0f / 256.0f;

        testCase ("Axis events from pointer versions below 5 complete their own frame", [&]
        {
            WaylandPointerAxisState state;

            for (const uint32_t version : { 1u, 4u })
            {
                state.bindPointer (version);
                expect (state.addAxisValue (vertical, axisValueForDetents (1.0f)) == FrameCompletion::required);

                const auto wheelDetails = state.commitFrame();
                expect (wheelDetails[vertical].has_value());
                expect (! wheelDetails[horizontal].has_value());

                if (! wheelDetails[vertical].has_value())
                    return;

                expectEquals (wheelDetails[vertical]->deltaY, -deltaPerDetent);
                expectEquals (wheelDetails[vertical]->deltaX, 0.0f);
            }

            state.bindPointer (5);
            expect (state.addAxisValue (vertical, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
        });

        testCase ("Both axes dispatch in one frame and repeated values accumulate", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (5);

            expect (state.addAxisValue (vertical, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
            expect (state.addAxisValue (vertical, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
            expect (state.addAxisValue (horizontal, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);

            const auto wheelDetails = state.commitFrame();
            expect (wheelDetails[vertical].has_value() && wheelDetails[horizontal].has_value());

            if (! (wheelDetails[vertical].has_value() && wheelDetails[horizontal].has_value()))
                return;

            expectEquals (wheelDetails[vertical]->deltaY, -2.0f * deltaPerDetent);
            expectEquals (wheelDetails[vertical]->deltaX, 0.0f);
            expectEquals (wheelDetails[horizontal]->deltaX, -deltaPerDetent);
            expectEquals (wheelDetails[horizontal]->deltaY, 0.0f);
        });

        testCase ("Discrete wheel steps override raw axis distance", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (5);

            // The axis and axis_discrete events describe the same scroll, so their amounts must not be added.
            expect (state.addAxisValue (vertical, axisValueForDetents (10.0f)) == FrameCompletion::notRequired);
            state.addDiscreteAxisSteps (vertical, 1);
            state.addDiscreteAxisSteps (vertical, 2);

            const auto wheelDetails = state.commitFrame();
            expect (wheelDetails[vertical].has_value());

            if (! wheelDetails[vertical].has_value())
                return;

            expectEquals (wheelDetails[vertical]->deltaY, -3.0f * deltaPerDetent);
        });

        testCase ("High-resolution wheel data overrides raw axis distance", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (8);

            expect (state.addAxisValue (vertical, axisValueForDetents (10.0f)) == FrameCompletion::notRequired);
            state.addAxisValue120 (vertical, 30);
            state.addAxisValue120 (vertical, 30);

            const auto wheelDetails = state.commitFrame();
            expect (wheelDetails[vertical].has_value());

            if (! wheelDetails[vertical].has_value())
                return;

            expectEquals (wheelDetails[vertical]->deltaY, -0.5f * deltaPerDetent);
        });

        testCase ("High-resolution wheel data overrides discrete steps", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (8);

            state.addDiscreteAxisSteps (vertical, 3);
            state.addAxisValue120 (vertical, 30);
            state.addAxisValue120 (vertical, 30);

            const auto wheelDetails = state.commitFrame();
            expect (wheelDetails[vertical].has_value());

            if (! wheelDetails[vertical].has_value())
                return;

            expectEquals (wheelDetails[vertical]->deltaY, -0.5f * deltaPerDetent);
        });

        testCase ("Touchpad and continuous scrolling are reported as smooth", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (5);

            const auto smoothFor = [&] (std::optional<uint32_t> source)
            {
                if (source.has_value())
                    state.setAxisSource (*source);

                expect (state.addAxisValue (vertical, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
                const auto wheelDetails = state.commitFrame();
                expect (wheelDetails[vertical].has_value());

                if (! wheelDetails[vertical].has_value())
                    return false;

                expect (! wheelDetails[vertical]->isInertial);
                return wheelDetails[vertical]->isSmooth;
            };

            expect (smoothFor (WaylandProtocol::wlPointerAxisSourceFinger));
            expect (smoothFor (WaylandProtocol::wlPointerAxisSourceContinuous));
            expect (! smoothFor (WaylandProtocol::wlPointerAxisSourceWheel));
            expect (! smoothFor (std::nullopt));
        });

        testCase ("Reversing vertical scrolling does not reverse horizontal scrolling", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (9);

            expect (state.addAxisValue (vertical, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
            expect (state.addAxisValue (horizontal, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
            state.setAxisRelativeDirection (vertical, WaylandProtocol::wlPointerAxisRelativeDirectionInverted);
            state.setAxisRelativeDirection (horizontal, WaylandProtocol::wlPointerAxisRelativeDirectionIdentical);

            const auto wheelDetails = state.commitFrame();
            expect (wheelDetails[vertical].has_value() && wheelDetails[horizontal].has_value());

            if (! (wheelDetails[vertical].has_value() && wheelDetails[horizontal].has_value()))
                return;

            expect (wheelDetails[vertical]->isReversed);
            expect (! wheelDetails[horizontal]->isReversed);
        });

        testCase ("Committing a frame clears its accumulated axis state", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (9);

            state.setAxisSource (WaylandProtocol::wlPointerAxisSourceFinger);
            state.addAxisValue120 (vertical, 240);
            state.setAxisRelativeDirection (vertical, WaylandProtocol::wlPointerAxisRelativeDirectionInverted);
            expect (state.commitFrame()[vertical].has_value());

            // A frame with nothing accumulated dispatches nothing.
            {
                const auto wheelDetails = state.commitFrame();
                expect (! wheelDetails[vertical].has_value() && ! wheelDetails[horizontal].has_value());
            }

            // The next frame's continuous-only scroll should not inherit the value120,
            // source or direction of the previous frame.
            expect (state.addAxisValue (vertical, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
            const auto wheelDetails = state.commitFrame();
            expect (wheelDetails[vertical].has_value());

            if (! wheelDetails[vertical].has_value())
                return;

            expectEquals (wheelDetails[vertical]->deltaY, -deltaPerDetent);
            expect (! wheelDetails[vertical]->isSmooth);
            expect (! wheelDetails[vertical]->isReversed);
        });

        testCase ("An axis index beyond the two scroll axes is ignored", [&]
        {
            WaylandPointerAxisState state;
            state.bindPointer (4);

            // Even on a version that dispatches per axis event, an unknown axis never completes a frame.
            expect (state.addAxisValue (2, axisValueForDetents (1.0f)) == FrameCompletion::notRequired);
            state.addDiscreteAxisSteps (2, 1);
            state.addAxisValue120 (2, 120);
            state.setAxisRelativeDirection (2, WaylandProtocol::wlPointerAxisRelativeDirectionInverted);

            const auto wheelDetails = state.commitFrame();
            expect (! wheelDetails[vertical].has_value() && ! wheelDetails[horizontal].has_value());
        });
    }

private:
    // wl_pointer axis values are 24.8 fixed point and use about 15 units per wheel detent.
    static int32_t axisValueForDetents (float detents)
    {
        constexpr auto unitsPerDetent = 15.0f;
        constexpr auto fixedPointScale = 256.0f;

        return (int32_t) (detents * unitsPerDetent * fixedPointScale);
    }
};

static WaylandPointerAxisStateTests waylandPointerAxisStateTests;

#endif

} // namespace juce
