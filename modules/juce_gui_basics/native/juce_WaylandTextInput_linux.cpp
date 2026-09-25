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
WaylandTextInput::WaylandTextInput (WaylandTextInputDelegate& delegateIn)
    : delegate (delegateIn)
{
}

WaylandTextInput::~WaylandTextInput()
{
    unbind();
}

void WaylandTextInput::unbind()
{
    cancelPendingUpdate();
    disable();
    proxy.reset();
    focusSurface = nullptr;
    requestedSurface = nullptr;
    requestedTarget = nullptr;
    commitCount = 0;
}

void WaylandTextInput::addClient (wl_surface* surface, WaylandTextInputClient& client)
{
    const auto* existing = findClientForSurface (surface);

    // A peer must register its surface only once.
    jassert (existing == nullptr);

    if (surface != nullptr && existing == nullptr)
        surfaceClients.push_back ({ surface, &client });
}

void WaylandTextInput::removeClient (WaylandTextInputClient& client)
{
    const auto matches = [&] (const SurfaceClient& item) { return item.client == &client; };
    const auto it = std::find_if (surfaceClients.begin(), surfaceClients.end(), matches);

    if (it == surfaceClients.end())
        return;

    const auto removingFocusSurface = focusSurface == it->surface;
    const auto removingRequestedSurface = requestedSurface == it->surface;

    if (removingFocusSurface || removingRequestedSurface)
        disable();

    if (removingFocusSurface)
        focusSurface = nullptr;

    if (removingRequestedSurface)
    {
        requestedSurface = nullptr;
        requestedTarget = nullptr;
    }

    surfaceClients.erase (it);
}

WaylandTextInputClient* WaylandTextInput::findClientForSurface (wl_surface* surface) const
{
    const auto matches = [surface] (const SurfaceClient& item) { return item.surface == surface; };
    const auto it = std::find_if (surfaceClients.begin(), surfaceClients.end(), matches);
    return it != surfaceClients.end() ? it->client : nullptr;
}

#if JUCE_WAYLAND_PEER_DIAGNOSTICS
detail::WaylandPeerDiagnostics::TextInput WaylandTextInput::getDiagnostics (wl_surface* surface) const
{
    detail::WaylandPeerDiagnostics::TextInput result;
    result.available = proxy != nullptr;
    result.commits = commitCount;

    if (surface != requestedSurface)
        return result;

    result.active = enabled && getActiveTarget() != nullptr;
    result.compositionLength = composition.has_value() ? composition->range.getLength() : 0;

    if (lastSentState.has_value())
    {
        result.caret = lastSentState->caret;
        result.hasSurroundingText = lastSentState->surrounding.has_value();

        if (lastSentState->surrounding.has_value())
        {
            const auto& surrounding = *lastSentState->surrounding;
            result.surroundingBytes = (int) surrounding.text.getNumBytesAsUTF8();
            result.cursor = surrounding.cursor;
            result.anchor = surrounding.anchor;
        }
    }

    return result;
}
#endif

void WaylandTextInput::bind (zwp_text_input_manager_v3* manager, wl_seat* seat)
{
    if (proxy != nullptr || manager == nullptr || seat == nullptr)
        return;

    proxy.reset (WaylandProtocol::zwpTextInputManagerV3GetTextInput (manager, seat));

    if (proxy != nullptr)
        WaylandProtocol::zwpTextInputV3AddListener (proxy.get(), &listener, this);
}

void WaylandTextInput::requireInput (wl_surface* surface, TextInputTarget& target)
{
    auto* targetComponent = dynamic_cast<Component*> (&target);

    if (targetComponent == nullptr)
        return;

    const auto isSameTarget = requestedSurface == surface
                           && requestedTarget.get() == targetComponent;

    if (! isSameTarget)
    {
        disable();
        requestedSurface = surface;
        requestedTarget = targetComponent;
    }

    if (! enabled && targetHasFocus())
        enable();
}

void WaylandTextInput::closeContext (wl_surface* surface)
{
    if (requestedSurface != surface)
        return;

    disable();

    if (targetHasFocus())
        enable();
}

void WaylandTextInput::dismissPendingInput (wl_surface* surface)
{
    if (requestedSurface != surface && focusSurface != surface)
        return;

    disable();
    requestedSurface = nullptr;
    requestedTarget = nullptr;
}

void WaylandTextInput::handleEnter (wl_surface* surface)
{
    focusSurface = findClientForSurface (surface) != nullptr ? surface : nullptr;

    if (targetHasFocus())
        enable();
}

void WaylandTextInput::handleLeave()
{
    cancelPendingUpdate();
    resetComposition();
    pending = {};
    lastSentState.reset();
    lastObservedState.reset();
    waitingForMatchingDone = false;
    enabled = false;
    focusSurface = nullptr;
}

void WaylandTextInput::handlePreeditString (const char* text, int32_t cursorBegin, int32_t cursorEnd)
{
    pending.preedit = String (CharPointer_UTF8 (text != nullptr ? text : ""));
    pending.preeditCursorBegin = cursorBegin;
    pending.preeditCursorEnd = cursorEnd;
}

void WaylandTextInput::handleCommitString (const char* text)
{
    pending.commit = String (CharPointer_UTF8 (text != nullptr ? text : ""));
}

