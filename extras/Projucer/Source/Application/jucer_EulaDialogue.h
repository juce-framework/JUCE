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

#ifndef PROJUCER_EULADIALOGUE_H_INCLUDED
#define PROJUCER_EULADIALOGUE_H_INCLUDED


class EULADialogue : public AlertWindow
{
public:
    EULADialogue()
        : AlertWindow ("End User Licence Agreement",
                       "Please accept the End User Licence Agreement to run the Projucer.",
                       AlertWindow::NoIcon,
                       nullptr)
    {
        setColour (AlertWindow::backgroundColourId, ProjucerDialogLookAndFeel::getBackgroundColour());
        setColour (AlertWindow::textColourId, ProjucerDialogLookAndFeel::getBrightButtonColour());
        setLookAndFeel (&lookAndFeel);

        addButton ("Accept",  EULADialogue::accepted);
        addButton ("Decline", EULADialogue::declined);

        addCustomComponent (&component);
    }

    enum EULADialogueResult
    {
        accepted,
        declined
    };

private:
    struct EULADialogueComponent : public Component
    {
        EULADialogueComponent()
        {
            setSize (700, 550);

            editor.setSize (getWidth(), getHeight() - 50);
            editor.setReadOnly (true);
            editor.setCaretVisible (false);
            editor.setMultiLine (true, true);
            editor.setScrollbarsShown (true);
            editor.setFont (Font (Font::getDefaultMonospacedFontName(), 13.0f, Font::plain));
            editor.setText (String (BinaryData::projucer_EULA_txt));

            addAndMakeVisible (editor);
        }

        TextEditor editor;
    };

    EULADialogueComponent component;
    ProjucerDialogLookAndFeel lookAndFeel;
};


#endif  // PROJUCER_EULADIALOGUE_H_INCLUDED
