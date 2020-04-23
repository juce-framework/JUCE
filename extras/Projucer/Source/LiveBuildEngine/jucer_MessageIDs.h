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


namespace MessageTypes
{
    #define DECLARE_ID(name)  const Identifier name (#name)

    DECLARE_ID (PING);
    DECLARE_ID (BUILDINFO);
    DECLARE_ID (COMPILEUNIT);
    DECLARE_ID (USERFILE);
    DECLARE_ID (DIAGNOSTIC);
    DECLARE_ID (DIAGNOSTIC_LIST);

    DECLARE_ID (ACTIVITY_LIST);
    DECLARE_ID (MISSING_SYSTEM_HEADERS);
    DECLARE_ID (BUILD_FAILED);
    DECLARE_ID (CHANGE_CODE);
    DECLARE_ID (HIGHLIGHT_CODE);
    DECLARE_ID (CRASH);
    DECLARE_ID (LAUNCHED);
    DECLARE_ID (APPQUIT);
    DECLARE_ID (KEY);
    DECLARE_ID (QUIT_IDE);

    DECLARE_ID (CLEAN_ALL);
    DECLARE_ID (OPEN_PREVIEW);
    DECLARE_ID (RELOAD);
    DECLARE_ID (LIVE_FILE_CHANGES);
    DECLARE_ID (CHANGE);
    DECLARE_ID (LIVE_FILE_UPDATE);
    DECLARE_ID (LIVE_FILE_RESET);
    DECLARE_ID (LAUNCH_APP);
    DECLARE_ID (FOREGROUND);
    DECLARE_ID (QUIT_SERVER);

    #undef DECLARE_ID
}
