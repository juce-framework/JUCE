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
private:
    Image* image;
    Component* const source;
    DragAndDropContainer* const owner;

    ComponentDeletionWatcher* sourceWatcher;
    Component* mouseDragSource;
    ComponentDeletionWatcher* mouseDragSourceWatcher;
    DragAndDropTarget* currentlyOver;
    ComponentDeletionWatcher* currentlyOverWatcher;

    String dragDesc;
    int xOff, yOff;
    bool hasCheckedForExternalDrag, drawImage;

    DragImageComponent (const DragImageComponent&);
    const DragImageComponent& operator= (const DragImageComponent&);

public:
    DragImageComponent (Image* const im,
                        const String& desc,
                        Component* const s,
                        DragAndDropContainer* const o)
        : image (im),
          source (s),
          owner (o),
          currentlyOver (0),
          currentlyOverWatcher (0),
          dragDesc (desc),
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

        int mx, my;
        Desktop::getLastMouseDownPosition (mx, my);
        source->globalPositionToRelative (mx, my);

        xOff = jlimit (0, im->getWidth(), mx);
        yOff = jlimit (0, im->getHeight(), my);

        startTimer (200);

        setInterceptsMouseClicks (false, false);
        setAlwaysOnTop (true);
    }

    ~DragImageComponent()
    {
        if (owner->dragImageComponent == this)
            owner->dragImageComponent = 0;

        if (! mouseDragSourceWatcher->hasBeenDeleted())
        {
            mouseDragSource->removeMouseListener (this);

            if (currentlyOverWatcher != 0 && ! currentlyOverWatcher->hasBeenDeleted())
                if (currentlyOver->isInterestedInDragSource (dragDesc, source))
                    currentlyOver->itemDragExit (dragDesc, source);
        }

        delete mouseDragSourceWatcher;
        delete sourceWatcher;
        delete image;
        delete currentlyOverWatcher;
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

    DragAndDropTarget* findTarget (const int screenX, const int screenY,
                                   int& relX, int& relY) const throw()
    {
        Component* hit = getParentComponent();

        if (hit == 0)
        {
            hit = Desktop::getInstance().findComponentAt (screenX, screenY);
        }
        else
        {
            int rx = screenX, ry = screenY;
            hit->globalPositionToRelative (rx, ry);

            hit = hit->getComponentAt (rx, ry);
        }

        // (note: use a local copy of the dragDesc member in case the callback runs
        // a modal loop and deletes this object before the method completes)
        const String dragDescLocal (dragDesc);

        while (hit != 0)
        {
            DragAndDropTarget* const ddt = dynamic_cast <DragAndDropTarget*> (hit);

            if (ddt != 0 && ddt->isInterestedInDragSource (dragDescLocal, source))
            {
                relX = screenX;
                relY = screenY;
                hit->globalPositionToRelative (relX, relY);
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
            int relX = 0, relY = 0;

            if (isVisible())
            {
                setVisible (false);
                ddt = findTarget (e.getScreenX(),
                                  e.getScreenY(),
                                  relX, relY);

                // fade this component and remove it - it'll be deleted later by the timer callback

                dropAccepted = ddt != 0;

                setVisible (true);

                if (dropAccepted || sourceWatcher->hasBeenDeleted())
                {
                    fadeOutComponent (120);
                }
                else
                {
                    int targetX = source->getWidth() / 2;
                    int targetY = source->getHeight() / 2;
                    source->relativePositionToGlobal (targetX, targetY);

                    int ourCentreX = getWidth() / 2;
                    int ourCentreY = getHeight() / 2;
                    relativePositionToGlobal (ourCentreX, ourCentreY);

                    fadeOutComponent (120,
                                      targetX - ourCentreX,
                                      targetY - ourCentreY);
                }
            }

            if (getParentComponent() != 0)
                getParentComponent()->removeChildComponent (this);

            if (dropAccepted && ddt != 0)
            {
                // (note: use a local copy of the dragDesc member in case the callback runs
                // a modal loop and deletes this object before the method completes)
                const String dragDescLocal (dragDesc);

                currentlyOver = 0;
                deleteAndZero (currentlyOverWatcher);

                ddt->itemDropped (dragDescLocal, source, relX, relY);
            }

            // careful - this object could now be deleted..
        }
    }

    void updateLocation (const bool canDoExternalDrag, int x, int y)
    {
        // (note: use a local copy of the dragDesc member in case the callback runs
        // a modal loop and deletes this object before it returns)
        const String dragDescLocal (dragDesc);

        int newX = x - xOff;
        int newY = y - yOff;

        if (getParentComponent() != 0)
            getParentComponent()->globalPositionToRelative (newX, newY);

        if (newX != getX() || newY != getY())
        {
            setTopLeftPosition (newX, newY);

            int relX = 0, relY = 0;
            DragAndDropTarget* const ddt = findTarget (x, y, relX, relY);

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
                deleteAndZero (currentlyOverWatcher);

                if (ddt != 0)
                {
                    currentlyOverWatcher = new ComponentDeletionWatcher (dynamic_cast <Component*> (ddt));

                    if (currentlyOver->isInterestedInDragSource (dragDescLocal, source))
                        currentlyOver->itemDragEnter (dragDescLocal, source, relX, relY);
                }
            }
            else if (currentlyOverWatcher != 0 && currentlyOverWatcher->hasBeenDeleted())
            {
                currentlyOver = 0;
                deleteAndZero (currentlyOverWatcher);
            }

            if (currentlyOver != 0
                 && currentlyOver->isInterestedInDragSource (dragDescLocal, source))
                currentlyOver->itemDragMove (dragDescLocal, source, relX, relY);

            if (currentlyOver == 0
                 && canDoExternalDrag
                 && ! hasCheckedForExternalDrag)
            {
                if (Desktop::getInstance().findComponentAt (x, y) == 0)
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
            updateLocation (true, e.getScreenX(), e.getScreenY());
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
};


//==============================================================================
DragAndDropContainer::DragAndDropContainer()
   : dragImageComponent (0)
{
}

DragAndDropContainer::~DragAndDropContainer()
{
    if (dragImageComponent != 0)
        delete dragImageComponent;
}

void DragAndDropContainer::startDragging (const String& sourceDescription,
                                          Component* sourceComponent,
                                          Image* im,
                                          const bool allowDraggingToExternalWindows)
{
    if (dragImageComponent != 0)
    {
        if (im != 0)
            delete im;
    }
    else
    {
        Component* const thisComp = dynamic_cast <Component*> (this);

        if (thisComp != 0)
        {
            int mx, my;
            Desktop::getLastMouseDownPosition (mx, my);

            if (im == 0)
            {
                im = sourceComponent->createComponentSnapshot (Rectangle (0, 0, sourceComponent->getWidth(), sourceComponent->getHeight()));

                if (im->getFormat() != Image::ARGB)
                {
                    Image* newIm = new Image (Image::ARGB, im->getWidth(), im->getHeight(), true);
                    Graphics g2 (*newIm);
                    g2.drawImageAt (im, 0, 0);

                    delete im;
                    im = newIm;
                }

                im->multiplyAllAlphas (0.6f);

                const int lo = 150;
                const int hi = 400;

                int rx = mx, ry = my;
                sourceComponent->globalPositionToRelative (rx, ry);
                const int cx = jlimit (0, im->getWidth(), rx);
                const int cy = jlimit (0, im->getHeight(), ry);

                for (int y = im->getHeight(); --y >= 0;)
                {
                    const double dy = (y - cy) * (y - cy);

                    for (int x = im->getWidth(); --x >= 0;)
                    {
                        const int dx = x - cx;
                        const int distance = roundDoubleToInt (sqrt (dx * dx + dy));

                        if (distance > lo)
                        {
                            const float alpha = (distance > hi) ? 0
                                                                : (hi - distance) / (float) (hi - lo)
                                                                   + Random::getSystemRandom().nextFloat() * 0.008f;

                            im->multiplyAlphaAt (x, y, alpha);
                        }
                    }
                }
            }

            DragImageComponent* const dic
                = new DragImageComponent (im,
                                          sourceDescription,
                                          sourceComponent,
                                          this);

            dragImageComponent = dic;
            currentDragDesc = sourceDescription;

            if (allowDraggingToExternalWindows)
            {
                if (! Desktop::canUseSemiTransparentWindows())
                    dic->setOpaque (true);

                dic->addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                                    | ComponentPeer::windowIsTemporary);
            }
            else
                thisComp->addChildComponent (dic);

            dic->updateLocation (false, mx, my);
            dic->setVisible (true);
        }
        else
        {
            // this class must only be implemented by an object that
            // is also a Component.
            jassertfalse

            if (im != 0)
                delete im;
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
