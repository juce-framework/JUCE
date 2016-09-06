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

// these classes are C++11-only
#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS && JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS && JUCE_COMPILER_SUPPORTS_LAMBDAS

struct DemoFlexPanel   : public juce::Component,
                         private juce::TextEditor::Listener,
                         private juce::ComboBox::Listener
{
    DemoFlexPanel (juce::Colour col, FlexItem& item)  : flexItem (item), colour (col)
    {
        int x = 70;
        int y = 3;

        setupTextEditor (flexOrderEditor, { x, y, 20, 18 }, "0");
        addLabel ("order", flexOrderEditor);
        y += 20;

        setupTextEditor (flexGrowEditor, { x, y, 20, 18 }, "0");
        addLabel ("flex-grow", flexGrowEditor);
        y += 20;

        setupTextEditor (flexShrinkEditor, { x, y, 20, 18 }, "1");
        addLabel ("flex-shrink", flexShrinkEditor);
        y += 20;

        setupTextEditor (flexBasisEditor, { x, y, 33, 18 }, "100");
        addLabel ("flex-basis", flexBasisEditor);
        y += 20;

        alignSelfCombo.addItem ("auto",       1);
        alignSelfCombo.addItem ("flex-start", 2);
        alignSelfCombo.addItem ("flex-end",   3);
        alignSelfCombo.addItem ("center",     4);
        alignSelfCombo.addItem ("stretch",    5);

        alignSelfCombo.setBounds (x, y, 90, 18);
        alignSelfCombo.addListener (this);
        alignSelfCombo.setSelectedId (5);
        alignSelfCombo.setColour (ComboBox::outlineColourId, Colours::transparentBlack);
        addAndMakeVisible (alignSelfCombo);
        addLabel ("align-self", alignSelfCombo);
    }

    void setupTextEditor (TextEditor& te, Rectangle<int> b, StringRef initialText)
    {
        te.setBounds (b);
        te.setText (initialText);
        te.addListener (this);

        addAndMakeVisible (te);
    }

    void addLabel (const String& name, Component& target)
    {
        auto label = new Label (name, name);
        label->attachToComponent (&target, true);
        labels.add (label);
        addAndMakeVisible (label);
    }

    void comboBoxChanged (ComboBox* cb) override
    {
        auto selectedID = cb->getSelectedId();

        if (selectedID == 1)  flexItem.alignSelf = FlexItem::AlignSelf::autoAlign;
        if (selectedID == 2)  flexItem.alignSelf = FlexItem::AlignSelf::flexStart;
        if (selectedID == 3)  flexItem.alignSelf = FlexItem::AlignSelf::flexEnd;
        if (selectedID == 4)  flexItem.alignSelf = FlexItem::AlignSelf::center;
        if (selectedID == 5)  flexItem.alignSelf = FlexItem::AlignSelf::stretch;

        if (auto parent = getParentComponent())
            parent->resized();
    }

    void textEditorTextChanged (TextEditor& textEditor) override
    {
        auto textIntValue = textEditor.getText().getFloatValue();

        if (&textEditor == &flexOrderEditor)   flexItem.order      = (int) textIntValue;
        if (&textEditor == &flexGrowEditor)    flexItem.flexGrow   = textIntValue;
        if (&textEditor == &flexBasisEditor)   flexItem.flexBasis  = textIntValue;
        if (&textEditor == &flexShrinkEditor)  flexItem.flexShrink = textIntValue;

        if (auto parent = getParentComponent())
            parent->resized();
    }

    void paint (Graphics& g) override
    {
        auto r = getLocalBounds();

        g.setColour (colour);
        g.fillRect (r);

        g.setColour (Colours::black);
        g.drawFittedText ("w: " + String (r.getWidth()) + newLine + "h: " + String (r.getHeight()),
                          r.reduced (4), Justification::bottomRight, 2);
    }

    FlexItem& flexItem;

    TextEditor flexOrderEditor, flexGrowEditor, flexShrinkEditor, flexBasisEditor;
    ComboBox alignSelfCombo;

    juce::Colour colour;
    OwnedArray<Label> labels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoFlexPanel)

};

