/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
 #import <WebKit/WebKit.h>
 #import <IOKit/IOKitLib.h>
 #import <IOKit/IOCFPlugIn.h>
 #import <IOKit/hid/IOHIDLib.h>
 #import <IOKit/hid/IOHIDKeys.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>

 #if JUCE_PUSH_NOTIFICATIONS
  #import <Foundation/NSUserNotification.h>

  #include "native/juce_mac_PushNotifications.cpp"
 #endif

//==============================================================================
#elif JUCE_IOS
 #import <WebKit/WebKit.h>

 #if JUCE_PUSH_NOTIFICATIONS
  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
   #import <UserNotifications/UserNotifications.h>
  #endif

  #include "native/juce_ios_PushNotifications.cpp"
 #endif

//==============================================================================
#elif JUCE_ANDROID
 #if JUCE_PUSH_NOTIFICATIONS
  #include "native/juce_android_PushNotifications.cpp"
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
#elif JUCE_LINUX && JUCE_WEB_BROWSER
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", "-Wparentheses")

 // If you're missing this header, you need to install the webkit2gtk-4.0 package
 #include <gtk/gtk.h>

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE

 // If you're missing these headers, you need to install the webkit2gtk-4.0 package
 #include <gtk/gtkx.h>
 #include <glib-unix.h>
 #include <webkit2/webkit2.h>
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
#include "misc/juce_SystemTrayIconComponent.cpp"
#include "misc/juce_LiveConstantEditor.cpp"
#include "misc/juce_AnimatedAppComponent.cpp"

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
 #include "native/juce_win32_HWNDComponent.cpp"
 #if JUCE_WEB_BROWSER
  #include "native/juce_win32_WebBrowserComponent.cpp"
 #endif
 #include "native/juce_win32_SystemTrayIcon.cpp"

//==============================================================================
#elif JUCE_LINUX
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

 #include "native/juce_linux_XEmbedComponent.cpp"

 #if JUCE_WEB_BROWSER
  #include "native/juce_linux_X11_WebBrowserComponent.cpp"
 #endif

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE

 #include "native/juce_linux_X11_SystemTrayIcon.cpp"

//==============================================================================
#elif JUCE_ANDROID
 #include "native/juce_AndroidViewComponent.cpp"

 #if JUCE_WEB_BROWSER
  #include "native/juce_android_WebBrowserComponent.cpp"
 #endif
#endif

//==============================================================================
#if ! JUCE_WINDOWS
 juce::ScopedDPIAwarenessDisabler::ScopedDPIAwarenessDisabler()  { ignoreUnused (previousContext); }
 juce::ScopedDPIAwarenessDisabler::~ScopedDPIAwarenessDisabler() {}
#endif
