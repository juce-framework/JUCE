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

#ifdef JUCE_GUI_EXTRA_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1

#include "juce_gui_extra.h"

//==============================================================================
#if JUCE_MAC
 #import <WebKit/WebKit.h>
 #import <IOKit/IOKitLib.h>
 #import <IOKit/IOCFPlugIn.h>
 #import <IOKit/hid/IOHIDLib.h>
 #import <IOKit/hid/IOHIDKeys.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>

#elif JUCE_IOS

//==============================================================================
#elif JUCE_WINDOWS
 #include <windowsx.h>
 #include <vfw.h>
 #include <commdlg.h>

 #if JUCE_WEB_BROWSER
  #include <exdisp.h>
  #include <exdispid.h>
 #endif

//==============================================================================
#elif JUCE_LINUX
 #include <X11/Xlib.h>
 #include <X11/Xatom.h>
 #include <X11/Xutil.h>
 #undef SIZEOF
 #undef KeyPress
#endif

//==============================================================================
namespace juce
{

#include "documents/juce_FileBasedDocument.cpp"
#include "code_editor/juce_CodeDocument.cpp"
#include "code_editor/juce_CodeEditorComponent.cpp"
#include "code_editor/juce_CPlusPlusCodeTokeniser.cpp"
#include "code_editor/juce_XMLCodeTokeniser.cpp"
#include "code_editor/juce_LuaCodeTokeniser.cpp"
#include "misc/juce_BubbleMessageComponent.cpp"
#include "misc/juce_ColourSelector.cpp"
#include "misc/juce_KeyMappingEditorComponent.cpp"
#include "misc/juce_PreferencesPanel.cpp"
#include "misc/juce_RecentlyOpenedFilesList.cpp"
#include "misc/juce_SplashScreen.cpp"
#include "misc/juce_SystemTrayIconComponent.cpp"
#include "misc/juce_LiveConstantEditor.cpp"
#include "misc/juce_AnimatedAppComponent.cpp"

}

using namespace juce;

namespace juce
{

//==============================================================================
#if JUCE_MAC || JUCE_IOS
 #if JUCE_MAC
  #include "native/juce_mac_NSViewComponent.mm"
  #include "native/juce_mac_AppleRemote.mm"
  #include "native/juce_mac_SystemTrayIcon.cpp"
 #endif

 #if JUCE_IOS
  #include "native/juce_ios_UIViewComponent.mm"
 #endif

 #if JUCE_WEB_BROWSER
  #include "native/juce_mac_WebBrowserComponent.mm"
 #endif

//==============================================================================
#elif JUCE_WINDOWS
 #include "native/juce_win32_ActiveXComponent.cpp"
 #if JUCE_WEB_BROWSER
  #include "native/juce_win32_WebBrowserComponent.cpp"
 #endif
 #include "native/juce_win32_SystemTrayIcon.cpp"

//==============================================================================
#elif JUCE_LINUX
 #if JUCE_WEB_BROWSER
  #include "native/juce_linux_WebBrowserComponent.cpp"
 #endif
 #include "native/juce_linux_SystemTrayIcon.cpp"

//==============================================================================
#elif JUCE_ANDROID
 #if JUCE_WEB_BROWSER
  #include "native/juce_android_WebBrowserComponent.cpp"
 #endif
#endif

#if JUCE_WEB_BROWSER
 bool WebBrowserComponent::pageAboutToLoad (const String&)  { return true; }
 void WebBrowserComponent::pageFinishedLoading (const String&) {}
 void WebBrowserComponent::windowCloseRequest() {}
 void WebBrowserComponent::newWindowAttemptingToLoad (const String&) {}
#endif
}
