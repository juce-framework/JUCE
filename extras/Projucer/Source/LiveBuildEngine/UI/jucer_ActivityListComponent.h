/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class CurrentActivitiesComp   : public Component,
                                private ChangeListener,
                                private ListBoxModel,
                                private Timer
{
public:
    CurrentActivitiesComp (ActivityList& activities)
        : Component ("Activities"), activityList (activities)
    {
        addAndMakeVisible (&list);
        list.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        list.setRowHeight (16);
        list.setModel (this);

        activityList.addChangeListener (this);
    }

    ~CurrentActivitiesComp() override
    {
        activityList.removeChangeListener (this);
    }

    void resized() override         { list.setBounds (getLocalBounds()); }

    int getNumRows() override
    {
        return activityList.getNumActivities();
    }

    void paintListBoxItem (int rowNumber, Graphics& g,
                           int width, int height, bool /*rowIsSelected*/) override
    {
        const StringArray activities (activityList.getActivities());

        if (rowNumber >= 0 && rowNumber < activities.size())
        {
            g.setColour (findColour (defaultTextColourId));

            g.setFont ((float) height * 0.7f);
            g.drawText (activities [rowNumber],
                        4, 0, width - 5, height, Justification::centredLeft, true);
        }
    }

    void paint (Graphics& g) override
    {
        if (getNumRows() == 0)
            TreePanelBase::drawEmptyPanelMessage (*this, g, "(No activities)");
    }

    static int getMaxPanelHeight()      { return 200; }

private:
    ActivityList& activityList;
    ListBox list;
    int panelHeightToSet;

    void timerCallback() override
    {
        stopTimer();

        if (ConcertinaPanel* cp = findParentComponentOfClass<ConcertinaPanel>())
            cp->setPanelSize (this, panelHeightToSet, true);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        list.updateContent();

        panelHeightToSet = jmax (3, getNumRows()) * list.getRowHeight() + 15;

        if (! isTimerRunning())
            startTimer (100);

        repaint();
    }
};
