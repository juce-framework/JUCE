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

/**
    A namespace to hold all the possible command IDs.
*/
namespace CommandIDs
{
    enum
    {
        newProject              = 0x300000,
        newProjectFromClipboard = 0x300001,
        newPIP                  = 0x300002,
        open                    = 0x300003,
        closeDocument           = 0x300004,
        saveDocument            = 0x300005,
        saveDocumentAs          = 0x300006,

        launchDemoRunner        = 0x300007,

        closeProject            = 0x300010,
        saveProject             = 0x300011,
        saveAll                 = 0x300012,
        openInIDE               = 0x300013,
        saveAndOpenInIDE        = 0x300014,
        createNewExporter       = 0x300015,

        showUTF8Tool            = 0x300020,
        showGlobalPathsWindow   = 0x300021,
        showTranslationTool     = 0x300022,
        showSVGPathTool         = 0x300023,
        showAboutWindow         = 0x300024,
        checkForNewVersion      = 0x300025,
        enableNewVersionCheck   = 0x300026,
        enableGUIEditor         = 0x300027,

        showProjectSettings     = 0x300030,
        showFileExplorerPanel   = 0x300033,
        showModulesPanel        = 0x300034,
        showExportersPanel      = 0x300035,
        showExporterSettings    = 0x300036,

        closeWindow             = 0x300040,
        closeAllWindows         = 0x300041,
        closeAllDocuments       = 0x300042,
        goToPreviousDoc         = 0x300043,
        goToNextDoc             = 0x300044,
        goToCounterpart         = 0x300045,
        deleteSelectedItem      = 0x300046,
        goToPreviousWindow      = 0x300047,
        goToNextWindow          = 0x300048,
        clearRecentFiles        = 0x300049,

        showFindPanel           = 0x300050,
        findSelection           = 0x300051,
        findNext                = 0x300052,
        findPrevious            = 0x300053,

        enableSnapToGrid        = 0x300070,
        zoomIn                  = 0x300071,
        zoomOut                 = 0x300072,
        zoomNormal              = 0x300073,
        spaceBarDrag            = 0x300074,

        loginLogout             = 0x300090,

        showForum               = 0x300100,
        showAPIModules          = 0x300101,
        showAPIClasses          = 0x300102,
        showTutorials           = 0x300103,

        addNewGUIFile           = 0x300200,

        lastCommandIDEntry
    };
}

namespace CommandCategories
{
    static const char* const general       = "General";
    static const char* const editing       = "Editing";
    static const char* const view          = "View";
    static const char* const windows       = "Windows";
}
