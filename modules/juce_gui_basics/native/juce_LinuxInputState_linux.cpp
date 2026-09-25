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

LinuxInputState& LinuxInputState::get()
{
    static LinuxInputState state;
    return state;
}

void LinuxInputState::pointerEntered (LinuxInputBackend backend)
{
    if (pointerBackend.has_value() && *pointerBackend != backend)
        ModifierKeys::currentModifiers = ModifierKeys::getCurrentModifiers().withoutMouseButtons();

    pointerBackend = backend;
}

void LinuxInputState::keyboardEntered (LinuxInputBackend backend)
{
    keyboardBackend = backend;
}

void LinuxInputState::pointerLeft (LinuxInputBackend backend)
{
    if (pointerBackend == backend)
        pointerBackend.reset();
}

void LinuxInputState::keyboardLeft (LinuxInputBackend backend)
{
    if (keyboardBackend == backend)
        keyboardBackend.reset();
}

bool LinuxInputState::mayUpdatePointerState (LinuxInputBackend backend) const noexcept
{
    return pointerBackend.value_or (backend) == backend;
}

bool LinuxInputState::mayUpdateKeyboardState (LinuxInputBackend backend) const noexcept
{
    return keyboardBackend.value_or (backend) == backend;
}

ModifierKeys getLinuxRealtimeModifiers (LinuxInputBackend fallback)
{
    const auto x11Holds = [fallback] (std::optional<LinuxInputBackend> backend)
    {
        return backend.value_or (fallback) == LinuxInputBackend::x11;
    };

    const auto& state = LinuxInputState::get();

    // Wayland has no query, and its cached state is exact while it holds a device.
    // The fallback applies while no window has gained one.
    if (x11Holds (state.getPointerBackend()) || x11Holds (state.getKeyboardBackend()))
        return XWindowSystem::getInstance()->getNativeRealtimeModifiers();

    return ModifierKeys::getCurrentModifiers();
}

//==============================================================================
#if JUCE_UNIT_TESTS

class LinuxInputStateTests final : public UnitTest
{
public:
    LinuxInputStateTests()
        : UnitTest ("LinuxInputState", UnitTestCategories::gui) {}

