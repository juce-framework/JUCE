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

class BuildStatusTabComp   : public Component,
                             private ChangeListener,
                             private Timer
{
public:
    BuildStatusTabComp (ErrorList& el, ActivityList& al)
        : errorList (el), activityList (al)
    {
        setInterceptsMouseClicks (false, false);
        addAndMakeVisible (&spinner);
        activityList.addChangeListener (this);
        errorList.addChangeListener (this);
    }

    ~BuildStatusTabComp()
    {
        activityList.removeChangeListener (this);
        errorList.removeChangeListener (this);
    }

    enum { size = 20 };

    void updateStatus()
    {
        State newState = nothing;

        if (activityList.getNumActivities() > 0)    newState = busy;
        else if (errorList.getNumErrors() > 0)      newState = errors;
        else if (errorList.getNumWarnings() > 0)    newState = warnings;

        if (newState != state)
        {
            state = newState;
            setSize (state != nothing ? size : 0, size);
            spinner.setVisible (state == busy);
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
        if (state == errors || state == warnings)
        {
            g.setColour (findColour (mainBackgroundColourId).contrasting (state == errors ? Colours::red
                                                                                          : Colours::yellow, 0.4f));
            const Path& icon = (state == errors) ? getIcons().warning
                                                 : getIcons().info;

            g.fillPath (icon, RectanglePlacement (RectanglePlacement::centred)
                                .getTransformToFit (icon.getBounds(),
                                                    getCentralArea().reduced (1, 1).toFloat()));
        }
    }

    void resized() override
    {
        spinner.setBounds (getCentralArea());
    }

    Rectangle<int> getCentralArea() const
    {
        return getLocalBounds().withTrimmedRight (4);
    }

private:
    ErrorList& errorList;
    ActivityList& activityList;

    void changeListenerCallback (ChangeBroadcaster*) override   { if (! isTimerRunning()) startTimer (150); }
    void timerCallback() override                               { stopTimer(); updateStatus(); }

    enum State
    {
        nothing,
        busy,
        errors,
        warnings
    };

    State state;

    //==============================================================================
    class Spinner  : public Component,
                     private Timer
    {
    public:
        Spinner()
        {
            setInterceptsMouseClicks (false, false);
        }

        void paint (Graphics& g) override
        {
            if (TabBarButton* tbb = findParentComponentOfClass<TabBarButton>())
            {
                getLookAndFeel().drawSpinningWaitAnimation (g, ProjucerLookAndFeel::getTabBackgroundColour (*tbb).contrasting(),
                                                            0, 0, getWidth(), getHeight());
                startTimer (1000 / 20);
            }
        }

        void timerCallback() override
        {
            if (isVisible())
                repaint();
            else
                stopTimer();
        }
    };

    Spinner spinner;
};
