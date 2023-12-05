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

#ifdef JUCE_GUI_BASICS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define NS_FORMAT_FUNCTION(F,A) // To avoid spurious warnings from GCC

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1
#define JUCE_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1

#include "juce_gui_basics.h"

#include <cctype>

//==============================================================================
#if JUCE_MAC
 #import <IOKit/pwr_mgt/IOPMLib.h>
 #import <MetalKit/MetalKit.h>

#elif JUCE_IOS
 #if JUCE_PUSH_NOTIFICATIONS
  #import <UserNotifications/UserNotifications.h>
 #endif

 #import <MetalKit/MetalKit.h>
 #import <UIKit/UIActivityViewController.h>

//==============================================================================
#elif JUCE_WINDOWS
 #include <windowsx.h>
 #include <vfw.h>
 #include <commdlg.h>
 #include <commctrl.h>
 #include <sapi.h>
 #include <dxgi.h>

 #if JUCE_MINGW
  // Some MinGW headers use 'new' as a parameter name
  JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wkeyword-macro")
  #define new new_
  JUCE_END_IGNORE_WARNINGS_GCC_LIKE
 #endif

 #include <uiautomation.h>

 #undef new

 #if JUCE_WEB_BROWSER
  #include <exdisp.h>
  #include <exdispid.h>
 #endif

 #if JUCE_MINGW
  #include <imm.h>
 #elif ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "vfw32.lib")
  #pragma comment(lib, "imm32.lib")
  #pragma comment(lib, "comctl32.lib")
  #pragma comment(lib, "dxgi.lib")

  #if JUCE_OPENGL
   #pragma comment(lib, "OpenGL32.Lib")
   #pragma comment(lib, "GlU32.Lib")
  #endif

  #if JUCE_DIRECT2D
   #pragma comment (lib, "Dwrite.lib")
   #pragma comment (lib, "D2d1.lib")
  #endif
 #endif
#endif

//==============================================================================
#include "detail/juce_AccessibilityHelpers.h"
#include "detail/juce_ButtonAccessibilityHandler.h"
#include "detail/juce_ScalingHelpers.h"
#include "detail/juce_ComponentHelpers.h"
#include "detail/juce_FocusHelpers.h"
#include "detail/juce_FocusRestorer.h"
#include "detail/juce_ViewportHelpers.h"
#include "detail/juce_LookAndFeelHelpers.h"
#include "detail/juce_PointerState.h"
#include "detail/juce_CustomMouseCursorInfo.h"
#include "detail/juce_MouseInputSourceImpl.h"
#include "detail/juce_MouseInputSourceList.h"
#include "detail/juce_ToolbarItemDragAndDropOverlayComponent.h"
#include "detail/juce_ScopedMessageBoxInterface.h"
#include "detail/juce_ScopedMessageBoxImpl.h"
#include "detail/juce_ScopedContentSharerInterface.h"
#include "detail/juce_ScopedContentSharerImpl.h"
#include "detail/juce_WindowingHelpers.h"
#include "detail/juce_AlertWindowHelpers.h"
#include "detail/juce_TopLevelWindowManager.h"

//==============================================================================
#if JUCE_IOS || JUCE_WINDOWS
 #include "native/juce_MultiTouchMapper.h"
#endif

#if JUCE_ANDROID || JUCE_WINDOWS || JUCE_IOS || JUCE_UNIT_TESTS
 #include "native/accessibility/juce_AccessibilityTextHelpers.h"
#endif

#if JUCE_MAC || JUCE_IOS
 #include "native/accessibility/juce_AccessibilitySharedCode_mac.mm"
 #include "native/juce_CGMetalLayerRenderer_mac.h"

 #if JUCE_IOS
  #include "native/juce_UIViewComponentPeer_ios.mm"
  #include "native/accessibility/juce_Accessibility_ios.mm"
  #include "native/juce_WindowUtils_ios.mm"
  #include "native/juce_Windowing_ios.mm"
  #include "native/juce_NativeMessageBox_ios.mm"
  #include "native/juce_NativeModalWrapperComponent_ios.h"
  #include "native/juce_FileChooser_ios.mm"

  #if JUCE_CONTENT_SHARING
   #include "native/juce_ContentSharer_ios.cpp"
  #endif

 #else
  #include "native/accessibility/juce_Accessibility_mac.mm"
  #include "native/juce_PerScreenDisplayLinks_mac.h"
  #include "native/juce_NSViewComponentPeer_mac.mm"
  #include "native/juce_WindowUtils_mac.mm"
  #include "native/juce_Windowing_mac.mm"
  #include "native/juce_NativeMessageBox_mac.mm"
  #include "native/juce_MainMenu_mac.mm"
  #include "native/juce_FileChooser_mac.mm"
 #endif

 #include "native/juce_MouseCursor_mac.mm"

