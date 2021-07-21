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

#if (defined (__IPHONE_11_0) && __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_11_0)
 #define JUCE_IOS_CONTAINER_API_AVAILABLE 1
#endif

constexpr auto juceUIAccessibilityContainerTypeNone =
                   #if JUCE_IOS_CONTAINER_API_AVAILABLE
                    UIAccessibilityContainerTypeNone;
                   #else
                    0;
                   #endif

constexpr auto juceUIAccessibilityContainerTypeDataTable =
                   #if JUCE_IOS_CONTAINER_API_AVAILABLE
                    UIAccessibilityContainerTypeDataTable;
                   #else
                    1;
                   #endif

constexpr auto juceUIAccessibilityContainerTypeList =
                   #if JUCE_IOS_CONTAINER_API_AVAILABLE
                    UIAccessibilityContainerTypeList;
                   #else
                    2;
                   #endif

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& handler)
        : accessibilityElement (AccessibilityElement::create (handler))
    {}

    UIAccessibilityElement* getAccessibilityElement() const noexcept
    {
        return accessibilityElement.get();
    }

private:
    //==============================================================================
    class AccessibilityElement  : public AccessibleObjCClass<UIAccessibilityElement>
    {
    public:
        static Holder create (AccessibilityHandler& handler)
        {
            static AccessibilityElement cls;
            Holder element ([cls.createInstance() initWithAccessibilityContainer: (id) handler.getComponent().getWindowHandle()]);
            object_setInstanceVariable (element.get(), "handler", &handler);
            return element;
        }

    private:
        AccessibilityElement()
        {
            addMethod (@selector (isAccessibilityElement),     getIsAccessibilityElement,     "c@:");
            addMethod (@selector (accessibilityContainer),     getAccessibilityContainer,     "@@:");
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (accessibilityHitTest:),      accessibilityHitTest,          "@@:", @encode (CGPoint));
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            addMethod (@selector (accessibilityFrame),         getAccessibilityFrame,         @encode (CGRect), "@:");
            addMethod (@selector (accessibilityTraits),        getAccessibilityTraits,        "i@:");
            addMethod (@selector (accessibilityLabel),         getAccessibilityTitle,         "@@:");
            addMethod (@selector (accessibilityHint),          getAccessibilityHelp,          "@@:");
            addMethod (@selector (accessibilityValue),         getAccessibilityValue,         "@@:");
            addMethod (@selector (setAccessibilityValue:),     setAccessibilityValue,         "v@:@");
            addMethod (@selector (accessibilityElements),      getAccessibilityChildren,      "@@:");

            addMethod (@selector (accessibilityElementDidBecomeFocused), onFocusGain, "v@:");
            addMethod (@selector (accessibilityElementDidLoseFocus),     onFocusLoss, "v@:");
            addMethod (@selector (accessibilityElementIsFocused),        isFocused,   "c@:");
            addMethod (@selector (accessibilityViewIsModal),             getIsAccessibilityModal, "c@:");

            addMethod (@selector (accessibilityActivate),  accessibilityPerformPress,     "c@:");
            addMethod (@selector (accessibilityIncrement), accessibilityPerformIncrement, "c@:");
            addMethod (@selector (accessibilityDecrement), accessibilityPerformDecrement, "c@:");

            addMethod (@selector (accessibilityLineNumberForPoint:),   getAccessibilityLineNumberForPoint,   "i@:", @encode (CGPoint));
            addMethod (@selector (accessibilityContentForLineNumber:), getAccessibilityContentForLineNumber, "@@:i");
            addMethod (@selector (accessibilityFrameForLineNumber:),   getAccessibilityFrameForLineNumber,   @encode (CGRect), "@:i");
            addMethod (@selector (accessibilityPageContent),           getAccessibilityPageContent,          "@@:");

            addMethod (@selector (accessibilityContainerType), getAccessibilityContainerType, "i@:");

            addMethod (@selector (accessibilityDataTableCellElementForRow:column:), getAccessibilityDataTableCellElementForRowColumn, "@@:ii");
            addMethod (@selector (accessibilityRowCount),                           getAccessibilityRowCount,                         "i@:");
            addMethod (@selector (accessibilityColumnCount),                        getAccessibilityColumnCount,                      "i@:");
            addMethod (@selector (accessibilityRowRange),                           getAccessibilityRowIndexRange,                    @encode (NSRange), "@:");
            addMethod (@selector (accessibilityColumnRange),                        getAccessibilityColumnIndexRange,                 @encode (NSRange), "@:");

            addProtocol (@protocol (UIAccessibilityReadingContent));

            registerClass();
        }

        //==============================================================================
        static id getAccessibilityContainer (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* parent = handler->getParent())
                    return (id) parent->getNativeImplementation();

                return (id) handler->getComponent().getWindowHandle();
            }

            return nil;
        }

        static id accessibilityHitTest (id self, SEL, CGPoint point)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* child = handler->getChildAt (roundToIntPoint (point)))
                    return (id) child->getNativeImplementation();

                return self;
            }

            return nil;
        }

        static CGRect getAccessibilityFrame (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return convertToCGRect (handler->getComponent().getScreenBounds());

            return CGRectZero;
        }

        static UIAccessibilityTraits getAccessibilityTraits (id self, SEL)
        {
            auto traits = UIAccessibilityTraits{};

            if (auto* handler = getHandler (self))
            {
                traits |= [&handler]
                {
                    switch (handler->getRole())
                    {
                        case AccessibilityRole::button:
                        case AccessibilityRole::toggleButton:
                        case AccessibilityRole::radioButton:
                        case AccessibilityRole::comboBox:      return UIAccessibilityTraitButton;

                        case AccessibilityRole::label:
                        case AccessibilityRole::staticText:    return UIAccessibilityTraitStaticText;

                        case AccessibilityRole::image:         return UIAccessibilityTraitImage;
                        case AccessibilityRole::tableHeader:   return UIAccessibilityTraitHeader;
                        case AccessibilityRole::hyperlink:     return UIAccessibilityTraitLink;
                        case AccessibilityRole::ignored:       return UIAccessibilityTraitNotEnabled;

                        case AccessibilityRole::slider:
                        case AccessibilityRole::editableText:
                        case AccessibilityRole::menuItem:
                        case AccessibilityRole::menuBar:
                        case AccessibilityRole::popupMenu:
                        case AccessibilityRole::table:
                        case AccessibilityRole::column:
                        case AccessibilityRole::row:
                        case AccessibilityRole::cell:
                        case AccessibilityRole::list:
                        case AccessibilityRole::listItem:
                        case AccessibilityRole::tree:
                        case AccessibilityRole::treeItem:
                        case AccessibilityRole::progressBar:
                        case AccessibilityRole::group:
                        case AccessibilityRole::dialogWindow:
                        case AccessibilityRole::window:
                        case AccessibilityRole::scrollBar:
                        case AccessibilityRole::tooltip:
                        case AccessibilityRole::splashScreen:
                        case AccessibilityRole::unspecified:   break;
                    }

                    return UIAccessibilityTraitNone;
                }();

                if (handler->getCurrentState().isSelected())
                    traits |= UIAccessibilityTraitSelected;

                if (auto* valueInterface = getValueInterface (self))
                    if (! valueInterface->isReadOnly() && valueInterface->getRange().isValid())
                        traits |= UIAccessibilityTraitAdjustable;
            }

            return traits | sendSuperclassMessage<UIAccessibilityTraits> (self, @selector (accessibilityTraits));
        }

        static void onFocusGain (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                const WeakReference<Component> safeComponent (&handler->getComponent());

                performActionIfSupported (self, AccessibilityActionType::focus);

                if (safeComponent != nullptr)
                    handler->grabFocus();
            }
        }

        static void onFocusLoss (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                handler->giveAwayFocus();
        }

        static BOOL isFocused (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return handler->hasFocus (false);

            return NO;
        }

        static NSInteger getAccessibilityLineNumberForPoint (id self, SEL, CGPoint point)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* textInterface = handler->getTextInterface())
                {
                    auto pointInt = roundToIntPoint (point);

                    if (handler->getComponent().getScreenBounds().contains (pointInt))
                    {
                        auto textBounds = textInterface->getTextBounds ({ 0, textInterface->getTotalNumCharacters() });

                        for (int i = 0; i < textBounds.getNumRectangles(); ++i)
                            if (textBounds.getRectangle (i).contains (pointInt))
                                return (NSInteger) i;
                    }
                }
            }

            return NSNotFound;
        }

        static NSString* getAccessibilityContentForLineNumber (id self, SEL, NSInteger lineNumber)
        {
            if (auto* textInterface = getTextInterface (self))
            {
                auto lines = StringArray::fromLines (textInterface->getText ({ 0, textInterface->getTotalNumCharacters() }));

                if ((int) lineNumber < lines.size())
                    return juceStringToNS (lines[(int) lineNumber]);
            }

            return nil;
        }

        static CGRect getAccessibilityFrameForLineNumber (id self, SEL, NSInteger lineNumber)
        {
            if (auto* textInterface = getTextInterface (self))
            {
                auto textBounds = textInterface->getTextBounds ({ 0, textInterface->getTotalNumCharacters() });

                if (lineNumber < textBounds.getNumRectangles())
                    return convertToCGRect (textBounds.getRectangle ((int) lineNumber));
            }

            return CGRectZero;
        }

        static NSString* getAccessibilityPageContent (id self, SEL)
        {
            if (auto* textInterface = getTextInterface (self))
                return juceStringToNS (textInterface->getText ({ 0, textInterface->getTotalNumCharacters() }));

            return nil;
        }

        static NSInteger getAccessibilityContainerType (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (getTableInterface (self) != nullptr)
                    return juceUIAccessibilityContainerTypeDataTable;

                const auto role = handler->getRole();

                if (role == AccessibilityRole::popupMenu
                    || role == AccessibilityRole::list
                    || role == AccessibilityRole::tree)
                {
                    return juceUIAccessibilityContainerTypeList;
                }
            }

            return juceUIAccessibilityContainerTypeNone;
        }

        static id getAccessibilityDataTableCellElementForRowColumn (id self, SEL, NSUInteger row, NSUInteger column)
        {
            if (auto* tableInterface = getTableInterface (self))
                if (auto* cellHandler = tableInterface->getCellHandler ((int) row, (int) column))
                    return (id) cellHandler->getNativeImplementation();

            return nil;
        }

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityElement)
    };

    //==============================================================================
    AccessibilityElement::Holder accessibilityElement;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeImpl)
};

