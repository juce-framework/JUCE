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
struct WaylandTextInputDelegate
{
    virtual ~WaylandTextInputDelegate() = default;

    virtual void flush() = 0;
};

struct WaylandTextInputClient
{
    virtual ~WaylandTextInputClient() = default;

    virtual TextInputTarget* getTextInputTarget() = 0;
    virtual wl_surface* getTextInputFocusSurface() const = 0;
    virtual Rectangle<int> getTextInputCaretRectangle (TextInputTarget&) const = 0;
};

//==============================================================================
class WaylandTextInput final : private AsyncUpdater
{
public:
    explicit WaylandTextInput (WaylandTextInputDelegate&);
    ~WaylandTextInput() override;

   #if JUCE_WAYLAND_PEER_DIAGNOSTICS
    detail::WaylandPeerDiagnostics::TextInput getDiagnostics (wl_surface*) const;
   #endif

    void bind (zwp_text_input_manager_v3*, wl_seat*);
    void unbind();

    void addClient (wl_surface*, WaylandTextInputClient&);
    void removeClient (WaylandTextInputClient&);

    void requireInput (wl_surface*, TextInputTarget&);
    void refreshState (wl_surface*);
    void closeContext (wl_surface*);
    void dismissPendingInput (wl_surface*);

private:
    struct SurfaceClient
    {
        wl_surface* surface = nullptr;
        WaylandTextInputClient* client = nullptr;
    };

    struct SurroundingText
    {
        String text;
        int32_t cursor = 0;
        int32_t anchor = 0;

        bool operator== (const SurroundingText&) const;
    };

    struct TargetState
    {
        String text;
        int cursor = 0;
        int anchor = 0;
        std::optional<SurroundingText> surrounding;
        Rectangle<int> caret;
        TextInputTarget::VirtualKeyboardType keyboardType = TextInputTarget::textKeyboard;

        bool operator== (const TargetState&) const;
    };

    struct ContentType
    {
        uint32_t hint;
        uint32_t purpose;
    };

    struct PendingEvents
    {
        std::optional<String> preedit;
        std::optional<String> commit;
        int32_t preeditCursorBegin = 0;
        int32_t preeditCursorEnd = 0;
        uint32_t deleteBefore = 0;
        uint32_t deleteAfter = 0;
    };

    struct Composition
    {
        Range<int> range;
        String document;
    };

    static std::optional<SurroundingText> makeSurroundingText (const String&, int cursor, int anchor);
    TargetState getTargetState (TextInputTarget&) const;
    void handleAsyncUpdate() override;
    void detectExternalEdits (const TargetState&);
    bool targetHasFocus() const;
    WaylandTextInputClient* findClientForSurface (wl_surface*) const;
    void applyPendingEvents (TextInputTarget&);
    void handleEnter (wl_surface*);
    void handleLeave();
    void handlePreeditString (const char*, int32_t cursorBegin, int32_t cursorEnd);
    void handleCommitString (const char*);
    void handleDeleteSurroundingText (uint32_t beforeLength, uint32_t afterLength);
    void handleDone (uint32_t serial);
    void enable();
    void disable();
    void commitState();
    void sendTargetState (const TargetState&, bool includeContentType, uint32_t cause);
    static ContentType getContentType (TextInputTarget::VirtualKeyboardType);
    TextInputTarget* getRequestedTarget() const;
    TextInputTarget* getActiveTarget() const;
    void resetComposition();
    void discardChangedComposition (TextInputTarget&);
    void removeCurrentPreedit (TextInputTarget&);
    void insertPendingPreedit (TextInputTarget&);
    static int getCodepointOffset (const String&, int32_t utf8ByteOffset);
    static int getTrailingCodepointCount (const String&, uint32_t utf8ByteCount);
    static int getLeadingCodepointCount (const String&, uint32_t utf8ByteCount);
    static int shiftPositionPastRemovedRange (int, Range<int>);
    static void deleteSurroundingText (TextInputTarget&, uint32_t beforeLength, uint32_t afterLength);

    static const zwp_text_input_v3_listener listener;

    WaylandTextInputDelegate& delegate;
    std::vector<SurfaceClient> surfaceClients;
    std::unique_ptr<zwp_text_input_v3, FunctionPointerDestructor<WaylandProtocol::zwpTextInputV3Destroy>> proxy;
    wl_surface* focusSurface = nullptr;
    wl_surface* requestedSurface = nullptr;
    WeakReference<Component> requestedTarget;

    std::optional<TargetState> lastSentState;
    std::optional<TargetState> lastObservedState;
    bool applyingEvents = false;
    bool waitingForMatchingDone = false;
    uint32_t pendingChangeCause = WaylandProtocol::zwpTextInputV3ChangeCauseInputMethod;
    std::optional<Composition> composition;
    PendingEvents pending;
    uint32_t commitCount = 0;
    bool enabled = false;

   #if JUCE_UNIT_TESTS
    friend class WaylandTextInputTests;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandTextInput)
};

} // namespace juce
