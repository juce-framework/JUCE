/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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

        showForum               = 0x300100,
        showAPIModules          = 0x300101,
        showAPIClasses          = 0x300102,
        showTutorials           = 0x300103,

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
