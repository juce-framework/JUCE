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
namespace ComTypes
{

/*
    These interfaces would normally be included in the system platform headers.
    However, those headers are likely to be incomplete when building with
    MinGW. In order to allow building accessible applications under MinGW,
    we reproduce all necessary definitions here.
*/

struct UiaPoint
{
    double x;
    double y;
};

struct UiaRect
{
    double left;
    double top;
    double width;
    double height;
};

enum NavigateDirection
{
    NavigateDirection_Parent = 0,
    NavigateDirection_NextSibling = 1,
    NavigateDirection_PreviousSibling = 2,
    NavigateDirection_FirstChild = 3,
    NavigateDirection_LastChild = 4
};

enum ExpandCollapseState
{
    ExpandCollapseState_Collapsed = 0,
    ExpandCollapseState_Expanded = 1,
    ExpandCollapseState_PartiallyExpanded = 2,
    ExpandCollapseState_LeafNode = 3
};

enum TextPatternRangeEndpoint
{
    TextPatternRangeEndpoint_Start = 0,
    TextPatternRangeEndpoint_End = 1
};

enum TextUnit
{
    TextUnit_Character = 0,
    TextUnit_Format = 1,
    TextUnit_Word = 2,
    TextUnit_Line = 3,
    TextUnit_Paragraph = 4,
    TextUnit_Page = 5,
    TextUnit_Document = 6
};

enum SupportedTextSelection
{
    SupportedTextSelection_None = 0,
    SupportedTextSelection_Single = 1,
    SupportedTextSelection_Multiple = 2
};

enum CaretPosition
{
    CaretPosition_Unknown = 0,
    CaretPosition_EndOfLine = 1,
    CaretPosition_BeginningOfLine = 2
};

enum ToggleState
{
    ToggleState_Off = 0,
    ToggleState_On = 1,
    ToggleState_Indeterminate = 2
};

enum WindowVisualState
{
    WindowVisualState_Normal = 0,
    WindowVisualState_Maximized = 1,
    WindowVisualState_Minimized = 2
};

enum WindowInteractionState
{
    WindowInteractionState_Running = 0,
    WindowInteractionState_Closing = 1,
    WindowInteractionState_ReadyForUserInteraction = 2,
    WindowInteractionState_BlockedByModalWindow = 3,
    WindowInteractionState_NotResponding = 4
};

const long UIA_InvokePatternId = 10000;
const long UIA_SelectionPatternId = 10001;
const long UIA_ValuePatternId = 10002;
const long UIA_RangeValuePatternId = 10003;
const long UIA_ExpandCollapsePatternId = 10005;
const long UIA_GridPatternId = 10006;
const long UIA_GridItemPatternId = 10007;
const long UIA_WindowPatternId = 10009;
const long UIA_SelectionItemPatternId = 10010;
const long UIA_TextPatternId = 10014;
const long UIA_TogglePatternId = 10015;
const long UIA_TransformPatternId = 10016;
const long UIA_TextPattern2Id = 10024;
const long UIA_StructureChangedEventId = 20002;
const long UIA_MenuOpenedEventId = 20003;
const long UIA_AutomationFocusChangedEventId = 20005;
const long UIA_MenuClosedEventId = 20007;
const long UIA_LayoutInvalidatedEventId = 20008;
const long UIA_Invoke_InvokedEventId = 20009;
const long UIA_SelectionItem_ElementSelectedEventId = 20012;
const long UIA_Text_TextSelectionChangedEventId = 20014;
const long UIA_Text_TextChangedEventId = 20015;
const long UIA_Window_WindowOpenedEventId = 20016;
const long UIA_Window_WindowClosedEventId = 20017;
const long UIA_IsPeripheralPropertyId = 30150;
const long UIA_IsReadOnlyAttributeId = 40015;
const long UIA_CaretPositionAttributeId = 40038;
const long UIA_ButtonControlTypeId = 50000;
const long UIA_CheckBoxControlTypeId = 50002;
const long UIA_ComboBoxControlTypeId = 50003;
const long UIA_EditControlTypeId = 50004;
const long UIA_HyperlinkControlTypeId = 50005;
const long UIA_ImageControlTypeId = 50006;
const long UIA_ListItemControlTypeId = 50007;
const long UIA_ListControlTypeId = 50008;
const long UIA_MenuBarControlTypeId = 50010;
const long UIA_MenuItemControlTypeId = 50011;
const long UIA_ProgressBarControlTypeId = 50012;
const long UIA_RadioButtonControlTypeId = 50013;
const long UIA_ScrollBarControlTypeId = 50014;
const long UIA_SliderControlTypeId = 50015;
const long UIA_TextControlTypeId = 50020;
const long UIA_ToolTipControlTypeId = 50022;
const long UIA_TreeControlTypeId = 50023;
const long UIA_TreeItemControlTypeId = 50024;
const long UIA_CustomControlTypeId = 50025;
const long UIA_GroupControlTypeId = 50026;
const long UIA_DataItemControlTypeId = 50029;
const long UIA_WindowControlTypeId = 50032;
const long UIA_HeaderControlTypeId = 50034;
const long UIA_HeaderItemControlTypeId = 50035;
const long UIA_TableControlTypeId = 50036;

interface IRawElementProviderFragmentRoot;
interface IRawElementProviderFragment;

JUCE_COMCLASS (IRawElementProviderFragmentRoot, "620ce2a5-ab8f-40a9-86cb-de3c75599b58") : public IUnknown
{
public:
    JUCE_COMCALL ElementProviderFromPoint (double x, double y, __RPC__deref_out_opt IRawElementProviderFragment** pRetVal) = 0;
    JUCE_COMCALL GetFocus (__RPC__deref_out_opt IRawElementProviderFragment * *pRetVal) = 0;
};

JUCE_COMCLASS (IRawElementProviderFragment, "f7063da8-8359-439c-9297-bbc5299a7d87") : public IUnknown
{
public:
    JUCE_COMCALL Navigate (NavigateDirection direction, __RPC__deref_out_opt IRawElementProviderFragment** pRetVal) = 0;
    JUCE_COMCALL GetRuntimeId (__RPC__deref_out_opt SAFEARRAY * *pRetVal) = 0;
    JUCE_COMCALL get_BoundingRectangle (__RPC__out UiaRect * pRetVal) = 0;
    JUCE_COMCALL GetEmbeddedFragmentRoots (__RPC__deref_out_opt SAFEARRAY * *pRetVal) = 0;
    JUCE_COMCALL SetFocus() = 0;
    JUCE_COMCALL get_FragmentRoot (__RPC__deref_out_opt IRawElementProviderFragmentRoot * *pRetVal) = 0;
};

JUCE_COMCLASS (IExpandCollapseProvider, "d847d3a5-cab0-4a98-8c32-ecb45c59ad24") : public IUnknown
{
public:
    JUCE_COMCALL Expand() = 0;
    JUCE_COMCALL Collapse() = 0;
    JUCE_COMCALL get_ExpandCollapseState (__RPC__out ExpandCollapseState * pRetVal) = 0;
};

JUCE_COMCLASS (IGridItemProvider, "d02541f1-fb81-4d64-ae32-f520f8a6dbd1") : public IUnknown
{
public:
    JUCE_COMCALL get_Row (__RPC__out int* pRetVal) = 0;
    JUCE_COMCALL get_Column (__RPC__out int* pRetVal) = 0;
    JUCE_COMCALL get_RowSpan (__RPC__out int* pRetVal) = 0;
    JUCE_COMCALL get_ColumnSpan (__RPC__out int* pRetVal) = 0;
    JUCE_COMCALL get_ContainingGrid (__RPC__deref_out_opt IRawElementProviderSimple * *pRetVal) = 0;
};

JUCE_COMCLASS (IGridProvider, "b17d6187-0907-464b-a168-0ef17a1572b1") : public IUnknown
{
public:
    JUCE_COMCALL GetItem (int row, int column, __RPC__deref_out_opt IRawElementProviderSimple** pRetVal) = 0;
    JUCE_COMCALL get_RowCount (__RPC__out int* pRetVal) = 0;
    JUCE_COMCALL get_ColumnCount (__RPC__out int* pRetVal) = 0;
};

JUCE_COMCLASS (IInvokeProvider, "54fcb24b-e18e-47a2-b4d3-eccbe77599a2") : public IUnknown
{
public:
    JUCE_COMCALL Invoke() = 0;
};

JUCE_COMCLASS (IRangeValueProvider, "36dc7aef-33e6-4691-afe1-2be7274b3d33") : public IUnknown
{
public:
    JUCE_COMCALL SetValue (double val) = 0;
    JUCE_COMCALL get_Value (__RPC__out double* pRetVal) = 0;
    JUCE_COMCALL get_IsReadOnly (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_Maximum (__RPC__out double* pRetVal) = 0;
    JUCE_COMCALL get_Minimum (__RPC__out double* pRetVal) = 0;
    JUCE_COMCALL get_LargeChange (__RPC__out double* pRetVal) = 0;
    JUCE_COMCALL get_SmallChange (__RPC__out double* pRetVal) = 0;
};

JUCE_COMCLASS (ISelectionProvider, "fb8b03af-3bdf-48d4-bd36-1a65793be168") : public IUnknown
{
public:
    JUCE_COMCALL GetSelection (__RPC__deref_out_opt SAFEARRAY * *pRetVal) = 0;
    JUCE_COMCALL get_CanSelectMultiple (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_IsSelectionRequired (__RPC__out BOOL * pRetVal) = 0;
};

JUCE_COMCLASS (ISelectionProvider2, "14f68475-ee1c-44f6-a869-d239381f0fe7") : public ISelectionProvider
{
    JUCE_COMCALL get_FirstSelectedItem (IRawElementProviderSimple * *retVal) = 0;
    JUCE_COMCALL get_LastSelectedItem (IRawElementProviderSimple * *retVal) = 0;
    JUCE_COMCALL get_CurrentSelectedItem (IRawElementProviderSimple * *retVal) = 0;
    JUCE_COMCALL get_ItemCount (int* retVal) = 0;
};

JUCE_COMCLASS (ISelectionItemProvider, "2acad808-b2d4-452d-a407-91ff1ad167b2") : public IUnknown
{
public:
    JUCE_COMCALL Select() = 0;
    JUCE_COMCALL AddToSelection() = 0;
    JUCE_COMCALL RemoveFromSelection() = 0;
    JUCE_COMCALL get_IsSelected (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_SelectionContainer (__RPC__deref_out_opt IRawElementProviderSimple * *pRetVal) = 0;
};

JUCE_COMCLASS (ITextRangeProvider, "5347ad7b-c355-46f8-aff5-909033582f63") : public IUnknown
{
public:
    JUCE_COMCALL Clone (__RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
    JUCE_COMCALL Compare (__RPC__in_opt ITextRangeProvider * range, __RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL CompareEndpoints (TextPatternRangeEndpoint endpoint, __RPC__in_opt ITextRangeProvider * targetRange, TextPatternRangeEndpoint targetEndpoint, __RPC__out int* pRetVal) = 0;
    JUCE_COMCALL ExpandToEnclosingUnit (TextUnit unit) = 0;
    JUCE_COMCALL FindAttribute (TEXTATTRIBUTEID attributeId, VARIANT val, BOOL backward, __RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
    JUCE_COMCALL FindText (__RPC__in BSTR text, BOOL backward, BOOL ignoreCase, __RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
    JUCE_COMCALL GetAttributeValue (TEXTATTRIBUTEID attributeId, __RPC__out VARIANT * pRetVal) = 0;
    JUCE_COMCALL GetBoundingRectangles (__RPC__deref_out_opt SAFEARRAY * *pRetVal) = 0;
    JUCE_COMCALL GetEnclosingElement (__RPC__deref_out_opt IRawElementProviderSimple * *pRetVal) = 0;
    JUCE_COMCALL GetText (int maxLength, __RPC__deref_out_opt BSTR* pRetVal) = 0;
    JUCE_COMCALL Move (TextUnit unit, int count, __RPC__out int* pRetVal) = 0;
    JUCE_COMCALL MoveEndpointByUnit (TextPatternRangeEndpoint endpoint, TextUnit unit, int count, __RPC__out int* pRetVal) = 0;
    JUCE_COMCALL MoveEndpointByRange (TextPatternRangeEndpoint endpoint, __RPC__in_opt ITextRangeProvider * targetRange, TextPatternRangeEndpoint targetEndpoint) = 0;
    JUCE_COMCALL Select() = 0;
    JUCE_COMCALL AddToSelection() = 0;
    JUCE_COMCALL RemoveFromSelection() = 0;
    JUCE_COMCALL ScrollIntoView (BOOL alignToTop) = 0;
    JUCE_COMCALL GetChildren (__RPC__deref_out_opt SAFEARRAY * *pRetVal) = 0;
};

JUCE_COMCLASS (ITextProvider, "3589c92c-63f3-4367-99bb-ada653b77cf2") : public IUnknown
{
public:
    JUCE_COMCALL GetSelection (__RPC__deref_out_opt SAFEARRAY * *pRetVal) = 0;
    JUCE_COMCALL GetVisibleRanges (__RPC__deref_out_opt SAFEARRAY * *pRetVal) = 0;
    JUCE_COMCALL RangeFromChild (__RPC__in_opt IRawElementProviderSimple * childElement, __RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
    JUCE_COMCALL RangeFromPoint (UiaPoint point, __RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
    JUCE_COMCALL get_DocumentRange (__RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
    JUCE_COMCALL get_SupportedTextSelection (__RPC__out SupportedTextSelection * pRetVal) = 0;
};

JUCE_COMCLASS (ITextProvider2, "0dc5e6ed-3e16-4bf1-8f9a-a979878bc195") : public ITextProvider
{
public:
    JUCE_COMCALL RangeFromAnnotation (__RPC__in_opt IRawElementProviderSimple * annotationElement, __RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
    JUCE_COMCALL GetCaretRange (__RPC__out BOOL * isActive, __RPC__deref_out_opt ITextRangeProvider * *pRetVal) = 0;
};

JUCE_COMCLASS (IToggleProvider, "56d00bd0-c4f4-433c-a836-1a52a57e0892") : public IUnknown
{
public:
    JUCE_COMCALL Toggle() = 0;
    JUCE_COMCALL get_ToggleState (__RPC__out ToggleState * pRetVal) = 0;
};

JUCE_COMCLASS (ITransformProvider, "6829ddc4-4f91-4ffa-b86f-bd3e2987cb4c") : public IUnknown
{
public:
    JUCE_COMCALL Move (double x, double y) = 0;
    JUCE_COMCALL Resize (double width, double height) = 0;
    JUCE_COMCALL Rotate (double degrees) = 0;
    JUCE_COMCALL get_CanMove (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_CanResize (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_CanRotate (__RPC__out BOOL * pRetVal) = 0;
};

JUCE_COMCLASS (IValueProvider, "c7935180-6fb3-4201-b174-7df73adbf64a") : public IUnknown
{
public:
    JUCE_COMCALL SetValue (__RPC__in LPCWSTR val) = 0;
    JUCE_COMCALL get_Value (__RPC__deref_out_opt BSTR * pRetVal) = 0;
    JUCE_COMCALL get_IsReadOnly (__RPC__out BOOL * pRetVal) = 0;
};

JUCE_COMCLASS (IWindowProvider, "987df77b-db06-4d77-8f8a-86a9c3bb90b9") : public IUnknown
{
public:
    JUCE_COMCALL SetVisualState (WindowVisualState state) = 0;
    JUCE_COMCALL Close() = 0;
    JUCE_COMCALL WaitForInputIdle (int milliseconds, __RPC__out BOOL* pRetVal) = 0;
    JUCE_COMCALL get_CanMaximize (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_CanMinimize (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_IsModal (__RPC__out BOOL * pRetVal) = 0;
    JUCE_COMCALL get_WindowVisualState (__RPC__out WindowVisualState * pRetVal) = 0;
    JUCE_COMCALL get_WindowInteractionState (__RPC__out WindowInteractionState * pRetVal) = 0;
    JUCE_COMCALL get_IsTopmost (__RPC__out BOOL * pRetVal) = 0;
};

constexpr CLSID CLSID_SpVoice { 0x96749377, 0x3391, 0x11D2, { 0x9E, 0xE3, 0x00, 0xC0, 0x4F, 0x79, 0x73, 0x96 } };

} // namespace ComTypes
} // namespace juce

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (juce::ComTypes::IRawElementProviderFragmentRoot, 0x620ce2a5, 0xab8f, 0x40a9, 0x86, 0xcb, 0xde, 0x3c, 0x75, 0x59, 0x9b, 0x58)
__CRT_UUID_DECL (juce::ComTypes::IRawElementProviderFragment,     0xf7063da8, 0x8359, 0x439c, 0x92, 0x97, 0xbb, 0xc5, 0x29, 0x9a, 0x7d, 0x87)
__CRT_UUID_DECL (juce::ComTypes::IExpandCollapseProvider,         0xd847d3a5, 0xcab0, 0x4a98, 0x8c, 0x32, 0xec, 0xb4, 0x5c, 0x59, 0xad, 0x24)
__CRT_UUID_DECL (juce::ComTypes::IGridItemProvider,               0xd02541f1, 0xfb81, 0x4d64, 0xae, 0x32, 0xf5, 0x20, 0xf8, 0xa6, 0xdb, 0xd1)
__CRT_UUID_DECL (juce::ComTypes::IGridProvider,                   0xb17d6187, 0x0907, 0x464b, 0xa1, 0x68, 0x0e, 0xf1, 0x7a, 0x15, 0x72, 0xb1)
__CRT_UUID_DECL (juce::ComTypes::IInvokeProvider,                 0x54fcb24b, 0xe18e, 0x47a2, 0xb4, 0xd3, 0xec, 0xcb, 0xe7, 0x75, 0x99, 0xa2)
__CRT_UUID_DECL (juce::ComTypes::IRangeValueProvider,             0x36dc7aef, 0x33e6, 0x4691, 0xaf, 0xe1, 0x2b, 0xe7, 0x27, 0x4b, 0x3d, 0x33)
__CRT_UUID_DECL (juce::ComTypes::ISelectionProvider,              0xfb8b03af, 0x3bdf, 0x48d4, 0xbd, 0x36, 0x1a, 0x65, 0x79, 0x3b, 0xe1, 0x68)
__CRT_UUID_DECL (juce::ComTypes::ISelectionProvider2,             0x14f68475, 0xee1c, 0x44f6, 0xa8, 0x69, 0xd2, 0x39, 0x38, 0x1f, 0x0f, 0xe7)
__CRT_UUID_DECL (juce::ComTypes::ISelectionItemProvider,          0x2acad808, 0xb2d4, 0x452d, 0xa4, 0x07, 0x91, 0xff, 0x1a, 0xd1, 0x67, 0xb2)
__CRT_UUID_DECL (juce::ComTypes::ITextRangeProvider,              0x5347ad7b, 0xc355, 0x46f8, 0xaf, 0xf5, 0x90, 0x90, 0x33, 0x58, 0x2f, 0x63)
__CRT_UUID_DECL (juce::ComTypes::ITextProvider,                   0x3589c92c, 0x63f3, 0x4367, 0x99, 0xbb, 0xad, 0xa6, 0x53, 0xb7, 0x7c, 0xf2)
__CRT_UUID_DECL (juce::ComTypes::ITextProvider2,                  0x0dc5e6ed, 0x3e16, 0x4bf1, 0x8f, 0x9a, 0xa9, 0x79, 0x87, 0x8b, 0xc1, 0x95)
__CRT_UUID_DECL (juce::ComTypes::IToggleProvider,                 0x56d00bd0, 0xc4f4, 0x433c, 0xa8, 0x36, 0x1a, 0x52, 0xa5, 0x7e, 0x08, 0x92)
__CRT_UUID_DECL (juce::ComTypes::ITransformProvider,              0x6829ddc4, 0x4f91, 0x4ffa, 0xb8, 0x6f, 0xbd, 0x3e, 0x29, 0x87, 0xcb, 0x4c)
__CRT_UUID_DECL (juce::ComTypes::IValueProvider,                  0xc7935180, 0x6fb3, 0x4201, 0xb1, 0x74, 0x7d, 0xf7, 0x3a, 0xdb, 0xf6, 0x4a)
__CRT_UUID_DECL (juce::ComTypes::IWindowProvider,                 0x987df77b, 0xdb06, 0x4d77, 0x8f, 0x8a, 0x86, 0xa9, 0xc3, 0xbb, 0x90, 0xb9)
#endif
