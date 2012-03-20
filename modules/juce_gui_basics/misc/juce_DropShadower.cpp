/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

class ShadowWindow  : public Component
{
public:
    ShadowWindow (Component& owner, const int type_, const Image shadowImageSections [12])
        : topLeft (shadowImageSections [type_ * 3]),
          bottomRight (shadowImageSections [type_ * 3 + 1]),
          filler (shadowImageSections [type_ * 3 + 2]),
          type (type_)
    {
        setInterceptsMouseClicks (false, false);

        if (owner.isOnDesktop())
        {
            setSize (1, 1); // to keep the OS happy by not having zero-size windows
            addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                            | ComponentPeer::windowIsTemporary
                            | ComponentPeer::windowIgnoresKeyPresses);
        }
        else if (owner.getParentComponent() != nullptr)
        {
            owner.getParentComponent()->addChildComponent (this);
        }
    }

    void paint (Graphics& g)
    {
        g.setOpacity (1.0f);

        if (type < 2)
        {
            int imH = jmin (topLeft.getHeight(), getHeight() / 2);
            g.drawImage (topLeft,
                         0, 0, topLeft.getWidth(), imH,
                         0, 0, topLeft.getWidth(), imH);

            imH = jmin (bottomRight.getHeight(), getHeight() - getHeight() / 2);
            g.drawImage (bottomRight,
                         0, getHeight() - imH, bottomRight.getWidth(), imH,
                         0, bottomRight.getHeight() - imH, bottomRight.getWidth(), imH);

            g.setTiledImageFill (filler, 0, 0, 1.0f);
            g.fillRect (0, topLeft.getHeight(), getWidth(), getHeight() - (topLeft.getHeight() + bottomRight.getHeight()));
        }
        else
        {
            int imW = jmin (topLeft.getWidth(), getWidth() / 2);
            g.drawImage (topLeft,
                         0, 0, imW, topLeft.getHeight(),
                         0, 0, imW, topLeft.getHeight());

            imW = jmin (bottomRight.getWidth(), getWidth() - getWidth() / 2);
            g.drawImage (bottomRight,
                         getWidth() - imW, 0, imW, bottomRight.getHeight(),
                         bottomRight.getWidth() - imW, 0, imW, bottomRight.getHeight());

            g.setTiledImageFill (filler, 0, 0, 1.0f);
            g.fillRect (topLeft.getWidth(), 0, getWidth() - (topLeft.getWidth() + bottomRight.getWidth()), getHeight());
        }
    }

    void resized()
    {
        repaint();  // (needed for correct repainting)
    }

private:
    const Image topLeft, bottomRight, filler;
    const int type;   // 0 = left, 1 = right, 2 = top, 3 = bottom. left + right are full-height

    JUCE_DECLARE_NON_COPYABLE (ShadowWindow);
};


//==============================================================================
DropShadower::DropShadower (const float alpha_,
                            const int xOffset_,
                            const int yOffset_,
                            const float blurRadius_)
   : owner (nullptr),
     xOffset (xOffset_),
     yOffset (yOffset_),
     alpha (alpha_),
     blurRadius (blurRadius_),
     reentrant (false)
{
}

DropShadower::~DropShadower()
{
    if (owner != nullptr)
        owner->removeComponentListener (this);

    reentrant = true;
    shadowWindows.clear();
}

void DropShadower::setOwner (Component* componentToFollow)
{
    if (componentToFollow != owner)
    {
        if (owner != nullptr)
            owner->removeComponentListener (this);

        // (the component can't be null)
        jassert (componentToFollow != nullptr);

        owner = componentToFollow;

        jassert (owner != nullptr);
        jassert (owner->isOpaque()); // doesn't work properly for semi-transparent comps!

        owner->addComponentListener (this);

        updateShadows();
    }
}

void DropShadower::componentMovedOrResized (Component&, bool /*wasMoved*/, bool /*wasResized*/)
{
    updateShadows();
}

void DropShadower::componentBroughtToFront (Component&)
{
    bringShadowWindowsToFront();
}

void DropShadower::componentParentHierarchyChanged (Component&)
{
    shadowWindows.clear();
    updateShadows();
}

void DropShadower::componentVisibilityChanged (Component&)
{
    updateShadows();
}

