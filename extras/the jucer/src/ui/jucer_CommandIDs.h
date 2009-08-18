/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
    static const int open                   = 0x20002;
    static const int close                  = 0x20003;
    static const int save                   = 0x20004;
    static const int saveAs                 = 0x20005;
    static const int undo                   = 0x20006;
    static const int redo                   = 0x20007;

    static const int test                   = 0x20009;
    static const int toFront                = 0x2000a;
    static const int toBack                 = 0x2000b;

    static const int group                  = 0x20017;
    static const int ungroup                = 0x20018;

    static const int showPrefs              = 0x2000c;
    static const int useTabbedWindows       = 0x2000d;

    static const int showGrid               = 0x2000e;
    static const int enableSnapToGrid       = 0x2000f;

    static const int editCompLayout         = 0x20010;
    static const int editCompGraphics       = 0x20011;

    static const int bringBackLostItems     = 0x20012;

    static const int zoomIn                 = 0x20013;
    static const int zoomOut                = 0x20014;
    static const int zoomNormal             = 0x20015;
    static const int spaceBarDrag           = 0x20016;

    static const int compOverlay0           = 0x20020;
    static const int compOverlay33          = 0x20021;
    static const int compOverlay66          = 0x20022;
    static const int compOverlay100         = 0x20023;

    static const int newDocumentBase        = 0x32001;
    static const int newComponentBase       = 0x30001;
    static const int newElementBase         = 0x31001;
}

namespace CommandCategories
{
    static const tchar* const general       = T("General");
    static const tchar* const editing       = T("Editing");
    static const tchar* const view          = T("View");
}
