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

#pragma once

/**
    A namespace to hold all the possible command IDs.
*/
namespace CommandIDs
{
    enum
    {
        newProject              = 0x300000,
        open                    = 0x300001,
        closeDocument           = 0x300002,
        saveDocument            = 0x300003,
        saveDocumentAs          = 0x300004,

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
        showAppUsageWindow      = 0x300025,

        showProjectSettings     = 0x300030,
        showProjectTab          = 0x300031,
        showBuildTab            = 0x300032,
        showFileExplorerPanel   = 0x300033,
        showModulesPanel        = 0x300034,
        showExportersPanel      = 0x300035,
        showExporterSettings    = 0x300036,

        closeWindow             = 0x300040,
        closeAllDocuments       = 0x300041,
        goToPreviousDoc         = 0x300042,
        goToNextDoc             = 0x300043,
        goToCounterpart         = 0x300044,
        deleteSelectedItem      = 0x300045,

        showFindPanel           = 0x300050,
        findSelection           = 0x300051,
        findNext                = 0x300052,
        findPrevious            = 0x300053,

        cleanAll                = 0x300060,
        toggleBuildEnabled      = 0x300061,
        showWarnings            = 0x300062,
        reinstantiateComp       = 0x300063,
        launchApp               = 0x300064,
        killApp                 = 0x300065,
        buildNow                = 0x300066,
        toggleContinuousBuild   = 0x300067,

        enableSnapToGrid        = 0x300070,
        zoomIn                  = 0x300071,
        zoomOut                 = 0x300072,
        zoomNormal              = 0x300073,
        spaceBarDrag            = 0x300074,

        nextError               = 0x300080,
        prevError               = 0x300081,

        loginLogout             = 0x300090,

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
