/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
// These functions are called by our child process on startup, to launch
// the compilation server

String createCommandLineForLaunchingServer (const String& pipeName,
                                            const String& projectUID,
                                            const File& cacheLocation);

void* createClangServer (const String& commandLine);
void destroyClangServer (void*);

// Called if our child process is asked to shutdown by the user, so it can pass
// that shutdown event up to the parent (IDE) process..
void sendQuitMessageToIDE (void*);
