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

#ifndef __JUCE_GUI_EXTRA_JUCEHEADER__
#define __JUCE_GUI_EXTRA_JUCEHEADER__

#include "../juce_gui_basics/juce_gui_basics.h"


//=============================================================================
/** Config: JUCE_WEB_BROWSER
    This lets you disable the WebBrowserComponent class (Mac and Windows).
    If you're not using any embedded web-pages, turning this off may reduce your code size.
*/
#ifndef JUCE_WEB_BROWSER
 #define JUCE_WEB_BROWSER 1
#endif

//=============================================================================
namespace juce
{

// START_AUTOINCLUDE documents, code_editor, embedding, lookandfeel, misc
#ifndef __JUCE_FILEBASEDDOCUMENT_JUCEHEADER__
 #include "documents/juce_FileBasedDocument.h"
#endif
#ifndef __JUCE_CODEDOCUMENT_JUCEHEADER__
 #include "code_editor/juce_CodeDocument.h"
#endif
#ifndef __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
 #include "code_editor/juce_CodeEditorComponent.h"
#endif
#ifndef __JUCE_CODETOKENISER_JUCEHEADER__
 #include "code_editor/juce_CodeTokeniser.h"
#endif
#ifndef __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__
 #include "code_editor/juce_CPlusPlusCodeTokeniser.h"
#endif
#ifndef __JUCE_CPLUSPLUSCODETOKENISERFUNCTIONS_JUCEHEADER__
 #include "code_editor/juce_CPlusPlusCodeTokeniserFunctions.h"
#endif
#ifndef __JUCE_ACTIVEXCONTROLCOMPONENT_JUCEHEADER__
 #include "embedding/juce_ActiveXControlComponent.h"
#endif
#ifndef __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
 #include "embedding/juce_NSViewComponent.h"
#endif
#ifndef __JUCE_UIVIEWCOMPONENT_JUCEHEADER__
 #include "embedding/juce_UIViewComponent.h"
#endif
#ifndef __JUCE_OLDSCHOOLLOOKANDFEEL_JUCEHEADER__
 #include "lookandfeel/juce_OldSchoolLookAndFeel.h"
#endif
#ifndef __JUCE_APPLEREMOTE_JUCEHEADER__
 #include "misc/juce_AppleRemote.h"
#endif
#ifndef __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__
 #include "misc/juce_BubbleMessageComponent.h"
#endif
#ifndef __JUCE_COLOURSELECTOR_JUCEHEADER__
 #include "misc/juce_ColourSelector.h"
#endif
#ifndef __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
 #include "misc/juce_KeyMappingEditorComponent.h"
#endif
#ifndef __JUCE_PREFERENCESPANEL_JUCEHEADER__
 #include "misc/juce_PreferencesPanel.h"
#endif
#ifndef __JUCE_RECENTLYOPENEDFILESLIST_JUCEHEADER__
 #include "misc/juce_RecentlyOpenedFilesList.h"
#endif
#ifndef __JUCE_SPLASHSCREEN_JUCEHEADER__
 #include "misc/juce_SplashScreen.h"
#endif
#ifndef __JUCE_SYSTEMTRAYICONCOMPONENT_JUCEHEADER__
 #include "misc/juce_SystemTrayIconComponent.h"
#endif
#ifndef __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
 #include "misc/juce_WebBrowserComponent.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_GUI_EXTRA_JUCEHEADER__
