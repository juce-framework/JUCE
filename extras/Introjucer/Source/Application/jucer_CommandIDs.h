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

/**
    A namespace to hold all the possible command IDs.
*/
namespace CommandIDs
{
    enum
    {
        newProject             = 0x200010,
        open                   = 0x200020,
        closeDocument          = 0x200030,
        saveDocument           = 0x200040,

        closeProject           = 0x200051,
        saveProject            = 0x200060,
        saveAll                = 0x200080,
        openInIDE              = 0x200072,
        saveAndOpenInIDE       = 0x200073,

        updateModules          = 0x200075,
        showUTF8Tool           = 0x200076,
        showAppearanceSettings = 0x200077,
        showConfigPanel        = 0x200074,
        showFilePanel          = 0x200078,

        closeWindow            = 0x201001,
        closeAllDocuments      = 0x201000,
        goToPreviousDoc        = 0x201002,
        goToNextDoc            = 0x201003,
        goToCounterpart        = 0x201004,
        deleteSelectedItem     = 0x201005,

        showFindPanel          = 0x2010a0,
        findSelection          = 0x2010a1,
        findNext               = 0x2010a2,
        findPrevious           = 0x2010a3
    };
}

namespace CommandCategories
{
    static const char* const general       = "General";
    static const char* const editing       = "Editing";
    static const char* const view          = "View";
    static const char* const windows       = "Windows";
}
