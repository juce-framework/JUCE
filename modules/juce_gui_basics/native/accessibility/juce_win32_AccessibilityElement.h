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

#define UIA_FullDescriptionPropertyId 30159
#define UIA_IsDialogPropertyId        30174

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
