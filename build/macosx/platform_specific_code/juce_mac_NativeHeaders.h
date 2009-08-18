/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_MAC_NATIVEHEADERS_JUCEHEADER__
#define __JUCE_MAC_NATIVEHEADERS_JUCEHEADER__

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#include <Cocoa/Cocoa.h>


//==============================================================================
BEGIN_JUCE_NAMESPACE

class AutoPool
{
public:
    AutoPool()      { pool = [[NSAutoreleasePool alloc] init]; }
    ~AutoPool()     { [pool release]; }

private:
    NSAutoreleasePool* pool;
};

END_JUCE_NAMESPACE

//==============================================================================
static const JUCE_NAMESPACE::String nsStringToJuce (NSString* s)
{
    return JUCE_NAMESPACE::String::fromUTF8 ((JUCE_NAMESPACE::uint8*) [s UTF8String]);
}

static NSString* juceStringToNS (const JUCE_NAMESPACE::String& s)
{
    return [NSString stringWithUTF8String: (const char*) s.toUTF8()];
}


#endif   // __JUCE_MAC_NATIVEHEADERS_JUCEHEADER__