//==============================================================================
struct FlexBoxDemo   : public juce::Component,
                       private juce::Button::Listener
{
    FlexBoxDemo()
    {
        setupPropertiesPanel();
        setupFlexBoxItems();
    }

    void resized() override
    {
        flexBox.performLayout (getFlexBoxBounds());
    }

    Rectangle<float> getFlexBoxBounds() const
    {
        return getLocalBounds().withTrimmedLeft (300)
                               .reduced (10)
                               .toFloat();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightgrey);

        g.setColour (Colours::white);
        g.fillRect (getFlexBoxBounds());
    }

    void setupPropertiesPanel()
    {
        auto directionGroup = addControl (new GroupComponent ("direction", "flex-direction"));
        directionGroup->setBounds (10, 30, 140, 110);

        int i = 0;
        int groupID = 1234;
        int leftMargin = 15;
        int topMargin = 45;

        setupToggleButton (flexDirectionRowButton,           "row",            groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (flexDirectionRowReverseButton,    "row-reverse",    groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (flexDirectionColumnButton,        "column",         groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (flexDirectionColumnReverseButton, "column-reverse", groupID, leftMargin, topMargin + i++ * 22);
        flexDirectionRowButton.setToggleState (true, dontSendNotification);

        auto wrapGroup = addControl (new GroupComponent ("wrap", "flex-wrap"));
        wrapGroup->setBounds (160, 30, 140, 110);

        i = 0;
        ++groupID;
        leftMargin = 165;

        setupToggleButton (flexNoWrapButton,      "nowrap",       groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (flexWrapButton,        "wrap",         groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (flexWrapReverseButton, "wrap-reverse", groupID, leftMargin, topMargin + i++ * 22);
        flexWrapButton.setToggleState (true, sendNotification);

        auto justifyGroup = addControl (new GroupComponent ("justify", "justify-content"));
        justifyGroup->setBounds (10, 150, 140, 140);

        i = 0;
        ++groupID;
        leftMargin = 15;
        topMargin = 165;

        setupToggleButton (justifyFlexStartButton,    "flex-start",    groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (justifyFlexEndButton,      "flex-end",      groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (justifyCenterButton,       "center",        groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (justifySpaceBetweenButton, "space-between", groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (justifySpaceAroundButton,  "space-around",  groupID, leftMargin, topMargin + i++ * 22);
        justifyFlexStartButton.setToggleState (true, sendNotification);

        auto alignGroup = addControl (new GroupComponent ("align", "align-items"));
        alignGroup->setBounds (160, 150, 140, 140);

        i = 0;
        ++groupID;
        leftMargin = 165;
        topMargin = 165;

        setupToggleButton (alignStretchButton,   "stretch",    groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignFlexStartButton, "flex-start", groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignFlexEndButton,   "flex-end",   groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignCenterButton,    "center",     groupID, leftMargin, topMargin + i++ * 22);
        alignStretchButton.setToggleState (true, sendNotification);

        auto alignContentGroup = addControl (new GroupComponent ("content", "align-content"));
        alignContentGroup->setBounds (10, 300, 140, 160);

        i = 0;
        ++groupID;
        leftMargin = 15;
        topMargin = 315;

        setupToggleButton (alignContentStretchButton,      "stretch",       groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignContentFlexStartButton,    "flex-start",    groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignContentFlexEndButton,      "flex-end",      groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignContentCenterButton,       "center",        groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignContentSpaceBetweenButton, "space-between", groupID, leftMargin, topMargin + i++ * 22);
        setupToggleButton (alignContentSpaceAroundButton,  "space-around",  groupID, leftMargin, topMargin + i++ * 22);
        alignContentStretchButton.setToggleState (true, sendNotification);
    }

    void setupFlexBoxItems()
    {
        addItem (Colours::orange);
        addItem (Colours::aqua);
        addItem (Colours::lightcoral);
        addItem (Colours::aquamarine);
        addItem (Colours::forestgreen);
    }

    void addItem (Colour colour)
    {
        flexBox.items.add (FlexItem (100, 150)
                             .withMargin (10)
                             .withWidth (200));

        auto& flexItem = flexBox.items.getReference (flexBox.items.size() - 1);

        auto panel = new DemoFlexPanel (colour, flexItem);
        panels.add (panel);
        flexItem.associatedComponent = panel;
        addAndMakeVisible (panel);
    }

    void setupToggleButton (ToggleButton& tb, StringRef text, int groupID, int x, int y)
    {
        tb.setButtonText (text);
        tb.setRadioGroupId (groupID);
        tb.setToggleState (false, dontSendNotification);
        tb.addListener (this);
        tb.setBounds (x, y, 130, 22);

        addAndMakeVisible (tb);
    }

    template <typename ComponentType>
    ComponentType* addControl (ComponentType* newControlComp)
    {
        controls.add (newControlComp);
        addAndMakeVisible (newControlComp);
        return newControlComp;
    }

    void buttonClicked (Button* b) override
    {
        if (b->getToggleState())
        {
            if (b == &flexDirectionRowButton)                   flexBox.flexDirection   = FlexBox::Direction::row;
            else if (b == &flexDirectionRowReverseButton)       flexBox.flexDirection   = FlexBox::Direction::rowReverse;
            else if (b == &flexDirectionColumnButton)           flexBox.flexDirection   = FlexBox::Direction::column;
            else if (b == &flexDirectionColumnReverseButton)    flexBox.flexDirection   = FlexBox::Direction::columnReverse;
            else if (b == &flexNoWrapButton)                    flexBox.flexWrap        = FlexBox::Wrap::noWrap;
            else if (b == &flexWrapButton)                      flexBox.flexWrap        = FlexBox::Wrap::wrap;
            else if (b == &flexWrapReverseButton)               flexBox.flexWrap        = FlexBox::Wrap::wrapReverse;
            else if (b == &justifyFlexStartButton)              flexBox.justifyContent  = FlexBox::JustifyContent::flexStart;
            else if (b == &justifyFlexEndButton)                flexBox.justifyContent  = FlexBox::JustifyContent::flexEnd;
            else if (b == &justifyCenterButton)                 flexBox.justifyContent  = FlexBox::JustifyContent::center;
            else if (b == &justifySpaceBetweenButton)           flexBox.justifyContent  = FlexBox::JustifyContent::spaceBetween;
            else if (b == &justifySpaceAroundButton)            flexBox.justifyContent  = FlexBox::JustifyContent::spaceAround;
            else if (b == &alignStretchButton)                  flexBox.alignItems      = FlexBox::AlignItems::stretch;
            else if (b == &alignFlexStartButton)                flexBox.alignItems      = FlexBox::AlignItems::flexStart;
            else if (b == &alignFlexEndButton)                  flexBox.alignItems      = FlexBox::AlignItems::flexEnd;
            else if (b == &alignCenterButton)                   flexBox.alignItems      = FlexBox::AlignItems::center;
            else if (b == &alignContentStretchButton)           flexBox.alignContent    = FlexBox::AlignContent::stretch;
            else if (b == &alignContentFlexStartButton)         flexBox.alignContent    = FlexBox::AlignContent::flexStart;
            else if (b == &alignContentFlexEndButton)           flexBox.alignContent    = FlexBox::AlignContent::flexEnd;
            else if (b == &alignContentCenterButton)            flexBox.alignContent    = FlexBox::AlignContent::center;
            else if (b == &alignContentSpaceBetweenButton)      flexBox.alignContent    = FlexBox::AlignContent::spaceBetween;
            else if (b == &alignContentSpaceAroundButton)       flexBox.alignContent    = FlexBox::AlignContent::spaceAround;
            else return;

            resized();
        }
    }

    FlexBox flexBox;

    OwnedArray<DemoFlexPanel> panels;
    OwnedArray<Component> controls;

    ToggleButton flexDirectionRowButton,
                 flexDirectionRowReverseButton,
                 flexDirectionColumnButton,
                 flexDirectionColumnReverseButton,

                 flexNoWrapButton,
                 flexWrapButton,
                 flexWrapReverseButton,

                 justifyFlexStartButton,
                 justifyFlexEndButton,
                 justifyCenterButton,
                 justifySpaceBetweenButton,
                 justifySpaceAroundButton,

                 alignStretchButton,
                 alignFlexStartButton,
                 alignFlexEndButton,
                 alignCenterButton,

                 alignContentStretchButton,
                 alignContentFlexStartButton,
                 alignContentFlexEndButton,
                 alignContentCenterButton,
                 alignContentSpaceBetweenButton,
                 alignContentSpaceAroundButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlexBoxDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<FlexBoxDemo> demo ("10 Components: FlexBox");

#endif
