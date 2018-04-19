/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_EditingPanelBase.h"
#include "jucer_JucerDocumentEditor.h"

//==============================================================================
class EditingPanelBase::MagnifierComponent  : public Component
{
public:
    MagnifierComponent (Component* c) : content (c)
    {
        addAndMakeVisible (content.get());
        childBoundsChanged (content.get());
    }

    void childBoundsChanged (Component* child)
    {
        auto childArea = getLocalArea (child, child->getLocalBounds());
        setSize (childArea.getWidth(), childArea.getHeight());
    }

    double getScaleFactor() const   { return scaleFactor; }

    void setScaleFactor (double newScale)
    {
        scaleFactor = newScale;
        content->setTransform (AffineTransform::scale ((float) scaleFactor));
    }

private:
    double scaleFactor = 1.0;
    std::unique_ptr<Component> content;
};

//==============================================================================
class ZoomingViewport   : public Viewport
{
public:
    ZoomingViewport (EditingPanelBase* p) : panel (p)
    {
    }

    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
    {
        if (e.mods.isCtrlDown() || e.mods.isAltDown() || e.mods.isCommandDown())
            mouseMagnify (e, 1.0f / (1.0f - wheel.deltaY));
        else
            Viewport::mouseWheelMove (e, wheel);
    }

    void mouseMagnify (const MouseEvent& e, float factor)
    {
        panel->setZoom (panel->getZoom() * factor, e.x, e.y);
    }

    void dragKeyHeldDown (const bool isKeyDown)
    {
        if (isSpaceDown != isKeyDown)
        {
            isSpaceDown = isKeyDown;

            if (isSpaceDown)
            {
                auto dc = new DraggerOverlayComp();
                addAndMakeVisible (dc);
                dc->setBounds (getLocalBounds());
            }
            else
            {
                for (int i = getNumChildComponents(); --i >= 0;)
                    std::unique_ptr<DraggerOverlayComp> deleter (dynamic_cast<DraggerOverlayComp*> (getChildComponent (i)));
            }
        }
    }

private:
    EditingPanelBase* const panel;
    bool isSpaceDown = false;

    //==============================================================================
    class DraggerOverlayComp    : public Component
    {
    public:
        DraggerOverlayComp()
        {
            setMouseCursor (MouseCursor::DraggingHandCursor);
            setAlwaysOnTop (true);
        }

        void mouseDown (const MouseEvent&)
        {
            if (Viewport* viewport = findParentComponentOfClass<Viewport>())
            {
                startX = viewport->getViewPositionX();
                startY = viewport->getViewPositionY();
            }
        }

        void mouseDrag (const MouseEvent& e)
        {
            if (Viewport* viewport = findParentComponentOfClass<Viewport>())
                viewport->setViewPosition (jlimit (0, jmax (0, viewport->getViewedComponent()->getWidth() - viewport->getViewWidth()),
                                                   startX - e.getDistanceFromDragStartX()),
                                           jlimit (0, jmax (0, viewport->getViewedComponent()->getHeight() - viewport->getViewHeight()),
                                                   startY - e.getDistanceFromDragStartY()));
        }

    private:
        int startX, startY;
    };
};


//==============================================================================
EditingPanelBase::EditingPanelBase (JucerDocument& doc, Component* props, Component* editorComp)
    : document (doc),
      editor (editorComp),
      propsPanel (props)
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

void EditingPanelBase::paint (Graphics& g)
{
    g.fillAll (findColour (secondaryBackgroundColourId));
}

void EditingPanelBase::visibilityChanged()
{
    if (isVisible())
    {
        updatePropertiesList();

        if (Component* p = getParentComponent())
        {
            resized();

            if (JucerDocumentEditor* const cdh = dynamic_cast<JucerDocumentEditor*> (p->getParentComponent()))
                cdh->setViewportToLastPos (viewport, *this);

            resized();
        }
    }
    else
    {
        if (Component* p = getParentComponent())
            if (JucerDocumentEditor* const cdh = dynamic_cast<JucerDocumentEditor*> (p->getParentComponent()))
                cdh->storeLastViewportPos (viewport, *this);
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

    jassert (viewport != nullptr);
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
