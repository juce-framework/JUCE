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
 #import <WebKit/WebKit.h>
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
#define JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN \
    jassert ((MessageManager::getInstanceWithoutCreating() != nullptr \
               && MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager()) \
              || getPeer() == nullptr);

namespace juce
{
    bool juce_areThereAnyAlwaysOnTopWindows();

    bool isEmbeddedInForegroundProcess (Component* c);

   #if ! JUCE_WINDOWS
    bool isEmbeddedInForegroundProcess (Component*) { return false; }
   #endif

    /*  Returns true if this process is in the foreground, or if the viewComponent
        is embedded into a window owned by the foreground process.
    */
    static bool isForegroundOrEmbeddedProcess (Component* viewComponent)
    {
        return Process::isForegroundProcess() || isEmbeddedInForegroundProcess (viewComponent);
    }

    bool isWindowOnCurrentVirtualDesktop (void*);

    struct CustomMouseCursorInfo
    {
        ScaledImage image;
        Point<int> hotspot;
    };

    template <typename MemberFn>
    static const AccessibilityHandler* getEnclosingHandlerWithInterface (const AccessibilityHandler* handler, MemberFn fn)
    {
        if (handler == nullptr)
            return nullptr;

        if ((handler->*fn)() != nullptr)
            return handler;

        return getEnclosingHandlerWithInterface (handler->getParent(), fn);
    }
} // namespace juce

#include "mouse/juce_PointerState.h"

#include "accessibility/juce_AccessibilityHandler.cpp"
#include "components/juce_Component.cpp"
#include "components/juce_ComponentListener.cpp"
#include "components/juce_FocusTraverser.cpp"
#include "mouse/juce_MouseInputSource.cpp"
#include "desktop/juce_Displays.cpp"
#include "desktop/juce_Desktop.cpp"
#include "components/juce_ModalComponentManager.cpp"
#include "mouse/juce_ComponentDragger.cpp"
#include "mouse/juce_DragAndDropContainer.cpp"
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
#include "filebrowser/juce_ContentSharer.cpp"
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
#include "layout/juce_SidePanel.cpp"
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
#include "menus/juce_BurgerMenuComponent.cpp"
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
#include "properties/juce_MultiChoicePropertyComponent.cpp"
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
#include "windows/juce_VBlankAttachement.cpp"
#include "commands/juce_ApplicationCommandInfo.cpp"
#include "commands/juce_ApplicationCommandManager.cpp"
#include "commands/juce_ApplicationCommandTarget.cpp"
#include "commands/juce_KeyPressMappingSet.cpp"
#include "application/juce_Application.cpp"
#include "misc/juce_BubbleComponent.cpp"
#include "misc/juce_DropShadower.cpp"
#include "misc/juce_FocusOutline.cpp"
#include "misc/juce_JUCESplashScreen.cpp"

#include "layout/juce_FlexBox.cpp"
#include "layout/juce_GridItem.cpp"
#include "layout/juce_Grid.cpp"

#if JUCE_IOS || JUCE_WINDOWS
 #include "native/juce_MultiTouchMapper.h"
#endif

#if JUCE_ANDROID || JUCE_WINDOWS || JUCE_IOS || JUCE_UNIT_TESTS
 #include "native/accessibility/juce_AccessibilityTextHelpers.h"
#endif

#if JUCE_MAC || JUCE_IOS
 #include "native/accessibility/juce_mac_AccessibilitySharedCode.mm"

 #if JUCE_IOS
  #include "native/juce_ios_UIViewComponentPeer.mm"
  #include "native/accessibility/juce_ios_Accessibility.mm"
  #include "native/juce_ios_Windowing.mm"
  #include "native/juce_ios_FileChooser.mm"

  #if JUCE_CONTENT_SHARING
   #include "native/juce_ios_ContentSharer.cpp"
  #endif

 #else
  #include "native/accessibility/juce_mac_Accessibility.mm"
  #include "native/juce_mac_PerScreenDisplayLinks.h"
  #include "native/juce_mac_NSViewComponentPeer.mm"
  #include "native/juce_mac_Windowing.mm"
  #include "native/juce_mac_MainMenu.mm"
  #include "native/juce_mac_FileChooser.mm"
 #endif

 #include "native/juce_mac_MouseCursor.mm"

#elif JUCE_WINDOWS
 #include "native/accessibility/juce_win32_ComInterfaces.h"
 #include "native/accessibility/juce_win32_WindowsUIAWrapper.h"
 #include "native/accessibility/juce_win32_AccessibilityElement.h"
 #include "native/accessibility/juce_win32_UIAHelpers.h"
 #include "native/accessibility/juce_win32_UIAProviders.h"
 #include "native/accessibility/juce_win32_AccessibilityElement.cpp"
 #include "native/accessibility/juce_win32_Accessibility.cpp"
 #include "native/juce_win32_Windowing.cpp"
 #include "native/juce_win32_DragAndDrop.cpp"
 #include "native/juce_win32_FileChooser.cpp"

