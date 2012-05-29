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

#include "../jucer_Headers.h"
#include "jucer_EditingPanelBase.h"
#include "jucer_JucerDocumentHolder.h"

//==============================================================================
class EditingPanelBase::MagnifierComponent  : public Component
{
public:
    MagnifierComponent (Component* content_)
        : scaleFactor (1.0), content (content_)
    {
        addAndMakeVisible (content);
        childBoundsChanged (content);
    }

    void childBoundsChanged (Component* child)
    {
        const Rectangle<int> childArea (getLocalArea (child, child->getLocalBounds()));
        setSize (childArea.getWidth(), childArea.getHeight());
    }

    double getScaleFactor() const   { return scaleFactor; }

    void setScaleFactor (double newScale)
    {
        scaleFactor = newScale;
        content->setTransform (AffineTransform::scale ((float) scaleFactor,
                                                       (float) scaleFactor));
    }

private:
    double scaleFactor;
    ScopedPointer<Component> content;
};

//==============================================================================
class ZoomingViewport   : public Viewport
{
public:
    ZoomingViewport (EditingPanelBase* const panel_)
        : panel (panel_),
          isSpaceDown (false)
    {
    }

    ~ZoomingViewport() {}

    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
    {
        if (e.mods.isCtrlDown() || e.mods.isAltDown())
        {
            const double factor = (wheel.deltaY > 0) ? 2.0 : 0.5;

            panel->setZoom (panel->getZoom() * factor, wheel.deltaX, wheel.deltaY);
        }
        else
        {
            Viewport::mouseWheelMove (e, wheel);
        }
    }

    void dragKeyHeldDown (const bool isKeyDown)
    {
        if (isSpaceDown != isKeyDown)
        {
            isSpaceDown = isKeyDown;

            if (isSpaceDown)
            {
                DraggerOverlayComp* const dc = new DraggerOverlayComp();
                addAndMakeVisible (dc);
                dc->setBounds (0, 0, getWidth(), getHeight());
            }
            else
            {
                for (int i = getNumChildComponents(); --i >= 0;)
                {
                    if (dynamic_cast <DraggerOverlayComp*> (getChildComponent (i)) != 0)
                    {
                        delete getChildComponent (i);
                    }
                }
            }
        }
    }

private:
    EditingPanelBase* const panel;
    bool isSpaceDown;

    //==============================================================================
    class DraggerOverlayComp    : public Component
    {
    public:
        DraggerOverlayComp()
        {
            setMouseCursor (MouseCursor::DraggingHandCursor);
            setAlwaysOnTop (true);
        }

        ~DraggerOverlayComp()
        {
        }

        void mouseDown (const MouseEvent& e)
        {
            Viewport* viewport = findParentComponentOfClass<Viewport>();

            if (viewport != 0)
            {
                startX = viewport->getViewPositionX();
                startY = viewport->getViewPositionY();
            }
        }

        void mouseDrag (const MouseEvent& e)
        {
            Viewport* viewport = findParentComponentOfClass<Viewport>();

            if (viewport != 0)
            {
                viewport->setViewPosition (jlimit (0, jmax (0, viewport->getViewedComponent()->getWidth() - viewport->getViewWidth()),
                                                   startX - e.getDistanceFromDragStartX()),
                                           jlimit (0, jmax (0, viewport->getViewedComponent()->getHeight() - viewport->getViewHeight()),
                                                   startY - e.getDistanceFromDragStartY()));
            }
        }

    private:
        int startX, startY;
    };
};


//==============================================================================
EditingPanelBase::EditingPanelBase (JucerDocument& document_,
                                    Component* propsPanel_,
                                    Component* editorComp)
    : document (document_),
      editor (editorComp),
      propsPanel (propsPanel_)
{
    addAndMakeVisible (viewport = new ZoomingViewport (this));
    addAndMakeVisible (propsPanel);

    viewport->setViewedComponent (magnifier = new MagnifierComponent (editor));
}

EditingPanelBase::~EditingPanelBase()
{
    deleteAllChildren();
}

void EditingPanelBase::resized()
{
    const int contentW = jmax (1, getWidth() - 260);

    propsPanel->setBounds (contentW + 4, 4, jmax (100, getWidth() - contentW - 8), getHeight() - 8);

    viewport->setBounds (4, 4, contentW - 8, getHeight() - 8);

    if (document.isFixedSize())
        editor->setSize (jmax (document.getInitialWidth(),
                               roundToInt ((viewport->getWidth() - viewport->getScrollBarThickness()) / getZoom())),
                         jmax (document.getInitialHeight(),
                               roundToInt ((viewport->getHeight() - viewport->getScrollBarThickness()) / getZoom())));
    else
        editor->setSize (viewport->getWidth(),
                         viewport->getHeight());
}

void EditingPanelBase::visibilityChanged()
{
    if (isVisible())
    {
        updatePropertiesList();

        if (getParentComponent() != 0)
        {
            resized();
            JucerDocumentHolder* const cdh = dynamic_cast <JucerDocumentHolder*> (getParentComponent()->getParentComponent());

            if (cdh != 0)
                cdh->setViewportToLastPos (viewport, *this);

            resized();
        }
    }
    else
    {
        if (getParentComponent() != 0)
        {
            JucerDocumentHolder* const cdh = dynamic_cast <JucerDocumentHolder*> (getParentComponent()->getParentComponent());

            if (cdh != 0)
                cdh->storeLastViewportPos (viewport, *this);
        }
    }

    editor->setVisible (isVisible());
}

double EditingPanelBase::getZoom() const
{
    return magnifier->getScaleFactor();
}

void EditingPanelBase::setZoom (double newScale)
{
    setZoom (jlimit (1.0 / 8.0, 16.0, newScale),
             viewport->getWidth() / 2,
             viewport->getHeight() / 2);
}

void EditingPanelBase::setZoom (double newScale, int anchorX, int anchorY)
{
    Point<int> anchor (editor->getLocalPoint (viewport, Point<int> (anchorX, anchorY)));

    magnifier->setScaleFactor (newScale);

    resized();
    anchor = viewport->getLocalPoint (editor, anchor);

    viewport->setViewPosition (jlimit (0, jmax (0, viewport->getViewedComponent()->getWidth() - viewport->getViewWidth()),
                                       viewport->getViewPositionX() + anchor.getX() - anchorX),
                               jlimit (0, jmax (0, viewport->getViewedComponent()->getHeight() - viewport->getViewHeight()),
                                       viewport->getViewPositionY() + anchor.getY() - anchorY));
}

void EditingPanelBase::xyToTargetXY (int& x, int& y) const
{
    Point<int> pos (editor->getLocalPoint (this, Point<int> (x, y)));
    x = pos.getX();
    y = pos.getY();
}

void EditingPanelBase::dragKeyHeldDown (bool isKeyDown)
{
    ((ZoomingViewport*) viewport)->dragKeyHeldDown (isKeyDown);
}
