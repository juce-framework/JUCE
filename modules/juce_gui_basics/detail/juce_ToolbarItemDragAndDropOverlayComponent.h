/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::detail
{

class ToolbarItemDragAndDropOverlayComponent    : public Component
{
public:
    ToolbarItemDragAndDropOverlayComponent()
        : isDragging (false)
    {
        setAlwaysOnTop (true);
        setRepaintsOnMouseActivity (true);
        setMouseCursor (MouseCursor::DraggingHandCursor);
    }

    void paint (Graphics& g) override
    {
        if (ToolbarItemComponent* const tc = getToolbarItemComponent())
        {
            if (isMouseOverOrDragging()
                  && tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar)
            {
                g.setColour (findColour (Toolbar::editingModeOutlineColourId, true));
                g.drawRect (getLocalBounds(), jmin (2, (getWidth() - 1) / 2,
                                                       (getHeight() - 1) / 2));
            }
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        isDragging = false;

        if (ToolbarItemComponent* const tc = getToolbarItemComponent())
        {
            tc->dragOffsetX = e.x;
            tc->dragOffsetY = e.y;
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown() && ! isDragging)
        {
            isDragging = true;

            if (DragAndDropContainer* const dnd = DragAndDropContainer::findParentDragContainerFor (this))
            {
                dnd->startDragging (Toolbar::toolbarDragDescriptor, getParentComponent(), ScaledImage(), true, nullptr, &e.source);

                if (ToolbarItemComponent* const tc = getToolbarItemComponent())
                {
                    tc->isBeingDragged = true;

                    if (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar)
                        tc->setVisible (false);
                }
            }
        }
    }

    void mouseUp (const MouseEvent&) override
    {
        isDragging = false;

        if (ToolbarItemComponent* const tc = getToolbarItemComponent())
        {
            tc->isBeingDragged = false;

            if (Toolbar* const tb = tc->getToolbar())
                tb->updateAllItemPositions (true);
            else if (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar)
                delete tc;
        }
    }

    void parentSizeChanged() override
    {
        setBounds (0, 0, getParentWidth(), getParentHeight());
    }

private:
    //==============================================================================
    bool isDragging;

    ToolbarItemComponent* getToolbarItemComponent() const noexcept
    {
        return dynamic_cast<ToolbarItemComponent*> (getParentComponent());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarItemDragAndDropOverlayComponent)
};

} // namespace juce::detail
