/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

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

#include "../JuceDemoHeader.h"


//==============================================================================
class KeyMappingsDemo   : public Component
{
public:
    KeyMappingsDemo()
        : keyMappingEditor (*MainAppWindow::getApplicationCommandManager().getKeyMappings(), true)
    {
        setOpaque (true);
        addAndMakeVisible (keyMappingEditor);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colour::greyLevel (0.93f));
    }

    void resized() override
    {
        keyMappingEditor.setBounds (getLocalBounds().reduced (4));
    }

private:
    KeyMappingEditorComponent keyMappingEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappingsDemo);
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<KeyMappingsDemo> demo ("01 Shortcut Keys");
