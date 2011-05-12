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

//==============================================================================
#ifndef __JUCE_ANDROID_NATIVEINCLUDES_JUCEHEADER__
#define __JUCE_ANDROID_NATIVEINCLUDES_JUCEHEADER__


#include "../../core/juce_TargetPlatform.h"
#include "../../../juce_Config.h"

#include <jni.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/ptrace.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <pwd.h>
#include <dirent.h>
#include <fnmatch.h>

#if JUCE_OPENGL
 #include <GLES/gl.h>
#endif


#endif   // __JUCE_ANDROID_NATIVEINCLUDES_JUCEHEADER__
