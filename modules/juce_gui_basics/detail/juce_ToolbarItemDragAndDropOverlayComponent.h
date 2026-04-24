/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::detail
{

class ToolbarItemDragAndDropOverlayComponent    : public Component
{
public:
    ToolbarItemDragAndDropOverlayComponent()
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
    bool isDragging = false;

    ToolbarItemComponent* getToolbarItemComponent() const noexcept
    {
        return dynamic_cast<ToolbarItemComponent*> (getParentComponent());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarItemDragAndDropOverlayComponent)
};

} // namespace juce::detail
