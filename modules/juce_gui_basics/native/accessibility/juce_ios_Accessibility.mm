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

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

template <typename> struct Signature;

template <typename Result, typename... Args>
struct Signature<Result (Args...)> {};

// @selector isn't constexpr, so the 'sel' members are functions rather than static constexpr data members
struct SignatureHasText                         : Signature<BOOL()>                                                              { static auto sel() { return @selector (hasText); } };
struct SignatureSetSelectedTextRange            : Signature<void (UITextRange*)>                                                 { static auto sel() { return @selector (setSelectedTextRange:); } };
struct SignatureSelectedTextRange               : Signature<UITextRange*()>                                                      { static auto sel() { return @selector (selectedTextRange); } };
struct SignatureMarkedTextRange                 : Signature<UITextRange*()>                                                      { static auto sel() { return @selector (markedTextRange); } };
struct SignatureSetMarkedTextSelectedRange      : Signature<void (NSString*, NSRange)>                                           { static auto sel() { return @selector (setMarkedText:selectedRange:); } };
struct SignatureUnmarkText                      : Signature<void()>                                                              { static auto sel() { return @selector (unmarkText); } };
struct SignatureMarkedTextStyle                 : Signature<NSDictionary<NSAttributedStringKey, id>*()>                          { static auto sel() { return @selector (markedTextStyle); } };
struct SignatureSetMarkedTextStyle              : Signature<void (NSDictionary<NSAttributedStringKey, id>*)>                     { static auto sel() { return @selector (setMarkedTextStyle:); } };
struct SignatureBeginningOfDocument             : Signature<UITextPosition*()>                                                   { static auto sel() { return @selector (beginningOfDocument); } };
struct SignatureEndOfDocument                   : Signature<UITextPosition*()>                                                   { static auto sel() { return @selector (endOfDocument); } };
struct SignatureTokenizer                       : Signature<id<UITextInputTokenizer>()>                                          { static auto sel() { return @selector (tokenizer); } };
struct SignatureBaseWritingDirection            : Signature<NSWritingDirection (UITextPosition*, UITextStorageDirection)>        { static auto sel() { return @selector (baseWritingDirectionForPosition:inDirection:); } };
struct SignatureCaretRectForPosition            : Signature<CGRect (UITextPosition*)>                                            { static auto sel() { return @selector (caretRectForPosition:); } };
struct SignatureCharacterRangeByExtending       : Signature<UITextRange* (UITextPosition*, UITextLayoutDirection)>               { static auto sel() { return @selector (characterRangeByExtendingPosition:inDirection:); } };
struct SignatureCharacterRangeAtPoint           : Signature<UITextRange* (CGPoint)>                                              { static auto sel() { return @selector (characterRangeAtPoint:); } };
struct SignatureClosestPositionToPoint          : Signature<UITextPosition* (CGPoint)>                                           { static auto sel() { return @selector (closestPositionToPoint:); } };
struct SignatureClosestPositionToPointInRange   : Signature<UITextPosition* (CGPoint, UITextRange*)>                             { static auto sel() { return @selector (closestPositionToPoint:withinRange:); } };
struct SignatureComparePositionToPosition       : Signature<NSComparisonResult (UITextPosition*, UITextPosition*)>               { static auto sel() { return @selector (comparePosition:toPosition:); } };
struct SignatureOffsetFromPositionToPosition    : Signature<NSInteger (UITextPosition*, UITextPosition*)>                        { static auto sel() { return @selector (offsetFromPosition:toPosition:); } };
struct SignaturePositionFromPositionInDirection : Signature<UITextPosition* (UITextPosition*, UITextLayoutDirection, NSInteger)> { static auto sel() { return @selector (positionFromPosition:inDirection:offset:); } };
struct SignaturePositionFromPositionOffset      : Signature<UITextPosition* (UITextPosition*, NSInteger)>                        { static auto sel() { return @selector (positionFromPosition:offset:); } };
struct SignatureFirstRectForRange               : Signature<CGRect (UITextRange*)>                                               { static auto sel() { return @selector (firstRectForRange:); } };
struct SignatureSelectionRectsForRange          : Signature<NSArray<UITextSelectionRect*>* (UITextRange*)>                       { static auto sel() { return @selector (selectionRectsForRange:); } };
struct SignaturePositionWithinRange             : Signature<UITextPosition* (UITextRange*, UITextLayoutDirection)>               { static auto sel() { return @selector (positionWithinRange:farthestInDirection:); } };
struct SignatureReplaceRangeWithText            : Signature<void (UITextRange*, NSString*)>                                      { static auto sel() { return @selector (replaceRange:withText:); } };
struct SignatureSetBaseWritingDirection         : Signature<void (NSWritingDirection, UITextRange*)>                             { static auto sel() { return @selector (setBaseWritingDirection:forRange:); } };
struct SignatureTextInRange                     : Signature<NSString* (UITextRange*)>                                            { static auto sel() { return @selector (textInRange:); } };
struct SignatureTextRangeFromPosition           : Signature<UITextRange* (UITextPosition*, UITextPosition*)>                     { static auto sel() { return @selector (textRangeFromPosition:toPosition:); } };
struct SignatureSetInputDelegate                : Signature<void (id)>                                                           { static auto sel() { return @selector (setInputDelegate:); } };
struct SignatureInputDelegate                   : Signature<id()>                                                                { static auto sel() { return @selector (inputDelegate); } };
struct SignatureKeyboardType                    : Signature<UIKeyboardType()>                                                    { static auto sel() { return @selector (keyboardType); } };
struct SignatureAutocapitalizationType          : Signature<UITextAutocapitalizationType()>                                      { static auto sel() { return @selector (autocapitalizationType); } };
struct SignatureAutocorrectionType              : Signature<UITextAutocorrectionType()>                                          { static auto sel() { return @selector (autocorrectionType); } };

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
        template <typename Func, typename... Items>
        static constexpr void forEach (Func&& func, Items&&... items)
        {
            (void) std::initializer_list<int> { ((void) func (std::forward<Items> (items)), 0)... };
        }

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

                            case AccessibilityRole::editableText:  return UIAccessibilityTraitKeyboardKey;

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

            addMethod (@selector (accessibilityElementIsFocused), [] (id self, SEL) -> BOOL
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
                addMethod (@selector (deleteBackward), [] (id, SEL) {});
                addMethod (@selector (insertText:), [] (id, SEL, NSString*) {});

                forEach ([this] (auto signature) { addPassthroughMethodWithSignature (signature); },
                         SignatureHasText{},
                         SignatureSetSelectedTextRange{},
                         SignatureSelectedTextRange{},
                         SignatureMarkedTextRange{},
                         SignatureSetMarkedTextSelectedRange{},
                         SignatureUnmarkText{},
                         SignatureMarkedTextStyle{},
                         SignatureSetMarkedTextStyle{},
                         SignatureBeginningOfDocument{},
                         SignatureEndOfDocument{},
                         SignatureTokenizer{},
                         SignatureBaseWritingDirection{},
                         SignatureCaretRectForPosition{},
                         SignatureCharacterRangeByExtending{},
                         SignatureCharacterRangeAtPoint{},
                         SignatureClosestPositionToPoint{},
                         SignatureClosestPositionToPointInRange{},
                         SignatureComparePositionToPosition{},
                         SignatureOffsetFromPositionToPosition{},
                         SignaturePositionFromPositionInDirection{},
                         SignaturePositionFromPositionOffset{},
                         SignatureFirstRectForRange{},
                         SignatureSelectionRectsForRange{},
                         SignaturePositionWithinRange{},
                         SignatureReplaceRangeWithText{},
                         SignatureSetBaseWritingDirection{},
                         SignatureTextInRange{},
                         SignatureTextRangeFromPosition{},
                         SignatureSetInputDelegate{},
                         SignatureInputDelegate{},
                         SignatureKeyboardType{},
                         SignatureAutocapitalizationType{},
                         SignatureAutocorrectionType{});

                addProtocol (@protocol (UITextInput));
            }

            if (@available (iOS 11.0, *))
            {
                addMethod (@selector (accessibilityRowRange),                           getAccessibilityRowIndexRange);
                addMethod (@selector (accessibilityColumnRange),                        getAccessibilityColumnIndexRange);
                addProtocol (@protocol (UIAccessibilityContainerDataTableCell));
            }

            addIvar<UIAccessibilityElement*> ("container");

            registerClass();
        }

    private:
        template <typename Result>
        static auto getResult (NSInvocation* invocation, detail::Tag<Result>)
        {
            Result result{};
            [invocation getReturnValue: &result];
            return result;
        }

        static void getResult (NSInvocation*, detail::Tag<void>) {}

        template <typename HasSelector, typename Result, typename... Args>
        auto makePassthroughCallback (HasSelector, Signature<Result (Args...)>)
        {
            return [] (id self, SEL, Args... args) -> Result
            {
                if (auto* input = getPeerTextInput (self))
                {
                    const auto s = detail::makeCompileTimeStr (@encode (Result), @encode (id), @encode (SEL), @encode (Args)...);
                    const auto signature = [NSMethodSignature signatureWithObjCTypes: s.data()];
                    const auto invocation = [NSInvocation invocationWithMethodSignature: signature];
                    invocation.selector = HasSelector::sel();

                    // Indices 0 and 1 are 'id self' and 'SEL _cmd' respectively
                    auto counter = 2;
                    forEach ([&] (auto& arg) { [invocation setArgument: &arg atIndex: counter++]; }, args...);

                    [invocation invokeWithTarget: input];

                    return getResult (invocation, detail::Tag<Result>{});
                }

                jassertfalse;
                return {};
            };
        }

        template <typename Signature>
        void addPassthroughMethodWithSignature (Signature signature)
        {
            addMethod (Signature::sel(), makePassthroughCallback (signature, signature));
        }

        static UIAccessibilityElement* getContainer (id self)
        {
            return getIvar<UIAccessibilityElement*> (self, "container");
        }

        static UIViewComponentPeer* getPeer (id self)
        {
            if (auto* handler = getHandler (self))
                return static_cast<UIViewComponentPeer*> (handler->getComponent().getPeer());

            return nil;
        }

        static JuceTextView* getPeerTextInput (id self)
        {
            if (auto* peer = getPeer (self))
                return peer->hiddenTextInput.get();

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
