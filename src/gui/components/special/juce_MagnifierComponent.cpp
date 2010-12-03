/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MagnifierComponent.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../windows/juce_ComponentPeer.h"


//==============================================================================
class MagnifyingPeer    : public ComponentPeer
{
public:
    //==============================================================================
    MagnifyingPeer (Component* const component_,
                    MagnifierComponent* const magnifierComp_)
        : ComponentPeer (component_, 0),
          magnifierComp (magnifierComp_)
    {
    }

    ~MagnifyingPeer()
    {
    }

    //==============================================================================
    void* getNativeHandle() const                   { return 0; }
    void setVisible (bool)                          {}
    void setTitle (const String&)                   {}
    void setPosition (int, int)                     {}
    void setSize (int, int)                         {}
    void setBounds (int, int, int, int, bool)       {}
    void setMinimised (bool)                        {}
    void setAlpha (float /*newAlpha*/)              {}
    bool isMinimised() const                        { return false; }
    void setFullScreen (bool)                       {}
    bool isFullScreen() const                       { return false; }
    const BorderSize getFrameSize() const           { return BorderSize (0); }
    bool setAlwaysOnTop (bool)                      { return true; }
    void toFront (bool)                             {}
    void toBehind (ComponentPeer*)                  {}
    void setIcon (const Image&)                     {}

    bool isFocused() const
    {
        return magnifierComp->hasKeyboardFocus (true);
    }

    void grabFocus()
    {
        ComponentPeer* peer = magnifierComp->getPeer();
        if (peer != 0)
            peer->grabFocus();
    }

    void textInputRequired (const Point<int>& position)
    {
        ComponentPeer* peer = magnifierComp->getPeer();
        if (peer != 0)
            peer->textInputRequired (position);
    }

    const Rectangle<int> getBounds() const
    {
        return Rectangle<int> (magnifierComp->getScreenX(), magnifierComp->getScreenY(),
                               component->getWidth(), component->getHeight());
    }

    const Point<int> getScreenPosition() const
    {
        return magnifierComp->getScreenPosition();
    }

    const Point<int> localToGlobal (const Point<int>& relativePosition)
    {
        const double zoom = magnifierComp->getScaleFactor();
        return magnifierComp->localPointToGlobal (Point<int> (roundToInt (relativePosition.getX() * zoom),
                                                              roundToInt (relativePosition.getY() * zoom)));
    }

    const Point<int> globalToLocal (const Point<int>& screenPosition)
    {
        const Point<int> p (magnifierComp->getLocalPoint (0, screenPosition));
        const double zoom = magnifierComp->getScaleFactor();

        return Point<int> (roundToInt (p.getX() / zoom),
                           roundToInt (p.getY() / zoom));
    }

    bool contains (const Point<int>& position, bool) const
    {
        return isPositiveAndBelow (position.getX(), magnifierComp->getWidth())
                && isPositiveAndBelow (position.getY(), magnifierComp->getHeight());
    }

    void repaint (const Rectangle<int>& area)
    {
        const double zoom = magnifierComp->getScaleFactor();

        magnifierComp->repaint ((int) (area.getX() * zoom),
                                (int) (area.getY() * zoom),
                                roundToInt (area.getWidth() * zoom) + 1,
                                roundToInt (area.getHeight() * zoom) + 1);
    }

    void performAnyPendingRepaintsNow()
    {
    }

    //==============================================================================
private:
    MagnifierComponent* const magnifierComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MagnifyingPeer);
};


//==============================================================================
class PeerHolderComp    : public Component
{
public:
    PeerHolderComp (MagnifierComponent* const magnifierComp_)
        : magnifierComp (magnifierComp_)
    {
        setVisible (true);
    }

    ~PeerHolderComp()
    {
    }

    ComponentPeer* createNewPeer (int, void*)
    {
        return new MagnifyingPeer (this, magnifierComp);
    }

    void childBoundsChanged (Component* c)
    {
        if (c != 0)
        {
            setSize (c->getWidth(), c->getHeight());
            magnifierComp->childBoundsChanged (this);
        }
    }

    void mouseWheelMove (const MouseEvent& e, float ix, float iy)
    {
        // unhandled mouse wheel moves can be referred upwards to the parent comp..
        Component* const p = magnifierComp->getParentComponent();

        if (p != 0)
            p->mouseWheelMove (e.getEventRelativeTo (p), ix, iy);
    }

private:
    MagnifierComponent* const magnifierComp;

    JUCE_DECLARE_NON_COPYABLE (PeerHolderComp);
};


