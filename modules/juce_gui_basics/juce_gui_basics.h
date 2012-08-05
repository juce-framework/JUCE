/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_GUI_BASICS_JUCEHEADER__
#define __JUCE_GUI_BASICS_JUCEHEADER__

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

// START_AUTOINCLUDE components, mouse, keyboard, buttons, drawables,
// filebrowser, layout, lookandfeel, menus, positioning, properties,
// widgets, windows, commands, application, misc
#ifndef __JUCE_CACHEDCOMPONENTIMAGE_JUCEHEADER__
 #include "components/juce_CachedComponentImage.h"
#endif
#ifndef __JUCE_COMPONENT_JUCEHEADER__
 #include "components/juce_Component.h"
#endif
#ifndef __JUCE_COMPONENTLISTENER_JUCEHEADER__
 #include "components/juce_ComponentListener.h"
#endif
#ifndef __JUCE_DESKTOP_JUCEHEADER__
 #include "components/juce_Desktop.h"
#endif
#ifndef __JUCE_MODALCOMPONENTMANAGER_JUCEHEADER__
 #include "components/juce_ModalComponentManager.h"
#endif
#ifndef __JUCE_COMPONENTDRAGGER_JUCEHEADER__
 #include "mouse/juce_ComponentDragger.h"
#endif
#ifndef __JUCE_DRAGANDDROPCONTAINER_JUCEHEADER__
 #include "mouse/juce_DragAndDropContainer.h"
#endif
#ifndef __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
 #include "mouse/juce_DragAndDropTarget.h"
#endif
#ifndef __JUCE_FILEDRAGANDDROPTARGET_JUCEHEADER__
 #include "mouse/juce_FileDragAndDropTarget.h"
#endif
#ifndef __JUCE_LASSOCOMPONENT_JUCEHEADER__
 #include "mouse/juce_LassoComponent.h"
#endif
#ifndef __JUCE_MOUSECURSOR_JUCEHEADER__
 #include "mouse/juce_MouseCursor.h"
#endif
#ifndef __JUCE_MOUSEEVENT_JUCEHEADER__
 #include "mouse/juce_MouseEvent.h"
#endif
#ifndef __JUCE_MOUSEINPUTSOURCE_JUCEHEADER__
 #include "mouse/juce_MouseInputSource.h"
#endif
#ifndef __JUCE_MOUSELISTENER_JUCEHEADER__
 #include "mouse/juce_MouseListener.h"
#endif
#ifndef __JUCE_SELECTEDITEMSET_JUCEHEADER__
 #include "mouse/juce_SelectedItemSet.h"
#endif
#ifndef __JUCE_TEXTDRAGANDDROPTARGET_JUCEHEADER__
 #include "mouse/juce_TextDragAndDropTarget.h"
#endif
#ifndef __JUCE_TOOLTIPCLIENT_JUCEHEADER__
 #include "mouse/juce_TooltipClient.h"
#endif
#ifndef __JUCE_CARETCOMPONENT_JUCEHEADER__
 #include "keyboard/juce_CaretComponent.h"
#endif
#ifndef __JUCE_KEYBOARDFOCUSTRAVERSER_JUCEHEADER__
 #include "keyboard/juce_KeyboardFocusTraverser.h"
#endif
#ifndef __JUCE_KEYLISTENER_JUCEHEADER__
 #include "keyboard/juce_KeyListener.h"
#endif
#ifndef __JUCE_KEYPRESS_JUCEHEADER__
 #include "keyboard/juce_KeyPress.h"
#endif
#ifndef __JUCE_MODIFIERKEYS_JUCEHEADER__
 #include "keyboard/juce_ModifierKeys.h"
#endif
#ifndef __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__
 #include "keyboard/juce_SystemClipboard.h"
#endif
#ifndef __JUCE_TEXTEDITORKEYMAPPER_JUCEHEADER__
 #include "keyboard/juce_TextEditorKeyMapper.h"
#endif
#ifndef __JUCE_TEXTINPUTTARGET_JUCEHEADER__
 #include "keyboard/juce_TextInputTarget.h"
#endif
#ifndef __JUCE_ARROWBUTTON_JUCEHEADER__
 #include "buttons/juce_ArrowButton.h"