void WaylandTextInput::handleDeleteSurroundingText (uint32_t beforeLength, uint32_t afterLength)
{
    pending.deleteBefore = beforeLength;
    pending.deleteAfter = afterLength;
}

void WaylandTextInput::handleDone (uint32_t serial)
{
    const ScopedValueSetter scope (applyingEvents, true);
    waitingForMatchingDone = serial != commitCount;
    auto* target = getActiveTarget();

    if (target != nullptr)
    {
        discardChangedComposition (*target);
        detectExternalEdits (getTargetState (*target));
        applyPendingEvents (*target);
        insertPendingPreedit (*target);
        lastObservedState = getTargetState (*target);

        // The snapshot excludes pre-edit text but includes the caret position after composition.
        // Otherwise a later repaint can mistake this caret movement for an external edit.
        sendTargetState (*lastObservedState, false, pendingChangeCause);
    }

    pending = {};
}

void WaylandTextInput::applyPendingEvents (TextInputTarget& target)
{
    removeCurrentPreedit (target);
    deleteSurroundingText (target, pending.deleteBefore, pending.deleteAfter);

    if (pending.commit.has_value())
        target.insertTextAtCaret (*pending.commit);
}

void WaylandTextInput::enable()
{
    auto* target = getActiveTarget();

    if (proxy == nullptr || target == nullptr || enabled)
        return;

    WaylandProtocol::zwpTextInputV3Enable (proxy.get());
    enabled = true;
    lastObservedState = getTargetState (*target);
    sendTargetState (*lastObservedState, true, WaylandProtocol::zwpTextInputV3ChangeCauseOther);
}

void WaylandTextInput::disable()
{
    cancelPendingUpdate();
    const ScopedValueSetter scope (applyingEvents, true);
    lastSentState.reset();
    lastObservedState.reset();
    waitingForMatchingDone = false;
    pendingChangeCause = WaylandProtocol::zwpTextInputV3ChangeCauseInputMethod;
    resetComposition();
    pending = {};

    if (! std::exchange (enabled, false) || proxy == nullptr)
        return;

    WaylandProtocol::zwpTextInputV3Disable (proxy.get());
    commitState();
}

void WaylandTextInput::commitState()
{
    if (proxy == nullptr)
        return;

    WaylandProtocol::zwpTextInputV3Commit (proxy.get());
    ++commitCount;
    delegate.flush();
}

bool WaylandTextInput::SurroundingText::operator== (const SurroundingText& other) const
{
    return text == other.text && cursor == other.cursor && anchor == other.anchor;
}

bool WaylandTextInput::TargetState::operator== (const TargetState& other) const
{
    return text == other.text && cursor == other.cursor && anchor == other.anchor
        && surrounding == other.surrounding && caret == other.caret && keyboardType == other.keyboardType;
}

std::optional<WaylandTextInput::SurroundingText> WaylandTextInput::makeSurroundingText (const String& text,
                                                                                    int cursor, int anchor)
{
    constexpr int maximumBytes = 4000;
    const auto length = text.length();
    cursor = jlimit (0, length, cursor);
    anchor = jlimit (0, length, anchor);

    const auto textStart = text.toUTF8();
    const auto textEnd = textStart.findTerminatingNull();
    const auto cursorPtr = textStart + cursor;
    const auto anchorPtr = textStart + anchor;
    auto windowStart = cursor < anchor ? cursorPtr : anchorPtr;
    auto windowEnd = cursor < anchor ? anchorPtr : cursorPtr;
    auto bytes = windowEnd.getAddress() - windowStart.getAddress();

    if (bytes > maximumBytes)
        return std::nullopt;

    for (;;)
    {
        auto grown = false;

        if (windowStart != textStart)
        {
            const auto candidate = windowStart - 1;
            const auto extra = windowStart.getAddress() - candidate.getAddress();

            if (bytes + extra <= maximumBytes)
            {
                windowStart = candidate;
                bytes += extra;
                grown = true;
            }
        }

        if (windowEnd != textEnd)
        {
            const auto candidate = windowEnd + 1;
            const auto extra = candidate.getAddress() - windowEnd.getAddress();

            if (bytes + extra <= maximumBytes)
            {
                windowEnd = candidate;
                bytes += extra;
                grown = true;
            }
        }

        if (! grown)
            break;
    }

    return SurroundingText { String (windowStart, windowEnd),
                             (int32_t) (cursorPtr.getAddress() - windowStart.getAddress()),
                             (int32_t) (anchorPtr.getAddress() - windowStart.getAddress()) };
}

WaylandTextInput::TargetState WaylandTextInput::getTargetState (TextInputTarget& target) const
{
    auto text = target.getTextInRange ({ 0, target.getTotalNumChars() });
    auto cursor = target.getCaretPosition();
    const auto selection = target.getHighlightedRegion();
    auto anchor = cursor == selection.getStart() ? selection.getEnd() : selection.getStart();

    if (composition.has_value())
    {
        const auto range = composition->range;
        text = text.replaceSection (range.getStart(), range.getLength(), {});
        cursor = shiftPositionPastRemovedRange (cursor, range);
        anchor = shiftPositionPastRemovedRange (anchor, range);
    }

    TargetState state;
    state.text = text;
    state.cursor = cursor;
    state.anchor = anchor;
    state.keyboardType = target.getKeyboardType();

    if (state.keyboardType != TextInputTarget::passwordKeyboard)
        state.surrounding = makeSurroundingText (text, cursor, anchor);

    if (auto* client = findClientForSurface (requestedSurface))
        state.caret = client->getTextInputCaretRectangle (target);

    return state;
}

