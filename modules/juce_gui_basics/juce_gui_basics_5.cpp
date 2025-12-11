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

#ifdef JUCE_GUI_BASICS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "juce_gui_basics.h"

#include "detail/juce_ComponentHelpers.h"
#include "detail/juce_WindowingHelpers.h"
#include "detail/juce_FocusRestorer.h"

#include "commands/juce_ApplicationCommandInfo.cpp"
#include "commands/juce_ApplicationCommandManager.cpp"
#include "commands/juce_ApplicationCommandTarget.cpp"
#include "commands/juce_KeyPressMappingSet.cpp"

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
#include "keyboard/juce_ModifierKeys.cpp"

#include "mouse/juce_ComponentDragger.cpp"
#include "mouse/juce_DragAndDropContainer.cpp"
#include "mouse/juce_MouseEvent.cpp"
#include "mouse/juce_MouseInactivityDetector.cpp"
#include "mouse/juce_MouseListener.cpp"

#include "windows/juce_CallOutBox.cpp"
#include "windows/juce_DialogWindow.cpp"
#include "windows/juce_DocumentWindow.cpp"
#include "windows/juce_MessageBoxOptions.cpp"
#include "windows/juce_ResizableWindow.cpp"
#include "windows/juce_ThreadWithProgressWindow.cpp"
#include "windows/juce_TooltipWindow.cpp"
#include "windows/juce_VBlankAttachment.cpp"
#include "windows/juce_NativeScaleFactorNotifier.cpp"

#include "menus/juce_BurgerMenuComponent.cpp"
#include "menus/juce_MenuBarComponent.cpp"
#include "menus/juce_MenuBarModel.cpp"

#include "misc/juce_BubbleComponent.cpp"
#include "misc/juce_FocusOutline.cpp"
