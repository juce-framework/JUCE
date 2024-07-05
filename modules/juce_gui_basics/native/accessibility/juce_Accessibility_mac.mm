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

static void juceFreeAccessibilityPlatformSpecificData (NSAccessibilityElement<NSAccessibility>*)  {}

namespace juce
{

#define JUCE_NATIVE_ACCESSIBILITY_INCLUDED 1

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& handler)
        : accessibilityElement (AccessibilityElement::create (handler))
    {
    }

    NSAccessibilityElement<NSAccessibility>* getAccessibilityElement() const noexcept
    {
        return accessibilityElement.get();
    }

private:
    //==============================================================================
    class AccessibilityElement final : public AccessibleObjCClass<NSAccessibilityElement<NSAccessibility>>
    {
    public:
        static Holder create (AccessibilityHandler& handler)
        {
            static AccessibilityElement cls;
            Holder element ([cls.createInstance() init]);
            object_setInstanceVariable (element.get(), "handler", &handler);
            return element;
        }

    private:
        AccessibilityElement()
        {
            addMethod (@selector (accessibilityNotifiesWhenDestroyed), [] (id, SEL)  { return YES; });
            addMethod (@selector (isAccessibilityElement), getIsAccessibilityElement);

            addMethod (@selector (isAccessibilityEnabled), [] (id self, SEL) -> BOOL
            {
                if (auto* handler = getHandler (self))
                    return handler->getComponent().isEnabled();

                return NO;
            });

            addMethod (@selector (accessibilityWindow),                    getAccessibilityWindow);
            addMethod (@selector (accessibilityTopLevelUIElement),         getAccessibilityWindow);
            addMethod (@selector (accessibilityChildren),                  getAccessibilityChildren);
            addMethod (@selector (isAccessibilityModal),                   getIsAccessibilityModal);

            addMethod (@selector (accessibilityFocusedUIElement), [] (id self, SEL) -> id
            {
                if (auto* handler = getHandler (self))
                {
                    if (auto* modal = Component::getCurrentlyModalComponent())
                    {
                        const auto& handlerComponent = handler->getComponent();

                        if (! handlerComponent.isParentOf (modal)
                             && handlerComponent.isCurrentlyBlockedByAnotherModalComponent())
                        {
                            if (auto* modalHandler = modal->getAccessibilityHandler())
                            {
                                if (auto* focusChild = modalHandler->getChildFocus())
                                    return static_cast<id> (focusChild->getNativeImplementation());

                                return static_cast<id> (modalHandler->getNativeImplementation());
                            }
                        }
                    }

                    if (auto* focusChild = handler->getChildFocus())
                        return static_cast<id> (focusChild->getNativeImplementation());
                }

                return nil;
            });

            addMethod (@selector (accessibilityHitTest:), [] (id self, SEL, NSPoint point) -> id
            {
                if (auto* handler = getHandler (self))
                {
                    if (auto* child = handler->getChildAt (roundToIntPoint (flippedScreenPoint (point))))
                        return static_cast<id> (child->getNativeImplementation());

                    return self;
                }

                return nil;
            });

            addMethod (@selector (accessibilityParent), [] (id self, SEL) -> id
            {
                if (auto* handler = getHandler (self))
                {
                    if (auto* parentHandler = handler->getParent())
                        return NSAccessibilityUnignoredAncestor (static_cast<id> (parentHandler->getNativeImplementation()));

                    return NSAccessibilityUnignoredAncestor (static_cast<id> (handler->getComponent().getWindowHandle()));
                }

                return nil;
            });

            addMethod (@selector (isAccessibilityFocused), [] (id self, SEL)
            {
                return [[self accessibilityWindow] accessibilityFocusedUIElement] == self;
            });

            addMethod (@selector (setAccessibilityFocused:), [] (id self, SEL, BOOL focused)
            {
                if (auto* handler = getHandler (self))
                {
                    if (focused)
                    {
                        const WeakReference<Component> safeComponent (&handler->getComponent());

                        performActionIfSupported (self, AccessibilityActionType::focus);

                        if (safeComponent != nullptr)
                            handler->grabFocus();
                    }
                    else
                    {
                        handler->giveAwayFocus();
                    }
                }
            });

            addMethod (@selector (accessibilityFrame), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                    return flippedScreenRect (makeNSRect (handler->getComponent().getScreenBounds()));

                return NSZeroRect;
            });

            addMethod (@selector (accessibilityRole), [] (id self, SEL) -> NSAccessibilityRole
            {
                if (auto* handler = getHandler (self))
                {
                    switch (handler->getRole())
                    {
                        case AccessibilityRole::popupMenu:
                        case AccessibilityRole::tooltip:
                        case AccessibilityRole::splashScreen:
                        case AccessibilityRole::dialogWindow:
                        case AccessibilityRole::window:        return NSAccessibilityWindowRole;

                        case AccessibilityRole::tableHeader:
                        case AccessibilityRole::unspecified:
                        case AccessibilityRole::group:         return NSAccessibilityGroupRole;

                        case AccessibilityRole::label:
                        case AccessibilityRole::staticText:    return NSAccessibilityStaticTextRole;

                        case AccessibilityRole::tree:
                        case AccessibilityRole::list:          return NSAccessibilityOutlineRole;

                        case AccessibilityRole::listItem:
                        case AccessibilityRole::treeItem:      return NSAccessibilityRowRole;

                        case AccessibilityRole::button:        return NSAccessibilityButtonRole;
                        case AccessibilityRole::toggleButton:  return NSAccessibilityCheckBoxRole;
                        case AccessibilityRole::radioButton:   return NSAccessibilityRadioButtonRole;
                        case AccessibilityRole::comboBox:      return NSAccessibilityPopUpButtonRole;
                        case AccessibilityRole::image:         return NSAccessibilityImageRole;
                        case AccessibilityRole::slider:        return NSAccessibilitySliderRole;
                        case AccessibilityRole::editableText:  return NSAccessibilityTextAreaRole;
                        case AccessibilityRole::menuItem:      return NSAccessibilityMenuItemRole;
                        case AccessibilityRole::menuBar:       return NSAccessibilityMenuRole;
                        case AccessibilityRole::table:         return NSAccessibilityOutlineRole;
                        case AccessibilityRole::column:        return NSAccessibilityColumnRole;
                        case AccessibilityRole::row:           return NSAccessibilityRowRole;
                        case AccessibilityRole::cell:          return NSAccessibilityCellRole;
                        case AccessibilityRole::hyperlink:     return NSAccessibilityLinkRole;
                        case AccessibilityRole::progressBar:   return NSAccessibilityProgressIndicatorRole;
                        case AccessibilityRole::scrollBar:     return NSAccessibilityScrollBarRole;

                        case AccessibilityRole::ignored:       break;
                    }

                    return NSAccessibilityUnknownRole;
                }

                return nil;
            });

            addMethod (@selector (accessibilitySubrole), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                {
                    if (auto* textInterface = getTextInterface (self))
                        if (textInterface->isDisplayingProtectedText())
                            return NSAccessibilitySecureTextFieldSubrole;

                    const auto handlerRole = handler->getRole();

                    if (handlerRole == AccessibilityRole::window)                                     return NSAccessibilityStandardWindowSubrole;
                    if (handlerRole == AccessibilityRole::dialogWindow)                               return NSAccessibilityDialogSubrole;
                    if (handlerRole == AccessibilityRole::tooltip
                        || handlerRole == AccessibilityRole::splashScreen)                            return NSAccessibilityFloatingWindowSubrole;
                    if (handlerRole == AccessibilityRole::toggleButton)                               return NSAccessibilityToggleSubrole;
                    if (handlerRole == AccessibilityRole::treeItem
                        || handlerRole == AccessibilityRole::listItem)                                return NSAccessibilityOutlineRowSubrole;
                    if (handlerRole == AccessibilityRole::row && getCellInterface (self) != nullptr)  return NSAccessibilityTableRowSubrole;

                    const auto& handlerComponent = handler->getComponent();

                    if (auto* documentWindow = handlerComponent.findParentComponentOfClass<DocumentWindow>())
                    {
                        if (handlerRole == AccessibilityRole::button)
                        {
                            if (&handlerComponent == documentWindow->getCloseButton())     return NSAccessibilityCloseButtonSubrole;
                            if (&handlerComponent == documentWindow->getMinimiseButton())  return NSAccessibilityMinimizeButtonSubrole;
                            if (&handlerComponent == documentWindow->getMaximiseButton())  return NSAccessibilityFullScreenButtonSubrole;
                        }
                    }
                }

                return NSAccessibilityUnknownRole;
            });

            addMethod (@selector (accessibilityLabel), [] (id self, SEL) -> NSString*
            {
                if (auto* handler = getHandler (self))
                    return juceStringToNS (handler->getDescription());

                return nil;
            });

            addMethod (@selector (accessibilityValue), [] (id self, SEL) -> id
            {
                if (auto* handler = getHandler (self))
                {
                    if (! handler->getCurrentState().isCheckable())
                        return getAccessibilityValueFromInterfaces (*handler);

                    const auto checked = handler->getCurrentState().isChecked();

                    if (   handler->getRole() == AccessibilityRole::toggleButton
                        || handler->getRole() == AccessibilityRole::radioButton)
                    {
                        return checked ? @YES : @NO;
                    }

                    return juceStringToNS (checked ? TRANS ("On") : TRANS ("Off"));
                }

                return nil;
            });

            addMethod (@selector (accessibilityTitle),                     getAccessibilityTitle);
            addMethod (@selector (accessibilityHelp),                      getAccessibilityHelp);
            addMethod (@selector (setAccessibilityValue:),                 setAccessibilityValue);

            addMethod (@selector (accessibilitySelectedChildren), [] (id self, SEL)
            {
                return getSelectedChildren ([self accessibilityChildren]);
            });

            addMethod (@selector (setAccessibilitySelectedChildren:), [] (id self, SEL, NSArray* selected)
            {
                setSelectedChildren ([self accessibilityChildren], selected);
            });

            addMethod (@selector (accessibilityOrientation), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                    return handler->getComponent().getBounds().toFloat().getAspectRatio() > 1.0f
                           ? NSAccessibilityOrientationHorizontal
                           : NSAccessibilityOrientationVertical;

                return NSAccessibilityOrientationUnknown;
            });

            addMethod (@selector (accessibilityInsertionPointLineNumber), [] (id self, SEL) -> NSInteger
            {
                if (auto* textInterface = getTextInterface (self))
                    return [self accessibilityLineForIndex: textInterface->getTextInsertionOffset()];

                return 0;
            });

            addMethod (@selector (accessibilityVisibleCharacterRange), [] (id self, SEL)
            {
                if (auto* textInterface = getTextInterface (self))
                    return juceRangeToNS ({ 0, textInterface->getTotalNumCharacters() });

                return NSMakeRange (0, 0);
            });

            addMethod (@selector (accessibilityNumberOfCharacters), [] (id self, SEL)
            {
                if (auto* textInterface = getTextInterface (self))
                    return textInterface->getTotalNumCharacters();

                return 0;
            });

            addMethod (@selector (accessibilitySelectedText), [] (id self, SEL) -> NSString*
            {
                if (auto* textInterface = getTextInterface (self))
                    return juceStringToNS (textInterface->getText (textInterface->getSelection()));

                return nil;
            });

            addMethod (@selector (accessibilitySelectedTextRange), [] (id self, SEL)
            {
                if (auto* textInterface = getTextInterface (self))
                {
                    const auto currentSelection = textInterface->getSelection();

                    if (currentSelection.isEmpty())
                        return NSMakeRange ((NSUInteger) textInterface->getTextInsertionOffset(), 0);

                    return juceRangeToNS (currentSelection);
                }

                return NSMakeRange (0, 0);
            });

            addMethod (@selector (accessibilityAttributedStringForRange:), [] (id self, SEL, NSRange range) -> NSAttributedString*
            {
                NSString* string = [self accessibilityStringForRange: range];

                if (string != nil)
                    return [[[NSAttributedString alloc] initWithString: string] autorelease];

                return nil;
            });

            addMethod (@selector (accessibilityRangeForLine:), [] (id self, SEL, NSInteger line)
            {
                if (auto* textInterface = getTextInterface (self))
                {
                    auto text = textInterface->getText ({ 0, textInterface->getTotalNumCharacters() });
                    auto lines = StringArray::fromLines (text);

                    if (line < lines.size())
                    {
                        auto lineText = lines[(int) line];
                        auto start = text.indexOf (lineText);

                        if (start >= 0)
                            return NSMakeRange ((NSUInteger) start, (NSUInteger) lineText.length());
                    }
                }

                return NSMakeRange (0, 0);
            });

            addMethod (@selector (accessibilityStringForRange:), [] (id self, SEL, NSRange range) -> NSString*
            {
                if (auto* textInterface = getTextInterface (self))
                    return juceStringToNS (textInterface->getText (nsRangeToJuce (range)));

                return nil;
            });

            addMethod (@selector (accessibilityRangeForPosition:), [] (id self, SEL, NSPoint position)
            {
                if (auto* handler = getHandler (self))
                {
                    if (auto* textInterface = handler->getTextInterface())
                    {
                        auto screenPoint = roundToIntPoint (flippedScreenPoint (position));

                        if (handler->getComponent().getScreenBounds().contains (screenPoint))
                        {
                            auto offset = textInterface->getOffsetAtPoint (screenPoint);

                            if (offset >= 0)
                                return NSMakeRange ((NSUInteger) offset, 1);
                        }
                    }
                }

                return NSMakeRange (0, 0);
            });

            addMethod (@selector (accessibilityRangeForIndex:), [] (id self, SEL, NSInteger index)
            {
                if (auto* textInterface = getTextInterface (self))
                    if (isPositiveAndBelow (index, textInterface->getTotalNumCharacters()))
                        return NSMakeRange ((NSUInteger) index, 1);

                return NSMakeRange (0, 0);
            });

            addMethod (@selector (accessibilityFrameForRange:), [] (id self, SEL, NSRange range)
            {
                if (auto* textInterface = getTextInterface (self))
                    return flippedScreenRect (makeNSRect (textInterface->getTextBounds (nsRangeToJuce (range)).getBounds()));

                return NSZeroRect;
            });

            addMethod (@selector (accessibilityLineForIndex:), [] (id self, SEL, NSInteger index)
            {
                if (auto* textInterface = getTextInterface (self))
                {
                    auto text = textInterface->getText ({ 0, (int) index });

                    if (! text.isEmpty())
                        return StringArray::fromLines (text).size() - 1;
                }

                return 0;
            });

            addMethod (@selector (setAccessibilitySelectedTextRange:), [] (id self, SEL, NSRange selectedRange)
            {
                if (auto* textInterface = getTextInterface (self))
                    textInterface->setSelection (nsRangeToJuce (selectedRange));
            });

            addMethod (@selector (accessibilityRows), [] (id self, SEL) -> NSArray*
            {
                if (auto* tableInterface = getTableInterface (self))
                {
                    auto* rows = [[NSMutableArray new] autorelease];

                    for (int row = 0, numRows = tableInterface->getNumRows(); row < numRows; ++row)
                    {
                        if (auto* rowHandler = tableInterface->getRowHandler (row))
                        {
                            [rows addObject: static_cast<id> (rowHandler->getNativeImplementation())];
                        }
                        else
                        {
                            [rows addObject: [NSAccessibilityElement accessibilityElementWithRole: NSAccessibilityRowRole
                                                                                            frame: NSZeroRect
                                                                                            label: @"Offscreen Row"
                                                                                           parent: self]];
                        }
                    }

                    return rows;
                }

                return nil;
            });

            addMethod (@selector (accessibilitySelectedRows), [] (id self, SEL)
            {
                return getSelectedChildren ([self accessibilityRows]);
            });

            addMethod (@selector (setAccessibilitySelectedRows:), [] (id self, SEL, NSArray* selected)
            {
                setSelectedChildren ([self accessibilityRows], selected);
            });

            addMethod (@selector (accessibilityHeader), [] (id self, SEL) -> id
            {
                if (auto* tableInterface = getTableInterface (self))
                    if (auto* handler = tableInterface->getHeaderHandler())
                        return static_cast<id> (handler->getNativeImplementation());

                return nil;
            });

            addMethod (@selector (accessibilityRowCount),         getAccessibilityRowCount);
            addMethod (@selector (accessibilityColumnCount),      getAccessibilityColumnCount);
            addMethod (@selector (accessibilityRowIndexRange),    getAccessibilityRowIndexRange);
            addMethod (@selector (accessibilityColumnIndexRange), getAccessibilityColumnIndexRange);

            addMethod (@selector (accessibilityIndex), [] (id self, SEL) -> NSInteger
            {
                if (auto* handler = getHandler (self))
                {
                    if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (handler, &AccessibilityHandler::getTableInterface))
                    {
                        if (auto* tableInterface = tableHandler->getTableInterface())
                        {
                            NSAccessibilityRole handlerRole = [self accessibilityRole];

                            if ([handlerRole isEqual: NSAccessibilityRowRole])
                                if (const auto span = tableInterface->getRowSpan (*handler))
                                    return span->begin;

                            if ([handlerRole isEqual: NSAccessibilityColumnRole])
                                if (const auto span = tableInterface->getColumnSpan (*handler))
                                    return span->begin;
                        }
                    }
                }

                return 0;
            });

            addMethod (@selector (accessibilityDisclosureLevel), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                    if (auto* cellInterface = handler->getCellInterface())
                        return cellInterface->getDisclosureLevel();

                return 0;
            });

            addMethod (@selector (accessibilityDisclosedRows), [] (id self, SEL) -> id
            {
                if (auto* handler = getHandler (self))
                {
                    if (auto* cellInterface = handler->getCellInterface())
                    {
                        const auto rows = cellInterface->getDisclosedRows();
                        auto* result = [NSMutableArray arrayWithCapacity: rows.size()];

                        for (const auto& row : rows)
                        {
                            if (row != nullptr)
                                [result addObject: static_cast<id> (row->getNativeImplementation())];
                            else
                                [result addObject: [NSAccessibilityElement accessibilityElementWithRole: NSAccessibilityRowRole
                                                                                                  frame: NSZeroRect
                                                                                                  label: @"Offscreen Row"
                                                                                                 parent: self]];
                        }

                        return result;
                    }
                }

                return nil;
            });

            addMethod (@selector (isAccessibilityExpanded), [] (id self, SEL) -> BOOL
            {
                if (auto* handler = getHandler (self))
                    return handler->getCurrentState().isExpanded();

                return NO;
            });

            addMethod (@selector (accessibilityPerformIncrement), accessibilityPerformIncrement);
            addMethod (@selector (accessibilityPerformDecrement), accessibilityPerformDecrement);
            addMethod (@selector (accessibilityPerformDelete), [] (id self, SEL)
            {
                if (auto* handler = getHandler (self))
                {
                    if (hasEditableText (*handler))
                    {
                        handler->getTextInterface()->setText ({});
                        return YES;
                    }

                    if (auto* valueInterface = handler->getValueInterface())
                    {
                        if (! valueInterface->isReadOnly())
                        {
                            valueInterface->setValue ({});
                            return YES;
                        }
                    }
                }

                return NO;
            });

            addMethod (@selector (accessibilityPerformPress),     accessibilityPerformPress);

            addMethod (@selector (accessibilityPerformShowMenu), [] (id self, SEL)
            {
                return performActionIfSupported (self, AccessibilityActionType::showMenu);
            });

            addMethod (@selector (accessibilityPerformRaise), [] (id self, SEL)
            {
                [self setAccessibilityFocused: YES]; return YES;
            });

            addMethod (@selector (isAccessibilitySelectorAllowed:), [] (id self, SEL, SEL selector) -> BOOL
            {
                if (auto* handler = getHandler (self))
                {
                    const auto handlerRole = handler->getRole();
                    const auto currentState = handler->getCurrentState();

                    for (auto textSelector : { @selector (accessibilityInsertionPointLineNumber),
                                               @selector (accessibilityVisibleCharacterRange),
                                               @selector (accessibilityNumberOfCharacters),
                                               @selector (accessibilitySelectedText),
                                               @selector (accessibilitySelectedTextRange),
                                               @selector (accessibilityAttributedStringForRange:),
                                               @selector (accessibilityRangeForLine:),
                                               @selector (accessibilityStringForRange:),
                                               @selector (accessibilityRangeForPosition:),
                                               @selector (accessibilityRangeForIndex:),
                                               @selector (accessibilityFrameForRange:),
                                               @selector (accessibilityLineForIndex:),
                                               @selector (setAccessibilitySelectedTextRange:) })
                    {
                        if (selector == textSelector)
                            return handler->getTextInterface() != nullptr;
                    }

                    for (auto tableSelector : { @selector (accessibilityRowCount),
                                                @selector (accessibilityRows),
                                                @selector (accessibilitySelectedRows),
                                                @selector (accessibilityColumnCount),
                                                @selector (accessibilityHeader) })
                    {
                        if (selector == tableSelector)
                            return handler->getTableInterface() != nullptr;
                    }

                    for (auto cellSelector : { @selector (accessibilityRowIndexRange),
                                               @selector (accessibilityColumnIndexRange),
                                               @selector (accessibilityIndex),
                                               @selector (accessibilityDisclosureLevel) })
                    {
                        if (selector == cellSelector)
                            return handler->getCellInterface() != nullptr;
                    }

                    for (auto valueSelector : { @selector (accessibilityValue),
                                                @selector (setAccessibilityValue:),
                                                @selector (accessibilityPerformDelete),
                                                @selector (accessibilityPerformIncrement),
                                                @selector (accessibilityPerformDecrement) })
                    {
                        if (selector != valueSelector)
                            continue;

                        auto* valueInterface = handler->getValueInterface();

                        if (selector == @selector (accessibilityValue))
                            return valueInterface != nullptr
                                   || hasEditableText (*handler)
                                   || currentState.isCheckable();

                        auto hasEditableValue = [valueInterface] { return valueInterface != nullptr && ! valueInterface->isReadOnly(); };

                        if (selector == @selector (setAccessibilityValue:)
                            || selector == @selector (accessibilityPerformDelete))
                            return hasEditableValue() || hasEditableText (*handler);

                        auto isRanged = [valueInterface] { return valueInterface != nullptr && valueInterface->getRange().isValid(); };

                        if (selector == @selector (accessibilityPerformIncrement)
                            || selector == @selector (accessibilityPerformDecrement))
                            return hasEditableValue() && isRanged();

                        return NO;
                    }

                    for (auto actionSelector : { @selector (accessibilityPerformPress),
                                                 @selector (accessibilityPerformShowMenu),
                                                 @selector (accessibilityPerformRaise),
                                                 @selector (setAccessibilityFocused:) })
                    {
                        if (selector != actionSelector)
                            continue;

                        if (selector == @selector (accessibilityPerformPress))
                            return (handler->getCurrentState().isCheckable() && handler->getActions().contains (AccessibilityActionType::toggle))
                                   || handler->getActions().contains (AccessibilityActionType::press);

                        if (selector == @selector (accessibilityPerformShowMenu))
                            return handler->getActions().contains (AccessibilityActionType::showMenu);

                        if (selector == @selector (accessibilityPerformRaise))
                            return [[self accessibilityRole] isEqual: NSAccessibilityWindowRole];

                        if (selector == @selector (setAccessibilityFocused:))
                            return currentState.isFocusable();
                    }

                    if (selector == @selector (accessibilitySelectedChildren))
                        return handlerRole == AccessibilityRole::popupMenu;

                    if (selector == @selector (accessibilityOrientation))
                        return handlerRole == AccessibilityRole::scrollBar;

                    if (selector == @selector (isAccessibilityExpanded))
                        return currentState.isExpandable();

                    return sendSuperclassMessage<BOOL> (self, @selector (isAccessibilitySelectorAllowed:), selector);
                }

                return NO;
            });

            addMethod (@selector (accessibilityChildrenInNavigationOrder), getAccessibilityChildren);

            registerClass();
        }

        //==============================================================================
        static bool isSelectable (AccessibleState state) noexcept
        {
            return state.isSelectable() || state.isMultiSelectable();
        }

        static NSArray* getSelectedChildren (NSArray* children)
        {
            NSMutableArray* selected = [[NSMutableArray new] autorelease];

            for (id child in children)
            {
                if (auto* handler = getHandler (child))
                {
                    const auto currentState = handler->getCurrentState();

                    if (isSelectable (currentState) && currentState.isSelected())
                        [selected addObject: child];
                }
            }

            return selected;
        }

        static void setSelected (id item, bool selected)
        {
            auto* handler = getHandler (item);

            if (handler == nullptr)
                return;

            const auto currentState = handler->getCurrentState();

            if (isSelectable (currentState))
            {
                if (currentState.isSelected() != selected)
                    handler->getActions().invoke (AccessibilityActionType::toggle);
            }
            else if (currentState.isFocusable())
            {
                [item setAccessibilityFocused: selected];
            }
        }

        static void setSelectedChildren (NSArray* children, NSArray* selected)
        {
            for (id child in children)
                setSelected (child, [selected containsObject: child]);
        }

        //==============================================================================
        static id getAccessibilityWindow (id self, SEL)
        {
            return [[self accessibilityParent] accessibilityWindow];
        }

        static NSArray* getAccessibilityChildren (id self, SEL)
        {
            if (auto* handler = getHandler (self))
            {
                auto children = handler->getChildren();

                auto* accessibleChildren = [NSMutableArray arrayWithCapacity: (NSUInteger) children.size()];

                for (auto* childHandler : children)
                    [accessibleChildren addObject: static_cast<id> (childHandler->getNativeImplementation())];

                if (id nativeChild = static_cast<id> (AccessibilityHandler::getNativeChildForComponent (handler->getComponent())))
                {
                    // Having both native and non-native children would require implementing an
                    // ordering. However, this situation doesn't occur with any of our current
                    // use-cases.
                    jassert ([accessibleChildren count] == 0);

                    if ([nativeChild isAccessibilityElement])
                        [accessibleChildren addObject:nativeChild];
                    else if (auto* childrenOfChild = [nativeChild accessibilityChildren]; childrenOfChild != nil)
                        [accessibleChildren addObjectsFromArray:(NSArray* _Nonnull) childrenOfChild];
                }

                return accessibleChildren;
            }

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
    const String voiceOverKeyString ("voiceOverOnOffKey");
    const String applicationIDString ("com.apple.universalaccess");

    CFUniquePtr<CFStringRef> cfKey (voiceOverKeyString.toCFString());
    CFUniquePtr<CFStringRef> cfID  (applicationIDString.toCFString());

    CFUniquePtr<CFPropertyListRef> value (CFPreferencesCopyAppValue (cfKey.get(), cfID.get()));

    if (value != nullptr)
        return CFBooleanGetValue ((CFBooleanRef) value.get());

    return false;
}

