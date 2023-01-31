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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                     juce_gui_extra
  vendor:                 juce
  version:                7.0.5
  name:                   JUCE extended GUI classes
  description:            Miscellaneous GUI classes for specialised tasks.
  website:                http://www.juce.com/juce
  license:                GPL/Commercial
  minimumCppStandard:     17

  dependencies:           juce_gui_basics
  OSXFrameworks:          WebKit
  iOSFrameworks:          WebKit
  WeakiOSFrameworks:      UserNotifications
  WeakMacOSFrameworks:    UserNotifications

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_GUI_EXTRA_H_INCLUDED

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/** Config: JUCE_WEB_BROWSER
    This lets you disable the WebBrowserComponent class.
    If you're not using any embedded web-pages, turning this off may reduce your code size.
*/
#ifndef JUCE_WEB_BROWSER
 #define JUCE_WEB_BROWSER 1
#endif

/** Config: JUCE_USE_WIN_WEBVIEW2
    Enables the use of the Microsoft Edge (Chromium) WebView2 browser on Windows,
    currently in developer preview.

    If using the Projucer, the Microsoft.Web.WebView2 package will be added to the
    project solution if this flag is enabled. If you are building using CMake you
    will need to manually add the package via the Visual Studio package manager.

    In addition to enabling this macro, you will need to use the
    WindowsWebView2WebBrowserComponent wrapper - see the documentation of that
    class for more details.
*/
#ifndef JUCE_USE_WIN_WEBVIEW2
 #define JUCE_USE_WIN_WEBVIEW2 0
#endif

/** Config: JUCE_ENABLE_LIVE_CONSTANT_EDITOR
    This lets you turn on the JUCE_ENABLE_LIVE_CONSTANT_EDITOR support (desktop only). By default
    this will be enabled for debug builds and disabled for release builds. See the documentation
    for that macro for more details.
*/
#ifndef JUCE_ENABLE_LIVE_CONSTANT_EDITOR
 #if JUCE_DEBUG && ! (JUCE_IOS || JUCE_ANDROID)
  #define JUCE_ENABLE_LIVE_CONSTANT_EDITOR 1
 #endif
#endif

//==============================================================================
#include "documents/juce_FileBasedDocument.h"
#include "code_editor/juce_CodeDocument.h"
#include "code_editor/juce_CodeEditorComponent.h"
#include "code_editor/juce_CodeTokeniser.h"
#include "code_editor/juce_CPlusPlusCodeTokeniser.h"
#include "code_editor/juce_CPlusPlusCodeTokeniserFunctions.h"
#include "code_editor/juce_XMLCodeTokeniser.h"
#include "code_editor/juce_LuaCodeTokeniser.h"
#include "embedding/juce_ActiveXControlComponent.h"
#include "embedding/juce_AndroidViewComponent.h"
#include "embedding/juce_NSViewComponent.h"
#include "embedding/juce_UIViewComponent.h"
#include "embedding/juce_XEmbedComponent.h"
#include "embedding/juce_HWNDComponent.h"
#include "misc/juce_AppleRemote.h"
#include "misc/juce_BubbleMessageComponent.h"
#include "misc/juce_ColourSelector.h"
#include "misc/juce_KeyMappingEditorComponent.h"
#include "misc/juce_PreferencesPanel.h"
#include "misc/juce_PushNotifications.h"
#include "misc/juce_RecentlyOpenedFilesList.h"
#include "misc/juce_SplashScreen.h"
#include "misc/juce_SystemTrayIconComponent.h"
#include "misc/juce_WebBrowserComponent.h"
#include "misc/juce_LiveConstantEditor.h"
#include "misc/juce_AnimatedAppComponent.h"
