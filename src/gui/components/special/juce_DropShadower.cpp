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

#include "juce_DropShadower.h"
#include "../../graphics/imaging/juce_ImageCache.h"
#include "../../graphics/imaging/juce_ImageConvolutionKernel.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../juce_ComponentDeletionWatcher.h"
#include "../juce_Desktop.h"


//==============================================================================
class ShadowWindow  : public Component
{
    Component* owner;
    Image** shadowImageSections;
    const int type;   // 0 = left, 1 = right, 2 = top, 3 = bottom. left + right are full-height

public:
    ShadowWindow (Component* const owner_,
                  const int type_,
                  Image** const shadowImageSections_)
        : owner (owner_),
          shadowImageSections (shadowImageSections_),
          type (type_)
    {
        setInterceptsMouseClicks (false, false);

        if (owner_->isOnDesktop())
        {
            setSize (1, 1); // to keep the OS happy by not having zero-size windows
            addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                            | ComponentPeer::windowIsTemporary
                            | ComponentPeer::windowIgnoresKeyPresses);
        }
        else if (owner_->getParentComponent() != 0)
        {
            owner_->getParentComponent()->addChildComponent (this);
        }
    }

    ~ShadowWindow()
    {
    }

    void paint (Graphics& g)
    {
        Image* const topLeft      = shadowImageSections [type * 3];
        Image* const bottomRight  = shadowImageSections [type * 3 + 1];
        Image* const filler       = shadowImageSections [type * 3 + 2];

        g.setOpacity (1.0f);

        if (type < 2)
        {
            int imH = jmin (topLeft->getHeight(), getHeight() / 2);
            g.drawImage (topLeft,
                         0, 0, topLeft->getWidth(), imH,
                         0, 0, topLeft->getWidth(), imH);

            imH = jmin (bottomRight->getHeight(), getHeight() - getHeight() / 2);
            g.drawImage (bottomRight,
                         0, getHeight() - imH, bottomRight->getWidth(), imH,
                         0, bottomRight->getHeight() - imH, bottomRight->getWidth(), imH);

            g.setTiledImageFill (*filler, 0, 0, 1.0f);
            g.fillRect (0, topLeft->getHeight(), getWidth(), getHeight() - (topLeft->getHeight() + bottomRight->getHeight()));
        }
        else
        {
            int imW = jmin (topLeft->getWidth(), getWidth() / 2);
            g.drawImage (topLeft,
                         0, 0, imW, topLeft->getHeight(),
                         0, 0, imW, topLeft->getHeight());

            imW = jmin (bottomRight->getWidth(), getWidth() - getWidth() / 2);
            g.drawImage (bottomRight,
                         getWidth() - imW, 0, imW, bottomRight->getHeight(),
                         bottomRight->getWidth() - imW, 0, imW, bottomRight->getHeight());

            g.setTiledImageFill (*filler, 0, 0, 1.0f);
            g.fillRect (topLeft->getWidth(), 0, getWidth() - (topLeft->getWidth() + bottomRight->getWidth()), getHeight());
        }
    }

    void resized()
    {
        repaint();  // (needed for correct repainting)
    }

private:
    ShadowWindow (const ShadowWindow&);
    const ShadowWindow& operator= (const ShadowWindow&);
};


//==============================================================================
DropShadower::DropShadower (const float alpha_,
                            const int xOffset_,
                            const int yOffset_,
                            const float blurRadius_)
   : owner (0),
     numShadows (0),
     shadowEdge (jmax (xOffset_, yOffset_) + (int) blurRadius_),
     xOffset (xOffset_),
     yOffset (yOffset_),
     alpha (alpha_),
     blurRadius (blurRadius_),
     inDestructor (false),
     reentrant (false)
{
}

DropShadower::~DropShadower()
{
    if (owner != 0)
        owner->removeComponentListener (this);

    inDestructor = true;

    deleteShadowWindows();
}

void DropShadower::deleteShadowWindows()
{
    if (numShadows > 0)
    {
        int i;
        for (i = numShadows; --i >= 0;)
            delete shadowWindows[i];

        for (i = 12; --i >= 0;)
            delete shadowImageSections[i];

        numShadows = 0;
    }
}

