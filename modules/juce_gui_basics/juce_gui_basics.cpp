/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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

#include "juce_gui_basics.h"

//==============================================================================
#if JUCE_MAC
 #import <WebKit/WebKit.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>

 #if JUCE_SUPPORT_CARBON
  #import <Carbon/Carbon.h> // still needed for SetSystemUIMode()
 #endif

//==============================================================================
#elif JUCE_WINDOWS
 #include <windowsx.h>
 #include <vfw.h>
 #include <commdlg.h>

 #if JUCE_WEB_BROWSER
  #include <exdisp.h>
  #include <exdispid.h>
 #endif

 #if JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "vfw32.lib")
  #pragma comment(lib, "imm32.lib")
 #endif

 #if JUCE_OPENGL
  #if JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
   #pragma comment(lib, "OpenGL32.Lib")
   #pragma comment(lib, "GlU32.Lib")
  #endif
 #endif

 #if JUCE_DIRECT2D && JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment (lib, "Dwrite.lib")
  #pragma comment (lib, "D2d1.lib")
 #endif

 #if JUCE_MINGW
  #include <imm.h>
 #endif

//==============================================================================
#elif JUCE_LINUX
 #include <X11/Xlib.h>
 #include <X11/Xatom.h>
 #include <X11/Xresource.h>
 #include <X11/Xutil.h>
 #include <X11/Xmd.h>
 #include <X11/keysym.h>
 #include <X11/XKBlib.h>
 #include <X11/cursorfont.h>
 #include <unistd.h>

 #if JUCE_USE_XRANDR
  /* If you're trying to use Xrandr, you'll need to install the "libxrandr-dev" package..  */
  #include <X11/extensions/Xrandr.h>
 #endif

 #if JUCE_USE_XINERAMA
  /* If you're trying to use Xinerama, you'll need to install the "libxinerama-dev" package..  */
  #include <X11/extensions/Xinerama.h>
 #endif

 #if JUCE_USE_XSHM
  #include <X11/extensions/XShm.h>
  #include <sys/shm.h>
  #include <sys/ipc.h>
 #endif

 #if JUCE_USE_XRENDER
  // If you're missing these headers, try installing the libxrender-dev and libxcomposite-dev
  #include <X11/extensions/Xrender.h>
  #include <X11/extensions/Xcomposite.h>
 #endif

 #if JUCE_USE_XCURSOR
  // If you're missing this header, try installing the libxcursor-dev package
  #include <X11/Xcursor/Xcursor.h>
 #endif

 #undef SIZEOF
 #undef KeyPress
#endif

#include <map>
#include <set>

//==============================================================================
#define ASSERT_MESSAGE_MANAGER_IS_LOCKED \
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

#define ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN \
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager() || getPeer() == nullptr);

namespace juce
{
    extern bool juce_areThereAnyAlwaysOnTopWindows();
}