//==============================================================================
MagnifierComponent::MagnifierComponent (Component* const content_,
                                        const bool deleteContentCompWhenNoLongerNeeded)
    : content (content_),
      scaleFactor (0.0),
      peer (0),
      deleteContent (deleteContentCompWhenNoLongerNeeded),
      quality (Graphics::lowResamplingQuality),
      mouseSource (0, true)
{
    holderComp = new PeerHolderComp (this);
    setScaleFactor (1.0);
}

MagnifierComponent::~MagnifierComponent()
{
    delete holderComp;

    if (deleteContent)
        delete content;
}

void MagnifierComponent::setScaleFactor (double newScaleFactor)
{
    jassert (newScaleFactor > 0.0);  // hmm - unlikely to work well with a negative scale factor

    newScaleFactor = jlimit (1.0 / 8.0, 1000.0, newScaleFactor);

    if (scaleFactor != newScaleFactor)
    {
        scaleFactor = newScaleFactor;

        if (scaleFactor == 1.0)
        {
            holderComp->removeFromDesktop();
            peer = 0;
            addChildComponent (content);
            childBoundsChanged (content);
        }
        else
        {
            holderComp->addAndMakeVisible (content);
            holderComp->childBoundsChanged (content);
            childBoundsChanged (holderComp);
            holderComp->addToDesktop (0);
            peer = holderComp->getPeer();
        }

        repaint();
    }
}

void MagnifierComponent::setResamplingQuality (Graphics::ResamplingQuality newQuality)
{
    quality = newQuality;
}

void MagnifierComponent::paint (Graphics& g)
{
    const int w = holderComp->getWidth();
    const int h = holderComp->getHeight();

    if (w == 0 || h == 0)
        return;

    const Rectangle<int> r (g.getClipBounds());

    const int srcX = (int) (r.getX() / scaleFactor);
    const int srcY = (int) (r.getY() / scaleFactor);
    int srcW = roundToInt (r.getRight() / scaleFactor) - srcX;
    int srcH = roundToInt (r.getBottom() / scaleFactor) - srcY;

    if (scaleFactor >= 1.0)
    {
        ++srcW;
        ++srcH;
    }

    Image temp (Image::ARGB, jmax (w, srcX + srcW), jmax (h, srcY + srcH), false);
    const Rectangle<int> area (srcX, srcY, srcW, srcH);
    temp.clear (area);

    {
        Graphics g2 (temp);
        g2.reduceClipRegion (area);
        holderComp->paintEntireComponent (g2, false);
    }

    g.setImageResamplingQuality (quality);
    g.drawImageTransformed (temp, AffineTransform::scale ((float) scaleFactor, (float) scaleFactor), false);
}

void MagnifierComponent::childBoundsChanged (Component* c)
{
    if (c != 0)
        setSize (roundToInt (c->getWidth() * scaleFactor),
                 roundToInt (c->getHeight() * scaleFactor));
}

void MagnifierComponent::passOnMouseEventToPeer (const MouseEvent& e)
{
    if (peer != 0)
        mouseSource.handleEvent (peer, Point<int> (scaleInt (e.x), scaleInt (e.y)),
                                 e.eventTime.toMilliseconds(), ModifierKeys::getCurrentModifiers());
}

void MagnifierComponent::mouseDown (const MouseEvent& e)
{
    passOnMouseEventToPeer (e);
}

void MagnifierComponent::mouseUp (const MouseEvent& e)
{
    passOnMouseEventToPeer (e);
}

void MagnifierComponent::mouseDrag (const MouseEvent& e)
{
    passOnMouseEventToPeer (e);
}

void MagnifierComponent::mouseMove (const MouseEvent& e)
{
    passOnMouseEventToPeer (e);
}

void MagnifierComponent::mouseEnter (const MouseEvent& e)
{
    passOnMouseEventToPeer (e);
}

void MagnifierComponent::mouseExit (const MouseEvent& e)
{
    passOnMouseEventToPeer (e);
}

void MagnifierComponent::mouseWheelMove (const MouseEvent& e, float ix, float iy)
{
    if (peer != 0)
        peer->handleMouseWheel (e.source.getIndex(),
                                Point<int> (scaleInt (e.x), scaleInt (e.y)), e.eventTime.toMilliseconds(),
                                ix * 256.0f, iy * 256.0f);
    else
        Component::mouseWheelMove (e, ix, iy);
}

int MagnifierComponent::scaleInt (const int n) const
{
    return roundToInt (n / scaleFactor);
}


END_JUCE_NAMESPACE
