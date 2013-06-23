/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    static const int test                   = 0xf20009;
    static const int toFront                = 0xf2000a;
    static const int toBack                 = 0xf2000b;

    static const int group                  = 0xf20017;
    static const int ungroup                = 0xf20018;

    static const int showGrid               = 0xf2000e;
    static const int enableSnapToGrid       = 0xf2000f;

    static const int editCompLayout         = 0xf20010;
    static const int editCompGraphics       = 0xf20011;

    static const int bringBackLostItems     = 0xf20012;

    static const int zoomIn                 = 0xf20013;
    static const int zoomOut                = 0xf20014;
    static const int zoomNormal             = 0xf20015;
    static const int spaceBarDrag           = 0xf20016;

    static const int compOverlay0           = 0xf20020;
    static const int compOverlay33          = 0xf20021;
    static const int compOverlay66          = 0xf20022;
    static const int compOverlay100         = 0xf20023;

    static const int newDocumentBase        = 0xf32001;
    static const int newComponentBase       = 0xf30001;
    static const int newElementBase         = 0xf31001;
}
