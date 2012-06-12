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
    static const int newProject             = 0x200010;
    static const int open                   = 0x200020;
    static const int closeDocument          = 0x200030;
    static const int saveDocument           = 0x200040;

    static const int closeProject           = 0x200051;
    static const int saveProject            = 0x200060;
    static const int openInIDE              = 0x200072;
    static const int saveAndOpenInIDE       = 0x200073;
    static const int showProjectSettings    = 0x200074;
    static const int updateModules          = 0x200075;
    static const int showUTF8Tool           = 0x200076;

    static const int saveAll                = 0x200080;
    static const int undo                   = 0x200090;
    static const int redo                   = 0x2000a0;

    static const int closeWindow            = 0x201001;
    static const int closeAllDocuments      = 0x201000;

    static const int test                   = 0x202090;
    static const int toFront                = 0x2020a0;
    static const int toBack                 = 0x2030a1;
    static const int showOrHideProperties   = 0x2030b0;
    static const int showOrHideTree         = 0x2030b1;
    static const int showOrHideMarkers      = 0x2030b2;
    static const int toggleSnapping         = 0x2030b3;

    static const int makeLineSegment        = 0x2030c0;
    static const int makeCubicSegment       = 0x2030c1;
    static const int breakSegment           = 0x2030c2;
    static const int pointModeCorner        = 0x2030c3;
    static const int pointModeRounded       = 0x2030c4;
    static const int pointModeSymmetric     = 0x2030c5;

    static const int group                  = 0x202170;
    static const int ungroup                = 0x202180;

    static const int showPrefs              = 0x2020c0;
    static const int useTabbedWindows       = 0x2020d0;

    static const int showGrid               = 0x2020e0;
    static const int enableSnapToGrid       = 0x2020f0;

    static const int editCompLayout         = 0x202100;
    static const int editCompGraphics       = 0x202110;

    static const int bringBackLostItems     = 0x202120;

    static const int zoomIn                 = 0x202130;
    static const int zoomOut                = 0x202140;
    static const int zoomNormal             = 0x202150;
    static const int spaceBarDrag           = 0x202160;

    static const int compOverlay0           = 0x202200;
    static const int compOverlay33          = 0x202210;
    static const int compOverlay66          = 0x202220;
    static const int compOverlay100         = 0x202230;

    static const int newDocumentBase        = 0x322010;
    static const int newComponentBase       = 0x302010;
    static const int newElementBase         = 0x312010;
}

namespace CommandCategories
{
    static const char* const general       = "General";
    static const char* const editing       = "Editing";
    static const char* const view          = "View";
    static const char* const windows       = "Windows";
}