void WaylandTextInput::refreshState (wl_surface* surface)
{
    if (enabled && ! applyingEvents
        && (surface == requestedSurface || surface == focusSurface))
        triggerAsyncUpdate();
}

void WaylandTextInput::handleAsyncUpdate()
{
    if (auto* target = getActiveTarget())
    {
        discardChangedComposition (*target);
        const auto state = getTargetState (*target);

        detectExternalEdits (state);

        lastObservedState = state;
        sendTargetState (state, false, pendingChangeCause);
    }
}

void WaylandTextInput::detectExternalEdits (const TargetState& state)
{
    // A serial mismatch can delay sending IME changes. Compare with the last observed state
    // so a repaint does not report those changes as an external edit.
    if (lastObservedState.has_value()
        && (state.text != lastObservedState->text
            || state.cursor != lastObservedState->cursor
            || state.anchor != lastObservedState->anchor))
    {
        pendingChangeCause = WaylandProtocol::zwpTextInputV3ChangeCauseOther;
    }
}

void WaylandTextInput::sendTargetState (const TargetState& state, bool includeContentType, uint32_t cause)
{
    if (proxy == nullptr || ! enabled || waitingForMatchingDone)
        return;

    if (! includeContentType && lastSentState == state)
    {
        pendingChangeCause = WaylandProtocol::zwpTextInputV3ChangeCauseInputMethod;
        return;
    }

    // Enabling again clears context that can no longer be represented, including a selection
    // larger than the protocol limit. It also restores surrounding-text support when it fits again.
    if (lastSentState.has_value()
        && (lastSentState->surrounding.has_value() != state.surrounding.has_value()
            || lastSentState->keyboardType != state.keyboardType))
    {
        WaylandProtocol::zwpTextInputV3Enable (proxy.get());
        includeContentType = true;
    }

    if (state.surrounding.has_value())
    {
        const auto& surrounding = *state.surrounding;
        WaylandProtocol::zwpTextInputV3SetSurroundingText (proxy.get(), surrounding.text.toRawUTF8(),
                                                       surrounding.cursor, surrounding.anchor);
    }

    WaylandProtocol::zwpTextInputV3SetTextChangeCause (proxy.get(), cause);

    if (includeContentType)
    {
        const auto contentType = getContentType (state.keyboardType);
        WaylandProtocol::zwpTextInputV3SetContentType (proxy.get(), contentType.hint, contentType.purpose);
    }

    WaylandProtocol::zwpTextInputV3SetCursorRectangle (proxy.get(), state.caret.getX(), state.caret.getY(),
                                                     jmax (1, state.caret.getWidth()), jmax (1, state.caret.getHeight()));
    lastSentState = state;
    pendingChangeCause = WaylandProtocol::zwpTextInputV3ChangeCauseInputMethod;
    commitState();
}

WaylandTextInput::ContentType WaylandTextInput::getContentType (TextInputTarget::VirtualKeyboardType keyboardType)
{
    using namespace WaylandProtocol;

    switch (keyboardType)
    {
        case TextInputTarget::numericKeyboard:      return { zwpTextInputV3ContentHintNone, zwpTextInputV3ContentPurposeDigits };
        case TextInputTarget::decimalKeyboard:      return { zwpTextInputV3ContentHintNone, zwpTextInputV3ContentPurposeNumber };
        case TextInputTarget::urlKeyboard:          return { zwpTextInputV3ContentHintNone, zwpTextInputV3ContentPurposeUrl };
        case TextInputTarget::emailAddressKeyboard: return { zwpTextInputV3ContentHintNone, zwpTextInputV3ContentPurposeEmail };
        case TextInputTarget::phoneNumberKeyboard:  return { zwpTextInputV3ContentHintNone, zwpTextInputV3ContentPurposePhone };
        case TextInputTarget::passwordKeyboard:     return { zwpTextInputV3ContentHintSensitiveData, zwpTextInputV3ContentPurposePassword };
        case TextInputTarget::textKeyboard:         break;
    }

    return { zwpTextInputV3ContentHintNone, zwpTextInputV3ContentPurposeNormal };
}

TextInputTarget* WaylandTextInput::getRequestedTarget() const
{
    auto* component = requestedTarget.get();
    return component != nullptr ? dynamic_cast<TextInputTarget*> (component) : nullptr;
}

TextInputTarget* WaylandTextInput::getActiveTarget() const
{
    auto* target = getRequestedTarget();
    auto* client = findClientForSurface (requestedSurface);

    if (client == nullptr || target == nullptr || ! targetHasFocus())
        return nullptr;

    return client->getTextInputTarget() == target ? target : nullptr;
}

bool WaylandTextInput::targetHasFocus() const
{
    if (auto* client = findClientForSurface (requestedSurface))
        return focusSurface != nullptr && focusSurface == client->getTextInputFocusSurface();

    return false;
}