#include "components/juce_Component.cpp"
#include "components/juce_ComponentListener.cpp"
#include "mouse/juce_MouseInputSource.cpp"
#include "components/juce_Desktop.cpp"
#include "components/juce_ModalComponentManager.cpp"
#include "mouse/juce_ComponentDragger.cpp"
#include "mouse/juce_DragAndDropContainer.cpp"
#include "mouse/juce_MouseCursor.cpp"
#include "mouse/juce_MouseEvent.cpp"
#include "mouse/juce_MouseInactivityDetector.cpp"
#include "mouse/juce_MouseListener.cpp"
#include "keyboard/juce_CaretComponent.cpp"
#include "keyboard/juce_KeyboardFocusTraverser.cpp"
#include "keyboard/juce_KeyListener.cpp"
#include "keyboard/juce_KeyPress.cpp"
#include "keyboard/juce_ModifierKeys.cpp"
#include "buttons/juce_ArrowButton.cpp"
#include "buttons/juce_Button.cpp"
#include "buttons/juce_DrawableButton.cpp"
#include "buttons/juce_HyperlinkButton.cpp"
#include "buttons/juce_ImageButton.cpp"
#include "buttons/juce_ShapeButton.cpp"
#include "buttons/juce_TextButton.cpp"
#include "buttons/juce_ToggleButton.cpp"
#include "buttons/juce_ToolbarButton.cpp"
#include "drawables/juce_Drawable.cpp"
#include "drawables/juce_DrawableComposite.cpp"
#include "drawables/juce_DrawableImage.cpp"
#include "drawables/juce_DrawablePath.cpp"
#include "drawables/juce_DrawableRectangle.cpp"
#include "drawables/juce_DrawableShape.cpp"
#include "drawables/juce_DrawableText.cpp"
#include "drawables/juce_SVGParser.cpp"
#include "filebrowser/juce_DirectoryContentsDisplayComponent.cpp"
#include "filebrowser/juce_DirectoryContentsList.cpp"
#include "filebrowser/juce_FileBrowserComponent.cpp"
#include "filebrowser/juce_FileChooser.cpp"
#include "filebrowser/juce_FileChooserDialogBox.cpp"
#include "filebrowser/juce_FileListComponent.cpp"
#include "filebrowser/juce_FilenameComponent.cpp"
#include "filebrowser/juce_FileSearchPathListComponent.cpp"
#include "filebrowser/juce_FileTreeComponent.cpp"
#include "filebrowser/juce_ImagePreviewComponent.cpp"
#include "layout/juce_ComponentAnimator.cpp"
#include "layout/juce_ComponentBoundsConstrainer.cpp"
#include "layout/juce_ComponentBuilder.cpp"
#include "layout/juce_ComponentMovementWatcher.cpp"
#include "layout/juce_ConcertinaPanel.cpp"
#include "layout/juce_GroupComponent.cpp"
#include "layout/juce_MultiDocumentPanel.cpp"
#include "layout/juce_ResizableBorderComponent.cpp"
#include "layout/juce_ResizableCornerComponent.cpp"
#include "layout/juce_ResizableEdgeComponent.cpp"
#include "layout/juce_ScrollBar.cpp"
#include "layout/juce_StretchableLayoutManager.cpp"
#include "layout/juce_StretchableLayoutResizerBar.cpp"
#include "layout/juce_StretchableObjectResizer.cpp"
#include "layout/juce_TabbedButtonBar.cpp"
#include "layout/juce_TabbedComponent.cpp"
#include "layout/juce_Viewport.cpp"
#include "lookandfeel/juce_LookAndFeel.cpp"
#include "lookandfeel/juce_LookAndFeel_V2.cpp"
#include "lookandfeel/juce_LookAndFeel_V1.cpp"
#include "lookandfeel/juce_LookAndFeel_V3.cpp"
#include "lookandfeel/juce_LookAndFeel_V4.cpp"
#include "menus/juce_MenuBarComponent.cpp"
#include "menus/juce_MenuBarModel.cpp"
#include "menus/juce_PopupMenu.cpp"
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
#include "widgets/juce_ToolbarItemComponent.cpp"
#include "widgets/juce_Toolbar.cpp"
#include "widgets/juce_ToolbarItemPalette.cpp"
#include "widgets/juce_TreeView.cpp"
#include "windows/juce_AlertWindow.cpp"
#include "windows/juce_CallOutBox.cpp"
#include "windows/juce_ComponentPeer.cpp"
#include "windows/juce_DialogWindow.cpp"
#include "windows/juce_DocumentWindow.cpp"
#include "windows/juce_ResizableWindow.cpp"
#include "windows/juce_ThreadWithProgressWindow.cpp"
#include "windows/juce_TooltipWindow.cpp"
#include "windows/juce_TopLevelWindow.cpp"
#include "commands/juce_ApplicationCommandInfo.cpp"
#include "commands/juce_ApplicationCommandManager.cpp"
#include "commands/juce_ApplicationCommandTarget.cpp"
#include "commands/juce_KeyPressMappingSet.cpp"
#include "application/juce_Application.cpp"
#include "misc/juce_BubbleComponent.cpp"
#include "misc/juce_DropShadower.cpp"
#include "misc/juce_JUCESplashScreen.cpp"

// these classes are C++11-only
#if JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS
 #include "layout/juce_FlexBox.cpp"
 #if JUCE_HAS_CONSTEXPR
  #include "layout/juce_GridItem.cpp"
  #include "layout/juce_Grid.cpp"
  #if JUCE_UNIT_TESTS
   #include "layout/juce_GridUnitTests.cpp"
  #endif
 #endif
#endif

#if JUCE_IOS || JUCE_WINDOWS
 #include "native/juce_MultiTouchMapper.h"
#endif

#if JUCE_MAC || JUCE_IOS

 #if JUCE_CLANG
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wundeclared-selector"
 #endif

 #if JUCE_IOS
  #include "native/juce_ios_UIViewComponentPeer.mm"
  #include "native/juce_ios_Windowing.mm"
 #else
  #include "native/juce_mac_NSViewComponentPeer.mm"
  #include "native/juce_mac_Windowing.mm"
  #include "native/juce_mac_MainMenu.mm"
 #endif

 #if JUCE_CLANG
  #pragma clang diagnostic pop
 #endif

 #include "native/juce_mac_MouseCursor.mm"
 #include "native/juce_mac_FileChooser.mm"

#elif JUCE_WINDOWS
 #include "native/juce_win32_Windowing.cpp"
 #include "native/juce_win32_DragAndDrop.cpp"
 #include "native/juce_win32_FileChooser.cpp"

#elif JUCE_LINUX
 #include "native/juce_linux_X11.cpp"
 #include "native/juce_linux_X11_Clipboard.cpp"
 #include "native/juce_linux_X11_Windowing.cpp"
 #include "native/juce_linux_FileChooser.cpp"

#elif JUCE_ANDROID
 #include "native/juce_android_Windowing.cpp"
 #include "native/juce_android_FileChooser.cpp"

#endif