#endif
#ifndef __JUCE_BUTTON_JUCEHEADER__
 #include "buttons/juce_Button.h"
#endif
#ifndef __JUCE_DRAWABLEBUTTON_JUCEHEADER__
 #include "buttons/juce_DrawableButton.h"
#endif
#ifndef __JUCE_HYPERLINKBUTTON_JUCEHEADER__
 #include "buttons/juce_HyperlinkButton.h"
#endif
#ifndef __JUCE_IMAGEBUTTON_JUCEHEADER__
 #include "buttons/juce_ImageButton.h"
#endif
#ifndef __JUCE_SHAPEBUTTON_JUCEHEADER__
 #include "buttons/juce_ShapeButton.h"
#endif
#ifndef __JUCE_TEXTBUTTON_JUCEHEADER__
 #include "buttons/juce_TextButton.h"
#endif
#ifndef __JUCE_TOGGLEBUTTON_JUCEHEADER__
 #include "buttons/juce_ToggleButton.h"
#endif
#ifndef __JUCE_TOOLBARBUTTON_JUCEHEADER__
 #include "buttons/juce_ToolbarButton.h"
#endif
#ifndef __JUCE_DRAWABLE_JUCEHEADER__
 #include "drawables/juce_Drawable.h"
#endif
#ifndef __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
 #include "drawables/juce_DrawableComposite.h"
#endif
#ifndef __JUCE_DRAWABLEIMAGE_JUCEHEADER__
 #include "drawables/juce_DrawableImage.h"
#endif
#ifndef __JUCE_DRAWABLEPATH_JUCEHEADER__
 #include "drawables/juce_DrawablePath.h"
#endif
#ifndef __JUCE_DRAWABLERECTANGLE_JUCEHEADER__
 #include "drawables/juce_DrawableRectangle.h"
#endif
#ifndef __JUCE_DRAWABLESHAPE_JUCEHEADER__
 #include "drawables/juce_DrawableShape.h"
#endif
#ifndef __JUCE_DRAWABLETEXT_JUCEHEADER__
 #include "drawables/juce_DrawableText.h"
#endif
#ifndef __JUCE_DIRECTORYCONTENTSDISPLAYCOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_DirectoryContentsDisplayComponent.h"
#endif
#ifndef __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__
 #include "filebrowser/juce_DirectoryContentsList.h"
#endif
#ifndef __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_FileBrowserComponent.h"
#endif
#ifndef __JUCE_FILEBROWSERLISTENER_JUCEHEADER__
 #include "filebrowser/juce_FileBrowserListener.h"
#endif
#ifndef __JUCE_FILECHOOSER_JUCEHEADER__
 #include "filebrowser/juce_FileChooser.h"
#endif
#ifndef __JUCE_FILECHOOSERDIALOGBOX_JUCEHEADER__
 #include "filebrowser/juce_FileChooserDialogBox.h"
#endif
#ifndef __JUCE_FILEFILTER_JUCEHEADER__
 #include "filebrowser/juce_FileFilter.h"
#endif
#ifndef __JUCE_FILELISTCOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_FileListComponent.h"
#endif
#ifndef __JUCE_FILENAMECOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_FilenameComponent.h"
#endif
#ifndef __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_FilePreviewComponent.h"
#endif
#ifndef __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_FileSearchPathListComponent.h"
#endif
#ifndef __JUCE_FILETREECOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_FileTreeComponent.h"
#endif
#ifndef __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
 #include "filebrowser/juce_ImagePreviewComponent.h"
#endif
#ifndef __JUCE_WILDCARDFILEFILTER_JUCEHEADER__
 #include "filebrowser/juce_WildcardFileFilter.h"
#endif
#ifndef __JUCE_COMPONENTANIMATOR_JUCEHEADER__
 #include "layout/juce_ComponentAnimator.h"
#endif
#ifndef __JUCE_COMPONENTBOUNDSCONSTRAINER_JUCEHEADER__
 #include "layout/juce_ComponentBoundsConstrainer.h"
#endif
#ifndef __JUCE_COMPONENTBUILDER_JUCEHEADER__
 #include "layout/juce_ComponentBuilder.h"
#endif
#ifndef __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__
 #include "layout/juce_ComponentMovementWatcher.h"
#endif
#ifndef __JUCE_CONCERTINAPANEL_JUCEHEADER__
 #include "layout/juce_ConcertinaPanel.h"
