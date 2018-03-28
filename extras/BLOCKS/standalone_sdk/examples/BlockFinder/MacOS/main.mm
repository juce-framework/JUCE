/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include "../BlockFinder.h"

#import <Cocoa/Cocoa.h>

int main (int argc, const char * argv[])
{
    // Initialise the JUCE code.
    juce::ScopedJuceInitialiser_GUI platform;

    // Create our JUCE object.
    BlockFinder finder;

    // Run an event loop.
    while ([[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]]);

    return 0;
}
