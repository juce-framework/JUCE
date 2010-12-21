/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_MAC_NATIVEINCLUDES_JUCEHEADER__
#define __JUCE_MAC_NATIVEINCLUDES_JUCEHEADER__

/*
    This file wraps together all the mac-specific code, so that
    we can include all the native headers just once, and compile all our
    platform-specific stuff in one big lump, keeping it out of the way of
    the rest of the codebase.
*/

#include "../../core/juce_StandardHeader.h"

#define USE_COREGRAPHICS_RENDERING 1

#if JUCE_IOS
 #import <Foundation/Foundation.h>
 #import <UIKit/UIKit.h>
 #import <AudioToolbox/AudioToolbox.h>
 #import <AVFoundation/AVFoundation.h>
 #import <CoreData/CoreData.h>
 #import <MobileCoreServices/MobileCoreServices.h>
 #import <QuartzCore/QuartzCore.h>
 #include <sys/fcntl.h>
 #if JUCE_OPENGL
  #include <OpenGLES/ES1/gl.h>
  #include <OpenGLES/ES1/glext.h>
 #endif
#else
 #import <Cocoa/Cocoa.h>
 #import <CoreAudio/HostTime.h>
 #if JUCE_BUILD_NATIVE
  #import <CoreAudio/AudioHardware.h>
  #import <CoreMIDI/MIDIServices.h>
  #import <QTKit/QTKit.h>
  #import <WebKit/WebKit.h>
  #import <DiscRecording/DiscRecording.h>
  #import <IOKit/IOKitLib.h>
  #import <IOKit/IOCFPlugIn.h>
  #import <IOKit/hid/IOHIDLib.h>
  #import <IOKit/hid/IOHIDKeys.h>
  #import <IOKit/pwr_mgt/IOPMLib.h>
 #endif
 #if JUCE_BUILD_MISC && (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_AU)
  #include <Carbon/Carbon.h>
 #endif
 #include <sys/dir.h>
#endif

#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <fnmatch.h>
#include <utime.h>
#include <dlfcn.h>
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <mach/mach_time.h>
#include <mach-o/dyld.h>

#if MACOS_10_4_OR_EARLIER
 #include <GLUT/glut.h>
#endif

#if ! CGFLOAT_DEFINED
  #define CGFloat float
#endif

#endif   // __JUCE_MAC_NATIVEINCLUDES_JUCEHEADER__
