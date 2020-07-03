/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component that resizes its parent component when dragged.

    This component forms a bar along one edge of a component, allowing it to
    be dragged by that edges to resize it.

    To use it, just add it to your component, positioning it along the appropriate
    edge. Make sure you reposition the resizer component each time the parent's size
    changes, to keep it in the correct position.

    @see ResizableBorderComponent, ResizableCornerComponent

    @tags{GUI}
*/
class JUCE_API  ResizableEdgeComponent  : public Component
{
public:
    //==============================================================================
    enum Edge
    {
        leftEdge,   /**< Indicates a vertical bar that can be dragged left/right to move the component's left-hand edge. */
        rightEdge,  /**< Indicates a vertical bar that can be dragged left/right to move the component's right-hand edge. */
        topEdge,    /**< Indicates a horizontal bar that can be dragged up/down to move the top of the component. */
        bottomEdge  /**< Indicates a horizontal bar that can be dragged up/down to move the bottom of the component. */
    };

    /** Creates a resizer bar.

        Pass in the target component which you want to be resized when this one is
        dragged. The target component will usually be this component's parent, but this
        isn't mandatory.

        Remember that when the target component is resized, it'll need to move and
        resize this component to keep it in place, as this won't happen automatically.

        If the constrainer parameter is not a nullptr, then this object will be used to
        enforce limits on the size and position that the component can be stretched to.
        Make sure that the constrainer isn't deleted while still in use by this object.

        @see ComponentBoundsConstrainer
    */
    ResizableEdgeComponent (Component* componentToResize,
                            ComponentBoundsConstrainer* constrainer,
                            Edge edgeToResize);

    /** Destructor. */
    ~ResizableEdgeComponent() override;

    bool isVertical() const noexcept;

protected:
    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;

private:
    WeakReference<Component> component;
    ComponentBoundsConstrainer* constrainer;
    Rectangle<int> originalBounds;
    const Edge edge;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizableEdgeComponent)
};

} // namespace juce
