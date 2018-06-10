/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             FlexBoxDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Responsive layouts using FlexBox.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        FlexBoxDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct DemoFlexPanel   : public juce::Component
{
    DemoFlexPanel (juce::Colour col, FlexItem& item)
        : flexItem (item), colour (col)
    {
        int x = 70;
        int y = 3;

        setupTextEditor (flexOrderEditor, { x, y, 20, 18 }, "0", [this] { flexItem.order = (int) flexOrderEditor.getText().getFloatValue(); });
        addLabel ("order", flexOrderEditor);
        y += 20;

        setupTextEditor (flexGrowEditor, { x, y, 20, 18 }, "0", [this] { flexItem.flexGrow = flexGrowEditor.getText().getFloatValue(); });
        addLabel ("flex-grow", flexGrowEditor);
        y += 20;

        setupTextEditor (flexShrinkEditor, { x, y, 20, 18 }, "1", [this] { flexItem.flexShrink = flexShrinkEditor.getText().getFloatValue(); });
        addLabel ("flex-shrink", flexShrinkEditor);
        y += 20;

        setupTextEditor (flexBasisEditor, { x, y, 33, 18 }, "100", [this] { flexItem.flexBasis = flexBasisEditor.getText().getFloatValue(); });
        addLabel ("flex-basis", flexBasisEditor);
        y += 20;

        alignSelfCombo.addItem ("auto",       1);
        alignSelfCombo.addItem ("flex-start", 2);
        alignSelfCombo.addItem ("flex-end",   3);
        alignSelfCombo.addItem ("center",     4);
        alignSelfCombo.addItem ("stretch",    5);

        alignSelfCombo.setBounds (x, y, 90, 18);
        alignSelfCombo.onChange = [this] { updateAssignSelf(); };
        alignSelfCombo.setSelectedId (5);
        alignSelfCombo.setColour (ComboBox::outlineColourId, Colours::transparentBlack);
        addAndMakeVisible (alignSelfCombo);
        addLabel ("align-self", alignSelfCombo);
    }

    void setupTextEditor (TextEditor& te, Rectangle<int> b, StringRef initialText, std::function<void()> updateFn)
    {
        te.setBounds (b);
        te.setText (initialText);

        te.onTextChange = [this, updateFn]
        {
            updateFn();
            refreshLayout();
        };

        addAndMakeVisible (te);
    }

    void addLabel (const String& name, Component& target)
    {
        auto label = new Label (name, name);
        label->attachToComponent (&target, true);
        labels.add (label);
        addAndMakeVisible (label);
    }

    void updateAssignSelf()
    {
        switch (alignSelfCombo.getSelectedId())
        {
            case 1:  flexItem.alignSelf = FlexItem::AlignSelf::autoAlign; break;
            case 2:  flexItem.alignSelf = FlexItem::AlignSelf::flexStart; break;
            case 3:  flexItem.alignSelf = FlexItem::AlignSelf::flexEnd;   break;
            case 4:  flexItem.alignSelf = FlexItem::AlignSelf::center;    break;
            case 5:  flexItem.alignSelf = FlexItem::AlignSelf::stretch;   break;
        }

        refreshLayout();
    }

    void refreshLayout()
    {
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

    void lookAndFeelChanged() override
    {
        flexOrderEditor .applyFontToAllText (flexOrderEditor .getFont());
        flexGrowEditor  .applyFontToAllText (flexGrowEditor  .getFont());
        flexShrinkEditor.applyFontToAllText (flexShrinkEditor.getFont());
        flexBasisEditor .applyFontToAllText (flexBasisEditor .getFont());
    }

    FlexItem& flexItem;

    TextEditor flexOrderEditor, flexGrowEditor, flexShrinkEditor, flexBasisEditor;
    ComboBox alignSelfCombo;

    juce::Colour colour;
    OwnedArray<Label> labels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoFlexPanel)

};

