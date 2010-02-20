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

#include "juce_DragAndDropContainer.h"
#include "../juce_ComponentDeletionWatcher.h"
#include "../juce_Desktop.h"
#include "../../../events/juce_Timer.h"
#include "../../../core/juce_Random.h"
#include "../../graphics/imaging/juce_Image.h"
#include "juce_FileDragAndDropTarget.h"

bool juce_performDragDropFiles (const StringArray& files, const bool copyFiles, bool& shouldStop);
bool juce_performDragDropText (const String& text, bool& shouldStop);


//==============================================================================
class DragImageComponent  : public Component,
                            public Timer
{
public:
    DragImageComponent (Image* const im,
                        const String& desc,
                        Component* const s,
                        DragAndDropContainer* const o,
                        const Point<int>& imageOffset_)
        : image (im),
          source (s),
          owner (o),
          currentlyOver (0),
          dragDesc (desc),
          imageOffset (imageOffset_),
          hasCheckedForExternalDrag (false),
          drawImage (true)
    {
        setSize (im->getWidth(), im->getHeight());

        sourceWatcher = new ComponentDeletionWatcher (source);

        mouseDragSource = Component::getComponentUnderMouse();

        if (mouseDragSource == 0)
            mouseDragSource = source;

        mouseDragSourceWatcher = new ComponentDeletionWatcher (mouseDragSource);
        mouseDragSource->addMouseListener (this, false);

        startTimer (200);

        setInterceptsMouseClicks (false, false);
        setAlwaysOnTop (true);
    }

    ~DragImageComponent()
    {
        if ((DragImageComponent*) owner->dragImageComponent == this)
            owner->dragImageComponent.release();

        if (! mouseDragSourceWatcher->hasBeenDeleted())
        {
            mouseDragSource->removeMouseListener (this);

            if (currentlyOverWatcher != 0 && ! currentlyOverWatcher->hasBeenDeleted())
                if (currentlyOver->isInterestedInDragSource (dragDesc, source))
                    currentlyOver->itemDragExit (dragDesc, source);
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

    DragAndDropTarget* findTarget (const Point<int>& screenPos,
                                   Point<int>& relativePos) const
    {
        Component* hit = getParentComponent();

        if (hit == 0)
        {
            hit = Desktop::getInstance().findComponentAt (screenPos);
        }
        else
        {
            const Point<int> relPos (hit->globalPositionToRelative (screenPos));
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
                relativePos = hit->globalPositionToRelative (screenPos);
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
            if (! mouseDragSourceWatcher->hasBeenDeleted())
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

                if (dropAccepted || sourceWatcher->hasBeenDeleted())
                {
                    fadeOutComponent (120);
                }
                else
                {
                    const Point<int> target (source->relativePositionToGlobal (Point<int> (source->getWidth() / 2,
                                                                                           source->getHeight() / 2)));

                    const Point<int> ourCentre (relativePositionToGlobal (Point<int> (getWidth() / 2,
                                                                                      getHeight() / 2)));

                    fadeOutComponent (120,
                                      target.getX() - ourCentre.getX(),
                                      target.getY() - ourCentre.getY());
                }
            }

            if (getParentComponent() != 0)
                getParentComponent()->removeChildComponent (this);

            if (dropAccepted && ddt != 0)
            {
                // (note: use a local copy of the dragDesc member in case the callback runs
                // a modal loop and deletes this object before the method completes)
                const String dragDescLocal (dragDesc);

                currentlyOverWatcher = 0;
                currentlyOver = 0;

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
            newPos = getParentComponent()->globalPositionToRelative (newPos);

        //if (newX != getX() || newY != getY())
        {
            setTopLeftPosition (newPos.getX(), newPos.getY());

            Point<int> relPos;
            DragAndDropTarget* const ddt = findTarget (screenPos, relPos);

            drawImage = (ddt == 0) || ddt->shouldDrawDragImageWhenOver();

            if (ddt != currentlyOver)
            {
                if (currentlyOverWatcher != 0 && ! currentlyOverWatcher->hasBeenDeleted())
                {
                    Component* const over = dynamic_cast <Component*> (currentlyOver);

                    if (over != 0
                         && over->isValidComponent()
                         && ! (sourceWatcher->hasBeenDeleted())
                         && currentlyOver->isInterestedInDragSource (dragDescLocal, source))
                    {
                        currentlyOver->itemDragExit (dragDescLocal, source);
                    }
                }

                currentlyOver = ddt;
                currentlyOverWatcher = 0;

                if (ddt != 0)
                {
                    currentlyOverWatcher = new ComponentDeletionWatcher (dynamic_cast <Component*> (ddt));

                    if (currentlyOver->isInterestedInDragSource (dragDescLocal, source))
                        currentlyOver->itemDragEnter (dragDescLocal, source, relPos.getX(), relPos.getY());
                }
            }
            else if (currentlyOverWatcher != 0 && currentlyOverWatcher->hasBeenDeleted())
            {
                currentlyOver = 0;
                currentlyOverWatcher = 0;
            }

            if (currentlyOver != 0
                 && currentlyOver->isInterestedInDragSource (dragDescLocal, source))
                currentlyOver->itemDragMove (dragDescLocal, source, relPos.getX(), relPos.getY());

            if (currentlyOver == 0
                 && canDoExternalDrag
                 && ! hasCheckedForExternalDrag)
            {
                if (Desktop::getInstance().findComponentAt (screenPos) == 0)
                {
                    hasCheckedForExternalDrag = true;
                    StringArray files;
                    bool canMoveFiles = false;

                    if (owner->shouldDropFilesWhenDraggedExternally (dragDescLocal, source, files, canMoveFiles)
                         && files.size() > 0)
                    {
                        ComponentDeletionWatcher cdw (this);
                        setVisible (false);

                        if (ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())
                            DragAndDropContainer::performExternalDragDropOfFiles (files, canMoveFiles);

                        if (! cdw.hasBeenDeleted())
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
        if (sourceWatcher->hasBeenDeleted())
        {
            delete this;
        }
        else if (! isMouseButtonDownAnywhere())
        {
            if (! mouseDragSourceWatcher->hasBeenDeleted())
                mouseDragSource->removeMouseListener (this);

            delete this;
        }
    }

private:
    ScopedPointer<Image> image;
    Component* const source;
    DragAndDropContainer* const owner;

    ScopedPointer<ComponentDeletionWatcher> sourceWatcher, mouseDragSourceWatcher, currentlyOverWatcher;
    Component* mouseDragSource;
    DragAndDropTarget* currentlyOver;

    String dragDesc;
    const Point<int> imageOffset;
    bool hasCheckedForExternalDrag, drawImage;

    DragImageComponent (const DragImageComponent&);
    DragImageComponent& operator= (const DragImageComponent&);
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
                                          Image* dragImage_,
                                          const bool allowDraggingToExternalWindows,
                                          const Point<int>* imageOffsetFromMouse)
{
    ScopedPointer <Image> dragImage (dragImage_);

    if (dragImageComponent == 0)
    {
        Component* const thisComp = dynamic_cast <Component*> (this);

        if (thisComp != 0)
        {
            const Point<int> lastMouseDown (Desktop::getLastMouseDownPosition());
            Point<int> imageOffset;

            if (dragImage == 0)
            {
                dragImage = sourceComponent->createComponentSnapshot (Rectangle<int> (0, 0, sourceComponent->getWidth(), sourceComponent->getHeight()));

                if (dragImage->getFormat() != Image::ARGB)
                {
                    Image* newIm = Image::createNativeImage (Image::ARGB, dragImage->getWidth(), dragImage->getHeight(), true);
                    Graphics g2 (*newIm);
                    g2.drawImageAt (dragImage, 0, 0);

                    dragImage = newIm;
                }

                dragImage->multiplyAllAlphas (0.6f);

                const int lo = 150;
                const int hi = 400;

                Point<int> relPos (sourceComponent->globalPositionToRelative (lastMouseDown));
                Point<int> clipped (Rectangle<int> (0, 0, dragImage->getWidth(), dragImage->getHeight())
                                        .getConstrainedPoint (relPos));

                for (int y = dragImage->getHeight(); --y >= 0;)
                {
                    const double dy = (y - clipped.getY()) * (y - clipped.getY());

                    for (int x = dragImage->getWidth(); --x >= 0;)
                    {
                        const int dx = x - clipped.getX();
                        const int distance = roundToInt (sqrt (dx * dx + dy));

                        if (distance > lo)
                        {
                            const float alpha = (distance > hi) ? 0
                                                                : (hi - distance) / (float) (hi - lo)
                                                                   + Random::getSystemRandom().nextFloat() * 0.008f;

                            dragImage->multiplyAlphaAt (x, y, alpha);
                        }
                    }
                }

                imageOffset = Point<int>() - clipped;
            }
            else
            {
                if (imageOffsetFromMouse == 0)
                    imageOffset = Point<int> (dragImage->getWidth() / -2,
                                              dragImage->getHeight() / -2);
                else
                    imageOffset = *imageOffsetFromMouse;
            }

            dragImageComponent = new DragImageComponent (dragImage.release(), sourceDescription, sourceComponent,
                                                         this, imageOffset);

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

            ((DragImageComponent*) dragImageComponent)->updateLocation (false, lastMouseDown);
            dragImageComponent->setVisible (true);
        }
        else
        {
            // this class must only be implemented by an object that
            // is also a Component.
            jassertfalse
        }
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
    if (c == 0)
        return 0;

    // (unable to use the syntax findParentComponentOfClass <DragAndDropContainer> () because of a VC6 compiler bug)
    return c->findParentComponentOfClass ((DragAndDropContainer*) 0);
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
