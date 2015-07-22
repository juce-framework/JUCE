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
        saveDocumentAs         = 0x200041,

        closeProject           = 0x200051,
        saveProject            = 0x200060,
        saveAll                = 0x200080,
        openInIDE              = 0x200072,
        saveAndOpenInIDE       = 0x200073,

        showUTF8Tool           = 0x200076,
        showAppearanceSettings = 0x200077,
        showConfigPanel        = 0x200074,
        showFilePanel          = 0x200078,
        showTranslationTool    = 0x200079,
        showProjectSettings    = 0x20007a,
        showProjectModules     = 0x20007b,
        showSVGPathTool        = 0x20007c,

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