void WaylandTextInput::resetComposition()
{
    const ScopedValueSetter scope (applyingEvents, true);

    if (auto* target = getRequestedTarget())
    {
        discardChangedComposition (*target);
        const auto rangeToRemove = composition.has_value() ? std::optional { composition->range } : std::nullopt;
        const auto selection = target->getHighlightedRegion();

        removeCurrentPreedit (*target);

        if (rangeToRemove.has_value())
        {
            target->setHighlightedRegion ({ shiftPositionPastRemovedRange (selection.getStart(), *rangeToRemove),
                                            shiftPositionPastRemovedRange (selection.getEnd(), *rangeToRemove) });
        }
    }
    else
        composition.reset();
}

void WaylandTextInput::discardChangedComposition (TextInputTarget& target)
{
    if (composition.has_value()
        && composition->document != target.getTextInRange ({ 0, target.getTotalNumChars() }))
    {
        // An external edit invalidates the saved range. Deleting it could remove unrelated text.
        composition.reset();
        target.setTemporaryUnderlining ({});
    }
}

void WaylandTextInput::removeCurrentPreedit (TextInputTarget& target)
{
    if (composition.has_value())
    {
        target.setHighlightedRegion (composition->range);
        target.insertTextAtCaret ({});
    }

    target.setTemporaryUnderlining ({});
    composition.reset();
}

void WaylandTextInput::insertPendingPreedit (TextInputTarget& target)
{
    if (! pending.preedit.has_value() || pending.preedit->isEmpty())
        return;

    const auto start = target.getHighlightedRegion().getStart();
    const auto remainingChars = target.getTotalNumChars() - target.getHighlightedRegion().getLength();
    target.insertTextAtCaret (*pending.preedit);
    const auto insertedChars = jmax (0, target.getTotalNumChars() - remainingChars);
    composition = Composition { Range<int>::withStartAndLength (start, insertedChars),
                                target.getTextInRange ({ 0, target.getTotalNumChars() }) };
    target.setTemporaryUnderlining ({ composition->range });

    if (pending.preeditCursorBegin < 0 || pending.preeditCursorEnd < 0)
        return;

    const auto cursorBegin = jmin (insertedChars, getCodepointOffset (*pending.preedit, pending.preeditCursorBegin));
    const auto cursorEnd = jmin (insertedChars, getCodepointOffset (*pending.preedit, pending.preeditCursorEnd));
    target.setHighlightedRegion (Range<int> (jmin (cursorBegin, cursorEnd), jmax (cursorBegin, cursorEnd)) + start);
}

int WaylandTextInput::getCodepointOffset (const String& text, int32_t utf8ByteOffset)
{
    const auto* utf8 = text.toRawUTF8();
    auto bytes = jlimit (0, (int) text.getNumBytesAsUTF8(), (int) utf8ByteOffset);

    // GNOME with Mozc has been observed sending cursor offsets inside UTF-8 sequences.
    // Round down to a character boundary before decoding.
    while (bytes > 0 && (utf8[bytes] & 0xc0) == 0x80)
        --bytes;

    return String::fromUTF8 (utf8, bytes).length();
}

int WaylandTextInput::getTrailingCodepointCount (const String& text, uint32_t utf8ByteCount)
{
    const auto totalBytes = text.getNumBytesAsUTF8();
    const auto bytes = jmin (totalBytes, (size_t) utf8ByteCount);
    return (int) CharPointer_UTF8 (text.toRawUTF8() + totalBytes - bytes).length();
}

int WaylandTextInput::getLeadingCodepointCount (const String& text, uint32_t utf8ByteCount)
{
    const auto bytes = jmin (text.getNumBytesAsUTF8(), (size_t) utf8ByteCount);
    const auto* start = text.toRawUTF8();
    return (int) CharPointer_UTF8 (start).lengthUpTo (CharPointer_UTF8 (start + bytes));
}

int WaylandTextInput::shiftPositionPastRemovedRange (int position, Range<int> removed)
{
    return position - jlimit (0, removed.getLength(), position - removed.getStart());
}

void WaylandTextInput::deleteSurroundingText (TextInputTarget& target, uint32_t beforeLength, uint32_t afterLength)
{
    if (beforeLength == 0 && afterLength == 0)
        return;

    const auto selection = target.getHighlightedRegion();
    const auto totalChars = target.getTotalNumChars();
    const auto cursor = jlimit (0, totalChars, target.getCaretPosition());
    const auto before = target.getTextInRange ({ 0, cursor });
    const auto after = target.getTextInRange ({ cursor, totalChars });
    const auto charsBefore = getTrailingCodepointCount (before, beforeLength);
    const auto charsAfter = getLeadingCodepointCount (after, afterLength);

    const Range<int> removed { cursor - charsBefore, cursor + charsAfter };
    target.setHighlightedRegion (removed);
    target.insertTextAtCaret ({});

    const auto anchor = shiftPositionPastRemovedRange (cursor == selection.getStart() ? selection.getEnd() : selection.getStart(), removed);
    const auto caret = shiftPositionPastRemovedRange (cursor, removed);
    target.setHighlightedRegion (Range<int>::emptyRange (anchor));
    target.setHighlightedRegion ({ jmin (anchor, caret), jmax (anchor, caret) });
}

