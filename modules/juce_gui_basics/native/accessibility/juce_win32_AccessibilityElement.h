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

#define UIA_InvokePatternId 10000
#define UIA_SelectionPatternId 10001
#define UIA_ValuePatternId 10002
#define UIA_RangeValuePatternId 10003
#define UIA_ExpandCollapsePatternId 10005
#define UIA_GridPatternId 10006
#define UIA_GridItemPatternId 10007
#define UIA_WindowPatternId 10009
#define UIA_SelectionItemPatternId 10010
#define UIA_TextPatternId 10014
#define UIA_TogglePatternId 10015
#define UIA_TransformPatternId 10016
#define UIA_TextPattern2Id 10024
#define UIA_StructureChangedEventId 20002
#define UIA_MenuOpenedEventId 20003
#define UIA_AutomationFocusChangedEventId 20005
#define UIA_MenuClosedEventId 20007
#define UIA_LayoutInvalidatedEventId 20008
#define UIA_Invoke_InvokedEventId 20009
#define UIA_SelectionItem_ElementSelectedEventId 20012
#define UIA_Text_TextSelectionChangedEventId 20014
#define UIA_Text_TextChangedEventId 20015
#define UIA_Window_WindowOpenedEventId 20016
#define UIA_Window_WindowClosedEventId 20017
#define UIA_ProcessIdPropertyId 30002
#define UIA_ControlTypePropertyId 30003
#define UIA_NamePropertyId 30005
#define UIA_HasKeyboardFocusPropertyId 30008
#define UIA_IsKeyboardFocusablePropertyId 30009
#define UIA_IsEnabledPropertyId 30010
#define UIA_AutomationIdPropertyId 30011
#define UIA_HelpTextPropertyId 30013
#define UIA_IsControlElementPropertyId 30016
#define UIA_IsContentElementPropertyId 30017
#define UIA_IsPasswordPropertyId 30019
#define UIA_NativeWindowHandlePropertyId 30020
#define UIA_IsOffscreenPropertyId 30022
#define UIA_FrameworkIdPropertyId 30024
#define UIA_ValueValuePropertyId 30045
#define UIA_RangeValueValuePropertyId 30047
#define UIA_ToggleToggleStatePropertyId 30086
#define UIA_IsPeripheralPropertyId 30150
#define UIA_FullDescriptionPropertyId 30159
#define UIA_IsDialogPropertyId 30174
#define UIA_IsReadOnlyAttributeId 40015
#define UIA_CaretPositionAttributeId 40038
#define UIA_ButtonControlTypeId 50000
#define UIA_CheckBoxControlTypeId 50002
#define UIA_ComboBoxControlTypeId 50003
#define UIA_EditControlTypeId 50004
#define UIA_HyperlinkControlTypeId 50005
#define UIA_ImageControlTypeId 50006
#define UIA_ListItemControlTypeId 50007
#define UIA_ListControlTypeId 50008
#define UIA_MenuBarControlTypeId 50010
#define UIA_MenuItemControlTypeId 50011
#define UIA_ProgressBarControlTypeId 50012
#define UIA_RadioButtonControlTypeId 50013
#define UIA_ScrollBarControlTypeId 50014
#define UIA_SliderControlTypeId 50015
#define UIA_TextControlTypeId 50020
#define UIA_ToolTipControlTypeId 50022
#define UIA_TreeControlTypeId 50023
#define UIA_TreeItemControlTypeId 50024
#define UIA_CustomControlTypeId 50025
#define UIA_GroupControlTypeId 50026
#define UIA_DataItemControlTypeId 50029
#define UIA_WindowControlTypeId 50032
#define UIA_HeaderControlTypeId 50034
#define UIA_HeaderItemControlTypeId 50035
#define UIA_TableControlTypeId 50036

class AccessibilityNativeHandle  : public ComBaseClassHelper<IRawElementProviderSimple,
                                                             ComTypes::IRawElementProviderFragment,
                                                             ComTypes::IRawElementProviderFragmentRoot>
{
public:
    explicit AccessibilityNativeHandle (AccessibilityHandler& handler);

    //==============================================================================
    void invalidateElement() noexcept         { valid = false; }
    bool isElementValid() const noexcept      { return valid; }

    const AccessibilityHandler& getHandler()  { return accessibilityHandler; }

    //==============================================================================
    JUCE_COMRESULT QueryInterface (REFIID refId, void** result) override;

    //==============================================================================
    JUCE_COMRESULT get_HostRawElementProvider (IRawElementProviderSimple** provider) override;
    JUCE_COMRESULT get_ProviderOptions (ProviderOptions* options) override;
    JUCE_COMRESULT GetPatternProvider (PATTERNID pId, IUnknown** provider) override;
    JUCE_COMRESULT GetPropertyValue (PROPERTYID propertyId, VARIANT* pRetVal) override;

    JUCE_COMRESULT Navigate (ComTypes::NavigateDirection direction, ComTypes::IRawElementProviderFragment** pRetVal) override;
    JUCE_COMRESULT GetRuntimeId (SAFEARRAY** pRetVal) override;
    JUCE_COMRESULT get_BoundingRectangle (ComTypes::UiaRect* pRetVal) override;
    JUCE_COMRESULT GetEmbeddedFragmentRoots (SAFEARRAY** pRetVal) override;
    JUCE_COMRESULT SetFocus() override;
    JUCE_COMRESULT get_FragmentRoot (ComTypes::IRawElementProviderFragmentRoot** pRetVal) override;

    JUCE_COMRESULT ElementProviderFromPoint (double x, double y, ComTypes::IRawElementProviderFragment** pRetVal) override;
    JUCE_COMRESULT GetFocus (ComTypes::IRawElementProviderFragment** pRetVal) override;

private:
    //==============================================================================
    String getElementName() const;
    bool isFragmentRoot() const     { return accessibilityHandler.getComponent().isOnDesktop(); }

    //==============================================================================
    AccessibilityHandler& accessibilityHandler;

    static int idCounter;
    std::array<int, 2> rtid { UiaAppendRuntimeId, ++idCounter };
    bool valid = true;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeHandle)
};

}
