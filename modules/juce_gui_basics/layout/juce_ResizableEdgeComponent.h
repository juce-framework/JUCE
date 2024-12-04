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
