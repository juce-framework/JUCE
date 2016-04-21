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
namespace JucerCommandIDs
{
    enum
    {
        test                   = 0xf20009,
        toFront                = 0xf2000a,
        toBack                 = 0xf2000b,

        group                  = 0xf20017,
        ungroup                = 0xf20018,

        showGrid               = 0xf2000e,
        enableSnapToGrid       = 0xf2000f,

        editCompLayout         = 0xf20010,
        editCompGraphics       = 0xf20011,

        bringBackLostItems     = 0xf20012,

        zoomIn                 = 0xf20013,
        zoomOut                = 0xf20014,
        zoomNormal             = 0xf20015,
        spaceBarDrag           = 0xf20016,

        compOverlay0           = 0xf20020,
        compOverlay33          = 0xf20021,
        compOverlay66          = 0xf20022,
        compOverlay100         = 0xf20023,

        newDocumentBase        = 0xf32001,
        newComponentBase       = 0xf30001,
        newElementBase         = 0xf31001
    };
}
