/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::detail
{

struct AccessibilityHelpers
{
    AccessibilityHelpers() = delete;

    enum class Event
    {
        elementCreated,
        elementDestroyed,
        elementMovedOrResized,
        focusChanged,
        windowOpened,
        windowClosed
    };

    static void notifyAccessibilityEvent (const AccessibilityHandler&, Event);

    static String getApplicationOrPluginName()
    {
       #if defined (JucePlugin_Name)
        return JucePlugin_Name;
       #else
        if (auto* app = JUCEApplicationBase::getInstance())
            return app->getApplicationName();

        return "JUCE Application";
       #endif
    }

    template <typename MemberFn>
    static const AccessibilityHandler* getEnclosingHandlerWithInterface (const AccessibilityHandler* handler, MemberFn fn)
    {
        if (handler == nullptr)
            return nullptr;

        if ((handler->*fn)() != nullptr)
            return handler;

        return getEnclosingHandlerWithInterface (handler->getParent(), fn);
    }
};

} // namespace juce::detail
