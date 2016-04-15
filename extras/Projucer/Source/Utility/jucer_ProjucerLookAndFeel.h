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

#ifndef JUCER_PROJUCERLOOKANDFEEL_H_INCLUDED
#define JUCER_PROJUCERLOOKANDFEEL_H_INCLUDED


//==============================================================================
class ProjucerLookAndFeel   : public LookAndFeel_V3
{
public:
    ProjucerLookAndFeel();

    void fillWithBackgroundTexture (Graphics&);
    static void fillWithBackgroundTexture (Component&, Graphics&);

    void drawTabButton (TabBarButton& button, Graphics&, bool isMouseOver, bool isMouseDown) override;
    void drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, int, int) override {}
    int getTabButtonBestWidth (TabBarButton&, int tabDepth) override;
    void drawConcertinaPanelHeader (Graphics&, const juce::Rectangle<int>&, bool, bool, ConcertinaPanel&, Component&) override;
    static Colour getTabBackgroundColour (TabBarButton&);

private:
    Image backgroundTexture;
    Colour backgroundTextureBaseColour;
};



#endif
