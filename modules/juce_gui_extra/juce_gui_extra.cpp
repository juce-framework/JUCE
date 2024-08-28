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
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1
#define JUCE_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1

#ifndef JUCE_PUSH_NOTIFICATIONS
 #define JUCE_PUSH_NOTIFICATIONS 0
#endif

#include "juce_gui_extra.h"

//==============================================================================
#if JUCE_MAC
 #if JUCE_WEB_BROWSER
  #import <WebKit/WebKit.h>
 #endif
 #import <IOKit/IOKitLib.h>
 #import <IOKit/IOCFPlugIn.h>
 #import <IOKit/hid/IOHIDLib.h>
 #import <IOKit/hid/IOHIDKeys.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>

 #if JUCE_PUSH_NOTIFICATIONS
  #import <Foundation/NSUserNotification.h>

  #include "native/juce_PushNotifications_mac.cpp"
 #endif

//==============================================================================
#elif JUCE_IOS
 #if JUCE_WEB_BROWSER
  #import <WebKit/WebKit.h>
 #endif

 #if JUCE_PUSH_NOTIFICATIONS
  #import <UserNotifications/UserNotifications.h>
  #include "native/juce_PushNotifications_ios.cpp"
 #endif

//==============================================================================
#elif JUCE_ANDROID
 #if JUCE_PUSH_NOTIFICATIONS
  #include "native/juce_PushNotifications_android.cpp"
 #endif

//==============================================================================
#elif JUCE_WINDOWS
 #include <windowsx.h>
 #include <vfw.h>
 #include <commdlg.h>

 #if JUCE_WEB_BROWSER
  #include <exdisp.h>
  #include <exdispid.h>

  #if JUCE_USE_WIN_WEBVIEW2
   #include <windows.foundation.h>
   #include <windows.foundation.collections.h>
   #include <winuser.h>

   JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4265)
   #include <wrl.h>
   #include <wrl/wrappers/corewrappers.h>
   JUCE_END_IGNORE_WARNINGS_MSVC

   #include "WebView2.h"

   JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458)
   #include "WebView2EnvironmentOptions.h"
   JUCE_END_IGNORE_WARNINGS_MSVC
  #endif

 #endif

//==============================================================================
#elif (JUCE_LINUX || JUCE_BSD) && JUCE_WEB_BROWSER
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", "-Wparentheses", "-Wdeprecated-declarations")

 // If you're missing this header, you need to install the webkit2gtk-4.1 or webkit2gtk-4.0 package
 #include <gtk/gtk.h>
 #include <gtk/gtkx.h>
 #include <glib-unix.h>
 #include <webkit2/webkit2.h>
 #include <jsc/jsc.h>
 #include <libsoup/soup.h>

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

//==============================================================================
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
#include "misc/juce_PushNotifications.cpp"
#include "misc/juce_RecentlyOpenedFilesList.cpp"
#include "misc/juce_SplashScreen.cpp"

#if JUCE_MAC
 #include "native/juce_SystemTrayIcon_mac.cpp"
#elif JUCE_WINDOWS
 #include "native/juce_ActiveXComponent_windows.cpp"
 #include "native/juce_SystemTrayIcon_windows.cpp"
#elif JUCE_LINUX || JUCE_BSD
 #include "native/juce_SystemTrayIcon_linux.cpp"
#endif

#include "misc/juce_SystemTrayIconComponent.cpp"
#include "misc/juce_LiveConstantEditor.cpp"
#include "misc/juce_AnimatedAppComponent.cpp"
#include "misc/juce_WebBrowserComponent.cpp"
#include "misc/juce_WebControlRelays.cpp"

//==============================================================================
#if JUCE_MAC || JUCE_IOS

 #if JUCE_MAC
  #include "native/juce_NSViewFrameWatcher_mac.h"
  #include "native/juce_NSViewComponent_mac.mm"
  #include "native/juce_AppleRemote_mac.mm"
 #endif

 #if JUCE_IOS
  #include "native/juce_UIViewComponent_ios.mm"
 #endif

 #if JUCE_WEB_BROWSER
  #include "native/juce_WebBrowserComponent_mac.mm"
 #endif

//==============================================================================
#elif JUCE_WINDOWS
 #include "native/juce_HWNDComponent_windows.cpp"
 #if JUCE_WEB_BROWSER
  #include "native/juce_WebBrowserComponent_windows.cpp"
 #endif

//==============================================================================
#elif JUCE_LINUX || JUCE_BSD
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

 #include <juce_gui_basics/native/juce_ScopedWindowAssociation_linux.h>
 #include "native/juce_XEmbedComponent_linux.cpp"

 #if JUCE_WEB_BROWSER
  #if JUCE_USE_EXTERNAL_TEMPORARY_SUBPROCESS
   #include "juce_LinuxSubprocessHelperBinaryData.h"
  #endif

  #include "native/juce_WebBrowserComponent_linux.cpp"
 #endif

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
#elif JUCE_ANDROID
 #include "native/juce_AndroidViewComponent.cpp"

 #if JUCE_WEB_BROWSER
  #include "native/juce_WebBrowserComponent_android.cpp"
 #endif
#endif
