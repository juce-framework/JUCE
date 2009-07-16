/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#import <Cocoa/Cocoa.h>
#import <CoreAudio/HostTime.h>
#import <CoreAudio/AudioHardware.h>
#import <CoreMIDI/MIDIServices.h>
#import <QTKit/QTKit.h>
#import <WebKit/WebKit.h>
#import <DiscRecording/DiscRecording.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/IOCFPlugIn.h>
#import <IOKit/hid/IOHIDLib.h>
#import <IOKit/hid/IOHIDKeys.h>
#import <IOKit/network/IOEthernetInterface.h>
#import <IOKit/network/IONetworkInterface.h>
#import <IOKit/network/IOEthernetController.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <fnmatch.h>
#include <utime.h>
#include <dlfcn.h>

#if MACOS_10_4_OR_EARLIER
 #include <GLUT/glut.h>
 typedef int NSInteger;
 typedef unsigned int NSUInteger;
#endif

#endif   // __JUCE_MAC_NATIVEINCLUDES_JUCEHEADER__
