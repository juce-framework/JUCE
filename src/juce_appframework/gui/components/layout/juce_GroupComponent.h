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

#ifndef __JUCE_GROUPCOMPONENT_JUCEHEADER__
#define __JUCE_GROUPCOMPONENT_JUCEHEADER__

#include "../juce_Component.h"


//==============================================================================
/**
    A component that draws an outline around itself and has an optional title at
    the top, for drawing an outline around a group of controls.

*/
class JUCE_API  GroupComponent    : public Component
{
public:
    //==============================================================================
    /** Creates a GroupComponent.

        @param componentName    the name to give the component
        @param labelText        the text to show at the top of the outline
    */
    GroupComponent (const String& componentName,
                    const String& labelText);

    /** Destructor. */
    ~GroupComponent();

    //==============================================================================
    /** Changes the text that's shown at the top of the component. */
    void setText (const String& newText) throw();

    /** Returns the currently displayed text label. */
    const String getText() const throw();

    /** Sets the positioning of the text label.

        (The default is Justification::left)

        @see getTextLabelPosition
    */
    void setTextLabelPosition (const Justification& justification);

    /** Returns the current text label position.

        @see setTextLabelPosition
    */
    const Justification getTextLabelPosition() const throw()            { return justification; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        outlineColourId     = 0x1005400,    /**< The colour to use for drawing the line around the edge. */
        textColourId        = 0x1005410     /**< The colour to use to draw the text label. */
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void enablementChanged();
    /** @internal */
    void colourChanged();

private:
    String text;
    Justification justification;

    GroupComponent (const GroupComponent&);
    const GroupComponent& operator= (const GroupComponent&);
};


#endif   // __JUCE_GROUPCOMPONENT_JUCEHEADER__
