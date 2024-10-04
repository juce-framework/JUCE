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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_gui_basics
  vendor:             juce
  version:            8.0.2
  name:               JUCE GUI core classes
  description:        Basic user-interface components and related classes.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_graphics juce_data_structures
  OSXFrameworks:      Cocoa QuartzCore
  WeakOSXFrameworks:  Metal MetalKit
  iOSFrameworks:      CoreServices UIKit
  WeakiOSFrameworks:  Metal MetalKit

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_GUI_BASICS_H_INCLUDED

#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>

//==============================================================================
/** Config: JUCE_ENABLE_REPAINT_DEBUGGING
    If this option is turned on, each area of the screen that gets repainted will
    flash in a random colour, so that you can see exactly which bits of your
    components are being drawn.
*/
#ifndef JUCE_ENABLE_REPAINT_DEBUGGING
 #define JUCE_ENABLE_REPAINT_DEBUGGING 0
#endif

/** Config: JUCE_USE_XRANDR
    Enables Xrandr multi-monitor support (Linux only).
    Unless you specifically want to disable this, it's best to leave this option turned on.
    Note that your users do not need to have Xrandr installed for your JUCE app to run, as
    the availability of Xrandr is queried during runtime.
*/
#ifndef JUCE_USE_XRANDR
 #define JUCE_USE_XRANDR 1
#endif

/** Config: JUCE_USE_XINERAMA
    Enables Xinerama multi-monitor support (Linux only).
    Unless you specifically want to disable this, it's best to leave this option turned on.
    This will be used as a fallback if JUCE_USE_XRANDR not set or libxrandr cannot be found.
    Note that your users do not need to have Xinerama installed for your JUCE app to run, as
    the availability of Xinerama is queried during runtime.
*/
#ifndef JUCE_USE_XINERAMA
 #define JUCE_USE_XINERAMA 1
#endif

/** Config: JUCE_USE_XSHM
    Enables X shared memory for faster rendering on Linux. This is best left turned on
    unless you have a good reason to disable it.
*/
#ifndef JUCE_USE_XSHM
 #define JUCE_USE_XSHM 1
#endif

/** Config: JUCE_USE_XRENDER
    Enables XRender to allow semi-transparent windowing on Linux.
*/
#ifndef JUCE_USE_XRENDER
 #define JUCE_USE_XRENDER 0
#endif

/** Config: JUCE_USE_XCURSOR
    Uses XCursor to allow ARGB cursor on Linux. This is best left turned on unless you have
    a good reason to disable it.
*/
#ifndef JUCE_USE_XCURSOR
 #define JUCE_USE_XCURSOR 1
#endif

/** Config: JUCE_WIN_PER_MONITOR_DPI_AWARE
    Enables per-monitor DPI awareness on Windows 8.1 and above.
*/
#ifndef JUCE_WIN_PER_MONITOR_DPI_AWARE
 #define JUCE_WIN_PER_MONITOR_DPI_AWARE 1
#endif

//==============================================================================
namespace juce
{
    class Component;
    class LookAndFeel;
    class MouseInputSource;
    class ComponentPeer;
    class MouseEvent;
    struct MouseWheelDetails;
    struct PenDetails;
    class ToggleButton;
    class TextButton;
    class AlertWindow;
    class TextLayout;
    class ScrollBar;
    class ComboBox;
    class Button;
    class FilenameComponent;
    class ResizableWindow;
    class MenuBarComponent;
    class GlyphArrangement;
    class TableHeaderComponent;
    class Toolbar;
    class PopupMenu;
    class ProgressBar;
    class FileBrowserComponent;
    class DirectoryContentsDisplayComponent;
    class FilePreviewComponent;
    class CallOutBox;
    class Drawable;
    class DrawablePath;
    class DrawableComposite;
    class CaretComponent;
    class KeyPressMappingSet;
    class ApplicationCommandManagerListener;
    class DrawableButton;
    class Displays;
    class AccessibilityHandler;
    class KeyboardFocusTraverser;

    class FlexBox;
    class Grid;
    class FocusOutline;

   #if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
    Image createSnapshotOfNativeWindow (void* nativeWindowHandle);
   #endif

    namespace detail
    {
        struct ComponentHelpers;
        class MouseInputSourceImpl;
        class MouseInputSourceList;
        class PointerState;
        class ScopedMessageBoxImpl;
        class ToolbarItemDragAndDropOverlayComponent;
        class TopLevelWindowManager;
    } // namespace detail

} // namespace juce