static void sendAccessibilityEvent (id accessibilityElement,
                                    NSAccessibilityNotificationName notification,
                                    NSDictionary* userInfo)
{
    jassert (notification != NSAccessibilityNotificationName{});

    NSAccessibilityPostNotificationWithUserInfo (accessibilityElement, notification, userInfo);
}

static void sendHandlerNotification (const AccessibilityHandler& handler,
                                     NSAccessibilityNotificationName notification)
{
    if (! areAnyAccessibilityClientsActive() || notification == NSAccessibilityNotificationName{})
        return;

    if (id accessibilityElement = static_cast<id> (handler.getNativeImplementation()))
    {
        sendAccessibilityEvent (accessibilityElement, notification,
                                (notification == NSAccessibilityLayoutChangedNotification
                                     ? @{ NSAccessibilityUIElementsKey: @[ accessibilityElement ] }
                                     : nil));
    }
}

void detail::AccessibilityHelpers::notifyAccessibilityEvent (const AccessibilityHandler& handler, Event eventType)
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case Event::elementCreated:         return NSAccessibilityCreatedNotification;
            case Event::elementDestroyed:       return NSAccessibilityUIElementDestroyedNotification;
            case Event::elementMovedOrResized:  return NSAccessibilityLayoutChangedNotification;
            case Event::focusChanged:           return NSAccessibilityFocusedUIElementChangedNotification;
            case Event::windowOpened:           return NSAccessibilityWindowCreatedNotification;
            case Event::windowClosed:           break;
        }

        return NSAccessibilityNotificationName{};
    }();

    sendHandlerNotification (handler, notification);
}

