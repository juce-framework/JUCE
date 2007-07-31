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

#ifndef __WIN32_HEADERS_JUCEHEADER__
#define __WIN32_HEADERS_JUCEHEADER__

#include "../../../juce_Config.h"

#define STRICT
#define WIN32_LEAN_AND_MEAN

// don't want to get told about microsoft's mistakes..
#ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable : 4100 4201)
#endif

// use Platform SDK as win2000 unless this is disabled
#ifndef DISABLE_TRANSPARENT_WINDOWS
  #define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <vfw.h>
#include <tchar.h>

#undef PACKED

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

//==============================================================================
#ifndef JUCE_ENABLE_WIN98_COMPATIBILITY
  #define JUCE_ENABLE_WIN98_COMPATIBILITY 1
#endif

// helpers for dynamically loading unicode functions..

#if JUCE_ENABLE_WIN98_COMPATIBILITY
  #define UNICODE_FUNCTION(functionName, returnType, params) \
      typedef returnType (WINAPI *type##functionName) params; \
      static type##functionName w##functionName = 0;

  #define UNICODE_FUNCTION_LOAD(functionName) \
      w##functionName = (type##functionName) GetProcAddress (h, #functionName);  \
      jassert (w##functionName != 0);
#endif


#endif   // __WIN32_HEADERS_JUCEHEADER__
