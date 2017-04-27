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
        showGlobalPreferences   = 0x300021,
        showTranslationTool     = 0x300022,
        showSVGPathTool         = 0x300023,
        showAboutWindow         = 0x300024,

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
