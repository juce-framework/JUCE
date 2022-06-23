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

namespace juce
{

//==============================================================================
struct AccessibleObjCClassDeleter
{
    template <typename ElementType>
    void operator() (ElementType* element) const
    {
        juceFreeAccessibilityPlatformSpecificData (element);

        object_setInstanceVariable (element, "handler", nullptr);
        [element release];
    }
};

template <typename Base>
class AccessibleObjCClass  : public ObjCClass<Base>
{
public:
    using Holder = std::unique_ptr<Base, AccessibleObjCClassDeleter>;

protected:
    AccessibleObjCClass()  : ObjCClass<Base> ("JUCEAccessibilityElement_")
    {
        ObjCClass<Base>::template addIvar<AccessibilityHandler*> ("handler");
    }

    //==============================================================================
    static AccessibilityHandler* getHandler (id self)
    {
        return getIvar<AccessibilityHandler*> (self, "handler");
    }

    template <typename MemberFn>
    static auto getInterface (id self, MemberFn fn) noexcept -> decltype ((std::declval<AccessibilityHandler>().*fn)())
    {
        if (auto* handler = getHandler (self))
            return (handler->*fn)();

        return nullptr;
    }

    static AccessibilityTextInterface*  getTextInterface  (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getTextInterface); }
    static AccessibilityValueInterface* getValueInterface (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getValueInterface); }
    static AccessibilityTableInterface* getTableInterface (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getTableInterface); }
    static AccessibilityCellInterface*  getCellInterface  (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getCellInterface); }

    template <typename MemberFn>
    static auto getEnclosingInterface (AccessibilityHandler* handler, MemberFn fn) noexcept -> decltype ((std::declval<AccessibilityHandler>().*fn)())
    {
        if (handler == nullptr)
            return nullptr;

        if (auto* interface = (handler->*fn)())
            return interface;

        return getEnclosingInterface (handler->getParent(), fn);
    }

    static bool hasEditableText (AccessibilityHandler& handler) noexcept
    {
        return handler.getRole() == AccessibilityRole::editableText
            && handler.getTextInterface() != nullptr
            && ! handler.getTextInterface()->isReadOnly();
    }

    static id getAccessibilityValueFromInterfaces (const AccessibilityHandler& handler)
    {
        if (auto* textInterface = handler.getTextInterface())
            return juceStringToNS (textInterface->getText ({ 0, textInterface->getTotalNumCharacters() }));

        if (auto* valueInterface = handler.getValueInterface())
            return juceStringToNS (valueInterface->getCurrentValueAsString());

        return nil;
    }

    //==============================================================================
    static BOOL getIsAccessibilityElement (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            return ! handler->isIgnored()
                  && handler->getRole() != AccessibilityRole::window;

        return NO;
    }

    static void setAccessibilityValue (id self, SEL, NSString* value)
    {
        if (auto* handler = getHandler (self))
        {
            if (hasEditableText (*handler))
            {
                handler->getTextInterface()->setText (nsStringToJuce (value));
                return;
            }

            if (auto* valueInterface = handler->getValueInterface())
                if (! valueInterface->isReadOnly())
                    valueInterface->setValueAsString (nsStringToJuce (value));
        }
    }

    static BOOL performActionIfSupported (id self, AccessibilityActionType actionType)
    {
        if (auto* handler = getHandler (self))
            if (handler->getActions().invoke (actionType))
                return YES;

        return NO;
    }

    static BOOL accessibilityPerformPress (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            if (handler->getCurrentState().isCheckable() && handler->getActions().invoke (AccessibilityActionType::toggle))
                return YES;

        return performActionIfSupported (self, AccessibilityActionType::press);
    }

    static BOOL accessibilityPerformIncrement (id self, SEL)
    {
        if (auto* valueInterface = getValueInterface (self))
        {
            if (! valueInterface->isReadOnly())
            {
                auto range = valueInterface->getRange();

                if (range.isValid())
                {
                    valueInterface->setValue (jlimit (range.getMinimumValue(),
                                                      range.getMaximumValue(),
                                                      valueInterface->getCurrentValue() + range.getInterval()));
                    return YES;
                }
            }
        }

        return NO;
    }

    static BOOL accessibilityPerformDecrement (id self, SEL)
    {
        if (auto* valueInterface = getValueInterface (self))
        {
            if (! valueInterface->isReadOnly())
            {
                auto range = valueInterface->getRange();

                if (range.isValid())
                {
                    valueInterface->setValue (jlimit (range.getMinimumValue(),
                                                      range.getMaximumValue(),
                                                      valueInterface->getCurrentValue() - range.getInterval()));
                    return YES;
                }
            }
        }

        return NO;
    }

    static NSString* getAccessibilityTitle (id self, SEL)
    {
        if (auto* handler = getHandler (self))
        {
            auto title = handler->getTitle();

            if (title.isEmpty() && handler->getComponent().isOnDesktop())
                title = getAccessibleApplicationOrPluginName();

            NSString* nsString = juceStringToNS (title);

           #if ! JUCE_IOS
            if (nsString != nil && [[self accessibilityValue] isEqual: nsString])
                return @"";
           #endif

            return nsString;
        }

        return nil;
    }

    static NSString* getAccessibilityHelp (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            return juceStringToNS (handler->getHelp());

        return nil;
    }

    static BOOL getIsAccessibilityModal (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            return handler->getComponent().isCurrentlyModal();

        return NO;
    }

    static NSInteger getAccessibilityRowCount (id self, SEL)
    {
        if (auto* tableInterface = getTableInterface (self))
            return tableInterface->getNumRows();

        return 0;
    }

    static NSInteger getAccessibilityColumnCount (id self, SEL)
    {
        if (auto* tableInterface = getTableInterface (self))
            return tableInterface->getNumColumns();

        return 0;
    }

    static NSRange getAccessibilityRowIndexRange (id self, SEL)
    {
        if (auto* cellInterface = getCellInterface (self))
            return NSMakeRange ((NSUInteger) cellInterface->getRowIndex(),
                                (NSUInteger) cellInterface->getRowSpan());

        return NSMakeRange (0, 0);
    }

    static NSRange getAccessibilityColumnIndexRange (id self, SEL)
    {
        if (auto* cellInterface = getCellInterface (self))
            return NSMakeRange ((NSUInteger) cellInterface->getColumnIndex(),
                                (NSUInteger) cellInterface->getColumnSpan());

        return NSMakeRange (0, 0);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibleObjCClass)
};

}