    void runTest() override
    {
        const ModifierKeys left { ModifierKeys::leftButtonModifier };
        const ModifierKeys control { ModifierKeys::ctrlModifier };

        // The realtime query is only asked with a Wayland fallback, since X11 would open a display.
        const auto testState = [&] (const String& testName, auto&& test)
        {
            testCase (testName, [&]
            {
                const ScopedValueSetter modifierScope { ModifierKeys::currentModifiers, ModifierKeys{} };
                const ScopedValueSetter inputStateScope { LinuxInputState::get(), LinuxInputState{} };
                test (LinuxInputState::get());
            });
        };

        testState ("Any backend may update a device that no window has gained", [&] (const LinuxInputState& state)
        {
            expect (! state.getPointerBackend().has_value());
            expect (! state.getKeyboardBackend().has_value());
            expect (state.mayUpdatePointerState (LinuxInputBackend::x11));
            expect (state.mayUpdatePointerState (LinuxInputBackend::wayland));
            expect (state.mayUpdateKeyboardState (LinuxInputBackend::x11));
            expect (state.mayUpdateKeyboardState (LinuxInputBackend::wayland));
        });

        testState ("The pointer and the keyboard are tracked independently", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::x11);
            state.keyboardEntered (LinuxInputBackend::wayland);

            expect (state.getPointerBackend() == LinuxInputBackend::x11);
            expect (state.getKeyboardBackend() == LinuxInputBackend::wayland);
            expect (state.mayUpdatePointerState (LinuxInputBackend::x11));
            expect (! state.mayUpdatePointerState (LinuxInputBackend::wayland));
            expect (! state.mayUpdateKeyboardState (LinuxInputBackend::x11));
            expect (state.mayUpdateKeyboardState (LinuxInputBackend::wayland));
        });

        testState ("The most recent enter wins", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::wayland);
            state.pointerEntered (LinuxInputBackend::x11);
            expect (state.getPointerBackend() == LinuxInputBackend::x11);
            expect (! state.mayUpdatePointerState (LinuxInputBackend::wayland));

            state.pointerEntered (LinuxInputBackend::wayland);
            expect (state.getPointerBackend() == LinuxInputBackend::wayland);
            expect (! state.mayUpdatePointerState (LinuxInputBackend::x11));
        });

        testState ("A device is only forgotten by the backend that holds it", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::x11);
            state.keyboardEntered (LinuxInputBackend::x11);

            state.pointerLeft (LinuxInputBackend::wayland);
            state.keyboardLeft (LinuxInputBackend::wayland);
            expect (state.getPointerBackend() == LinuxInputBackend::x11);
            expect (state.getKeyboardBackend() == LinuxInputBackend::x11);

            state.pointerLeft (LinuxInputBackend::x11);
            expect (! state.getPointerBackend().has_value());
            expect (state.getKeyboardBackend() == LinuxInputBackend::x11);
            expect (state.mayUpdatePointerState (LinuxInputBackend::wayland));

            state.keyboardLeft (LinuxInputBackend::x11);
            expect (! state.getKeyboardBackend().has_value());
        });

        testState ("Updating X11 modifiers does not release a Wayland mouse button", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::wayland);
            ModifierKeys::currentModifiers = left;

            expect (! state.mayUpdatePointerState (LinuxInputBackend::x11));
            expect (getLinuxRealtimeModifiers (LinuxInputBackend::wayland).isLeftButtonDown());
        });

        testState ("Entering a Wayland window clears an unreleased X11 mouse button", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::x11);
            ModifierKeys::currentModifiers = left.withFlags (ModifierKeys::shiftModifier);

            state.pointerEntered (LinuxInputBackend::wayland);
            expect (! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown());
            expect (ModifierKeys::getCurrentModifiers().isShiftDown());
            expect (state.getPointerBackend() == LinuxInputBackend::wayland);
            expect (state.mayUpdatePointerState (LinuxInputBackend::wayland));
        });

        testState ("Entering another window of the same backend keeps its held mouse button", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::x11);
            ModifierKeys::currentModifiers = left;

            state.pointerEntered (LinuxInputBackend::x11);
            expect (ModifierKeys::getCurrentModifiers().isLeftButtonDown());
        });

        testState ("Keyboard focus and pointer input may belong to different backends", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::wayland);
            state.keyboardEntered (LinuxInputBackend::x11);

            expect (state.getPointerBackend() == LinuxInputBackend::wayland);
            expect (state.getKeyboardBackend() == LinuxInputBackend::x11);
            expect (state.mayUpdatePointerState (LinuxInputBackend::wayland));
            expect (! state.mayUpdatePointerState (LinuxInputBackend::x11));
            expect (state.mayUpdateKeyboardState (LinuxInputBackend::x11));
            expect (! state.mayUpdateKeyboardState (LinuxInputBackend::wayland));
        });

        testState ("Losing Wayland keyboard focus after X11 gains focus preserves X11 modifiers", [&] (LinuxInputState& state)
        {
            state.keyboardEntered (LinuxInputBackend::wayland);
            state.keyboardEntered (LinuxInputBackend::x11);
            ModifierKeys::currentModifiers = control;

            state.keyboardLeft (LinuxInputBackend::wayland);
            expect (ModifierKeys::getCurrentModifiers() == control);
            expect (state.getKeyboardBackend() == LinuxInputBackend::x11);
            expect (! state.mayUpdateKeyboardState (LinuxInputBackend::wayland));
        });

        testState ("Forgetting the Wayland pointer allows X11 pointer updates", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::wayland);
            ModifierKeys::currentModifiers = left;

            state.pointerLeft (LinuxInputBackend::wayland);
            expect (state.mayUpdatePointerState (LinuxInputBackend::x11));
            expect (state.mayUpdatePointerState (LinuxInputBackend::wayland));
        });

        testState ("Forgetting inactive Wayland devices preserves X11 buttons and keyboard modifiers", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::x11);
            state.keyboardEntered (LinuxInputBackend::x11);
            ModifierKeys::currentModifiers = left.withFlags (ModifierKeys::ctrlModifier);

            state.pointerLeft (LinuxInputBackend::wayland);
            state.keyboardLeft (LinuxInputBackend::wayland);
            expect (ModifierKeys::getCurrentModifiers() == left.withFlags (ModifierKeys::ctrlModifier));
            expect (state.getPointerBackend() == LinuxInputBackend::x11);
            expect (state.getKeyboardBackend() == LinuxInputBackend::x11);
        });

        testState ("An X11 pointer leave after a Wayland enter preserves the Wayland pointer backend", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::x11);
            state.pointerEntered (LinuxInputBackend::wayland);

            state.pointerLeft (LinuxInputBackend::x11);
            expect (state.getPointerBackend() == LinuxInputBackend::wayland);

            state.pointerLeft (LinuxInputBackend::wayland);
            expect (! state.getPointerBackend().has_value());
        });

        testState ("Forgetting the X11 pointer does not block Wayland input", [&] (LinuxInputState& state)
        {
            state.pointerEntered (LinuxInputBackend::x11);
            state.pointerLeft (LinuxInputBackend::x11);
            expect (state.mayUpdatePointerState (LinuxInputBackend::wayland));
        });
    }
};

static LinuxInputStateTests linuxInputStateTests;

#endif

} // namespace juce