//==============================================================================
struct FlexBoxDemo   : public juce::Component
{
    FlexBoxDemo()
    {
        setupPropertiesPanel();
        setupFlexBoxItems();

        setSize (1000, 500);
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
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colours::lightgrey));
        g.setColour (Colours::white);
        g.fillRect (getFlexBoxBounds());
    }

    void setupPropertiesPanel()
    {
        auto directionGroup = addControl (new GroupComponent ("direction", "flex-direction"));
        directionGroup->setBounds (10, 30, 140, 110);

        int i = 0;
        int groupID    = 1234;
        int leftMargin = 15;
        int topMargin  = 45;

        createToggleButton ("row",            groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.flexDirection = FlexBox::Direction::row; }).setToggleState (true, dontSendNotification);
        createToggleButton ("row-reverse",    groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexDirection = FlexBox::Direction::rowReverse; });
        createToggleButton ("column",         groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexDirection = FlexBox::Direction::column; });
        createToggleButton ("column-reverse", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexDirection = FlexBox::Direction::columnReverse; });

        auto wrapGroup = addControl (new GroupComponent ("wrap", "flex-wrap"));
        wrapGroup->setBounds (160, 30, 140, 110);

        i = 0;
        ++groupID;
        leftMargin = 165;

        createToggleButton ("nowrap",       groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexWrap = FlexBox::Wrap::noWrap; });
        createToggleButton ("wrap",         groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.flexWrap = FlexBox::Wrap::wrap; });
        createToggleButton ("wrap-reverse", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.flexWrap = FlexBox::Wrap::wrapReverse; });

        auto justifyGroup = addControl (new GroupComponent ("justify", "justify-content"));
        justifyGroup->setBounds (10, 150, 140, 140);

        i = 0;
        ++groupID;
        leftMargin = 15;
        topMargin  = 165;

        createToggleButton ("flex-start",    groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.justifyContent = FlexBox::JustifyContent::flexStart; });
        createToggleButton ("flex-end",      groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::flexEnd; });
        createToggleButton ("center",        groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::center; });
        createToggleButton ("space-between", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::spaceBetween; });
        createToggleButton ("space-around",  groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.justifyContent = FlexBox::JustifyContent::spaceAround; });

        auto alignGroup = addControl (new GroupComponent ("align", "align-items"));
        alignGroup->setBounds (160, 150, 140, 140);

        i = 0;
        ++groupID;
        leftMargin = 165;
        topMargin  = 165;

        createToggleButton ("stretch",    groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.alignItems = FlexBox::AlignItems::stretch; });
        createToggleButton ("flex-start", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignItems = FlexBox::AlignItems::flexStart; });
        createToggleButton ("flex-end",   groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignItems = FlexBox::AlignItems::flexEnd; });
        createToggleButton ("center",     groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignItems = FlexBox::AlignItems::center; });

        auto alignContentGroup = addControl (new GroupComponent ("content", "align-content"));
        alignContentGroup->setBounds (10, 300, 140, 160);

        i = 0;
        ++groupID;
        leftMargin = 15;
        topMargin  = 315;

        createToggleButton ("stretch",       groupID, leftMargin, topMargin + i++ * 22, true,  [this] { flexBox.alignContent = FlexBox::AlignContent::stretch; });
        createToggleButton ("flex-start",    groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::flexStart; });
        createToggleButton ("flex-end",      groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::flexEnd; });
        createToggleButton ("center",        groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::center; });
        createToggleButton ("space-between", groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::spaceBetween; });
        createToggleButton ("space-around",  groupID, leftMargin, topMargin + i++ * 22, false, [this] { flexBox.alignContent = FlexBox::AlignContent::spaceAround; });
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

        auto panel = panels.add (new DemoFlexPanel (colour, flexItem));
        flexItem.associatedComponent = panel;
        addAndMakeVisible (panel);
    }

    ToggleButton& createToggleButton (StringRef text, int groupID, int x, int y, bool toggleOn, std::function<void()> fn)
    {
        auto* tb = buttons.add (new ToggleButton());
        tb->setButtonText (text);
        tb->setRadioGroupId (groupID);
        tb->setToggleState (toggleOn, dontSendNotification);

        tb->onClick = [this, fn]
        {
            fn();
            resized();
        };

        tb->setBounds (x, y, 130, 22);
        addAndMakeVisible (tb);
        return *tb;
    }

    template <typename ComponentType>
    ComponentType* addControl (ComponentType* newControlComp)
    {
        controls.add (newControlComp);
        addAndMakeVisible (newControlComp);
        return newControlComp;
    }

    FlexBox flexBox;

    OwnedArray<DemoFlexPanel> panels;
    OwnedArray<Component> controls;
    OwnedArray<ToggleButton> buttons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlexBoxDemo)
};