void DropShadower::updateShadows()
{
    if (reentrant || owner == nullptr)
        return;

    ComponentPeer* const peer = owner->getPeer();
    const bool isOwnerVisible = owner->isVisible() && (peer == nullptr || ! peer->isMinimised());

    const bool createShadowWindows  = shadowWindows.size() == 0
                                         && owner->getWidth() > 0
                                         && owner->getHeight() > 0
                                         && isOwnerVisible
                                         && (Desktop::canUseSemiTransparentWindows()
                                              || owner->getParentComponent() != nullptr);

    {
        const ScopedValueSetter<bool> setter (reentrant, true, false);

        const int shadowEdge = jmax (xOffset, yOffset) + (int) blurRadius;

        if (createShadowWindows)
        {
            // keep a cached version of the image to save doing the gaussian too often
            String imageId;
            imageId << shadowEdge << ',' << xOffset << ',' << yOffset << ',' << alpha;

            const int hash = imageId.hashCode();

            Image bigIm (ImageCache::getFromHashCode (hash));

            if (bigIm.isNull())
            {
                bigIm = Image (Image::ARGB, shadowEdge * 5, shadowEdge * 5, true);

                Graphics bigG (bigIm);
                bigG.setColour (Colours::black.withAlpha (alpha));
                bigG.fillRect (shadowEdge + xOffset,
                               shadowEdge + yOffset,
                               bigIm.getWidth() - (shadowEdge * 2),
                               bigIm.getHeight() - (shadowEdge * 2));

                ImageConvolutionKernel blurKernel (roundToInt (blurRadius * 2.0f));
                blurKernel.createGaussianBlur (blurRadius);

                blurKernel.applyToImage (bigIm, bigIm,
                                         Rectangle<int> (xOffset, yOffset,
                                                         bigIm.getWidth(), bigIm.getHeight()));

                ImageCache::addImageToCache (bigIm, hash);
            }

            const int iw = bigIm.getWidth();
            const int ih = bigIm.getHeight();
            const int shadowEdge2 = shadowEdge * 2;

            setShadowImage (bigIm, 0, shadowEdge, shadowEdge2, 0, 0);
            setShadowImage (bigIm, 1, shadowEdge, shadowEdge2, 0, ih - shadowEdge2);
            setShadowImage (bigIm, 2, shadowEdge, shadowEdge, 0, shadowEdge2);
            setShadowImage (bigIm, 3, shadowEdge, shadowEdge2, iw - shadowEdge, 0);
            setShadowImage (bigIm, 4, shadowEdge, shadowEdge2, iw - shadowEdge, ih - shadowEdge2);
            setShadowImage (bigIm, 5, shadowEdge, shadowEdge, iw - shadowEdge, shadowEdge2);
            setShadowImage (bigIm, 6, shadowEdge, shadowEdge, shadowEdge, 0);
            setShadowImage (bigIm, 7, shadowEdge, shadowEdge, iw - shadowEdge2, 0);
            setShadowImage (bigIm, 8, shadowEdge, shadowEdge, shadowEdge2, 0);
            setShadowImage (bigIm, 9, shadowEdge, shadowEdge, shadowEdge, ih - shadowEdge);
            setShadowImage (bigIm, 10, shadowEdge, shadowEdge, iw - shadowEdge2, ih - shadowEdge);
            setShadowImage (bigIm, 11, shadowEdge, shadowEdge, shadowEdge2, ih - shadowEdge);

            for (int i = 0; i < 4; ++i)
                shadowWindows.add (new ShadowWindow (*owner, i, shadowImageSections));
        }

        if (shadowWindows.size() >= 4)
        {
            const int x = owner->getX();
            const int y = owner->getY() - shadowEdge;
            const int w = owner->getWidth();
            const int h = owner->getHeight() + shadowEdge + shadowEdge;

            for (int i = shadowWindows.size(); --i >= 0;)
            {
                // there seem to be rare situations where the dropshadower may be deleted by
                // callbacks during this loop, so use a weak ref to watch out for this..
                WeakReference<Component> sw (shadowWindows[i]);

                if (sw != nullptr)
                    sw->setAlwaysOnTop (owner->isAlwaysOnTop());

                if (sw != nullptr)
                    sw->setVisible (isOwnerVisible);

                if (sw != nullptr)
                {
                    switch (i)
                    {
                        case 0: sw->setBounds (x - shadowEdge, y, shadowEdge, h); break;
                        case 1: sw->setBounds (x + w, y, shadowEdge, h); break;
                        case 2: sw->setBounds (x, y, w, shadowEdge); break;
                        case 3: sw->setBounds (x, owner->getBottom(), w, shadowEdge); break;
                        default: break;
                    }
                }

                if (sw == nullptr)
                    return;
            }
        }
    }

    if (createShadowWindows)
        bringShadowWindowsToFront();
}

void DropShadower::setShadowImage (const Image& src, const int num, const int w, const int h,
                                   const int sx, const int sy)
{
    shadowImageSections[num] = Image (Image::ARGB, w, h, true);

    Graphics g (shadowImageSections[num]);
    g.drawImage (src, 0, 0, w, h, sx, sy, w, h);
}

void DropShadower::bringShadowWindowsToFront()
{
    if (! reentrant)
    {
        updateShadows();

        const ScopedValueSetter<bool> setter (reentrant, true, false);

        for (int i = shadowWindows.size(); --i >= 0;)
            shadowWindows.getUnchecked(i)->toBehind (owner);
    }
}
