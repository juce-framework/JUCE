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

#ifndef __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
#define __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__

#include "juce_ComponentBoundsConstrainer.h"


//==============================================================================
/**
    A component that resizes its parent window when dragged.

    This component forms a frame around the edge of a component, allowing it to
    be dragged by the edges or corners to resize it - like the way windows are
    resized in MSWindows or Linux.

    To use it, just add it to your component, making it fill the entire parent component
    (there's a mouse hit-test that only traps mouse-events which land around the
    edge of the component, so it's even ok to put it on top of any other components
    you're using). Make sure you rescale the resizer component to fill the parent
    each time the parent's size changes.

    @see ResizableCornerComponent
*/
class JUCE_API  ResizableBorderComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a resizer.

        Pass in the target component which you want to be resized when this one is
        dragged.

        The target component will usually be a parent of the resizer component, but this
        isn't mandatory.

        Remember that when the target component is resized, it'll need to move and
        resize this component to keep it in place, as this won't happen automatically.

        If the constrainer parameter is non-zero, then this object will be used to enforce
        limits on the size and position that the component can be stretched to. Make sure
        that the constrainer isn't deleted while still in use by this object.

        @see ComponentBoundsConstrainer
    */
    ResizableBorderComponent (Component* const componentToResize,
                              ComponentBoundsConstrainer* const constrainer);

    /** Destructor. */
    ~ResizableBorderComponent();


    //==============================================================================
    /** Specifies how many pixels wide the draggable edges of this component are.

        @see getBorderThickness
    */
    void setBorderThickness (const BorderSize& newBorderSize) throw();

    /** Returns the number of pixels wide that the draggable edges of this component are.

        @see setBorderThickness
    */
    const BorderSize getBorderThickness() const throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void mouseEnter (const MouseEvent& e);
    /** @internal */
    void mouseMove (const MouseEvent& e);
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    bool hitTest (int x, int y);

private:
    Component* const component;
    ComponentBoundsConstrainer* constrainer;
    BorderSize borderSize;
    int originalX, originalY, originalW, originalH;
    int mouseZone;

    void updateMouseZone (const MouseEvent& e) throw();

    ResizableBorderComponent (const ResizableBorderComponent&);
    const ResizableBorderComponent& operator= (const ResizableBorderComponent&);
};


#endif   // __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
