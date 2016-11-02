/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_gui_extra
  vendor:           juce
  version:          4.3.0
  name:             JUCE extended GUI classes
  description:      Miscellaneous GUI classes for specialised tasks.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_gui_basics
  OSXFrameworks:    WebKit

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_GUI_EXTRA_H_INCLUDED
#define JUCE_GUI_EXTRA_H_INCLUDED

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Config: JUCE_WEB_BROWSER
    This lets you disable the WebBrowserComponent class (Mac and Windows).
    If you're not using any embedded web-pages, turning this off may reduce your code size.
*/
#ifndef JUCE_WEB_BROWSER
 #define JUCE_WEB_BROWSER 1
#endif

/** Config: JUCE_ENABLE_LIVE_CONSTANT_EDITOR
    This lets you turn on the JUCE_ENABLE_LIVE_CONSTANT_EDITOR support. See the documentation
    for that macro for more details.
*/
#ifndef JUCE_ENABLE_LIVE_CONSTANT_EDITOR
 #if JUCE_DEBUG
  #define JUCE_ENABLE_LIVE_CONSTANT_EDITOR 1
 #endif
#endif

//==============================================================================
namespace juce
{

#include "documents/juce_FileBasedDocument.h"
#include "code_editor/juce_CodeDocument.h"
#include "code_editor/juce_CodeEditorComponent.h"
#include "code_editor/juce_CodeTokeniser.h"
#include "code_editor/juce_CPlusPlusCodeTokeniser.h"
#include "code_editor/juce_CPlusPlusCodeTokeniserFunctions.h"
#include "code_editor/juce_XMLCodeTokeniser.h"
#include "code_editor/juce_LuaCodeTokeniser.h"
#include "embedding/juce_ActiveXControlComponent.h"
#include "embedding/juce_NSViewComponent.h"
#include "embedding/juce_UIViewComponent.h"
#include "misc/juce_AppleRemote.h"
#include "misc/juce_BubbleMessageComponent.h"
#include "misc/juce_ColourSelector.h"
#include "misc/juce_KeyMappingEditorComponent.h"
#include "misc/juce_PreferencesPanel.h"
#include "misc/juce_RecentlyOpenedFilesList.h"
#include "misc/juce_SplashScreen.h"
#include "misc/juce_SystemTrayIconComponent.h"
#include "misc/juce_WebBrowserComponent.h"
#include "misc/juce_LiveConstantEditor.h"
#include "misc/juce_AnimatedAppComponent.h"

}

#endif   // JUCE_GUI_EXTRA_H_INCLUDED
