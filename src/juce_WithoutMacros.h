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

#ifndef __JUCE_WITHOUTMACROS_JUCEHEADER__
#define __JUCE_WITHOUTMACROS_JUCEHEADER__

//==============================================================================
/**
    The purpose of juce_WithoutMacros.h is to allow you to include juce.h without
    it defining macros that could interfere with other 3rd party header files.

    E.g. if code like this causes errors:
    @code
    #include <juce.h>
    #include "problematic_3rd_party_header_file.h" // causes errors because this code also uses
                                                   // macros such as "T" or "Point"...
    @endcode

    ..then you can avoid the problems by doing this:

    @code
    #include <src/juce_WithoutMacros.h> // includes everything in juce.h, but
                                        // without the macros that are likely to confict

    #include "problematic_3rd_party_header_file.h"

    #include <src/juce_DefineMacros.h>  // this is optional - including it after the
                                        // 3rd party file will re-define the juce macros
                                        // that were omitted previously. You may need to
                                        // avoid including this if your code later on uses
                                        // macros that conflict.
    @endcode
*/


//==============================================================================
#ifdef __JUCE_JUCEHEADER__
  #error "juce.h has already been included - this file is an ALTERNATIVE to juce.h, so you can only include one of them!"
#endif

#define JUCE_DONT_DEFINE_MACROS 1
#include "../juce.h"
#undef JUCE_DONT_DEFINE_MACROS

#endif   // __JUCE_WITHOUTMACROS_JUCEHEADER__