#elif JUCE_WINDOWS
 #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
  #include <juce_audio_plugin_client/AAX/juce_AAX_Modifier_Injector.h>
 #endif
 #include "native/accessibility/juce_ComInterfaces_windows.h"
 #include "native/accessibility/juce_WindowsUIAWrapper_windows.h"
 #include "native/accessibility/juce_AccessibilityElement_windows.h"
 #include "native/accessibility/juce_UIAHelpers_windows.h"
 #include "native/accessibility/juce_UIAProviders_windows.h"
 #include "native/accessibility/juce_AccessibilityElement_windows.cpp"
 #include "native/accessibility/juce_Accessibility_windows.cpp"
 #include "native/juce_WindowsHooks_windows.h"
 #include "native/juce_WindowUtils_windows.cpp"
 #include "native/juce_Windowing_windows.cpp"
 #include "native/juce_WindowsHooks_windows.cpp"
 #include "native/juce_NativeMessageBox_windows.cpp"
 #include "native/juce_DragAndDrop_windows.cpp"
 #include "native/juce_FileChooser_windows.cpp"

#elif JUCE_LINUX || JUCE_BSD
 #include "native/juce_XSymbols_linux.cpp"
 #include "native/juce_DragAndDrop_linux.cpp"

 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

 #include "native/juce_ScopedWindowAssociation_linux.h"
 #include "native/juce_WindowUtils_linux.cpp"
 #include "native/juce_Windowing_linux.cpp"
 #include "native/juce_NativeMessageBox_linux.cpp"
 #include "native/juce_XWindowSystem_linux.cpp"

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE

 #include "native/juce_FileChooser_linux.cpp"

#elif JUCE_ANDROID

 #include "juce_core/files/juce_common_MimeTypes.h"
 #include "native/accessibility/juce_Accessibility_android.cpp"
 #include "native/juce_WindowUtils_android.cpp"
 #include "native/juce_Windowing_android.cpp"
 #include "native/juce_NativeMessageBox_android.cpp"
 #include "native/juce_FileChooser_android.cpp"

 #if JUCE_CONTENT_SHARING
  #include "native/juce_ContentSharer_android.cpp"
 #endif

#endif

//==============================================================================
// Depends on types defined in platform-specific windowing files
#include "mouse/juce_MouseCursor.cpp"

#if JUCE_UNIT_TESTS
 #include "native/accessibility/juce_AccessibilityTextHelpers_test.cpp"
#endif