const zwp_text_input_v3_listener WaylandTextInput::listener
{
    [] (void* data, zwp_text_input_v3*, wl_surface* surface)
    {
        static_cast<WaylandTextInput*> (data)->handleEnter (surface);
    },
    [] (void* data, zwp_text_input_v3*, wl_surface*)
    {
        static_cast<WaylandTextInput*> (data)->handleLeave();
    },
    [] (void* data, zwp_text_input_v3*, const char* text, int32_t cursorBegin, int32_t cursorEnd)
    {
        static_cast<WaylandTextInput*> (data)->handlePreeditString (text, cursorBegin, cursorEnd);
    },
    [] (void* data, zwp_text_input_v3*, const char* text)
    {
        static_cast<WaylandTextInput*> (data)->handleCommitString (text);
    },
    [] (void* data, zwp_text_input_v3*, uint32_t beforeLength, uint32_t afterLength)
    {
        static_cast<WaylandTextInput*> (data)->handleDeleteSurroundingText (beforeLength, afterLength);
    },
    [] (void* data, zwp_text_input_v3*, uint32_t serial)
    {
        static_cast<WaylandTextInput*> (data)->handleDone (serial);
    }
};

#if JUCE_UNIT_TESTS

class WaylandTextInputTests final : public UnitTest
{
public:
    WaylandTextInputTests()
        : UnitTest ("WaylandTextInput", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("Pre-edit and commit events take effect only when done is received", [&]
        {
            Context context;
            context.editor.setText ("before", dontSendNotification);
            context.editor.setCaretPosition (context.editor.getTotalNumChars());
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("draft", 5, 5);
            expectEquals (context.editor.getText(), String { "before" });

            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "beforedraft" });

            context.input.handleCommitString ("final");
            expectEquals (context.editor.getText(), String { "beforedraft" });
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "beforefinal" });

            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "beforefinal" });
        });

        testCase ("The done event removes pre-edit before deleting surrounding text and inserting committed text", [&]
        {
            Context context;
            context.editor.setText (String::fromUTF8 (u8"aéz"), dontSendNotification);
            context.editor.setCaretPosition (2);
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("draft", 5, 5);
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String::fromUTF8 (u8"aédraftz"));

            context.input.handleDeleteSurroundingText (2, 1);
            context.input.handleCommitString ("X");
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "aX" });
        });

        testCase ("Rejected pre-edit leaves the surrounding text intact", [&]
        {
            Context context;
            context.editor.setInputRestrictions (0, "0123456789");
            context.editor.setText ("12", dontSendNotification);
            context.editor.setCaretPosition (0);
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("x", 1, 1);
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "12" });
            expectEquals (context.input.composition->range.getLength(), 0);
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "12" });
        });

        testCase ("Pre-edit cleanup removes only the characters accepted by the editor", [&]
        {
            Context context;
            context.editor.setInputRestrictions (0, "0123456789");
            context.editor.setText ("12", dontSendNotification);
            context.editor.setCaretPosition (0);
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("3x4", 3, 3);
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "3412" });
            expectEquals (context.editor.getCaretPosition(), 2);
            context.input.handleCommitString ("5");
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "512" });
        });

        testCase ("Pre-edit length follows newline normalization and selection replacement", [&]
        {
            Context context;
            context.editor.setMultiLine (true);
            context.editor.setText ("abcd", dontSendNotification);
            context.editor.setHighlightedRegion ({ 1, 3 });
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("x\r\ny", 4, 4);
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "ax\nyd" });
            expectEquals (context.input.composition->range.getLength(), 3);
            context.input.handleCommitString ("Z");
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "aZd" });
        });

        testCase ("Replacing the document during pre-edit does not delete the replacement", [&]
        {
            Context context;
            context.editor.setText ("before", dontSendNotification);
            context.editor.setCaretPosition (0);
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("draft", 5, 5);
            context.input.handleDone (0);
            context.editor.setText ("replacement", dontSendNotification);
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "replacement" });
        });

        testCase ("Surrounding deletion follows the caret and retains the remaining selection", [&]
        {
            for (const auto backwards : { false, true })
            {
                Context context;
                context.editor.setText ("abcde", dontSendNotification);
                context.editor.setCaretPosition (backwards ? 3 : 1);
                context.editor.setHighlightedRegion ({ 1, 3 });
                expectEquals (context.editor.getCaretPosition(), backwards ? 1 : 3);
                context.input.handleEnter (context.getSurface());
                context.input.handleDeleteSurroundingText (1, 1);
                context.input.handleDone (0);
                expectEquals (context.editor.getText(), String { backwards ? "cde" : "abe" });
                expectEquals (context.editor.getHighlightedText(), String { backwards ? "c" : "b" });
                context.input.handleCommitString ("X");
                context.input.handleDone (0);
                expectEquals (context.editor.getText(), String { backwards ? "Xde" : "aXe" });
            }
        });

        testCase ("Surrounding deletion counts UTF-8 characters on both sides of the cursor", [&]
        {
            Context context;
            context.editor.setText (String::fromUTF8 (u8"aé€£z"), dontSendNotification);
            context.editor.setCaretPosition (3);
            context.input.handleEnter (context.getSurface());
            context.input.handleDeleteSurroundingText (3, 2);
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String::fromUTF8 (u8"aéz"));
            expectEquals (context.editor.getCaretPosition(), 2);

            const auto allBytes = std::numeric_limits<uint32_t>::max();
            context.input.handleDeleteSurroundingText (allBytes, allBytes);
            context.input.handleDone (0);
            expect (context.editor.getText().isEmpty());
            expectEquals (context.editor.getCaretPosition(), 0);
        });

        testCase ("Long surrounding text keeps the selection and UTF-8 boundaries", [&]
        {
            const auto text = String::repeatedString (String::fromUTF8 (u8"é日😀"), 2000);
            for (const auto cursor : { 0, 3000, 6000 })
            {
                const auto anchor = jlimit (0, text.length(), cursor + (cursor == 6000 ? -3 : 3));
                const auto surrounding = WaylandTextInput::makeSurroundingText (text, cursor, anchor);
                expect (surrounding.has_value());
                if (! surrounding.has_value())
                    continue;
                expect (surrounding->text.getNumBytesAsUTF8() <= 4000);
                expect (CharPointer_UTF8::isValidString (surrounding->text.toRawUTF8(), (int) surrounding->text.getNumBytesAsUTF8()));
                const auto start = jmin (surrounding->cursor, surrounding->anchor);
                const auto end = jmax (surrounding->cursor, surrounding->anchor);
                expectEquals (String::fromUTF8 (surrounding->text.toRawUTF8() + start, end - start),
                              text.substring (jmin (cursor, anchor), jmax (cursor, anchor)));
                expect ((surrounding->cursor > surrounding->anchor) == (cursor > anchor));
            }
            expect (! WaylandTextInput::makeSurroundingText (text, 0, text.length()).has_value());
        });

        testCase ("A helper uses its focus surface and translated caret rectangle", [&]
        {
            Context context;
            int parentIdentity = 0;
            auto* parent = reinterpret_cast<wl_surface*> (&parentIdentity);
            Client parentClient;
            parentClient.focusSurface = parent;
            context.input.addClient (parent, parentClient);
            context.client.focusSurface = parent;
            context.client.caret = { 123, 234, 2, 20 };
            context.input.handleEnter (parent);
            expect (context.input.getActiveTarget() == &context.editor);
            expect (context.input.getTargetState (context.editor).caret == context.client.caret);
            context.input.handleCommitString ("helper");
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "helper" });
            context.input.removeClient (parentClient);
            expect (context.input.getActiveTarget() == nullptr);
            context.input.handleCommitString ("discarded");
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "helper" });
        });

        testCase ("Surrounding state excludes pre-edit and retains selection direction", [&]
        {
            Context context;
            context.editor.setText ("abcde", dontSendNotification);
            context.editor.setCaretPosition (3);
            context.editor.setHighlightedRegion ({ 1, 3 });
            const auto before = context.input.getTargetState (context.editor).surrounding;
            expect (before.has_value());
            expectEquals (before->cursor, 1);
            expectEquals (before->anchor, 3);
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("draft", 5, 5);
            context.input.handleDone (0);
            const auto during = context.input.getTargetState (context.editor).surrounding;
            expectEquals (during->text, String { "ade" });
            expectEquals (during->cursor, 1);
            expectEquals (during->anchor, 1);
        });

        testCase ("Local state updates are coalesced and wait for a matching done serial", [&]
        {
            Context context;
            context.bindFakeProxy();
            context.input.handleEnter (context.getSurface());
            expectEquals ((int) context.input.commitCount, 1);
            context.editor.setText ("local", dontSendNotification);
            context.input.refreshState (context.getSurface());
            context.input.refreshState (context.getSurface());
            expect (context.input.isUpdatePending());
            context.input.handleUpdateNowIfNeeded();
            expectEquals ((int) context.input.commitCount, 2);
            expectEquals (context.input.lastSentState->surrounding->text, String { "local" });
            context.input.refreshState (context.getSurface());
            context.input.handleUpdateNowIfNeeded();
            expectEquals ((int) context.input.commitCount, 2);
            expect (context.input.pendingChangeCause == WaylandProtocol::zwpTextInputV3ChangeCauseInputMethod);
            context.input.handleDone (1);
            context.editor.setText ("newer", dontSendNotification);
            context.input.refreshState (context.getSurface());
            context.input.handleUpdateNowIfNeeded();
            expectEquals ((int) context.input.commitCount, 2);
            expect (context.input.pendingChangeCause == WaylandProtocol::zwpTextInputV3ChangeCauseOther);
            context.input.handleDone (2);
            expectEquals ((int) context.input.commitCount, 3);
            expectEquals (context.input.lastSentState->surrounding->text, String { "newer" });
        });

        testCase ("Local edits are reported when done arrives before the scheduled refresh", [&]
        {
            Context context;
            context.bindFakeProxy();
            context.input.handleEnter (context.getSurface());
            context.editor.setText ("local", dontSendNotification);
            context.input.refreshState (context.getSurface());
            context.input.handleDone (0);
            expect (context.input.pendingChangeCause == WaylandProtocol::zwpTextInputV3ChangeCauseOther);
            expectEquals ((int) context.input.commitCount, 1);

            context.input.handleDone (1);
            expectEquals (context.input.lastSentState->surrounding->text, String { "local" });
            expectEquals ((int) context.input.commitCount, 2);
        });

        testCase ("Repainting a composition does not report a new external edit", [&]
        {
            Context context;
            context.bindFakeProxy();
            context.client.followCaret = true;
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("draft", 5, 5);
            context.input.handleDone (context.input.commitCount);
            const auto commits = context.input.commitCount;

            context.input.refreshState (context.getSurface());
            context.input.handleUpdateNowIfNeeded();
            expectEquals ((int) context.input.commitCount, (int) commits);
            expectEquals (context.editor.getText(), String { "draft" });

            context.input.handlePreeditString ("longer", 6, 6);
            context.input.handleDone (commits - 1);
            context.input.refreshState (context.getSurface());
            context.input.handleUpdateNowIfNeeded();
            expectEquals ((int) context.input.commitCount, (int) commits);
            expect (context.input.pendingChangeCause == WaylandProtocol::zwpTextInputV3ChangeCauseInputMethod);

            context.input.handlePreeditString ("longer", 6, 6);
            context.input.handleDone (commits);
            expectEquals ((int) context.input.commitCount, (int) commits + 1);
        });

        testCase ("Pre-edit cursor offsets stop at complete UTF-8 characters", [&]
        {
            const auto text = String::fromUTF8 (u8"日本");
            expectEquals (WaylandTextInput::getCodepointOffset (text, -1), 0);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 0), 0);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 1), 0);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 2), 0);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 3), 1);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 4), 1);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 5), 1);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 6), 2);
            expectEquals (WaylandTextInput::getCodepointOffset (text, 7), 2);
        });

        testCase ("Refreshes from unrelated surfaces and inactive input do not schedule updates", [&]
        {
            Context context;
            context.bindFakeProxy();
            context.input.refreshState (context.getSurface());
            expect (! context.input.isUpdatePending());

            context.input.handleEnter (context.getSurface());
            int unrelatedIdentity = 0;
            context.input.refreshState (reinterpret_cast<wl_surface*> (&unrelatedIdentity));
            expect (! context.input.isUpdatePending());
            expectEquals ((int) context.input.commitCount, 1);

            context.input.handleLeave();
            context.input.refreshState (context.getSurface());
            expect (! context.input.isUpdatePending());
        });

        testCase ("A refresh from the focused parent updates its subsurface editor", [&]
        {
            Context context;
            context.bindFakeProxy();
            int parentIdentity = 0;
            auto* parent = reinterpret_cast<wl_surface*> (&parentIdentity);
            Client parentClient;
            parentClient.focusSurface = parent;
            context.input.addClient (parent, parentClient);
            context.client.focusSurface = parent;
            context.input.handleEnter (parent);

            context.editor.setText ("edited", dontSendNotification);
            context.input.refreshState (parent);
            expect (context.input.isUpdatePending());
            context.input.handleUpdateNowIfNeeded();
            expectEquals (context.input.lastSentState->surrounding->text, String { "edited" });
            expectEquals ((int) context.input.commitCount, 2);

            context.input.removeClient (parentClient);
        });

        testCase ("Moving through repeated text sends an update even when the context window is unchanged", [&]
        {
            Context context;
            context.bindFakeProxy();
            context.editor.setText (String::repeatedString ("x", 5000), dontSendNotification);
            context.editor.setCaretPosition (2500);
            context.input.handleEnter (context.getSurface());
            const auto surrounding = context.input.lastSentState->surrounding;
            context.editor.setCaretPosition (2501);
            context.input.refreshState (context.getSurface());
            context.input.handleUpdateNowIfNeeded();
            expect (context.input.lastSentState->surrounding == surrounding);
            expectEquals ((int) context.input.commitCount, 2);
        });

        testCase ("An oversized selection drops surrounding state and a smaller one restores it", [&]
        {
            Context context;
            context.bindFakeProxy();
            context.editor.setText (String::repeatedString ("x", 5000), dontSendNotification);
            context.input.handleEnter (context.getSurface());
            expect (context.input.lastSentState->surrounding.has_value());
            context.editor.setHighlightedRegion ({ 0, 5000 });
            context.input.refreshState (context.getSurface());
            context.input.handleUpdateNowIfNeeded();
            expect (! context.input.lastSentState->surrounding.has_value());
            context.editor.setCaretPosition (2500);
            context.input.refreshState (context.getSurface());
            context.input.handleUpdateNowIfNeeded();
            expect (context.input.lastSentState->surrounding.has_value());
            expectEquals ((int) context.input.commitCount, 3);
        });

        testCase ("Removing a surface discards its pending text events", [&]
        {
            Context context;
            context.input.handleEnter (context.getSurface());
            context.input.handleCommitString ("discarded");
            context.input.removeClient (context.client);
            context.input.handleDone (0);
            expect (context.editor.getText().isEmpty());
            expect (context.input.getActiveTarget() == nullptr);

            context.input.addClient (context.getSurface(), context.client);
            context.input.requireInput (context.getSurface(), context.editor);
            context.input.handleEnter (context.getSurface());
            context.input.handleDone (0);
            expect (context.editor.getText().isEmpty());
        });

        testCase ("Removing a client removes pre-edit from a surviving editor", [&]
        {
            for (const auto isSubsurface : { false, true })
            {
                Context context;
                context.bindFakeProxy();
                int parentIdentity = 0;
                auto* parent = reinterpret_cast<wl_surface*> (&parentIdentity);
                Client parentClient;
                parentClient.focusSurface = parent;

                if (isSubsurface)
                {
                    context.input.addClient (parent, parentClient);
                    context.client.focusSurface = parent;
                }

                context.editor.setText ("before", dontSendNotification);
                context.editor.setCaretPosition (context.editor.getTotalNumChars());
                context.input.handleEnter (context.client.focusSurface);
                context.input.handlePreeditString ("draft", 5, 5);
                context.input.handleDone (context.input.commitCount);
                expectEquals (context.editor.getText(), String { "beforedraft" });

                context.input.handleCommitString ("discarded");
                context.input.removeClient (context.client);
                expectEquals (context.editor.getText(), String { "before" });
                expect (! context.input.enabled);
                expect (! context.input.composition.has_value());
                expect (context.input.getRequestedTarget() == nullptr);

                context.input.handleDone (context.input.commitCount);
                expectEquals (context.editor.getText(), String { "before" });

                if (isSubsurface)
                    context.input.removeClient (parentClient);
            }
        });

        testCase ("A target that loses focus does not receive pending text", [&]
        {
            Context context;
            context.input.handleEnter (context.getSurface());
            context.input.handleCommitString ("discarded");
            TextEditor otherEditor;
            context.client.target = &otherEditor;
            context.input.handleDone (0);
            expect (context.editor.getText().isEmpty());
            expect (otherEditor.getText().isEmpty());
            context.client.target = &context.editor;
        });

        testCase ("Destroying the requested target discards pending text", [&]
        {
            Context context;
            {
                TextEditor temporaryEditor;
                context.client.target = &temporaryEditor;
                context.input.requireInput (context.getSurface(), temporaryEditor);
                context.input.handleEnter (context.getSurface());
                context.input.handleCommitString ("discarded");
            }

            context.client.target = &context.editor;
            context.input.handleDone (0);
            expect (context.editor.getText().isEmpty());
            expect (context.input.getActiveTarget() == nullptr);
        });

        testCase ("Unbinding removes composition and clears the requested target", [&]
        {
            Context context;
            context.editor.setText ("before", dontSendNotification);
            context.editor.setCaretPosition (context.editor.getTotalNumChars());
            context.input.handleEnter (context.getSurface());
            context.input.handlePreeditString ("draft", 5, 5);
            context.input.handleDone (0);
            context.input.handleCommitString ("discarded");
            context.input.unbind();
            expectEquals (context.editor.getText(), String { "before" });
            expect (context.input.getActiveTarget() == nullptr);

            context.input.requireInput (context.getSurface(), context.editor);
            context.input.handleEnter (context.getSurface());
            context.input.handleDone (0);
            expectEquals (context.editor.getText(), String { "before" });
        });
    }