#include "mouse/juce_MouseCursor.h"
#include "mouse/juce_MouseListener.h"
#include "keyboard/juce_ModifierKeys.h"
#include "mouse/juce_MouseInputSource.h"
#include "mouse/juce_MouseEvent.h"
#include "keyboard/juce_KeyPress.h"
#include "keyboard/juce_KeyListener.h"
#include "components/juce_ComponentTraverser.h"
#include "components/juce_FocusTraverser.h"
#include "components/juce_ModalComponentManager.h"
#include "components/juce_ComponentListener.h"
#include "components/juce_CachedComponentImage.h"
#include "components/juce_Component.h"
#include "layout/juce_ComponentAnimator.h"
#include "desktop/juce_Desktop.h"
#include "desktop/juce_Displays.h"
#include "layout/juce_ComponentBoundsConstrainer.h"
#include "layout/juce_BorderedComponentBoundsConstrainer.h"
#include "mouse/juce_ComponentDragger.h"
#include "mouse/juce_DragAndDropTarget.h"
#include "mouse/juce_DragAndDropContainer.h"
#include "mouse/juce_FileDragAndDropTarget.h"
#include "mouse/juce_SelectedItemSet.h"
#include "mouse/juce_MouseInactivityDetector.h"
#include "mouse/juce_TextDragAndDropTarget.h"
#include "mouse/juce_TooltipClient.h"
#include "keyboard/juce_CaretComponent.h"
#include "keyboard/juce_KeyboardFocusTraverser.h"
#include "keyboard/juce_SystemClipboard.h"
#include "keyboard/juce_TextEditorKeyMapper.h"
#include "keyboard/juce_TextInputTarget.h"
#include "commands/juce_ApplicationCommandID.h"
#include "commands/juce_ApplicationCommandInfo.h"
#include "commands/juce_ApplicationCommandTarget.h"
#include "commands/juce_ApplicationCommandManager.h"
#include "commands/juce_KeyPressMappingSet.h"
#include "buttons/juce_Button.h"
#include "buttons/juce_ArrowButton.h"
#include "buttons/juce_DrawableButton.h"
#include "buttons/juce_HyperlinkButton.h"
#include "buttons/juce_ImageButton.h"
#include "buttons/juce_ShapeButton.h"
#include "buttons/juce_TextButton.h"
#include "buttons/juce_ToggleButton.h"
#include "layout/juce_AnimatedPosition.h"
#include "layout/juce_AnimatedPositionBehaviours.h"
#include "layout/juce_ComponentBuilder.h"
#include "layout/juce_ComponentMovementWatcher.h"
#include "layout/juce_ConcertinaPanel.h"
#include "layout/juce_GroupComponent.h"
#include "layout/juce_ResizableBorderComponent.h"
#include "layout/juce_ResizableCornerComponent.h"
#include "layout/juce_ResizableEdgeComponent.h"
#include "layout/juce_ScrollBar.h"
#include "layout/juce_StretchableLayoutManager.h"
#include "layout/juce_StretchableLayoutResizerBar.h"
#include "layout/juce_StretchableObjectResizer.h"
#include "layout/juce_TabbedButtonBar.h"
#include "layout/juce_TabbedComponent.h"
#include "accessibility/interfaces/juce_AccessibilityCellInterface.h"
#include "accessibility/interfaces/juce_AccessibilityTableInterface.h"
#include "accessibility/interfaces/juce_AccessibilityTextInterface.h"
#include "accessibility/interfaces/juce_AccessibilityValueInterface.h"
#include "accessibility/enums/juce_AccessibilityActions.h"
#include "accessibility/enums/juce_AccessibilityEvent.h"
#include "accessibility/enums/juce_AccessibilityRole.h"
#include "accessibility/juce_AccessibilityState.h"
#include "accessibility/juce_AccessibilityHandler.h"
#include "drawables/juce_Drawable.h"
#include "layout/juce_Viewport.h"
#include "menus/juce_PopupMenu.h"
#include "menus/juce_MenuBarModel.h"
#include "menus/juce_MenuBarComponent.h"
#include "positioning/juce_RelativeCoordinate.h"
#include "positioning/juce_MarkerList.h"
#include "positioning/juce_RelativePoint.h"
#include "positioning/juce_RelativeRectangle.h"
#include "positioning/juce_RelativeCoordinatePositioner.h"
#include "positioning/juce_RelativeParallelogram.h"
#include "positioning/juce_RelativePointPath.h"
#include "drawables/juce_DrawableShape.h"
#include "drawables/juce_DrawableComposite.h"
#include "drawables/juce_DrawableImage.h"
#include "drawables/juce_DrawablePath.h"
#include "drawables/juce_DrawableRectangle.h"
#include "drawables/juce_DrawableText.h"
#include "widgets/juce_TextEditor.h"
#include "widgets/juce_Label.h"
#include "widgets/juce_ComboBox.h"
#include "widgets/juce_ImageComponent.h"
#include "widgets/juce_ListBox.h"
#include "widgets/juce_ProgressBar.h"
#include "widgets/juce_Slider.h"
#include "widgets/juce_TableHeaderComponent.h"
#include "widgets/juce_TableListBox.h"
#include "widgets/juce_Toolbar.h"
#include "widgets/juce_ToolbarItemComponent.h"
#include "widgets/juce_ToolbarItemFactory.h"
#include "widgets/juce_ToolbarItemPalette.h"
#include "menus/juce_BurgerMenuComponent.h"
#include "buttons/juce_ToolbarButton.h"
#include "misc/juce_DropShadower.h"
#include "misc/juce_FocusOutline.h"
#include "widgets/juce_TreeView.h"
#include "windows/juce_TopLevelWindow.h"
#include "windows/juce_MessageBoxOptions.h"
#include "windows/juce_ScopedMessageBox.h"
#include "windows/juce_AlertWindow.h"
#include "windows/juce_CallOutBox.h"
#include "windows/juce_ComponentPeer.h"
#include "windows/juce_ResizableWindow.h"
#include "windows/juce_DocumentWindow.h"
#include "windows/juce_DialogWindow.h"
#include "windows/juce_NativeMessageBox.h"
#include "windows/juce_ThreadWithProgressWindow.h"
#include "windows/juce_TooltipWindow.h"
#include "windows/juce_VBlankAttachment.h"
#include "windows/juce_WindowUtils.h"
#include "windows/juce_NativeScaleFactorNotifier.h"
#include "layout/juce_MultiDocumentPanel.h"
#include "layout/juce_SidePanel.h"
#include "filebrowser/juce_FileBrowserListener.h"
#include "filebrowser/juce_DirectoryContentsList.h"
#include "filebrowser/juce_DirectoryContentsDisplayComponent.h"
#include "filebrowser/juce_FileBrowserComponent.h"
#include "filebrowser/juce_FileChooser.h"
#include "filebrowser/juce_FileChooserDialogBox.h"
#include "filebrowser/juce_FileListComponent.h"
#include "filebrowser/juce_FilenameComponent.h"
#include "filebrowser/juce_FilePreviewComponent.h"
#include "filebrowser/juce_FileSearchPathListComponent.h"
#include "filebrowser/juce_FileTreeComponent.h"
#include "filebrowser/juce_ImagePreviewComponent.h"
#include "filebrowser/juce_ContentSharer.h"
#include "properties/juce_PropertyComponent.h"
#include "properties/juce_BooleanPropertyComponent.h"
#include "properties/juce_ButtonPropertyComponent.h"
#include "properties/juce_ChoicePropertyComponent.h"
#include "properties/juce_PropertyPanel.h"
#include "properties/juce_SliderPropertyComponent.h"
#include "properties/juce_TextPropertyComponent.h"
#include "properties/juce_MultiChoicePropertyComponent.h"
#include "application/juce_Application.h"
#include "misc/juce_BubbleComponent.h"
#include "lookandfeel/juce_LookAndFeel.h"
#include "lookandfeel/juce_LookAndFeel_V2.h"
#include "lookandfeel/juce_LookAndFeel_V1.h"
#include "lookandfeel/juce_LookAndFeel_V3.h"
#include "lookandfeel/juce_LookAndFeel_V4.h"
#include "mouse/juce_LassoComponent.h"

