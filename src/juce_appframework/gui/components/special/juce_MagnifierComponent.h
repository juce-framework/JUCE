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

#ifndef __JUCE_MAGNIFIERCOMPONENT_JUCEHEADER__
#define __JUCE_MAGNIFIERCOMPONENT_JUCEHEADER__

#include "../juce_Component.h"


//==============================================================================
/**
    A component that contains another component, and can magnify or shrink it.

    This component will continually update its size so that it fits the zoomed
    version of the content component that you put inside it, so don't try to
    change the size of this component directly - instead change that of the
    content component.

    To make it all work, the magnifier uses extremely cunning ComponentPeer tricks
    to remap mouse events correctly. This means that the content component won't
    appear to be a direct child of this component, and instead will think its
    on the desktop.
*/
class JUCE_API  MagnifierComponent    : public Component
{
public:
    //==============================================================================
    /** Creates a MagnifierComponent.

        This component will continually update its size so that it fits the zoomed
        version of the content component that you put inside it, so don't try to
        change the size of this component directly - instead change that of the
        content component.

        @param contentComponent     the component to add as the magnified one
        @param deleteContentCompWhenNoLongerNeeded  if true, the content component will
                                    be deleted when this component is deleted. If false,
                                    it's the caller's responsibility to delete it later.
    */
    MagnifierComponent (Component* const contentComponent,
                        const bool deleteContentCompWhenNoLongerNeeded);

    /** Destructor. */
    ~MagnifierComponent();

    //==============================================================================
    /** Returns the current content component. */
    Component* getContentComponent() const throw()          { return content; }

    //==============================================================================
    /** Changes the zoom level.

        The scale factor must be greater than zero. Values less than 1 will shrink the
        image; values greater than 1 will multiply its size by this amount.

        When this is called, this component will change its size to fit the full extent
        of the newly zoomed content.
    */
    void setScaleFactor (double newScaleFactor);

    /** Returns the current zoom factor. */
    double getScaleFactor() const throw()                   { return scaleFactor; }


    //==============================================================================
    juce_UseDebuggingNewOperator

    /** @internal */
    void childBoundsChanged (Component*);

private:
    Component* content;
    Component* holderComp;
    double scaleFactor;
    ComponentPeer* peer;
    bool deleteContent;

    //==============================================================================
    void paint (Graphics& g);
    void mouseDown (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseMove (const MouseEvent& e);
    void mouseEnter (const MouseEvent& e);
    void mouseExit (const MouseEvent& e);
    void mouseWheelMove (const MouseEvent& e, float, float);

    int scaleInt (const int n) const throw();

    MagnifierComponent (const MagnifierComponent&);
    const MagnifierComponent& operator= (const MagnifierComponent&);
};


#endif   // __JUCE_MAGNIFIERCOMPONENT_JUCEHEADER__
