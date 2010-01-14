/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
    void setBounds (int, int, int, int, const bool) {}
    void setMinimised (bool)                        {}
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

    void textInputRequired (int x, int y)
    {
        ComponentPeer* peer = magnifierComp->getPeer();
        if (peer != 0)
            peer->textInputRequired (x, y);
    }

    void getBounds (int& x, int& y, int& w, int& h) const
    {
        x = magnifierComp->getScreenX();
        y = magnifierComp->getScreenY();
        w = component->getWidth();
        h = component->getHeight();
    }

    int getScreenX() const                          { return magnifierComp->getScreenX(); }
    int getScreenY() const                          { return magnifierComp->getScreenY(); }

    void relativePositionToGlobal (int& x, int& y)
    {
        const double zoom = magnifierComp->getScaleFactor();
        x = roundToInt (x * zoom);
        y = roundToInt (y * zoom);

        magnifierComp->relativePositionToGlobal (x, y);
    }

    void globalPositionToRelative (int& x, int& y)
    {
        magnifierComp->globalPositionToRelative (x, y);

        const double zoom = magnifierComp->getScaleFactor();
        x = roundToInt (x / zoom);
        y = roundToInt (y / zoom);
    }

    bool contains (int x, int y, bool) const
    {
        return ((unsigned int) x) < (unsigned int) magnifierComp->getWidth()
                && ((unsigned int) y) < (unsigned int) magnifierComp->getHeight();
    }

    void repaint (int x, int y, int w, int h)
    {
        const double zoom = magnifierComp->getScaleFactor();

        magnifierComp->repaint ((int) (x * zoom),
                                (int) (y * zoom),
                                roundToInt (w * zoom) + 1,
                                roundToInt (h * zoom) + 1);
    }

    void performAnyPendingRepaintsNow()
    {
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    MagnifierComponent* const magnifierComp;

    MagnifyingPeer (const MagnifyingPeer&);
    const MagnifyingPeer& operator= (const MagnifyingPeer&);
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

    PeerHolderComp (const PeerHolderComp&);
    const PeerHolderComp& operator= (const PeerHolderComp&);
};


//==============================================================================
MagnifierComponent::MagnifierComponent (Component* const content_,
                                        const bool deleteContentCompWhenNoLongerNeeded)
    : content (content_),
      scaleFactor (0.0),
      peer (0),
      deleteContent (deleteContentCompWhenNoLongerNeeded),
      quality (Graphics::lowResamplingQuality)
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

    const Rectangle r (g.getClipBounds());

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
    temp.clear (srcX, srcY, srcW, srcH);

    Graphics g2 (temp);
    g2.reduceClipRegion (srcX, srcY, srcW, srcH);
    holderComp->paintEntireComponent (g2);

    g.setImageResamplingQuality (quality);
    g.drawImage (&temp,
                 0, 0, (int) (w * scaleFactor), (int) (h * scaleFactor),
                 0, 0, w, h,
                 false);
}

void MagnifierComponent::childBoundsChanged (Component* c)
{
    if (c != 0)
        setSize (roundToInt (c->getWidth() * scaleFactor),
                 roundToInt (c->getHeight() * scaleFactor));
}

void MagnifierComponent::mouseDown (const MouseEvent& e)
{
    if (peer != 0)
        peer->handleMouseDown (scaleInt (e.x), scaleInt (e.y), e.eventTime.toMilliseconds());
}

void MagnifierComponent::mouseUp (const MouseEvent& e)
{
    if (peer != 0)
        peer->handleMouseUp (e.mods.getRawFlags(), scaleInt (e.x), scaleInt (e.y), e.eventTime.toMilliseconds());
}

void MagnifierComponent::mouseDrag (const MouseEvent& e)
{
    if (peer != 0)
        peer->handleMouseDrag (scaleInt (e.x), scaleInt (e.y), e.eventTime.toMilliseconds());
}

void MagnifierComponent::mouseMove (const MouseEvent& e)
{
    if (peer != 0)
        peer->handleMouseMove (scaleInt (e.x), scaleInt (e.y), e.eventTime.toMilliseconds());
}

void MagnifierComponent::mouseEnter (const MouseEvent& e)
{
    if (peer != 0)
        peer->handleMouseEnter (scaleInt (e.x), scaleInt (e.y), e.eventTime.toMilliseconds());
}

void MagnifierComponent::mouseExit (const MouseEvent& e)
{
    if (peer != 0)
        peer->handleMouseExit (scaleInt (e.x), scaleInt (e.y), e.eventTime.toMilliseconds());
}

void MagnifierComponent::mouseWheelMove (const MouseEvent& e, float ix, float iy)
{
    if (peer != 0)
        peer->handleMouseWheel (roundToInt (ix * 256.0f),
                                roundToInt (iy * 256.0f),
                                e.eventTime.toMilliseconds());
    else
        Component::mouseWheelMove (e, ix, iy);
}

int MagnifierComponent::scaleInt (const int n) const
{
    return roundToInt (n / scaleFactor);
}


END_JUCE_NAMESPACE
