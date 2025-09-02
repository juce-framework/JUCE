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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

//==============================================================================
struct WindowsAccessibility
{
    WindowsAccessibility() = delete;

    static long getUiaRootObjectId()
    {
        return static_cast<long> (UiaRootObjectId);
    }

    static bool handleWmGetObject (AccessibilityHandler* handler, WPARAM wParam, LPARAM lParam, LRESULT* res)
    {
        if (isStartingUpOrShuttingDown() || (handler == nullptr || ! isHandlerValid (*handler)))
            return false;

        if (auto* uiaWrapper = WindowsUIAWrapper::getInstance())
        {
            ComSmartPtr<IRawElementProviderSimple> provider;
            handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

            if (! uiaWrapper->isProviderDisconnecting (provider))
                *res = uiaWrapper->returnRawElementProvider ((HWND) handler->getComponent().getWindowHandle(), wParam, lParam, provider);

            return true;
        }

        return false;
    }

    static void revokeUIAMapEntriesForWindow (HWND hwnd)
    {
        if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
            uiaWrapper->returnRawElementProvider (hwnd, 0, 0, nullptr);
    }

    static bool isStartingUpOrShuttingDown()
    {
        if (auto* app = JUCEApplicationBase::getInstance())
            if (app->isInitialising())
                return true;

        if (auto* mm = MessageManager::getInstanceWithoutCreating())
            if (mm->hasStopMessageBeenSent())
                return true;

        return false;
    }

    static bool isHandlerValid (const AccessibilityHandler& handler)
    {
        if (auto* provider = handler.getNativeImplementation())
            return provider->isElementValid();

        return false;
    }

    static bool areAnyAccessibilityClientsActive()
    {
        const auto areClientsListening = []
        {
            if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
                return uiaWrapper->clientsAreListening() != 0;

            return false;
        };

        const auto isScreenReaderRunning = []
        {
            BOOL isRunning = FALSE;
            SystemParametersInfo (SPI_GETSCREENREADER, 0, (PVOID) &isRunning, 0);

            return isRunning != 0;
        };

        return areClientsListening() || isScreenReaderRunning();
    }
};

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& owner)
        : accessibilityElement (becomeComSmartPtrOwner (new AccessibilityNativeHandle (owner)))
    {
        ++providerCount;
    }

    ~AccessibilityNativeImpl()
    {
        ComSmartPtr<IRawElementProviderSimple> provider;
        accessibilityElement->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

        accessibilityElement->invalidateElement();
        --providerCount;

        if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
        {
            uiaWrapper->disconnectProvider (provider);

            if (providerCount == 0 && JUCEApplicationBase::isStandaloneApp())
                uiaWrapper->disconnectAllProviders();
        }
    }

    //==============================================================================
    ComSmartPtr<AccessibilityNativeHandle> accessibilityElement;
    static int providerCount;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeImpl)
};

int AccessibilityHandler::AccessibilityNativeImpl::providerCount = 0;

//==============================================================================
AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const
{
    return nativeImpl->accessibilityElement;
}

template <typename Callback>
void getProviderWithCheckedWrapper (const AccessibilityHandler& handler, Callback&& callback)
{
    if (! WindowsAccessibility::areAnyAccessibilityClientsActive()
        || WindowsAccessibility::isStartingUpOrShuttingDown()
        || ! WindowsAccessibility::isHandlerValid (handler))
        return;

    if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
    {
        ComSmartPtr<IRawElementProviderSimple> provider;
        handler.getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

        callback (uiaWrapper, provider);
    }
}

void sendAccessibilityAutomationEvent (const AccessibilityHandler& handler, EVENTID event)
{
    jassert (event != EVENTID{});

    getProviderWithCheckedWrapper (handler, [event] (WindowsUIAWrapper* uiaWrapper, ComSmartPtr<IRawElementProviderSimple>& provider)
    {
        uiaWrapper->raiseAutomationEvent (provider, event);
    });
}

void sendAccessibilityPropertyChangedEvent (const AccessibilityHandler& handler, PROPERTYID property, VARIANT newValue)
{
    jassert (property != PROPERTYID{});

    getProviderWithCheckedWrapper (handler, [property, newValue] (WindowsUIAWrapper* uiaWrapper, ComSmartPtr<IRawElementProviderSimple>& provider)
    {
        VARIANT oldValue;
        VariantHelpers::clear (&oldValue);

        uiaWrapper->raiseAutomationPropertyChangedEvent (provider, property, oldValue, newValue);
    });
}

