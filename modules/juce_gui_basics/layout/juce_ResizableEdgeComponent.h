/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_RESIZABLEEDGECOMPONENT_H_INCLUDED
#define JUCE_RESIZABLEEDGECOMPONENT_H_INCLUDED


//==============================================================================
/**
    A component that resizes its parent component when dragged.

    This component forms a bar along one edge of a component, allowing it to
    be dragged by that edges to resize it.

    To use it, just add it to your component, positioning it along the appropriate
    edge. Make sure you reposition the resizer component each time the parent's size
    changes, to keep it in the correct position.

    @see ResizbleBorderComponent, ResizableCornerComponent
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
    ~ResizableEdgeComponent();

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


#endif   // JUCE_RESIZABLEEDGECOMPONENT_H_INCLUDED
