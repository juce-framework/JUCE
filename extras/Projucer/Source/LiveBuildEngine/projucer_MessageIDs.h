/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
