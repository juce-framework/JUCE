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

static void juceFreeAccessibilityPlatformSpecificData (UIAccessibilityElement* element)
{
    if (auto* container = juce::getIvar<UIAccessibilityElement*> (element, "container"))
    {
        object_setInstanceVariable (element, "container", nullptr);
        object_setInstanceVariable (container, "handler", nullptr);

        [container release];
    }
}

namespace juce
{

#if defined (__IPHONE_11_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
 #define JUCE_IOS_CONTAINER_API_AVAILABLE 1
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability", "-Wunguarded-availability-new")

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

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

//==============================================================================
static NSArray* getContainerAccessibilityElements (AccessibilityHandler& handler)
{
    const auto children = handler.getChildren();

    NSMutableArray* accessibleChildren = [NSMutableArray arrayWithCapacity: (NSUInteger) children.size()];

    [accessibleChildren addObject: (id) handler.getNativeImplementation()];

    for (auto* childHandler : children)
    {
        id accessibleElement = [&childHandler]
        {
            id native = (id) childHandler->getNativeImplementation();

            if (childHandler->getChildren().size() > 0)
                return [native accessibilityContainer];

            return native;
        }();

        if (accessibleElement != nil)
            [accessibleChildren addObject: accessibleElement];
    }

    return accessibleChildren;
}

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& handler)
        : accessibilityElement (AccessibilityElement::create (handler))
    {
    }

    UIAccessibilityElement* getAccessibilityElement() const noexcept
    {
        return accessibilityElement.get();
    }

private:
    //==============================================================================
    class AccessibilityContainer  : public ObjCClass<UIAccessibilityElement>
    {
    public:
        AccessibilityContainer()
            : ObjCClass ("JUCEUIAccessibilityElementContainer_")
        {
            addMethod (@selector (isAccessibilityElement),     getIsAccessibilityElement);
            addMethod (@selector (accessibilityFrame),         getAccessibilityFrame);
            addMethod (@selector (accessibilityElements),      getAccessibilityElements);

           #if JUCE_IOS_CONTAINER_API_AVAILABLE
            if (@available (iOS 11.0, *))
                addMethod (@selector (accessibilityContainerType), getAccessibilityContainerType);
           #endif

            addIvar<AccessibilityHandler*> ("handler");

            registerClass();
        }

    private:
        static AccessibilityHandler* getHandler (id self)
        {
            return getIvar<AccessibilityHandler*> (self, "handler");
        }

        static BOOL getIsAccessibilityElement (id, SEL)
        {
            return NO;
        }

        static CGRect getAccessibilityFrame (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return convertToCGRect (handler->getComponent().getScreenBounds());

            return CGRectZero;
        }

        static NSArray* getAccessibilityElements (id self, SEL)
        {
            if (auto* handler = getHandler (self))
                return getContainerAccessibilityElements (*handler);

            return nil;
        }

        static NSInteger getAccessibilityContainerType (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (handler->getTableInterface() != nullptr)
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
    };

    //==============================================================================
    class AccessibilityElement  : public AccessibleObjCClass<UIAccessibilityElement>
    {
    public:
        enum class Type  { defaultElement, textElement };

        static Holder create (AccessibilityHandler& handler)
        {
            static AccessibilityElement cls     { Type::defaultElement };
            static AccessibilityElement textCls { Type::textElement };

            id instance = (hasEditableText (handler) ? textCls : cls).createInstance();

            Holder element ([instance initWithAccessibilityContainer: (id) handler.getComponent().getWindowHandle()]);
            object_setInstanceVariable (element.get(), "handler", &handler);
            return element;
        }

        AccessibilityElement (Type elementType)
        {
            addMethod (@selector (isAccessibilityElement),     getIsAccessibilityElement);
            addMethod (@selector (accessibilityContainer),     getAccessibilityContainer);
            addMethod (@selector (accessibilityFrame),         getAccessibilityFrame);
            addMethod (@selector (accessibilityTraits),        getAccessibilityTraits);
            addMethod (@selector (accessibilityLabel),         getAccessibilityTitle);
            addMethod (@selector (accessibilityHint),          getAccessibilityHelp);
            addMethod (@selector (accessibilityValue),         getAccessibilityValue);
            addMethod (@selector (setAccessibilityValue:),     setAccessibilityValue);

            addMethod (@selector (accessibilityElementDidBecomeFocused), onFocusGain);
            addMethod (@selector (accessibilityElementDidLoseFocus),     onFocusLoss);
            addMethod (@selector (accessibilityElementIsFocused),        isFocused);
            addMethod (@selector (accessibilityViewIsModal),             getIsAccessibilityModal);

            addMethod (@selector (accessibilityActivate),      accessibilityPerformActivate);
            addMethod (@selector (accessibilityIncrement),     accessibilityPerformIncrement);
            addMethod (@selector (accessibilityDecrement),     accessibilityPerformDecrement);
            addMethod (@selector (accessibilityPerformEscape), accessibilityPerformEscape);

           #if JUCE_IOS_CONTAINER_API_AVAILABLE
            if (@available (iOS 11.0, *))
            {
                addMethod (@selector (accessibilityDataTableCellElementForRow:column:), getAccessibilityDataTableCellElementForRowColumn);
                addMethod (@selector (accessibilityRowCount),                           getAccessibilityRowCount);
                addMethod (@selector (accessibilityColumnCount),                        getAccessibilityColumnCount);
                addProtocol (@protocol (UIAccessibilityContainerDataTable));

                addMethod (@selector (accessibilityRowRange),                           getAccessibilityRowIndexRange);
                addMethod (@selector (accessibilityColumnRange),                        getAccessibilityColumnIndexRange);
                addProtocol (@protocol (UIAccessibilityContainerDataTableCell));
            }
           #endif

            if (elementType == Type::textElement)
            {
                addMethod (@selector (accessibilityLineNumberForPoint:),   getAccessibilityLineNumberForPoint);
                addMethod (@selector (accessibilityContentForLineNumber:), getAccessibilityContentForLineNumber);
                addMethod (@selector (accessibilityFrameForLineNumber:),   getAccessibilityFrameForLineNumber);
                addMethod (@selector (accessibilityPageContent),           getAccessibilityPageContent);

                addProtocol (@protocol (UIAccessibilityReadingContent));
            }

            addIvar<UIAccessibilityElement*> ("container");

            registerClass();
        }

    private:
        //==============================================================================
        static UIAccessibilityElement* getContainer (id self)
        {
            return getIvar<UIAccessibilityElement*> (self, "container");
        }

        //==============================================================================
        static id getAccessibilityContainer (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (handler->getComponent().isOnDesktop())
                    return (id) handler->getComponent().getWindowHandle();

                if (handler->getChildren().size() > 0)
                {
                    if (UIAccessibilityElement* container = getContainer (self))
                        return container;

                    static AccessibilityContainer cls;

                    id windowHandle = (id) handler->getComponent().getWindowHandle();
                    UIAccessibilityElement* container = [cls.createInstance() initWithAccessibilityContainer: windowHandle];

                    [container retain];

                    object_setInstanceVariable (container, "handler", handler);
                    object_setInstanceVariable (self, "container", container);

                    return container;
                }

                if (auto* parent = handler->getParent())
                    return [(id) parent->getNativeImplementation() accessibilityContainer];
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
                        case AccessibilityRole::editableText:  return UIAccessibilityTraitKeyboardKey;
                        case AccessibilityRole::ignored:       return UIAccessibilityTraitNotEnabled;

                        case AccessibilityRole::slider:
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

                const auto state = handler->getCurrentState();

                if (state.isSelected() || state.isChecked())
                    traits |= UIAccessibilityTraitSelected;

                if (auto* valueInterface = getValueInterface (self))
                    if (! valueInterface->isReadOnly() && valueInterface->getRange().isValid())
                        traits |= UIAccessibilityTraitAdjustable;
            }

            return traits | sendSuperclassMessage<UIAccessibilityTraits> (self, @selector (accessibilityTraits));
        }

        static NSString* getAccessibilityValue (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (handler->getCurrentState().isCheckable())
                    return handler->getCurrentState().isChecked() ? @"1" : @"0";

                return (NSString*) getAccessibilityValueFromInterfaces (*handler);
            }

            return nil;
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

        static BOOL accessibilityPerformActivate (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                // Occasionally VoiceOver sends accessibilityActivate to the wrong element, so we first query
                // which element it thinks has focus and forward the event on to that element if it differs
                id focusedElement = UIAccessibilityFocusedElement (UIAccessibilityNotificationVoiceOverIdentifier);

                if (focusedElement != nullptr && ! [(id) handler->getNativeImplementation() isEqual: focusedElement])
                    return [focusedElement accessibilityActivate];

                if (handler->hasFocus (false))
                    return accessibilityPerformPress (self, {});
            }

            return NO;
        }

        static BOOL accessibilityPerformEscape (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                if (auto* modal = Component::getCurrentlyModalComponent())
                {
                    if (auto* modalHandler = modal->getAccessibilityHandler())
                    {
                        if (modalHandler == handler || modalHandler->isParentOf (handler))
                        {
                            modal->exitModalState (0);
                            return YES;
                        }
                    }
                }
            }

            return NO;
        }

        static id getAccessibilityDataTableCellElementForRowColumn (id self, SEL, NSUInteger row, NSUInteger column)
        {
            if (auto* tableInterface = getEnclosingInterface (getHandler (self), &AccessibilityHandler::getTableInterface))
                if (auto* cellHandler = tableInterface->getCellHandler ((int) row, (int) column))
                    return (id) cellHandler->getNativeImplementation();

            return nil;
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

static bool areAnyAccessibilityClientsActive()
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

    if (notification != UIAccessibilityNotifications{})
    {
        const bool moveToHandler = (eventType == InternalAccessibilityEvent::focusChanged && handler.hasFocus (false));

        sendAccessibilityEvent (notification,
                                moveToHandler ? (id) handler.getNativeImplementation() : nil);
    }

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
