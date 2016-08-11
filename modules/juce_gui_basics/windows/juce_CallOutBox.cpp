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

CallOutBox::CallOutBox (Component& c, const Rectangle<int>& area, Component* const parent)
    : arrowSize (16.0f), content (c), dismissalMouseClicksAreAlwaysConsumed (false)
{
    addAndMakeVisible (content);

    if (parent != nullptr)
    {
        parent->addChildComponent (this);
        updatePosition (area, parent->getLocalBounds());
        setVisible (true);
    }
    else
    {
        setAlwaysOnTop (juce_areThereAnyAlwaysOnTopWindows());

        updatePosition (area, Desktop::getInstance().getDisplays()
                                .getDisplayContaining (area.getCentre()).userArea);

        addToDesktop (ComponentPeer::windowIsTemporary);

        startTimer (100);
    }

    creationTime = Time::getCurrentTime();
}

CallOutBox::~CallOutBox()
{
}

//==============================================================================
class CallOutBoxCallback  : public ModalComponentManager::Callback,
                            private Timer
{
public:
    CallOutBoxCallback (Component* c, const Rectangle<int>& area, Component* parent)
        : content (c), callout (*c, area, parent)
    {
        callout.setVisible (true);
        callout.enterModalState (true, this);
        startTimer (200);
    }

    void modalStateFinished (int) override {}

    void timerCallback() override
    {
        if (! Process::isForegroundProcess())
            callout.dismiss();
    }

    ScopedPointer<Component> content;
    CallOutBox callout;

    JUCE_DECLARE_NON_COPYABLE (CallOutBoxCallback)
};

CallOutBox& CallOutBox::launchAsynchronously (Component* content, const Rectangle<int>& area, Component* parent)
{
    jassert (content != nullptr); // must be a valid content component!

    return (new CallOutBoxCallback (content, area, parent))->callout;
}

//==============================================================================
void CallOutBox::setArrowSize (const float newSize)
{
    arrowSize = newSize;
    refreshPath();
}

int CallOutBox::getBorderSize() const noexcept
{
    return jmax (getLookAndFeel().getCallOutBoxBorderSize (*this), (int) arrowSize);
}

void CallOutBox::paint (Graphics& g)
{
    getLookAndFeel().drawCallOutBoxBackground (*this, g, outline, background);
}

void CallOutBox::resized()
{
    const int borderSpace = getBorderSize();
    content.setTopLeftPosition (borderSpace, borderSpace);
    refreshPath();
}

void CallOutBox::moved()
{
    refreshPath();
}

void CallOutBox::childBoundsChanged (Component*)
{
    updatePosition (targetArea, availableArea);
}

bool CallOutBox::hitTest (int x, int y)
{
    return outline.contains ((float) x, (float) y);
}

void CallOutBox::inputAttemptWhenModal()
{
    if (dismissalMouseClicksAreAlwaysConsumed
         || targetArea.contains (getMouseXYRelative() + getBounds().getPosition()))
    {
        // if you click on the area that originally popped-up the callout, you expect it
        // to get rid of the box, but deleting the box here allows the click to pass through and
        // probably re-trigger it, so we need to dismiss the box asynchronously to consume the click..

        // For touchscreens, we make sure not to dismiss the CallOutBox immediately,
        // as Windows still sends touch events before the CallOutBox had a chance
        // to really open.

        RelativeTime elapsed = Time::getCurrentTime() - creationTime;
        if (elapsed.inMilliseconds() > 200)
            dismiss();
    }
    else
    {
        exitModalState (0);
        setVisible (false);
    }
}

void CallOutBox::setDismissalMouseClicksAreAlwaysConsumed (bool b) noexcept
{
    dismissalMouseClicksAreAlwaysConsumed = b;
}

enum { callOutBoxDismissCommandId = 0x4f83a04b };

void CallOutBox::handleCommandMessage (int commandId)
{
    Component::handleCommandMessage (commandId);

    if (commandId == callOutBoxDismissCommandId)
    {
        exitModalState (0);
        setVisible (false);
    }
}

void CallOutBox::dismiss()
{
    postCommandMessage (callOutBoxDismissCommandId);
}

bool CallOutBox::keyPressed (const KeyPress& key)
{
    if (key.isKeyCode (KeyPress::escapeKey))
    {
        inputAttemptWhenModal();
        return true;
    }

    return false;
}

void CallOutBox::updatePosition (const Rectangle<int>& newAreaToPointTo, const Rectangle<int>& newAreaToFitIn)
{
    targetArea = newAreaToPointTo;
    availableArea = newAreaToFitIn;

    const int borderSpace = getBorderSize();

    Rectangle<int> newBounds (content.getWidth()  + borderSpace * 2,
                              content.getHeight() + borderSpace * 2);

    const int hw = newBounds.getWidth() / 2;
    const int hh = newBounds.getHeight() / 2;
    const float hwReduced = (float) (hw - borderSpace * 2);
    const float hhReduced = (float) (hh - borderSpace * 2);
    const float arrowIndent = borderSpace - arrowSize;

    Point<float> targets[4] = { Point<float> ((float) targetArea.getCentreX(), (float) targetArea.getBottom()),
                                Point<float> ((float) targetArea.getRight(),   (float) targetArea.getCentreY()),
                                Point<float> ((float) targetArea.getX(),       (float) targetArea.getCentreY()),
                                Point<float> ((float) targetArea.getCentreX(), (float) targetArea.getY()) };

    Line<float> lines[4] = { Line<float> (targets[0].translated (-hwReduced, hh - arrowIndent),    targets[0].translated (hwReduced, hh - arrowIndent)),
                             Line<float> (targets[1].translated (hw - arrowIndent, -hhReduced),    targets[1].translated (hw - arrowIndent, hhReduced)),
                             Line<float> (targets[2].translated (-(hw - arrowIndent), -hhReduced), targets[2].translated (-(hw - arrowIndent), hhReduced)),
                             Line<float> (targets[3].translated (-hwReduced, -(hh - arrowIndent)), targets[3].translated (hwReduced, -(hh - arrowIndent))) };

    const Rectangle<float> centrePointArea (newAreaToFitIn.reduced (hw, hh).toFloat());
    const Point<float> targetCentre (targetArea.getCentre().toFloat());

    float nearest = 1.0e9f;

    for (int i = 0; i < 4; ++i)
    {
        Line<float> constrainedLine (centrePointArea.getConstrainedPoint (lines[i].getStart()),
                                     centrePointArea.getConstrainedPoint (lines[i].getEnd()));

        const Point<float> centre (constrainedLine.findNearestPointTo (targetCentre));
        float distanceFromCentre = centre.getDistanceFrom (targets[i]);

        if (! centrePointArea.intersects (lines[i]))
            distanceFromCentre += 1000.0f;

        if (distanceFromCentre < nearest)
        {
            nearest = distanceFromCentre;

            targetPoint = targets[i];
            newBounds.setPosition ((int) (centre.x - hw),
                                   (int) (centre.y - hh));
        }
    }

    setBounds (newBounds);
}

void CallOutBox::refreshPath()
{
    repaint();
    background = Image();
    outline.clear();

    const float gap = 4.5f;

    outline.addBubble (content.getBounds().toFloat().expanded (gap, gap),
                       getLocalBounds().toFloat(),
                       targetPoint - getPosition().toFloat(),
                       9.0f, arrowSize * 0.7f);
}

void CallOutBox::timerCallback()
{
    toFront (true);
    stopTimer();
}