private:
    struct Delegate final : WaylandTextInputDelegate
    {
        void flush() override {}
    };

    struct Client final : WaylandTextInputClient
    {
        TextInputTarget* getTextInputTarget() override { return target; }
        wl_surface* getTextInputFocusSurface() const override { return focusSurface; }
        Rectangle<int> getTextInputCaretRectangle (TextInputTarget& inputTarget) const override
        {
            return followCaret ? caret.translated (inputTarget.getCaretPosition(), 0) : caret;
        }

        bool followCaret = false;
        wl_surface* focusSurface = nullptr;
        Rectangle<int> caret;
        TextInputTarget* target = nullptr;
    };

    struct StubbedWaylandRequests
    {
        static wl_proxy* marshal (wl_proxy*, uint32_t, const wl_interface*, uint32_t, uint32_t, ...) { return nullptr; }
        static uint32_t getVersion (wl_proxy*) { return 1; }

        ErasedScopeGuard deleteOwnedSymbols { [owned = WaylandClientSymbols::getInstanceWithoutCreating() == nullptr]
        {
            if (owned)
                WaylandClientSymbols::deleteInstance();
        } };
        ScopedValueSetter<decltype (WaylandClientSymbols::wl_proxy_marshal_flags)> replaceMarshal { WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags, marshal };
        ScopedValueSetter<decltype (WaylandClientSymbols::wl_proxy_get_version)> replaceGetVersion { WaylandClientSymbols::getInstance()->wl_proxy_get_version, getVersion };
    };

    struct Context
    {
        Context()
        {
            client.target = &editor;
            client.focusSurface = getSurface();
            input.addClient (getSurface(), client);
            input.requireInput (getSurface(), editor);
        }

        void bindFakeProxy()
        {
            if (! stubbedRequests.has_value())
                stubbedRequests.emplace();

            input.proxy.reset (reinterpret_cast<zwp_text_input_v3*> (&proxyIdentity));
        }

        wl_surface* getSurface() { return reinterpret_cast<wl_surface*> (&surfaceIdentity); }

        ScopedJuceInitialiser_GUI guiInitialiser;
        int surfaceIdentity = 0;
        int proxyIdentity = 0;
        TextEditor editor;
        Delegate delegate;
        Client client;
        std::optional<StubbedWaylandRequests> stubbedRequests;
        WaylandTextInput input { delegate };
    };
};

static WaylandTextInputTests waylandTextInputTests;

#endif

} // namespace juce
