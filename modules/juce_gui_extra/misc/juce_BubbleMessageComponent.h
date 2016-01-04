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

#ifndef JUCE_BUBBLEMESSAGECOMPONENT_H_INCLUDED
#define JUCE_BUBBLEMESSAGECOMPONENT_H_INCLUDED


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
    BubbleMessageComponent (int fadeOutLengthMs = 150);

    /** Destructor. */
    ~BubbleMessageComponent();

    //==============================================================================
    /** Shows a message bubble at a particular position.

        This shows the bubble with its stem pointing to the given location
        (coordinates being relative to its parent component).

        For details about exactly how it decides where to position itself, see
        BubbleComponent::updatePosition().

        @param position                         the coords of the object to point to
        @param message                          the text to display
        @param numMillisecondsBeforeRemoving    how long to leave it on the screen before removing itself
                                                from its parent compnent. If this is 0 or less, it
                                                will stay there until manually removed.
        @param removeWhenMouseClicked           if this is true, the bubble will disappear as soon as a
                                                mouse button is pressed (anywhere on the screen)
        @param deleteSelfAfterUse               if true, then the component will delete itself after
                                                it becomes invisible
    */
    void showAt (const Rectangle<int>& position,
                 const AttributedString& message,
                 int numMillisecondsBeforeRemoving,
                 bool removeWhenMouseClicked = true,
                 bool deleteSelfAfterUse = false);

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
    void showAt (Component* component,
                 const AttributedString& message,
                 int numMillisecondsBeforeRemoving,
                 bool removeWhenMouseClicked = true,
                 bool deleteSelfAfterUse = false);


    //==============================================================================
    /** @internal */
    void getContentSize (int& w, int& h) override;
    /** @internal */
    void paintContent (Graphics& g, int w, int h) override;
    /** @internal */
    void timerCallback() override;

private:
    //==============================================================================
    int fadeOutLength, mouseClickCounter;
    TextLayout textLayout;
    int64 expiryTime;
    bool deleteAfterUse;

    void createLayout (const AttributedString&);
    void init (int, bool, bool);
    void hide (bool fadeOut);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BubbleMessageComponent)
};


#endif   // JUCE_BUBBLEMESSAGECOMPONENT_H_INCLUDED