#endif
#ifndef __JUCE_GROUPCOMPONENT_JUCEHEADER__
 #include "layout/juce_GroupComponent.h"
#endif
#ifndef __JUCE_MULTIDOCUMENTPANEL_JUCEHEADER__
 #include "layout/juce_MultiDocumentPanel.h"
#endif
#ifndef __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
 #include "layout/juce_ResizableBorderComponent.h"
#endif
#ifndef __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__
 #include "layout/juce_ResizableCornerComponent.h"
#endif
#ifndef __JUCE_RESIZABLEEDGECOMPONENT_JUCEHEADER__
 #include "layout/juce_ResizableEdgeComponent.h"
#endif
#ifndef __JUCE_SCROLLBAR_JUCEHEADER__
 #include "layout/juce_ScrollBar.h"
#endif
#ifndef __JUCE_STRETCHABLELAYOUTMANAGER_JUCEHEADER__
 #include "layout/juce_StretchableLayoutManager.h"
#endif
#ifndef __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__
 #include "layout/juce_StretchableLayoutResizerBar.h"
#endif
#ifndef __JUCE_STRETCHABLEOBJECTRESIZER_JUCEHEADER__
 #include "layout/juce_StretchableObjectResizer.h"
#endif
#ifndef __JUCE_TABBEDBUTTONBAR_JUCEHEADER__
 #include "layout/juce_TabbedButtonBar.h"
#endif
#ifndef __JUCE_TABBEDCOMPONENT_JUCEHEADER__
 #include "layout/juce_TabbedComponent.h"
#endif
#ifndef __JUCE_VIEWPORT_JUCEHEADER__
 #include "layout/juce_Viewport.h"
#endif
#ifndef __JUCE_LOOKANDFEEL_JUCEHEADER__
 #include "lookandfeel/juce_LookAndFeel.h"
#endif
#ifndef __JUCE_MENUBARCOMPONENT_JUCEHEADER__
 #include "menus/juce_MenuBarComponent.h"
#endif
#ifndef __JUCE_MENUBARMODEL_JUCEHEADER__
 #include "menus/juce_MenuBarModel.h"
#endif
#ifndef __JUCE_POPUPMENU_JUCEHEADER__
 #include "menus/juce_PopupMenu.h"
#endif
#ifndef __JUCE_MARKERLIST_JUCEHEADER__
 #include "positioning/juce_MarkerList.h"
#endif
#ifndef __JUCE_RELATIVECOORDINATE_JUCEHEADER__
 #include "positioning/juce_RelativeCoordinate.h"
#endif
#ifndef __JUCE_RELATIVECOORDINATEPOSITIONER_JUCEHEADER__
 #include "positioning/juce_RelativeCoordinatePositioner.h"
#endif
#ifndef __JUCE_RELATIVEPARALLELOGRAM_JUCEHEADER__
 #include "positioning/juce_RelativeParallelogram.h"
#endif
#ifndef __JUCE_RELATIVEPOINT_JUCEHEADER__
 #include "positioning/juce_RelativePoint.h"
#endif
#ifndef __JUCE_RELATIVEPOINTPATH_JUCEHEADER__
 #include "positioning/juce_RelativePointPath.h"
#endif
#ifndef __JUCE_RELATIVERECTANGLE_JUCEHEADER__
 #include "positioning/juce_RelativeRectangle.h"
#endif
#ifndef __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__
 #include "properties/juce_BooleanPropertyComponent.h"
#endif
#ifndef __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__
 #include "properties/juce_ButtonPropertyComponent.h"
#endif
#ifndef __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__
 #include "properties/juce_ChoicePropertyComponent.h"
#endif
#ifndef __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
 #include "properties/juce_PropertyComponent.h"
#endif
#ifndef __JUCE_PROPERTYPANEL_JUCEHEADER__
 #include "properties/juce_PropertyPanel.h"
#endif
#ifndef __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__
 #include "properties/juce_SliderPropertyComponent.h"
#endif
#ifndef __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
 #include "properties/juce_TextPropertyComponent.h"
#endif
#ifndef __JUCE_COMBOBOX_JUCEHEADER__
 #include "widgets/juce_ComboBox.h"
