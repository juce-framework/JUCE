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

    ~CurrentActivitiesComp()
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
            g.setColour (findColour (mainBackgroundColourId).contrasting (0.7f));

            g.setFont (height * 0.7f);
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
