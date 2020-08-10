/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

CallOutBox::CallOutBox (Component& c, Rectangle<int> area, Component* const parent)
    : content (c)
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

        updatePosition (area, Desktop::getInstance().getDisplays().findDisplayForRect (area).userArea);

        addToDesktop (ComponentPeer::windowIsTemporary);

        startTimer (100);
    }

    creationTime = Time::getCurrentTime();
}

CallOutBox::~CallOutBox() = default;

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

    std::unique_ptr<Component> content;
    CallOutBox callout;

    JUCE_DECLARE_NON_COPYABLE (CallOutBoxCallback)
};

CallOutBox& CallOutBox::launchAsynchronously (Component* content, Rectangle<int> area, Component* parent)
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

void CallOutBox::lookAndFeelChanged() { resized(); repaint(); }

void CallOutBox::paint (Graphics& g)
{
    getLookAndFeel().drawCallOutBoxBackground (*this, g, outline, background);
}

void CallOutBox::resized()
{
    auto borderSpace = getBorderSize();
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

        auto elapsed = Time::getCurrentTime() - creationTime;

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

    auto borderSpace = getBorderSize();

    Rectangle<int> newBounds (content.getWidth()  + borderSpace * 2,
                              content.getHeight() + borderSpace * 2);

    auto hw = newBounds.getWidth() / 2;
    auto hh = newBounds.getHeight() / 2;
    auto hwReduced = (float) (hw - borderSpace * 2);
    auto hhReduced = (float) (hh - borderSpace * 2);
    auto arrowIndent = (float) borderSpace - arrowSize;

    Point<float> targets[4] = { { (float) targetArea.getCentreX(), (float) targetArea.getBottom() },
                                { (float) targetArea.getRight(),   (float) targetArea.getCentreY() },
                                { (float) targetArea.getX(),       (float) targetArea.getCentreY() },
                                { (float) targetArea.getCentreX(), (float) targetArea.getY() } };

    Line<float> lines[4] = { { targets[0].translated (-hwReduced, hh - arrowIndent),    targets[0].translated (hwReduced, hh - arrowIndent) },
                             { targets[1].translated (hw - arrowIndent, -hhReduced),    targets[1].translated (hw - arrowIndent, hhReduced) },
                             { targets[2].translated (-(hw - arrowIndent), -hhReduced), targets[2].translated (-(hw - arrowIndent), hhReduced) },
                             { targets[3].translated (-hwReduced, -(hh - arrowIndent)), targets[3].translated (hwReduced, -(hh - arrowIndent)) } };

    auto centrePointArea = newAreaToFitIn.reduced (hw, hh).toFloat();
    auto targetCentre = targetArea.getCentre().toFloat();

    float nearest = 1.0e9f;

    for (int i = 0; i < 4; ++i)
    {
        Line<float> constrainedLine (centrePointArea.getConstrainedPoint (lines[i].getStart()),
                                     centrePointArea.getConstrainedPoint (lines[i].getEnd()));

        auto centre = constrainedLine.findNearestPointTo (targetCentre);
        auto distanceFromCentre = centre.getDistanceFrom (targets[i]);

        if (! centrePointArea.intersects (lines[i]))
            distanceFromCentre += 1000.0f;

        if (distanceFromCentre < nearest)
        {
            nearest = distanceFromCentre;
            targetPoint = targets[i];

            newBounds.setPosition ((int) (centre.x - (float) hw),
                                   (int) (centre.y - (float) hh));
        }
    }

    setBounds (newBounds);
}

void CallOutBox::refreshPath()
{
    repaint();
    background = {};
    outline.clear();

    const float gap = 4.5f;

    outline.addBubble (content.getBounds().toFloat().expanded (gap, gap),
                       getLocalBounds().toFloat(),
                       targetPoint - getPosition().toFloat(),
                       getLookAndFeel().getCallOutBoxCornerSize (*this), arrowSize * 0.7f);
}

void CallOutBox::timerCallback()
{
    toFront (true);
    stopTimer();
}

} // namespace juce
