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
class ComponentTransformsDemo  : public Component
{
public:
    ComponentTransformsDemo()
    {
        addAndMakeVisible (content = createContentComp());
        content->setSize (800, 600);

        for (int i = 0; i < 3; ++i)
        {
            CornerDragger* d = new CornerDragger();
            draggers.add (d);
            addAndMakeVisible (d);
        }

        draggers.getUnchecked(0)->relativePos = Point<float> (0.10f, 0.15f);
        draggers.getUnchecked(1)->relativePos = Point<float> (0.95f, 0.05f);
        draggers.getUnchecked(2)->relativePos = Point<float> (0.05f, 0.85f);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);

        g.setColour (Colours::white);
        g.setFont (15.0f);
        g.drawFittedText ("Drag the corner-points around to show how complex components can have affine-transforms applied...",
                          getLocalBounds().removeFromBottom (40).reduced (10, 0), Justification::centred, 3);
    }

    void resized() override
    {
        for (int i = 0; i < 3; ++i)
        {
            CornerDragger* d = draggers.getUnchecked(i);

            d->setCentrePosition (proportionOfWidth (d->relativePos.x),
                                  proportionOfHeight (d->relativePos.y));
        }
    }

    void childBoundsChanged (Component* child) override
    {
        if (dynamic_cast<CornerDragger*> (child) != nullptr)
            updateTransform();
    }

private:
    ScopedPointer<Component> content;

    static Component* createContentComp()
    {
        Array<JuceDemoTypeBase*>& demos (JuceDemoTypeBase::getDemoTypeList());

        for (int i = 0; i < demos.size(); ++i)
            if (demos.getUnchecked(i)->name.containsIgnoreCase ("Widgets"))
                return demos.getUnchecked (i)->createComponent();

        jassertfalse;
        return nullptr;
    }

    struct CornerDragger  : public Component
    {
        CornerDragger()
        {
            setSize (30, 30);
            setRepaintsOnMouseActivity (true);
        }

        void paint (Graphics& g) override
        {
            g.setColour (Colours::white.withAlpha (isMouseOverOrDragging() ? 0.9f : 0.5f));
            g.fillEllipse (getLocalBounds().reduced (3).toFloat());

            g.setColour (Colours::darkgreen);
            g.drawEllipse (getLocalBounds().reduced (3).toFloat(), 2.0f);
        }

        void resized() override
        {
            constrainer.setMinimumOnscreenAmounts (getHeight(), getWidth(), getHeight(), getWidth());
        }

        void moved() override
        {
            if (isMouseButtonDown())
                relativePos = getBounds().getCentre().toFloat() / Point<int> (getParentWidth(), getParentHeight()).toFloat();
        }

        void mouseDown (const MouseEvent& e) override   { dragger.startDraggingComponent (this, e); }
        void mouseDrag (const MouseEvent& e) override   { dragger.dragComponent (this, e, &constrainer); }

        Point<float> relativePos;

    private:
        ComponentBoundsConstrainer constrainer;
        ComponentDragger dragger;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CornerDragger)
    };

    OwnedArray<CornerDragger> draggers;

    Point<float> getDraggerPos (int index) const
    {
        return draggers.getUnchecked(index)->getBounds().getCentre().toFloat();
    }

    void updateTransform()
    {
        const Point<float> p0 (getDraggerPos(0));
        const Point<float> p1 (getDraggerPos(1));
        const Point<float> p2 (getDraggerPos(2));

        if (p0 != p1 && p1 != p2 && p0 != p2)
            content->setTransform (AffineTransform::fromTargetPoints (0, 0, p0.x, p0.y,
                                                                      (float) content->getWidth(), 0, p1.x, p1.y,
                                                                      0, (float) content->getHeight(), p2.x, p2.y));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentTransformsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<ComponentTransformsDemo> demo ("10 Components: Transforms");