//==============================================================================
AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const
{
    return (AccessibilityNativeHandle*) nativeImpl->getAccessibilityElement();
}

bool areAnyAccessibilityClientsActive()
{
    return UIAccessibilityIsVoiceOverRunning();
}

static void sendAccessibilityEvent (UIAccessibilityNotifications notification, id argument)
{
    if (! areAnyAccessibilityClientsActive())
        return;

    jassert (notification != UIAccessibilityNotifications{});

    UIAccessibilityPostNotification (notification, argument);
}

void notifyAccessibilityEventInternal (const AccessibilityHandler& handler, InternalAccessibilityEvent eventType)
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case InternalAccessibilityEvent::elementCreated:
            case InternalAccessibilityEvent::elementDestroyed:
            case InternalAccessibilityEvent::elementMovedOrResized:
            case InternalAccessibilityEvent::focusChanged:           return UIAccessibilityLayoutChangedNotification;

            case InternalAccessibilityEvent::windowOpened:
            case InternalAccessibilityEvent::windowClosed:           return UIAccessibilityScreenChangedNotification;
        }

        return UIAccessibilityNotifications{};
    }();

    const auto shouldMoveToHandler = (eventType == InternalAccessibilityEvent::elementCreated
                                      || eventType == InternalAccessibilityEvent::windowOpened
                                      || (eventType == InternalAccessibilityEvent::focusChanged && handler.hasFocus (false)));

    if (notification != UIAccessibilityNotifications{})
        sendAccessibilityEvent (notification, shouldMoveToHandler ? (id) handler.getNativeImplementation() : nil);
}

void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case AccessibilityEvent::textSelectionChanged:
            case AccessibilityEvent::rowSelectionChanged:
            case AccessibilityEvent::textChanged:
            case AccessibilityEvent::valueChanged:
            case AccessibilityEvent::titleChanged:          break;

            case AccessibilityEvent::structureChanged:      return UIAccessibilityLayoutChangedNotification;
        }

        return UIAccessibilityNotifications{};
    }();

    if (notification != UIAccessibilityNotifications{})
        sendAccessibilityEvent (notification, (id) getNativeImplementation());
}

void AccessibilityHandler::postAnnouncement (const String& announcementString, AnnouncementPriority)
{
    sendAccessibilityEvent (UIAccessibilityAnnouncementNotification, juceStringToNS (announcementString));
}

} // namespace juce
