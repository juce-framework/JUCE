/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#define DECLARE_ID(name)      static const Identifier name (#name)

namespace MessageTypes
{
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
}

#undef DECLARE_ID