#if JUCE_LINUX || JUCE_BSD
 #if JUCE_GUI_BASICS_INCLUDE_XHEADERS
  // If you're missing these headers, you need to install the libx11-dev package
  JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wvariadic-macros")
  #include <X11/Xlib.h>
  JUCE_END_IGNORE_WARNINGS_GCC_LIKE
  #include <X11/Xatom.h>
  #include <X11/Xresource.h>
  #include <X11/Xutil.h>
  #include <X11/Xmd.h>
  #include <X11/keysym.h>
  #include <X11/XKBlib.h>
  #include <X11/cursorfont.h>
  #include <unistd.h>

  #if JUCE_USE_XRANDR
   // If you're missing this header, you need to install the libxrandr-dev package
   #include <X11/extensions/Xrandr.h>
  #endif

  #if JUCE_USE_XINERAMA
   // If you're missing this header, you need to install the libxinerama-dev package
   #include <X11/extensions/Xinerama.h>
  #endif

  #if JUCE_USE_XSHM
   #include <X11/extensions/XShm.h>
   #include <sys/shm.h>
   #include <sys/ipc.h>
  #endif

  #if JUCE_USE_XRENDER
   // If you're missing these headers, you need to install the libxrender-dev and libxcomposite-dev packages
   #include <X11/extensions/Xrender.h>
   #include <X11/extensions/Xcomposite.h>
  #endif

  #if JUCE_USE_XCURSOR
   // If you're missing this header, you need to install the libxcursor-dev package
   #include <X11/Xcursor/Xcursor.h>
  #endif

  #undef SIZEOF
  #undef KeyPress

  #include "native/juce_XWindowSystem_linux.h"
  #include "native/juce_XSymbols_linux.h"
 #endif
#endif

#if JUCE_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER && JUCE_WINDOWS
 #include "native/juce_ScopedThreadDPIAwarenessSetter_windows.h"
#endif

#include "layout/juce_FlexItem.h"
#include "layout/juce_FlexBox.h"

#include "layout/juce_GridItem.h"
#include "layout/juce_Grid.h"
#include "native/juce_ScopedDPIAwarenessDisabler.h"