void detail::AccessibilityHelpers::notifyAccessibilityEvent (const AccessibilityHandler& handler, Event eventType)
{
    if (eventType == Event::elementCreated
        || eventType == Event::elementDestroyed)
    {
        if (auto* parent = handler.getParent())
            sendAccessibilityAutomationEvent (*parent, UIA_LayoutInvalidatedEventId);

        return;
    }

    if (eventType == Event::windowOpened
        || eventType == Event::windowClosed)
    {
        if (auto* peer = handler.getComponent().getPeer())
            if ((peer->getStyleFlags() & ComponentPeer::windowHasTitleBar) == 0)
                return;
    }

    auto event = [eventType]() -> EVENTID
    {
        switch (eventType)
        {
            case Event::focusChanged:           return UIA_AutomationFocusChangedEventId;
            case Event::windowOpened:           return UIA_Window_WindowOpenedEventId;
            case Event::windowClosed:           return UIA_Window_WindowClosedEventId;
            case Event::elementCreated:
            case Event::elementDestroyed:
            case Event::elementMovedOrResized:  break;
        }

        return {};
    }();

    if (event != EVENTID{})
        sendAccessibilityAutomationEvent (handler, event);
}

void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    if (eventType == AccessibilityEvent::titleChanged)
    {
        VARIANT newValue;
        VariantHelpers::setString (getTitle(), &newValue);

        sendAccessibilityPropertyChangedEvent (*this, UIA_NamePropertyId, newValue);
        return;
    }

    if (eventType == AccessibilityEvent::valueChanged)
    {
        if (auto* valueInterface = getValueInterface())
        {
            const auto propertyType = getRole() == AccessibilityRole::slider ? UIA_RangeValueValuePropertyId
                                                                             : UIA_ValueValuePropertyId;

            const auto value = getRole() == AccessibilityRole::slider
                               ? VariantHelpers::getWithValue (valueInterface->getCurrentValue())
                               : VariantHelpers::getWithValue (valueInterface->getCurrentValueAsString());

            sendAccessibilityPropertyChangedEvent (*this, propertyType, value);
        }

        return;
    }

    auto event = [eventType]() -> EVENTID
    {
        switch (eventType)
        {
            case AccessibilityEvent::textSelectionChanged:  return UIA_Text_TextSelectionChangedEventId;
            case AccessibilityEvent::textChanged:           return UIA_Text_TextChangedEventId;
            case AccessibilityEvent::structureChanged:      return UIA_StructureChangedEventId;
            case AccessibilityEvent::rowSelectionChanged:   return UIA_SelectionItem_ElementSelectedEventId;
            case AccessibilityEvent::titleChanged:
            case AccessibilityEvent::valueChanged:          break;
        }

        return {};
    }();

    if (event != EVENTID{})
        sendAccessibilityAutomationEvent (*this, event);
}

struct SpVoiceWrapper final : public DeletedAtShutdown
{
    SpVoiceWrapper()
    {
        [[maybe_unused]] auto hr = voice.CoCreateInstance (CLSID_SpVoice);

        jassert (SUCCEEDED (hr));
    }

    ~SpVoiceWrapper() override
    {
        clearSingletonInstance();
    }

    ComSmartPtr<ISpVoice> voice;

    JUCE_DECLARE_SINGLETON_INLINE (SpVoiceWrapper, false)
};


void AccessibilityHandler::postAnnouncement (const String& announcementString, AnnouncementPriority priority)
{
    if (! areAnyAccessibilityClientsActive())
        return;

    if (auto* sharedVoice = SpVoiceWrapper::getInstance())
    {
        auto voicePriority = [priority]
        {
            switch (priority)
            {
                case AnnouncementPriority::low:    return SPVPRI_OVER;
                case AnnouncementPriority::medium: return SPVPRI_NORMAL;
                case AnnouncementPriority::high:   return SPVPRI_ALERT;
            }

            jassertfalse;
            return SPVPRI_OVER;
        }();

        sharedVoice->voice->SetPriority (voicePriority);
        sharedVoice->voice->Speak (announcementString.toWideCharPointer(), SPF_ASYNC, nullptr);
    }
}

bool AccessibilityHandler::areAnyAccessibilityClientsActive()
{
    return WindowsAccessibility::areAnyAccessibilityClientsActive();
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce
