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

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

//==============================================================================
static NSArray* getContainerAccessibilityElements (AccessibilityHandler& handler)
{
    const auto children = handler.getChildren();

    NSMutableArray* accessibleChildren = [NSMutableArray arrayWithCapacity: (NSUInteger) children.size()];

    for (auto* childHandler : children)
    {
        id accessibleElement = [&childHandler]
        {
            id native = static_cast<id> (childHandler->getNativeImplementation());

            if (! childHandler->getChildren().empty())
                return [native accessibilityContainer];

            return native;
        }();

        if (accessibleElement != nil)
            [accessibleChildren addObject: accessibleElement];
    }

    [accessibleChildren addObject: static_cast<id> (handler.getNativeImplementation())];

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
    class AccessibilityContainer  : public AccessibleObjCClass<NSObject>
    {
    public:
        AccessibilityContainer()
            : AccessibleObjCClass ("JUCEUIAccessibilityContainer_")
        {
            addMethod (@selector (isAccessibilityElement), [] (id, SEL) { return false; });

            addMethod (@selector (accessibilityFrame), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                    return convertToCGRect (handler->getComponent().getScreenBounds());

                return CGRectZero;
            });

            addMethod (@selector (accessibilityElements), [] (id self, SEL) -> NSArray*
            {
                if (auto* handler = getHandler (self))
                    return getContainerAccessibilityElements (*handler);

                return nil;
            });

           #if JUCE_IOS_CONTAINER_API_AVAILABLE
            if (@available (iOS 11.0, *))
            {
                addMethod (@selector (accessibilityDataTableCellElementForRow:column:), [] (id self, SEL, NSUInteger row, NSUInteger column) -> id
                {
                    if (auto* tableHandler = getEnclosingHandlerWithInterface (getHandler (self), &AccessibilityHandler::getTableInterface))
                        if (auto* tableInterface = tableHandler->getTableInterface())
                            if (auto* cellHandler = tableInterface->getCellHandler ((int) row, (int) column))
                                if (auto* parent = getAccessibleParent (cellHandler))
                                    return static_cast<id> (parent->getNativeImplementation());

                    return nil;
                });

                addMethod (@selector (accessibilityRowCount),                           getAccessibilityRowCount);
                addMethod (@selector (accessibilityColumnCount),                        getAccessibilityColumnCount);

                addMethod (@selector (accessibilityHeaderElementsForColumn:), [] (id self, SEL, NSUInteger column) -> NSArray*
                {
                    if (auto* tableHandler = getEnclosingHandlerWithInterface (getHandler (self), &AccessibilityHandler::getTableInterface))
                    {
                        if (auto* tableInterface = tableHandler->getTableInterface())
                        {
                            if (auto* header = tableInterface->getHeaderHandler())
                            {
                                if (isPositiveAndBelow (column, header->getChildren().size()))
                                {
                                    auto* result = [NSMutableArray new];
                                    [result addObject: static_cast<id> (header->getChildren()[(size_t) column]->getNativeImplementation())];
                                    return result;
                                }
                            }
                        }
                    }

                    return nullptr;
                });

                addProtocol (@protocol (UIAccessibilityContainerDataTable));

                addMethod (@selector (accessibilityContainerType), [] (id self, SEL) -> NSInteger
                {
                    if (auto* handler = getHandler (self))
                    {
                        if (handler->getTableInterface() != nullptr)
                        {
                            if (@available (iOS 11.0, *))
                                return UIAccessibilityContainerTypeDataTable;

                            return 1; // UIAccessibilityContainerTypeDataTable
                        }

                        const auto handlerRole = handler->getRole();

                        if (handlerRole == AccessibilityRole::popupMenu
                            || handlerRole == AccessibilityRole::list
                            || handlerRole == AccessibilityRole::tree)
                        {
                            if (@available (iOS 11.0, *))
                                return UIAccessibilityContainerTypeList;

                            return 2; // UIAccessibilityContainerTypeList
                        }
                    }

                    if (@available (iOS 11.0, *))
                        return UIAccessibilityContainerTypeNone;

                    return 0; // UIAccessibilityContainerTypeNone
                });
            }
           #endif

            registerClass();
        }

    private:
        static const AccessibilityHandler* getAccessibleParent (const AccessibilityHandler* h)
        {
            if (h == nullptr)
                return nullptr;

            if ([static_cast<id> (h->getNativeImplementation()) isAccessibilityElement])
                return h;

            return getAccessibleParent (h->getParent());
        }

        static AccessibilityHandler* getHandler (id self)
        {
            return getIvar<AccessibilityHandler*> (self, "handler");
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

            Holder element ([instance initWithAccessibilityContainer: static_cast<id> (handler.getComponent().getWindowHandle())]);
            object_setInstanceVariable (element.get(), "handler", &handler);
            return element;
        }

        AccessibilityElement (Type elementType)
        {
            addMethod (@selector (isAccessibilityElement), [] (id self, SEL)
            {
                auto* handler = getHandler (self);

                const auto hasAccessiblePropertiesOrIsTableCell = [] (auto& handlerRef)
                {
                    const auto isTableCell = [&]
                    {
                        if (auto* tableHandler = getEnclosingHandlerWithInterface (&handlerRef, &AccessibilityHandler::getTableInterface))
                        {
                            if (auto* tableInterface = tableHandler->getTableInterface())
                            {
                                return tableInterface->getRowSpan    (handlerRef).hasValue()
                                    && tableInterface->getColumnSpan (handlerRef).hasValue();
                            }
                        }

                        return false;
                    };

                    return handlerRef.getTitle().isNotEmpty()
                        || handlerRef.getHelp().isNotEmpty()
                        || handlerRef.getTextInterface()  != nullptr
                        || handlerRef.getValueInterface() != nullptr
                        || isTableCell();
                };

                return handler != nullptr
                    && ! handler->isIgnored()
                    && handler->getRole() != AccessibilityRole::window
                    && hasAccessiblePropertiesOrIsTableCell (*handler);
            });

            addMethod (@selector (accessibilityContainer), [] (id self, SEL) -> id
            {
                if (auto* handler = getHandler (self))
                {
                    if (handler->getComponent().isOnDesktop())
                        return static_cast<id> (handler->getComponent().getWindowHandle());

                    if (! handler->getChildren().empty())
                    {
                        if (UIAccessibilityElement* container = getContainer (self))
                            return container;

                        static AccessibilityContainer cls;

                        id container = cls.createInstance();

                        object_setInstanceVariable (container, "handler", handler);
                        object_setInstanceVariable (self, "container", container);

                        return container;
                    }

                    if (auto* parent = handler->getParent())
                        return [static_cast<id> (parent->getNativeImplementation()) accessibilityContainer];
                }

                return nil;
            });

            addMethod (@selector (accessibilityFrame), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                    return convertToCGRect (handler->getComponent().getScreenBounds());

                return CGRectZero;
            });

            addMethod (@selector (accessibilityTraits), [] (id self, SEL)
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

                            case AccessibilityRole::editableText:
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
            });

            addMethod (@selector (accessibilityLabel), getAccessibilityTitle);
            addMethod (@selector (accessibilityHint),  getAccessibilityHelp);

            addMethod (@selector (accessibilityValue), [] (id self, SEL) -> NSString*
            {
                if (auto* handler = getHandler (self))
                {
                    if (handler->getCurrentState().isCheckable())
                        return handler->getCurrentState().isChecked() ? @"1" : @"0";

                    return (NSString*) getAccessibilityValueFromInterfaces (*handler);
                }

                return nil;
            });

            addMethod (@selector (setAccessibilityValue:), setAccessibilityValue);

            addMethod (@selector (accessibilityElementDidBecomeFocused), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                {
                    const WeakReference<Component> safeComponent (&handler->getComponent());

                    performActionIfSupported (self, AccessibilityActionType::focus);

                    if (safeComponent != nullptr)
                        handler->grabFocus();
                }
            });

            addMethod (@selector (accessibilityElementDidLoseFocus), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                    handler->giveAwayFocus();
            });

            addMethod (@selector (accessibilityElementIsFocused), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                    return handler->hasFocus (false);

                return NO;
            });

            addMethod (@selector (accessibilityViewIsModal), getIsAccessibilityModal);

            addMethod (@selector (accessibilityActivate), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                {
                    // Occasionally VoiceOver sends accessibilityActivate to the wrong element, so we first query
                    // which element it thinks has focus and forward the event on to that element if it differs
                    id focusedElement = UIAccessibilityFocusedElement (UIAccessibilityNotificationVoiceOverIdentifier);

                    if (focusedElement != nullptr && ! [static_cast<id> (handler->getNativeImplementation()) isEqual: focusedElement])
                        return [focusedElement accessibilityActivate];

                    if (handler->hasFocus (false))
                        return accessibilityPerformPress (self, {});
                }

                return NO;
            });

            addMethod (@selector (accessibilityIncrement), accessibilityPerformIncrement);
            addMethod (@selector (accessibilityDecrement), accessibilityPerformDecrement);

            addMethod (@selector (accessibilityPerformEscape), [] (id self, SEL)
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
            });

            if (elementType == Type::textElement)
            {
                addMethod (@selector (accessibilityLineNumberForPoint:), [] (id self, SEL, CGPoint point)
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
                });

                addMethod (@selector (accessibilityContentForLineNumber:), [] (id self, SEL, NSInteger lineNumber) -> NSString*
                {
                    if (auto* textInterface = getTextInterface (self))
                    {
                        auto lines = StringArray::fromLines (textInterface->getText ({ 0, textInterface->getTotalNumCharacters() }));

                        if ((int) lineNumber < lines.size())
                            return juceStringToNS (lines[(int) lineNumber]);
                    }

                    return nil;
                });

                addMethod (@selector (accessibilityFrameForLineNumber:), [] (id self, SEL, NSInteger lineNumber)
                {
                    if (auto* textInterface = getTextInterface (self))
                    {
                        auto textBounds = textInterface->getTextBounds ({ 0, textInterface->getTotalNumCharacters() });

                        if (lineNumber < textBounds.getNumRectangles())
                            return convertToCGRect (textBounds.getRectangle ((int) lineNumber));
                    }

                    return CGRectZero;
                });

                addMethod (@selector (accessibilityPageContent), [] (id self, SEL) -> NSString*
                {
                    if (auto* textInterface = getTextInterface (self))
                        return juceStringToNS (textInterface->getText ({ 0, textInterface->getTotalNumCharacters() }));

                    return nil;
                });

                addProtocol (@protocol (UIAccessibilityReadingContent));
            }

           #if JUCE_IOS_CONTAINER_API_AVAILABLE
            if (@available (iOS 11.0, *))
            {
                addMethod (@selector (accessibilityRowRange),                           getAccessibilityRowIndexRange);
                addMethod (@selector (accessibilityColumnRange),                        getAccessibilityColumnIndexRange);
                addProtocol (@protocol (UIAccessibilityContainerDataTableCell));
            }
           #endif

            addIvar<UIAccessibilityElement*> ("container");

            registerClass();
        }

    private:
        template <typename Return, typename Method>
        void addMethodWithReturn (SEL selector, Method method) { addMethod (selector, static_cast<Return (*) (id, SEL)> (method)); }

        //==============================================================================
        static UIAccessibilityElement* getContainer (id self)
        {
            return getIvar<UIAccessibilityElement*> (self, "container");
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
                                moveToHandler ? static_cast<id> (handler.getNativeImplementation()) : nil);
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
        sendAccessibilityEvent (notification, static_cast<id> (getNativeImplementation()));
}

void AccessibilityHandler::postAnnouncement (const String& announcementString, AnnouncementPriority)
{
    sendAccessibilityEvent (UIAccessibilityAnnouncementNotification, juceStringToNS (announcementString));
}

} // namespace juce
