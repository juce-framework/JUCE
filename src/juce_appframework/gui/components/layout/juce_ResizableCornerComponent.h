/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__
#define __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__

#include "juce_ComponentBoundsConstrainer.h"


//==============================================================================
/** A component that resizes a parent window when dragged.

    This is the small triangular stripey resizer component you get in the bottom-right
    of windows (more commonly on the Mac than Windows). Put one in the corner of
    a larger component and it will automatically resize its parent when it gets dragged
    around.

    @see ResizableFrameComponent
*/
class JUCE_API  ResizableCornerComponent  : public Component
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
        that the constrainer isn't deleted while still in use by this object. If you
        pass a zero in here, no limits will be put on the sizes it can be stretched to.

        @see ComponentBoundsConstrainer
    */
    ResizableCornerComponent (Component* const componentToResize,
                              ComponentBoundsConstrainer* const constrainer);

    /** Destructor. */
    ~ResizableCornerComponent();


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    bool hitTest (int x, int y);

private:
    //==============================================================================
    Component* const component;
    ComponentBoundsConstrainer* constrainer;
    int originalX, originalY, originalW, originalH;

    ResizableCornerComponent (const ResizableCornerComponent&);
    const ResizableCornerComponent& operator= (const ResizableCornerComponent&);
};


#endif   // __JUCE_RESIZABLECORNERCOMPONENT_JUCEHEADER__