#endif
#ifndef __JUCE_IMAGECOMPONENT_JUCEHEADER__
 #include "widgets/juce_ImageComponent.h"
#endif
#ifndef __JUCE_LABEL_JUCEHEADER__
 #include "widgets/juce_Label.h"
#endif
#ifndef __JUCE_LISTBOX_JUCEHEADER__
 #include "widgets/juce_ListBox.h"
#endif
#ifndef __JUCE_PROGRESSBAR_JUCEHEADER__
 #include "widgets/juce_ProgressBar.h"
#endif
#ifndef __JUCE_SLIDER_JUCEHEADER__
 #include "widgets/juce_Slider.h"
#endif
#ifndef __JUCE_TABLEHEADERCOMPONENT_JUCEHEADER__
 #include "widgets/juce_TableHeaderComponent.h"
#endif
#ifndef __JUCE_TABLELISTBOX_JUCEHEADER__
 #include "widgets/juce_TableListBox.h"
#endif
#ifndef __JUCE_TEXTEDITOR_JUCEHEADER__
 #include "widgets/juce_TextEditor.h"
#endif
#ifndef __JUCE_TOOLBAR_JUCEHEADER__
 #include "widgets/juce_Toolbar.h"
#endif
#ifndef __JUCE_TOOLBARITEMCOMPONENT_JUCEHEADER__
 #include "widgets/juce_ToolbarItemComponent.h"
#endif
#ifndef __JUCE_TOOLBARITEMFACTORY_JUCEHEADER__
 #include "widgets/juce_ToolbarItemFactory.h"
#endif
#ifndef __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__
 #include "widgets/juce_ToolbarItemPalette.h"
#endif
#ifndef __JUCE_TREEVIEW_JUCEHEADER__
 #include "widgets/juce_TreeView.h"
#endif
#ifndef __JUCE_ALERTWINDOW_JUCEHEADER__
 #include "windows/juce_AlertWindow.h"
#endif
#ifndef __JUCE_CALLOUTBOX_JUCEHEADER__
 #include "windows/juce_CallOutBox.h"
#endif
#ifndef __JUCE_COMPONENTPEER_JUCEHEADER__
 #include "windows/juce_ComponentPeer.h"
#endif
#ifndef __JUCE_DIALOGWINDOW_JUCEHEADER__
 #include "windows/juce_DialogWindow.h"
#endif
#ifndef __JUCE_DOCUMENTWINDOW_JUCEHEADER__
 #include "windows/juce_DocumentWindow.h"
#endif
#ifndef __JUCE_NATIVEMESSAGEBOX_JUCEHEADER__
 #include "windows/juce_NativeMessageBox.h"
#endif
#ifndef __JUCE_RESIZABLEWINDOW_JUCEHEADER__
 #include "windows/juce_ResizableWindow.h"
#endif
#ifndef __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__
 #include "windows/juce_ThreadWithProgressWindow.h"
#endif
#ifndef __JUCE_TOOLTIPWINDOW_JUCEHEADER__
 #include "windows/juce_TooltipWindow.h"
#endif
#ifndef __JUCE_TOPLEVELWINDOW_JUCEHEADER__
 #include "windows/juce_TopLevelWindow.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDID_JUCEHEADER__
 #include "commands/juce_ApplicationCommandID.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDINFO_JUCEHEADER__
 #include "commands/juce_ApplicationCommandInfo.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDMANAGER_JUCEHEADER__
 #include "commands/juce_ApplicationCommandManager.h"
#endif
#ifndef __JUCE_APPLICATIONCOMMANDTARGET_JUCEHEADER__
 #include "commands/juce_ApplicationCommandTarget.h"
#endif
#ifndef __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__
 #include "commands/juce_KeyPressMappingSet.h"
#endif
#ifndef __JUCE_APPLICATION_JUCEHEADER__
 #include "application/juce_Application.h"
#endif
#ifndef __JUCE_INITIALISATION_JUCEHEADER__
 #include "application/juce_Initialisation.h"
#endif
#ifndef __JUCE_BUBBLECOMPONENT_JUCEHEADER__
 #include "misc/juce_BubbleComponent.h"
#endif
#ifndef __JUCE_DROPSHADOWER_JUCEHEADER__
 #include "misc/juce_DropShadower.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_GUI_BASICS_JUCEHEADER__