#elif JUCE_LINUX || JUCE_BSD
 #include "native/x11/juce_linux_X11_Symbols.cpp"
 #include "native/x11/juce_linux_X11_DragAndDrop.cpp"

 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

 #include "native/x11/juce_linux_ScopedWindowAssociation.h"
 #include "native/juce_linux_Windowing.cpp"
 #include "native/x11/juce_linux_XWindowSystem.cpp"

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE

 #include "native/juce_linux_FileChooser.cpp"

#elif JUCE_ANDROID

namespace juce
{
static jobject makeAndroidRect (Rectangle<int> r)
{
    return getEnv()->NewObject (AndroidRect,
                                AndroidRect.constructor,
                                r.getX(),
                                r.getY(),
                                r.getRight(),
                                r.getBottom());
}

static jobject makeAndroidPoint (Point<int> p)
{
    return getEnv()->NewObject (AndroidPoint,
                                AndroidPoint.create,
                                p.getX(),
                                p.getY());
}
} // namespace juce

 #include "juce_core/files/juce_common_MimeTypes.h"
 #include "native/accessibility/juce_android_Accessibility.cpp"
 #include "native/juce_android_Windowing.cpp"
 #include "native/juce_android_FileChooser.cpp"

 #if JUCE_CONTENT_SHARING
  #include "native/juce_android_ContentSharer.cpp"
 #endif

#endif

namespace juce
{
   #if ! JUCE_NATIVE_ACCESSIBILITY_INCLUDED
    class AccessibilityHandler::AccessibilityNativeImpl { public: AccessibilityNativeImpl (AccessibilityHandler&) {} };
    void AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent) const {}
    void AccessibilityHandler::postAnnouncement (const String&, AnnouncementPriority) {}
    AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const { return nullptr; }
    void notifyAccessibilityEventInternal (const AccessibilityHandler&, InternalAccessibilityEvent) {}
    std::unique_ptr<AccessibilityHandler::AccessibilityNativeImpl> AccessibilityHandler::createNativeImpl (AccessibilityHandler&)
    {
        return nullptr;
    }
   #else
    std::unique_ptr<AccessibilityHandler::AccessibilityNativeImpl> AccessibilityHandler::createNativeImpl (AccessibilityHandler& handler)
    {
        return std::make_unique<AccessibilityNativeImpl> (handler);
    }
   #endif
}

//==============================================================================
#if JUCE_WINDOWS
namespace juce
{

JUCE_COMCLASS (JuceIVirtualDesktopManager, "a5cd92ff-29be-454c-8d04-d82879fb3f1b") : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
         __RPC__in HWND topLevelWindow,
         __RPC__out BOOL * onCurrentDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
         __RPC__in HWND topLevelWindow,
         __RPC__out GUID * desktopId) = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
         __RPC__in HWND topLevelWindow,
         __RPC__in REFGUID desktopId) = 0;
};

JUCE_COMCLASS (JuceVirtualDesktopManager, "aa509086-5ca9-4c25-8f95-589d3c07b48a");

} // namespace juce

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (juce::JuceIVirtualDesktopManager, 0xa5cd92ff, 0x29be, 0x454c, 0x8d, 0x04, 0xd8, 0x28, 0x79, 0xfb, 0x3f, 0x1b)
__CRT_UUID_DECL (juce::JuceVirtualDesktopManager,  0xaa509086, 0x5ca9, 0x4c25, 0x8f, 0x95, 0x58, 0x9d, 0x3c, 0x07, 0xb4, 0x8a)
#endif

bool juce::isWindowOnCurrentVirtualDesktop (void* x)
{
    if (x == nullptr)
        return false;

    static auto* desktopManager = []
    {
        JuceIVirtualDesktopManager* result = nullptr;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        if (SUCCEEDED (CoCreateInstance (__uuidof (JuceVirtualDesktopManager), nullptr, CLSCTX_ALL, IID_PPV_ARGS (&result))))
            return result;

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return static_cast<JuceIVirtualDesktopManager*> (nullptr);
    }();

    BOOL current = false;

    if (auto* dm = desktopManager)
        if (SUCCEEDED (dm->IsWindowOnCurrentVirtualDesktop (static_cast<HWND> (x), &current)))
            return current != false;

    return true;
}

#else
 bool juce::isWindowOnCurrentVirtualDesktop (void*) { return true; }
 juce::ScopedDPIAwarenessDisabler::ScopedDPIAwarenessDisabler()  { ignoreUnused (previousContext); }
 juce::ScopedDPIAwarenessDisabler::~ScopedDPIAwarenessDisabler() {}
#endif

// Depends on types defined in platform-specific windowing files
#include "mouse/juce_MouseCursor.cpp"

#if JUCE_UNIT_TESTS
#include "native/accessibility/juce_AccessibilityTextHelpers_test.cpp"
#endif
