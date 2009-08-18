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

#ifndef __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__
#define __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__

#include "juce_BubbleComponent.h"
#include "../../../events/juce_Timer.h"
#include "../../graphics/fonts/juce_TextLayout.h"


//==============================================================================
/**
    A speech-bubble component that displays a short message.

    This can be used to show a message with the tail of the speech bubble
    pointing to a particular component or location on the screen.

    @see BubbleComponent
*/
class JUCE_API  BubbleMessageComponent  : public BubbleComponent,
                                          private Timer
{
public:
    //==============================================================================
    /** Creates a bubble component.

        After creating one a BubbleComponent, do the following:
        - add it to an appropriate parent component, or put it on the
          desktop with Component::addToDesktop (0).
        - use the showAt() method to show a message.
        - it will make itself invisible after it times-out (and can optionally
          also delete itself), or you can reuse it somewhere else by calling
          showAt() again.
    */
    BubbleMessageComponent (const int fadeOutLengthMs = 150);

    /** Destructor. */
    ~BubbleMessageComponent();

    //==============================================================================
    /** Shows a message bubble at a particular position.

        This shows the bubble with its stem pointing to the given location
        (co-ordinates being relative to its parent component).

        For details about exactly how it decides where to position itself, see
        BubbleComponent::updatePosition().

        @param x                                the x co-ordinate of end of the bubble's tail
        @param y                                the y co-ordinate of end of the bubble's tail
        @param message                          the text to display
        @param numMillisecondsBeforeRemoving    how long to leave it on the screen before removing itself
                                                from its parent compnent. If this is 0 or less, it
                                                will stay there until manually removed.
        @param removeWhenMouseClicked           if this is true, the bubble will disappear as soon as a
                                                mouse button is pressed (anywhere on the screen)
        @param deleteSelfAfterUse               if true, then the component will delete itself after
                                                it becomes invisible
    */
    void showAt (int x, int y,
                 const String& message,
                 const int numMillisecondsBeforeRemoving,
                 const bool removeWhenMouseClicked = true,
                 const bool deleteSelfAfterUse = false);

    /** Shows a message bubble next to a particular component.

        This shows the bubble with its stem pointing at the given component.

        For details about exactly how it decides where to position itself, see
        BubbleComponent::updatePosition().

        @param component                        the component that you want to point at
        @param message                          the text to display
        @param numMillisecondsBeforeRemoving    how long to leave it on the screen before removing itself
                                                from its parent compnent. If this is 0 or less, it
                                                will stay there until manually removed.
        @param removeWhenMouseClicked           if this is true, the bubble will disappear as soon as a
                                                mouse button is pressed (anywhere on the screen)
        @param deleteSelfAfterUse               if true, then the component will delete itself after
                                                it becomes invisible
    */
    void showAt (Component* const component,
                 const String& message,
                 const int numMillisecondsBeforeRemoving,
                 const bool removeWhenMouseClicked = true,
                 const bool deleteSelfAfterUse = false);


    //==============================================================================
    /** @internal */
    void getContentSize (int& w, int& h);
    /** @internal */
    void paintContent (Graphics& g, int w, int h);
    /** @internal */
    void timerCallback();

    juce_UseDebuggingNewOperator

private:
    int fadeOutLength, mouseClickCounter;
    TextLayout textLayout;
    int64 expiryTime;
    bool deleteAfterUse;

    void init (const int numMillisecondsBeforeRemoving,
               const bool removeWhenMouseClicked,
               const bool deleteSelfAfterUse);

    BubbleMessageComponent (const BubbleMessageComponent&);
    const BubbleMessageComponent& operator= (const BubbleMessageComponent&);
};


#endif   // __JUCE_BUBBLEMESSAGECOMPONENT_JUCEHEADER__
