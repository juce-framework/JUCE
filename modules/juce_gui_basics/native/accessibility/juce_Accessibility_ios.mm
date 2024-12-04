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

template <typename> struct Signature {};

template <typename Result, typename... Args>
struct Signature<Result (Args...)> {};

// @selector isn't constexpr, so the 'sel' members are functions rather than static constexpr data members
struct SignatureHasText                         final : public Signature<BOOL()>                                                              { static auto sel() { return @selector (hasText); } };
struct SignatureSetSelectedTextRange            final : public Signature<void (UITextRange*)>                                                 { static auto sel() { return @selector (setSelectedTextRange:); } };
struct SignatureSelectedTextRange               final : public Signature<UITextRange*()>                                                      { static auto sel() { return @selector (selectedTextRange); } };
struct SignatureMarkedTextRange                 final : public Signature<UITextRange*()>                                                      { static auto sel() { return @selector (markedTextRange); } };
struct SignatureSetMarkedTextSelectedRange      final : public Signature<void (NSString*, NSRange)>                                           { static auto sel() { return @selector (setMarkedText:selectedRange:); } };
struct SignatureUnmarkText                      final : public Signature<void()>                                                              { static auto sel() { return @selector (unmarkText); } };
struct SignatureMarkedTextStyle                 final : public Signature<NSDictionary<NSAttributedStringKey, id>*()>                          { static auto sel() { return @selector (markedTextStyle); } };
struct SignatureSetMarkedTextStyle              final : public Signature<void (NSDictionary<NSAttributedStringKey, id>*)>                     { static auto sel() { return @selector (setMarkedTextStyle:); } };
struct SignatureBeginningOfDocument             final : public Signature<UITextPosition*()>                                                   { static auto sel() { return @selector (beginningOfDocument); } };
struct SignatureEndOfDocument                   final : public Signature<UITextPosition*()>                                                   { static auto sel() { return @selector (endOfDocument); } };
struct SignatureTokenizer                       final : public Signature<id<UITextInputTokenizer>()>                                          { static auto sel() { return @selector (tokenizer); } };
struct SignatureBaseWritingDirection            final : public Signature<NSWritingDirection (UITextPosition*, UITextStorageDirection)>        { static auto sel() { return @selector (baseWritingDirectionForPosition:inDirection:); } };
struct SignatureCaretRectForPosition            final : public Signature<CGRect (UITextPosition*)>                                            { static auto sel() { return @selector (caretRectForPosition:); } };
struct SignatureCharacterRangeByExtending       final : public Signature<UITextRange* (UITextPosition*, UITextLayoutDirection)>               { static auto sel() { return @selector (characterRangeByExtendingPosition:inDirection:); } };
struct SignatureCharacterRangeAtPoint           final : public Signature<UITextRange* (CGPoint)>                                              { static auto sel() { return @selector (characterRangeAtPoint:); } };
struct SignatureClosestPositionToPoint          final : public Signature<UITextPosition* (CGPoint)>                                           { static auto sel() { return @selector (closestPositionToPoint:); } };
struct SignatureClosestPositionToPointInRange   final : public Signature<UITextPosition* (CGPoint, UITextRange*)>                             { static auto sel() { return @selector (closestPositionToPoint:withinRange:); } };
struct SignatureComparePositionToPosition       final : public Signature<NSComparisonResult (UITextPosition*, UITextPosition*)>               { static auto sel() { return @selector (comparePosition:toPosition:); } };
struct SignatureOffsetFromPositionToPosition    final : public Signature<NSInteger (UITextPosition*, UITextPosition*)>                        { static auto sel() { return @selector (offsetFromPosition:toPosition:); } };
struct SignaturePositionFromPositionInDirection final : public Signature<UITextPosition* (UITextPosition*, UITextLayoutDirection, NSInteger)> { static auto sel() { return @selector (positionFromPosition:inDirection:offset:); } };
struct SignaturePositionFromPositionOffset      final : public Signature<UITextPosition* (UITextPosition*, NSInteger)>                        { static auto sel() { return @selector (positionFromPosition:offset:); } };
struct SignatureFirstRectForRange               final : public Signature<CGRect (UITextRange*)>                                               { static auto sel() { return @selector (firstRectForRange:); } };
struct SignatureSelectionRectsForRange          final : public Signature<NSArray<UITextSelectionRect*>* (UITextRange*)>                       { static auto sel() { return @selector (selectionRectsForRange:); } };
struct SignaturePositionWithinRange             final : public Signature<UITextPosition* (UITextRange*, UITextLayoutDirection)>               { static auto sel() { return @selector (positionWithinRange:farthestInDirection:); } };
struct SignatureReplaceRangeWithText            final : public Signature<void (UITextRange*, NSString*)>                                      { static auto sel() { return @selector (replaceRange:withText:); } };
struct SignatureSetBaseWritingDirection         final : public Signature<void (NSWritingDirection, UITextRange*)>                             { static auto sel() { return @selector (setBaseWritingDirection:forRange:); } };
struct SignatureTextInRange                     final : public Signature<NSString* (UITextRange*)>                                            { static auto sel() { return @selector (textInRange:); } };
struct SignatureTextRangeFromPosition           final : public Signature<UITextRange* (UITextPosition*, UITextPosition*)>                     { static auto sel() { return @selector (textRangeFromPosition:toPosition:); } };
struct SignatureSetInputDelegate                final : public Signature<void (id)>                                                           { static auto sel() { return @selector (setInputDelegate:); } };
struct SignatureInputDelegate                   final : public Signature<id()>                                                                { static auto sel() { return @selector (inputDelegate); } };
struct SignatureKeyboardType                    final : public Signature<UIKeyboardType()>                                                    { static auto sel() { return @selector (keyboardType); } };
struct SignatureAutocapitalizationType          final : public Signature<UITextAutocapitalizationType()>                                      { static auto sel() { return @selector (autocapitalizationType); } };
struct SignatureAutocorrectionType              final : public Signature<UITextAutocorrectionType()>                                          { static auto sel() { return @selector (autocorrectionType); } };

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
    class AccessibilityContainer final : public AccessibleObjCClass<NSObject>
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

            addMethod (@selector (accessibilityDataTableCellElementForRow:column:), [] (id self, SEL, NSUInteger row, NSUInteger column) -> id
            {
                if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (getHandler (self), &AccessibilityHandler::getTableInterface))
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
                if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (getHandler (self), &AccessibilityHandler::getTableInterface))
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
                        return UIAccessibilityContainerTypeDataTable;

                    const auto handlerRole = handler->getRole();

                    if (handlerRole == AccessibilityRole::popupMenu
                        || handlerRole == AccessibilityRole::list
                        || handlerRole == AccessibilityRole::tree)
                    {
                        return UIAccessibilityContainerTypeList;
                    }
                }

                return UIAccessibilityContainerTypeNone;
            });

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
    class AccessibilityElement final : public AccessibleObjCClass<UIAccessibilityElement>
    {
        template <typename Func, typename... Items>
        static constexpr void forEach (Func&& func, Items&&... items)
        {
            (func (std::forward<Items> (items)), ...);
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
                        if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (&handlerRef, &AccessibilityHandler::getTableInterface))
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

                    if (   ! handler->getChildren().empty()
                        || AccessibilityHandler::getNativeChildForComponent (handler->getComponent()) != nullptr)
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

            addMethod (@selector (accessibilityRowRange),                           getAccessibilityRowIndexRange);
            addMethod (@selector (accessibilityColumnRange),                        getAccessibilityColumnIndexRange);
            addProtocol (@protocol (UIAccessibilityContainerDataTableCell));

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

                    if (signature == nullptr)
                    {
                        jassertfalse;
                        return {};
                    }

                    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
                    const auto invocation = [NSInvocation invocationWithMethodSignature: signature];
                    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

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

void detail::AccessibilityHelpers::notifyAccessibilityEvent (const AccessibilityHandler& handler, Event eventType)
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case Event::elementCreated:
            case Event::elementDestroyed:
            case Event::elementMovedOrResized:
            case Event::focusChanged:            return UIAccessibilityLayoutChangedNotification;

            case Event::windowOpened:
            case Event::windowClosed:            return UIAccessibilityScreenChangedNotification;
        }

        return UIAccessibilityNotifications{};
    }();

    if (notification != UIAccessibilityNotifications{})
    {
        const bool moveToHandler = (eventType == Event::focusChanged && handler.hasFocus (false));

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
