/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_GUI_BASICS_H_INCLUDED
#define JUCE_GUI_BASICS_H_INCLUDED

#include "../juce_graphics/juce_graphics.h"
#include "../juce_data_structures/juce_data_structures.h"

//=============================================================================
/** Config: JUCE_ENABLE_REPAINT_DEBUGGING
    If this option is turned on, each area of the screen that gets repainted will
    flash in a random colour, so that you can see exactly which bits of your
    components are being drawn.
*/
#ifndef JUCE_ENABLE_REPAINT_DEBUGGING
 #define JUCE_ENABLE_REPAINT_DEBUGGING 0
#endif

/** JUCE_USE_XINERAMA: Enables Xinerama multi-monitor support (Linux only).
    Unless you specifically want to disable this, it's best to leave this option turned on.
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

//=============================================================================
namespace juce
{

class Component;
class LookAndFeel;
class MouseInputSource;
class MouseInputSourceInternal;
class ComponentPeer;
class MarkerList;
class RelativeRectangle;
class MouseEvent;
struct MouseWheelDetails;
class ToggleButton;
class TextButton;
class AlertWindow;
class TextLayout;
class ScrollBar;
class ComboBox;
class Button;
class FilenameComponent;
class DocumentWindow;
class ResizableWindow;
class GroupComponent;
class MenuBarComponent;
class DropShadower;
class GlyphArrangement;
class PropertyComponent;
class TableHeaderComponent;
class Toolbar;
class ToolbarItemComponent;
class PopupMenu;
class ProgressBar;
class FileBrowserComponent;
class DirectoryContentsDisplayComponent;
class FilePreviewComponent;
class ImageButton;
class CallOutBox;
class Drawable;
class DrawablePath;
class DrawableComposite;
class CaretComponent;
class BubbleComponent;
class KeyPressMappingSet;
class ApplicationCommandManagerListener;
class DrawableButton;

#include "mouse/juce_MouseCursor.h"
#include "mouse/juce_MouseListener.h"
#include "keyboard/juce_ModifierKeys.h"
#include "mouse/juce_MouseInputSource.h"
#include "mouse/juce_MouseEvent.h"
#include "keyboard/juce_KeyPress.h"
#include "keyboard/juce_KeyListener.h"
#include "keyboard/juce_KeyboardFocusTraverser.h"
#include "components/juce_ModalComponentManager.h"
#include "components/juce_ComponentListener.h"
#include "components/juce_CachedComponentImage.h"
#include "components/juce_Component.h"
#include "layout/juce_ComponentAnimator.h"
#include "components/juce_Desktop.h"
#include "layout/juce_ComponentBoundsConstrainer.h"
#include "mouse/juce_ComponentDragger.h"
#include "mouse/juce_DragAndDropTarget.h"
#include "mouse/juce_DragAndDropContainer.h"
#include "mouse/juce_FileDragAndDropTarget.h"
#include "mouse/juce_SelectedItemSet.h"
#include "mouse/juce_LassoComponent.h"
#include "mouse/juce_MouseInactivityDetector.h"
#include "mouse/juce_TextDragAndDropTarget.h"
#include "mouse/juce_TooltipClient.h"
#include "keyboard/juce_CaretComponent.h"
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
#include "drawables/juce_Drawable.h"
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
#include "buttons/juce_ToolbarButton.h"
#include "misc/juce_DropShadower.h"
#include "widgets/juce_TreeView.h"
#include "windows/juce_TopLevelWindow.h"
#include "windows/juce_AlertWindow.h"
#include "windows/juce_CallOutBox.h"
#include "windows/juce_ComponentPeer.h"
#include "windows/juce_ResizableWindow.h"
#include "windows/juce_DocumentWindow.h"
#include "windows/juce_DialogWindow.h"
#include "windows/juce_NativeMessageBox.h"
#include "windows/juce_ThreadWithProgressWindow.h"
#include "windows/juce_TooltipWindow.h"
#include "layout/juce_MultiDocumentPanel.h"
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
#include "properties/juce_PropertyComponent.h"
#include "properties/juce_BooleanPropertyComponent.h"
#include "properties/juce_ButtonPropertyComponent.h"
#include "properties/juce_ChoicePropertyComponent.h"
#include "properties/juce_PropertyPanel.h"
#include "properties/juce_SliderPropertyComponent.h"
#include "properties/juce_TextPropertyComponent.h"
#include "application/juce_Application.h"
#include "misc/juce_BubbleComponent.h"
#include "lookandfeel/juce_LookAndFeel.h"
#include "lookandfeel/juce_LookAndFeel_V2.h"
#include "lookandfeel/juce_LookAndFeel_V1.h"
#include "lookandfeel/juce_LookAndFeel_V3.h"

}

#endif   // JUCE_GUI_BASICS_H_INCLUDED