void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    auto notification = [eventType]
    {
        switch (eventType)
        {
            case AccessibilityEvent::textSelectionChanged:  return NSAccessibilitySelectedTextChangedNotification;
            case AccessibilityEvent::rowSelectionChanged:   return NSAccessibilitySelectedRowsChangedNotification;

            case AccessibilityEvent::textChanged:
            case AccessibilityEvent::valueChanged:          return NSAccessibilityValueChangedNotification;
            case AccessibilityEvent::titleChanged:          return NSAccessibilityTitleChangedNotification;
            case AccessibilityEvent::structureChanged:      return NSAccessibilityLayoutChangedNotification;
        }

        return NSAccessibilityNotificationName{};
    }();

    sendHandlerNotification (*this, notification);
}

void AccessibilityHandler::postAnnouncement (const String& announcementString, AnnouncementPriority priority)
{
    if (! areAnyAccessibilityClientsActive())
        return;

    auto nsPriority = [priority]
    {
        // The below doesn't get noticed by the @available check above
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability")

        switch (priority)
        {
            case AnnouncementPriority::low:    return NSAccessibilityPriorityLow;
            case AnnouncementPriority::medium: return NSAccessibilityPriorityMedium;
            case AnnouncementPriority::high:   return NSAccessibilityPriorityHigh;
        }

        jassertfalse;
        return NSAccessibilityPriorityLow;

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }();

    sendAccessibilityEvent (static_cast<id> ([NSApp mainWindow]),
                            NSAccessibilityAnnouncementRequestedNotification,
                            @{ NSAccessibilityAnnouncementKey: juceStringToNS (announcementString),
                               NSAccessibilityPriorityKey:     @(nsPriority) });
}

} // namespace juce
