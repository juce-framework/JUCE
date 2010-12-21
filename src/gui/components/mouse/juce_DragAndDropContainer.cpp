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

#include "juce_DragAndDropContainer.h"
#include "../windows/juce_ComponentPeer.h"
#include "../juce_Desktop.h"
#include "../../../events/juce_Timer.h"
#include "../../../maths/juce_Random.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../mouse/juce_MouseEvent.h"
#include "../mouse/juce_MouseInputSource.h"
#include "juce_FileDragAndDropTarget.h"

bool juce_performDragDropFiles (const StringArray& files, const bool copyFiles, bool& shouldStop);
bool juce_performDragDropText (const String& text, bool& shouldStop);


//==============================================================================
class DragImageComponent  : public Component,
                            public Timer
{
public:
    DragImageComponent (const Image& im,
                        const String& desc,
                        Component* const sourceComponent,
                        Component* const mouseDragSource_,
                        DragAndDropContainer* const o,
                        const Point<int>& imageOffset_)
        : image (im),
          source (sourceComponent),
          mouseDragSource (mouseDragSource_),
          owner (o),
          dragDesc (desc),
          imageOffset (imageOffset_),
          hasCheckedForExternalDrag (false),
          drawImage (true)
    {
        setSize (im.getWidth(), im.getHeight());

        if (mouseDragSource == 0)
            mouseDragSource = source;

        mouseDragSource->addMouseListener (this, false);

        startTimer (200);

        setInterceptsMouseClicks (false, false);
        setAlwaysOnTop (true);
    }

    ~DragImageComponent()
    {
        if (owner->dragImageComponent == this)
            owner->dragImageComponent.release();

        if (mouseDragSource != 0)
        {
            mouseDragSource->removeMouseListener (this);

            if (getCurrentlyOver() != 0 && getCurrentlyOver()->isInterestedInDragSource (dragDesc, source))
                getCurrentlyOver()->itemDragExit (dragDesc, source);
        }
    }

    void paint (Graphics& g)
    {
        if (isOpaque())
            g.fillAll (Colours::white);

        if (drawImage)
        {
            g.setOpacity (1.0f);
            g.drawImageAt (image, 0, 0);
        }
    }

    DragAndDropTarget* findTarget (const Point<int>& screenPos, Point<int>& relativePos)
    {
        Component* hit = getParentComponent();

        if (hit == 0)
        {
            hit = Desktop::getInstance().findComponentAt (screenPos);
        }
        else
        {
            const Point<int> relPos (hit->getLocalPoint (0, screenPos));
            hit = hit->getComponentAt (relPos.getX(), relPos.getY());
        }

        // (note: use a local copy of the dragDesc member in case the callback runs
        // a modal loop and deletes this object before the method completes)
        const String dragDescLocal (dragDesc);

        while (hit != 0)
        {
            DragAndDropTarget* const ddt = dynamic_cast <DragAndDropTarget*> (hit);

            if (ddt != 0 && ddt->isInterestedInDragSource (dragDescLocal, source))
            {
                relativePos = hit->getLocalPoint (0, screenPos);
                return ddt;
            }

            hit = hit->getParentComponent();
        }

        return 0;
    }

    void mouseUp (const MouseEvent& e)
    {
        if (e.originalComponent != this)
        {
            if (mouseDragSource != 0)
                mouseDragSource->removeMouseListener (this);

            bool dropAccepted = false;
            DragAndDropTarget* ddt = 0;
            Point<int> relPos;

            if (isVisible())
            {
                setVisible (false);
                ddt = findTarget (e.getScreenPosition(), relPos);

                // fade this component and remove it - it'll be deleted later by the timer callback

                dropAccepted = ddt != 0;

                setVisible (true);

                if (dropAccepted || source == 0)
                {
                    Desktop::getInstance().getAnimator().fadeOut (this, 120);
                }
                else
                {
                    const Point<int> target (source->localPointToGlobal (source->getLocalBounds().getCentre()));
                    const Point<int> ourCentre (localPointToGlobal (getLocalBounds().getCentre()));

                    Desktop::getInstance().getAnimator().animateComponent (this,
                                                                           getBounds() + (target - ourCentre),
                                                                           0.0f, 120,
                                                                           true, 1.0, 1.0);
                }
            }

            if (getParentComponent() != 0)
                getParentComponent()->removeChildComponent (this);

            if (dropAccepted && ddt != 0)
            {
                // (note: use a local copy of the dragDesc member in case the callback runs
                // a modal loop and deletes this object before the method completes)
                const String dragDescLocal (dragDesc);

                currentlyOverComp = 0;

                ddt->itemDropped (dragDescLocal, source, relPos.getX(), relPos.getY());
            }

            // careful - this object could now be deleted..
        }
    }

    void updateLocation (const bool canDoExternalDrag, const Point<int>& screenPos)
    {
        // (note: use a local copy of the dragDesc member in case the callback runs
        // a modal loop and deletes this object before it returns)
        const String dragDescLocal (dragDesc);

        Point<int> newPos (screenPos + imageOffset);

        if (getParentComponent() != 0)
            newPos = getParentComponent()->getLocalPoint (0, newPos);

        //if (newX != getX() || newY != getY())
        {
            setTopLeftPosition (newPos.getX(), newPos.getY());

            Point<int> relPos;
            DragAndDropTarget* const ddt = findTarget (screenPos, relPos);
            Component* ddtComp = dynamic_cast <Component*> (ddt);

            drawImage = (ddt == 0) || ddt->shouldDrawDragImageWhenOver();

            if (ddtComp != currentlyOverComp)
            {
                if (currentlyOverComp != 0 && source != 0
                      && getCurrentlyOver()->isInterestedInDragSource (dragDescLocal, source))
                {
                    getCurrentlyOver()->itemDragExit (dragDescLocal, source);
                }

                currentlyOverComp = ddtComp;

                if (ddt != 0 && ddt->isInterestedInDragSource (dragDescLocal, source))
                    ddt->itemDragEnter (dragDescLocal, source, relPos.getX(), relPos.getY());
            }

            DragAndDropTarget* target = getCurrentlyOver();
            if (target != 0 && target->isInterestedInDragSource (dragDescLocal, source))
                target->itemDragMove (dragDescLocal, source, relPos.getX(), relPos.getY());

            if (getCurrentlyOver() == 0 && canDoExternalDrag && ! hasCheckedForExternalDrag)
            {
                if (Desktop::getInstance().findComponentAt (screenPos) == 0)
                {
                    hasCheckedForExternalDrag = true;
                    StringArray files;
                    bool canMoveFiles = false;

                    if (owner->shouldDropFilesWhenDraggedExternally (dragDescLocal, source, files, canMoveFiles)
                         && files.size() > 0)
                    {
                        WeakReference<Component> cdw (this);
                        setVisible (false);

                        if (ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())
                            DragAndDropContainer::performExternalDragDropOfFiles (files, canMoveFiles);

                        if (cdw != 0)
                            delete this;

                        return;
                    }
                }
            }
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (e.originalComponent != this)
            updateLocation (true, e.getScreenPosition());
    }

    void timerCallback()
    {
        if (source == 0)
        {
            delete this;
        }
        else if (! isMouseButtonDownAnywhere())
        {
            if (mouseDragSource != 0)
                mouseDragSource->removeMouseListener (this);

            delete this;
        }
    }

private:
    Image image;
    WeakReference<Component> source;
    WeakReference<Component> mouseDragSource;
    DragAndDropContainer* const owner;

    WeakReference<Component> currentlyOverComp;
    DragAndDropTarget* getCurrentlyOver()
    {
        return dynamic_cast <DragAndDropTarget*> (static_cast <Component*> (currentlyOverComp));
    }

    String dragDesc;
    const Point<int> imageOffset;
    bool hasCheckedForExternalDrag, drawImage;

    JUCE_DECLARE_NON_COPYABLE (DragImageComponent);
};


//==============================================================================
DragAndDropContainer::DragAndDropContainer()
{
}

DragAndDropContainer::~DragAndDropContainer()
{
    dragImageComponent = 0;
}

void DragAndDropContainer::startDragging (const String& sourceDescription,
                                          Component* sourceComponent,
                                          const Image& dragImage_,
                                          const bool allowDraggingToExternalWindows,
                                          const Point<int>* imageOffsetFromMouse)
{
    Image dragImage (dragImage_);

    if (dragImageComponent == 0)
    {
        Component* const thisComp = dynamic_cast <Component*> (this);

        if (thisComp == 0)
        {
            jassertfalse;   // Your DragAndDropContainer needs to be a Component!
            return;
        }

        MouseInputSource* draggingSource = Desktop::getInstance().getDraggingMouseSource (0);

        if (draggingSource == 0 || ! draggingSource->isDragging())
        {
            jassertfalse;   // You must call startDragging() from within a mouseDown or mouseDrag callback!
            return;
        }

        const Point<int> lastMouseDown (Desktop::getLastMouseDownPosition());
        Point<int> imageOffset;

        if (dragImage.isNull())
        {
            dragImage = sourceComponent->createComponentSnapshot (sourceComponent->getLocalBounds())
                            .convertedToFormat (Image::ARGB);

            dragImage.multiplyAllAlphas (0.6f);

            const int lo = 150;
            const int hi = 400;

            Point<int> relPos (sourceComponent->getLocalPoint (0, lastMouseDown));
            Point<int> clipped (dragImage.getBounds().getConstrainedPoint (relPos));

            for (int y = dragImage.getHeight(); --y >= 0;)
            {
                const double dy = (y - clipped.getY()) * (y - clipped.getY());

                for (int x = dragImage.getWidth(); --x >= 0;)
                {
                    const int dx = x - clipped.getX();
                    const int distance = roundToInt (std::sqrt (dx * dx + dy));

                    if (distance > lo)
                    {
                        const float alpha = (distance > hi) ? 0
                                                            : (hi - distance) / (float) (hi - lo)
                                                               + Random::getSystemRandom().nextFloat() * 0.008f;

                        dragImage.multiplyAlphaAt (x, y, alpha);
                    }
                }
            }

            imageOffset = -clipped;
        }
        else
        {
            if (imageOffsetFromMouse == 0)
                imageOffset = -dragImage.getBounds().getCentre();
            else
                imageOffset = -(dragImage.getBounds().getConstrainedPoint (-*imageOffsetFromMouse));
        }

        dragImageComponent = new DragImageComponent (dragImage, sourceDescription, sourceComponent,
                                                     draggingSource->getComponentUnderMouse(), this, imageOffset);

        currentDragDesc = sourceDescription;

        if (allowDraggingToExternalWindows)
        {
            if (! Desktop::canUseSemiTransparentWindows())
                dragImageComponent->setOpaque (true);

            dragImageComponent->addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                                               | ComponentPeer::windowIsTemporary
                                               | ComponentPeer::windowIgnoresKeyPresses);
        }
        else
            thisComp->addChildComponent (dragImageComponent);

        static_cast <DragImageComponent*> (static_cast <Component*> (dragImageComponent))->updateLocation (false, lastMouseDown);
        dragImageComponent->setVisible (true);
    }
}

