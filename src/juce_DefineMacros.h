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

#ifndef __JUCE_DEFINEMACROS_JUCEHEADER__
#define __JUCE_DEFINEMACROS_JUCEHEADER__

//==============================================================================
/*
    This header file re-defines macros that were omitted if the
    JUCE_DONT_DEFINE_MACROS flag was set before including juce.h.

    For a better explanation, see the comments at the top of the
    juce_WithoutMacros.h file.
*/

//==============================================================================
#ifndef __JUCE_WITHOUTMACROS_JUCEHEADER__
  #error "This file is only supposed to be included after juce_WithoutMacros.h!"
#endif

#if (! DONT_SET_USING_JUCE_NAMESPACE) && defined (JUCE_NAMESPACE)

  // on the Mac, these symbols are defined in the Mac libraries, so
  // these macros make it easier to reference them without writing out
  // the namespace every time.
  #if JUCE_MAC
    #define Component       JUCE_NAMESPACE::Component
    #define MemoryBlock     JUCE_NAMESPACE::MemoryBlock
    #define Point           JUCE_NAMESPACE::Point
    #define Button          JUCE_NAMESPACE::Button
  #endif

  // "Rectangle" is defined in some of the newer windows header files..
  #if JUCE_WIN32
    #define Rectangle       JUCE_NAMESPACE::Rectangle
  #endif
#endif

#define T(stringLiteral)            JUCE_T(stringLiteral)

#endif   // __JUCE_DEFINEMACROS_JUCEHEADER__
