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

#ifndef __JUCE_RTASCOMPILEFLAGS_JUCEHEADER__
#define __JUCE_RTASCOMPILEFLAGS_JUCEHEADER__

#ifdef _MSC_VER

  #define kCompileAsCodeResource    0
  #define kBuildStandAlone          0
  #define kNoDSP                    0
  #define kNoDAE                    0
  #define kNoSDS                    0
  #define kNoViews                  0
  #define kUseDSPCodeDecode         0

  #define WINDOWS_VERSION           1
  #define PLUGIN_SDK_BUILD          1
  #define PLUGIN_SDK_DIRECTMIDI     1

#else

  #define kCompileAsCodeResource    0
  #define kNoDSP                    1
  #define kNoDAE                    0
  #define kNoSDS                    0
  #define kNoViews                  0
  #define kUseDSPCodeDecode         0

  #define PLUGIN_SDK_BUILD          1
  #define PLUGIN_SDK_DIRECTMIDI     1

#endif

#endif   // __JUCE_RTASCOMPILEFLAGS_JUCEHEADER__
