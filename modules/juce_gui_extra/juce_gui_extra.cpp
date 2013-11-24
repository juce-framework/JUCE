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

#if defined (JUCE_GUI_EXTRA_H_INCLUDED) && ! JUCE_AMALGAMATED_INCLUDE
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "../juce_core/native/juce_BasicNativeHeaders.h"
#include "juce_gui_extra.h"

//==============================================================================
#if JUCE_MAC
 #define Point CarbonDummyPointName
 #define Component CarbonDummyCompName
 #import <WebKit/WebKit.h>
 #import <IOKit/IOKitLib.h>
 #import <IOKit/IOCFPlugIn.h>
 #import <IOKit/hid/IOHIDLib.h>
 #import <IOKit/hid/IOHIDKeys.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>
 #import <Carbon/Carbon.h> // still needed for SetSystemUIMode()
 #undef Point
 #undef Component

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

#if JUCE_MAC || JUCE_IOS
 #include "../juce_core/native/juce_osx_ObjCHelpers.h"
#endif

#include "documents/juce_FileBasedDocument.cpp"
#include "code_editor/juce_CodeDocument.cpp"
#include "code_editor/juce_CodeEditorComponent.cpp"
#include "code_editor/juce_CPlusPlusCodeTokeniser.cpp"
#include "misc/juce_BubbleMessageComponent.cpp"
#include "misc/juce_ColourSelector.cpp"
#include "misc/juce_KeyMappingEditorComponent.cpp"
#include "misc/juce_PreferencesPanel.cpp"
#include "misc/juce_RecentlyOpenedFilesList.cpp"
#include "misc/juce_SplashScreen.cpp"
#include "misc/juce_SystemTrayIconComponent.cpp"

}

using namespace juce;

namespace juce
{

//==============================================================================
#if JUCE_MAC || JUCE_IOS
 #include "../juce_core/native/juce_osx_ObjCHelpers.h"
 #include "../juce_graphics/native/juce_mac_CoreGraphicsHelpers.h"

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
 #include "../juce_core/native/juce_win32_ComSmartPtr.h"
 #include "../juce_events/native/juce_win32_HiddenMessageWindow.h"
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

}
