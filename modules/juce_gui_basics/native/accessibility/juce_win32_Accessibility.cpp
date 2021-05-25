/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& owner)
        : accessibilityElement (new AccessibilityNativeHandle (owner))
    {
        ++providerCount;
    }

    ~AccessibilityNativeImpl()
    {
        accessibilityElement->invalidateElement();

        if (auto* wrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
        {
            ComSmartPtr<IRawElementProviderSimple> provider;
            accessibilityElement->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

            wrapper->disconnectProvider (provider);

            if (--providerCount == 0)
                wrapper->disconnectAllProviders();
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
    if (isStartingUpOrShuttingDown() || ! isHandlerValid (handler))
        return;

    if (auto* wrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
    {
        if (! wrapper->clientsAreListening())
            return;

        ComSmartPtr<IRawElementProviderSimple> provider;
        handler.getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

        callback (wrapper, provider);
    }
}

void sendAccessibilityAutomationEvent (const AccessibilityHandler& handler, EVENTID event)
{
    jassert (event != EVENTID{});

    getProviderWithCheckedWrapper (handler,  [event] (WindowsUIAWrapper* wrapper, ComSmartPtr<IRawElementProviderSimple>& provider)
    {
        wrapper->raiseAutomationEvent (provider, event);
    });
}

void sendAccessibilityPropertyChangedEvent (const AccessibilityHandler& handler, PROPERTYID property, VARIANT newValue)
{
    jassert (property != PROPERTYID{});

    getProviderWithCheckedWrapper (handler, [property, newValue] (WindowsUIAWrapper* wrapper, ComSmartPtr<IRawElementProviderSimple>& provider)
    {
        VARIANT oldValue;
        VariantHelpers::clear (&oldValue);

        wrapper->raiseAutomationPropertyChangedEvent (provider, property, oldValue, newValue);
    });
}

void notifyAccessibilityEventInternal (const AccessibilityHandler& handler, InternalAccessibilityEvent eventType)
{
    auto event = [eventType]() -> EVENTID
    {
        switch (eventType)
        {
            case InternalAccessibilityEvent::elementCreated:
            case InternalAccessibilityEvent::elementDestroyed:  return UIA_StructureChangedEventId;
            case InternalAccessibilityEvent::focusChanged:      return UIA_AutomationFocusChangedEventId;
            case InternalAccessibilityEvent::windowOpened:      return UIA_Window_WindowOpenedEventId;
            case InternalAccessibilityEvent::windowClosed:      return UIA_Window_WindowClosedEventId;
        }

        return {};
    }();

    if (event != EVENTID{})
        sendAccessibilityAutomationEvent (handler, event);
}

void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    auto event = [eventType] () -> EVENTID
    {
        switch (eventType)
        {
            case AccessibilityEvent::textSelectionChanged:  return UIA_Text_TextSelectionChangedEventId;
            case AccessibilityEvent::textChanged:           return UIA_Text_TextChangedEventId;
            case AccessibilityEvent::structureChanged:      return UIA_StructureChangedEventId;
            case AccessibilityEvent::rowSelectionChanged:   return UIA_SelectionItem_ElementSelectedEventId;
            case AccessibilityEvent::valueChanged:          break;
        }

        return {};
    }();

    if (event != EVENTID{})
        sendAccessibilityAutomationEvent (*this, event);
}

struct SpVoiceWrapper  : public DeletedAtShutdown
{
    SpVoiceWrapper()
    {
        auto hr = voice.CoCreateInstance (CLSID_SpVoice);

        jassert (SUCCEEDED (hr));
        ignoreUnused (hr);
    }

    ~SpVoiceWrapper() override
    {
        clearSingletonInstance();
    }

    ComSmartPtr<ISpVoice> voice;

    JUCE_DECLARE_SINGLETON (SpVoiceWrapper, false)
};

JUCE_IMPLEMENT_SINGLETON (SpVoiceWrapper)


void AccessibilityHandler::postAnnouncement (const String& announcementString, AnnouncementPriority priority)
{
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

AccessibilityHandler::AccessibilityNativeImpl* AccessibilityHandler::createNativeImpl (AccessibilityHandler& handler)
{
    return new AccessibilityHandler::AccessibilityNativeImpl (handler);
}

void AccessibilityHandler::DestroyNativeImpl::operator() (AccessibilityHandler::AccessibilityNativeImpl* impl) const noexcept
{
    delete impl;
}

//==============================================================================
namespace WindowsAccessibility
{
    void initialiseUIAWrapper()
    {
        WindowsUIAWrapper::getInstance();
    }

    long getUiaRootObjectId()
    {
        return static_cast<long> (UiaRootObjectId);
    }

    bool handleWmGetObject (AccessibilityHandler* handler, WPARAM wParam, LPARAM lParam, LRESULT* res)
    {
        if (isStartingUpOrShuttingDown() || (handler == nullptr || ! isHandlerValid (*handler)))
            return false;

        if (auto* wrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
        {
            ComSmartPtr<IRawElementProviderSimple> provider;
            handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

            if (! wrapper->isProviderDisconnecting (provider))
                *res = wrapper->returnRawElementProvider ((HWND) handler->getComponent().getWindowHandle(), wParam, lParam, provider);

            return true;
        }

        return false;
    }

    void revokeUIAMapEntriesForWindow (HWND hwnd)
    {
        if (auto* wrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
            wrapper->returnRawElementProvider (hwnd, 0, 0, nullptr);
    }
}


JUCE_IMPLEMENT_SINGLETON (WindowsUIAWrapper)

} // namespace juce