//==============================================================================
#include "accessibility/juce_AccessibilityHandler.cpp"
#include "application/juce_Application.cpp"
#include "buttons/juce_ArrowButton.cpp"
#include "buttons/juce_Button.cpp"
#include "buttons/juce_DrawableButton.cpp"
#include "buttons/juce_HyperlinkButton.cpp"
#include "buttons/juce_ImageButton.cpp"
#include "buttons/juce_ShapeButton.cpp"
#include "buttons/juce_TextButton.cpp"
#include "buttons/juce_ToggleButton.cpp"
#include "buttons/juce_ToolbarButton.cpp"
#include "commands/juce_ApplicationCommandInfo.cpp"
#include "commands/juce_ApplicationCommandManager.cpp"
#include "commands/juce_ApplicationCommandTarget.cpp"
#include "commands/juce_KeyPressMappingSet.cpp"
#include "components/juce_Component.cpp"
#include "components/juce_ComponentListener.cpp"
#include "components/juce_FocusTraverser.cpp"
#include "components/juce_ModalComponentManager.cpp"
#include "desktop/juce_Desktop.cpp"
#include "desktop/juce_Displays.cpp"
#include "detail/juce_AccessibilityHelpers.cpp"
#include "drawables/juce_Drawable.cpp"
#include "drawables/juce_DrawableComposite.cpp"
#include "drawables/juce_DrawableImage.cpp"
#include "drawables/juce_DrawablePath.cpp"
#include "drawables/juce_DrawableRectangle.cpp"
#include "drawables/juce_DrawableShape.cpp"
#include "drawables/juce_DrawableText.cpp"
#include "drawables/juce_SVGParser.cpp"
#include "filebrowser/juce_ContentSharer.cpp"
#include "filebrowser/juce_DirectoryContentsDisplayComponent.cpp"
#include "filebrowser/juce_DirectoryContentsList.cpp"
#include "filebrowser/juce_FileBrowserComponent.cpp"
#include "filebrowser/juce_FileChooser.cpp"
#include "filebrowser/juce_FileChooserDialogBox.cpp"
#include "filebrowser/juce_FileListComponent.cpp"
#include "filebrowser/juce_FileSearchPathListComponent.cpp"
#include "filebrowser/juce_FileTreeComponent.cpp"
#include "filebrowser/juce_FilenameComponent.cpp"
#include "filebrowser/juce_ImagePreviewComponent.cpp"
#include "keyboard/juce_CaretComponent.cpp"
#include "keyboard/juce_KeyListener.cpp"
#include "keyboard/juce_KeyPress.cpp"
#include "keyboard/juce_KeyboardFocusTraverser.cpp"
#include "keyboard/juce_ModifierKeys.cpp"
#include "layout/juce_ComponentAnimator.cpp"
#include "layout/juce_ComponentBoundsConstrainer.cpp"
#include "layout/juce_BorderedComponentBoundsConstrainer.cpp"
#include "layout/juce_ComponentBuilder.cpp"
#include "layout/juce_ComponentMovementWatcher.cpp"
#include "layout/juce_ConcertinaPanel.cpp"
#include "layout/juce_FlexBox.cpp"
#include "layout/juce_Grid.cpp"
#include "layout/juce_GridItem.cpp"
#include "layout/juce_GroupComponent.cpp"
#include "layout/juce_MultiDocumentPanel.cpp"
#include "layout/juce_ResizableBorderComponent.cpp"
#include "layout/juce_ResizableCornerComponent.cpp"
#include "layout/juce_ResizableEdgeComponent.cpp"
#include "layout/juce_ScrollBar.cpp"
#include "layout/juce_SidePanel.cpp"
#include "layout/juce_StretchableLayoutManager.cpp"
#include "layout/juce_StretchableLayoutResizerBar.cpp"
#include "layout/juce_StretchableObjectResizer.cpp"
#include "layout/juce_TabbedButtonBar.cpp"
#include "layout/juce_TabbedComponent.cpp"
#include "layout/juce_Viewport.cpp"
#include "lookandfeel/juce_LookAndFeel.cpp"
#include "lookandfeel/juce_LookAndFeel_V1.cpp"
#include "lookandfeel/juce_LookAndFeel_V2.cpp"
#include "lookandfeel/juce_LookAndFeel_V3.cpp"
#include "lookandfeel/juce_LookAndFeel_V4.cpp"
#include "menus/juce_BurgerMenuComponent.cpp"
#include "menus/juce_MenuBarComponent.cpp"
#include "menus/juce_MenuBarModel.cpp"
#include "menus/juce_PopupMenu.cpp"
#include "misc/juce_BubbleComponent.cpp"
#include "misc/juce_DropShadower.cpp"
#include "misc/juce_FocusOutline.cpp"
#include "misc/juce_JUCESplashScreen.cpp"
#include "mouse/juce_ComponentDragger.cpp"
#include "mouse/juce_DragAndDropContainer.cpp"
#include "mouse/juce_MouseEvent.cpp"
#include "mouse/juce_MouseInactivityDetector.cpp"
#include "mouse/juce_MouseInputSource.cpp"
#include "mouse/juce_MouseListener.cpp"
#include "native/accessibility/juce_Accessibility.cpp"
#include "native/juce_ScopedDPIAwarenessDisabler.cpp"
#include "positioning/juce_MarkerList.cpp"
#include "positioning/juce_RelativeCoordinate.cpp"
#include "positioning/juce_RelativeCoordinatePositioner.cpp"
#include "positioning/juce_RelativeParallelogram.cpp"
#include "positioning/juce_RelativePoint.cpp"
#include "positioning/juce_RelativePointPath.cpp"
#include "positioning/juce_RelativeRectangle.cpp"
#include "properties/juce_BooleanPropertyComponent.cpp"
#include "properties/juce_ButtonPropertyComponent.cpp"
#include "properties/juce_ChoicePropertyComponent.cpp"
#include "properties/juce_MultiChoicePropertyComponent.cpp"
#include "properties/juce_PropertyComponent.cpp"
#include "properties/juce_PropertyPanel.cpp"
#include "properties/juce_SliderPropertyComponent.cpp"
#include "properties/juce_TextPropertyComponent.cpp"
#include "widgets/juce_ComboBox.cpp"
#include "widgets/juce_ImageComponent.cpp"
#include "widgets/juce_Label.cpp"
#include "widgets/juce_ListBox.cpp"
#include "widgets/juce_ProgressBar.cpp"
#include "widgets/juce_Slider.cpp"
#include "widgets/juce_TableHeaderComponent.cpp"
#include "widgets/juce_TableListBox.cpp"
#include "widgets/juce_TextEditor.cpp"
#include "widgets/juce_Toolbar.cpp"
#include "widgets/juce_ToolbarItemComponent.cpp"
#include "widgets/juce_ToolbarItemPalette.cpp"
#include "widgets/juce_TreeView.cpp"
#include "windows/juce_NativeMessageBox.cpp"
#include "windows/juce_AlertWindow.cpp"
#include "windows/juce_CallOutBox.cpp"
#include "windows/juce_ComponentPeer.cpp"
#include "windows/juce_DialogWindow.cpp"
#include "windows/juce_DocumentWindow.cpp"
#include "windows/juce_MessageBoxOptions.cpp"
#include "windows/juce_ResizableWindow.cpp"
#include "windows/juce_ScopedMessageBox.cpp"
#include "windows/juce_ThreadWithProgressWindow.cpp"
#include "windows/juce_TooltipWindow.cpp"
#include "windows/juce_TopLevelWindow.cpp"
#include "windows/juce_VBlankAttachment.cpp"
#include "windows/juce_NativeScaleFactorNotifier.cpp"
