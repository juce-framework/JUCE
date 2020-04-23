/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