bool DragAndDropContainer::isDragAndDropActive() const
{
    return dragImageComponent != 0;
}

const String DragAndDropContainer::getCurrentDragDescription() const
{
    return (dragImageComponent != 0) ? currentDragDesc
                                     : String::empty;
}

DragAndDropContainer* DragAndDropContainer::findParentDragContainerFor (Component* c)
{
    return c == 0 ? 0 : c->findParentComponentOfClass ((DragAndDropContainer*) 0);
}

bool DragAndDropContainer::shouldDropFilesWhenDraggedExternally (const String&, Component*, StringArray&, bool&)
{
    return false;
}


//==============================================================================
void DragAndDropTarget::itemDragEnter (const String&, Component*, int, int)
{
}

void DragAndDropTarget::itemDragMove (const String&, Component*, int, int)
{
}

void DragAndDropTarget::itemDragExit (const String&, Component*)
{
}

bool DragAndDropTarget::shouldDrawDragImageWhenOver()
{
    return true;
}


//==============================================================================
void FileDragAndDropTarget::fileDragEnter (const StringArray&, int, int)
{
}

void FileDragAndDropTarget::fileDragMove (const StringArray&, int, int)
{
}

void FileDragAndDropTarget::fileDragExit (const StringArray&)
{
}


END_JUCE_NAMESPACE