void DropShadower::setOwner (Component* componentToFollow)
{
    if (componentToFollow != owner)
    {
        if (owner != 0)
            owner->removeComponentListener (this);

        // (the component can't be null)
        jassert (componentToFollow != 0);

        owner = componentToFollow;

        jassert (owner != 0);
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

void DropShadower::componentChildrenChanged (Component&)
{
}

void DropShadower::componentParentHierarchyChanged (Component&)
{
    deleteShadowWindows();
    updateShadows();
}

void DropShadower::componentVisibilityChanged (Component&)
{
    updateShadows();
}

void DropShadower::updateShadows()
{
    if (reentrant || inDestructor || (owner == 0))
        return;

    reentrant = true;

    ComponentPeer* const nw = owner->getPeer();

    const bool isOwnerVisible = owner->isVisible()
                                 && (nw == 0 || ! nw->isMinimised());

    const bool createShadowWindows  = numShadows == 0
                                         && owner->getWidth() > 0
                                         && owner->getHeight() > 0
                                         && isOwnerVisible
                                         && (Desktop::canUseSemiTransparentWindows()
                                              || owner->getParentComponent() != 0);

    if (createShadowWindows)
    {
        // keep a cached version of the image to save doing the gaussian too often
        String imageId;
        imageId << shadowEdge << T(',')
                << xOffset << T(',')
                << yOffset << T(',')
                << alpha;

        const int hash = imageId.hashCode();

        Image* bigIm = ImageCache::getFromHashCode (hash);

        if (bigIm == 0)
        {
            bigIm = Image::createNativeImage (Image::ARGB, shadowEdge * 5, shadowEdge * 5, true);

            Graphics bigG (*bigIm);
            bigG.setColour (Colours::black.withAlpha (alpha));
            bigG.fillRect (shadowEdge + xOffset,
                           shadowEdge + yOffset,
                           bigIm->getWidth() - (shadowEdge * 2),
                           bigIm->getHeight() - (shadowEdge * 2));

            ImageConvolutionKernel blurKernel (roundToInt (blurRadius * 2.0f));
            blurKernel.createGaussianBlur (blurRadius);

            blurKernel.applyToImage (*bigIm, 0,
                                     xOffset,
                                     yOffset,
                                     bigIm->getWidth(),
                                     bigIm->getHeight());

            ImageCache::addImageToCache (bigIm, hash);
        }

        const int iw = bigIm->getWidth();
        const int ih = bigIm->getHeight();
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

        ImageCache::release (bigIm);

        for (int i = 0; i < 4; ++i)
        {
            shadowWindows[numShadows] = new ShadowWindow (owner, i, shadowImageSections);
            ++numShadows;
        }
    }

    if (numShadows > 0)
    {
        for (int i = numShadows; --i >= 0;)
        {
            shadowWindows[i]->setAlwaysOnTop (owner->isAlwaysOnTop());
            shadowWindows[i]->setVisible (isOwnerVisible);
        }

        const int x = owner->getX();
        const int y = owner->getY() - shadowEdge;
        const int w = owner->getWidth();
        const int h = owner->getHeight() + shadowEdge + shadowEdge;

        shadowWindows[0]->setBounds (x - shadowEdge,
                                     y,
                                     shadowEdge,
                                     h);

        shadowWindows[1]->setBounds (x + w,
                                     y,
                                     shadowEdge,
                                     h);

        shadowWindows[2]->setBounds (x,
                                     y,
                                     w,
                                     shadowEdge);

        shadowWindows[3]->setBounds (x,
                                     owner->getBottom(),
                                     w,
                                     shadowEdge);
    }

    reentrant = false;

    if (createShadowWindows)
        bringShadowWindowsToFront();
}

void DropShadower::setShadowImage (Image* const src,
                                   const int num,
                                   const int w,
                                   const int h,
                                   const int sx,
                                   const int sy)
{
    shadowImageSections[num] = new Image (Image::ARGB, w, h, true);

    Graphics g (*shadowImageSections[num]);
    g.drawImage (src, 0, 0, w, h, sx, sy, w, h);
}

void DropShadower::bringShadowWindowsToFront()
{
    if (! (inDestructor || reentrant))
    {
        updateShadows();

        reentrant = true;

        for (int i = numShadows; --i >= 0;)
            shadowWindows[i]->toBehind (owner);

        reentrant = false;
    }
}

END_JUCE_NAMESPACE
