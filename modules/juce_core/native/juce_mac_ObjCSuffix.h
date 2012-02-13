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

#ifndef __JUCE_MAC_OBJCSUFFIX_JUCEHEADER__
#define __JUCE_MAC_OBJCSUFFIX_JUCEHEADER__

/** This suffix is used for naming all Obj-C classes that are used inside juce.

    Because of the flat naming structure used by Obj-C, you can get horrible situations where
    two DLLs are loaded into a host, each of which uses classes with the same names, and these get
    cross-linked so that when you make a call to a class that you thought was private, it ends up
    actually calling into a similarly named class in the other module's address space.

    By changing this macro to a unique value, you ensure that all the obj-C classes in your app
    have unique names, and should avoid this problem.

    If you're using the amalgamated version, you can just set this macro to something unique before
    you include juce_amalgamated.cpp.
*/
#ifndef JUCE_ObjCExtraSuffix
 #define JUCE_ObjCExtraSuffix 3
#endif

#ifndef DOXYGEN
 #define appendMacro1(a, b, c, d, e) a ## _ ## b ## _ ## c ## _ ## d ## _ ## e
 #define appendMacro2(a, b, c, d, e) appendMacro1(a, b, c, d, e)
 #define MakeObjCClassName(rootName) appendMacro2 (rootName, JUCE_MAJOR_VERSION, JUCE_MINOR_VERSION, JUCE_BUILDNUMBER, JUCE_ObjCExtraSuffix)
#endif

#endif   // __JUCE_MAC_OBJCSUFFIX_JUCEHEADER__
