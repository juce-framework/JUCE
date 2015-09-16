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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappingsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<KeyMappingsDemo> demo ("01 Shortcut Keys");
