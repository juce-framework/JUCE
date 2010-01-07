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

#ifndef __JUCE_SLIDER_JUCEHEADER__
#define __JUCE_SLIDER_JUCEHEADER__

#include "juce_SliderListener.h"
#include "juce_Label.h"
#include "../buttons/juce_Button.h"
#include "../../../events/juce_AsyncUpdater.h"
#include "../../../containers/juce_SortedSet.h"
#include "../../../containers/juce_Value.h"


//==============================================================================
/**
    A slider control for changing a value.

    The slider can be horizontal, vertical, or rotary, and can optionally have
    a text-box inside it to show an editable display of the current value.

    To use it, create a Slider object and use the setSliderStyle() method
    to set up the type you want. To set up the text-entry box, use setTextBoxStyle().

    To define the values that it can be set to, see the setRange() and setValue() methods.

    There are also lots of custom tweaks you can do by subclassing and overriding
    some of the virtual methods, such as changing the scaling, changing the format of
    the text display, custom ways of limiting the values, etc.

    You can register SliderListeners with a slider, which will be informed when the value
    changes, or a subclass can override valueChanged() to be informed synchronously.

    @see SliderListener
*/
class JUCE_API  Slider  : public Component,
                          public SettableTooltipClient,
                          private AsyncUpdater,
                          private ButtonListener,
                          private LabelListener,
                          private Value::Listener
{
public:
    //==============================================================================
    /** Creates a slider.

        When created, you'll need to set up the slider's style and range with setSliderStyle(),
        setRange(), etc.
    */
    Slider (const String& componentName);

    /** Destructor. */
    ~Slider();

    //==============================================================================
    /** The types of slider available.

        @see setSliderStyle, setRotaryParameters
    */
    enum SliderStyle
    {
        LinearHorizontal,       /**< A traditional horizontal slider. */
        LinearVertical,         /**< A traditional vertical slider. */
        LinearBar,              /**< A horizontal bar slider with the text label drawn on top of it. */
        Rotary,                 /**< A rotary control that you move by dragging the mouse in a circular motion, like a knob.
                                     @see setRotaryParameters */
        RotaryHorizontalDrag,   /**< A rotary control that you move by dragging the mouse left-to-right.
                                     @see setRotaryParameters */
        RotaryVerticalDrag,     /**< A rotary control that you move by dragging the mouse up-and-down.
                                     @see setRotaryParameters */
        IncDecButtons,          /**< A pair of buttons that increment or decrement the slider's value by the increment set in setRange(). */

        TwoValueHorizontal,     /**< A horizontal slider that has two thumbs instead of one, so it can show a minimum and maximum value.
                                     @see setMinValue, setMaxValue */
        TwoValueVertical,       /**< A vertical slider that has two thumbs instead of one, so it can show a minimum and maximum value.
                                     @see setMinValue, setMaxValue */

        ThreeValueHorizontal,   /**< A horizontal slider that has three thumbs instead of one, so it can show a minimum and maximum
                                     value, with the current value being somewhere between them.
                                     @see setMinValue, setMaxValue */
        ThreeValueVertical,     /**< A vertical slider that has three thumbs instead of one, so it can show a minimum and maximum
                                     value, with the current value being somewhere between them.
                                     @see setMinValue, setMaxValue */
    };

    /** Changes the type of slider interface being used.

        @param newStyle         the type of interface
        @see setRotaryParameters, setVelocityBasedMode,
    */
    void setSliderStyle (const SliderStyle newStyle);

    /** Returns the slider's current style.

        @see setSliderStyle
    */
    SliderStyle getSliderStyle() const                                      { return style; }

    //==============================================================================
    /** Changes the properties of a rotary slider.

        @param startAngleRadians        the angle (in radians, clockwise from the top) at which
                                        the slider's minimum value is represented
        @param endAngleRadians          the angle (in radians, clockwise from the top) at which
                                        the slider's maximum value is represented. This must be
                                        greater than startAngleRadians
        @param stopAtEnd                if true, then when the slider is dragged around past the
                                        minimum or maximum, it'll stop there; if false, it'll wrap
                                        back to the opposite value
    */
    void setRotaryParameters (const float startAngleRadians,
                              const float endAngleRadians,
                              const bool stopAtEnd);

    /** Sets the distance the mouse has to move to drag the slider across
        the full extent of its range.

        This only applies when in modes like RotaryHorizontalDrag, where it's using
        relative mouse movements to adjust the slider.
    */
    void setMouseDragSensitivity (const int distanceForFullScaleDrag);

    //==============================================================================
    /** Changes the way the the mouse is used when dragging the slider.

        If true, this will turn on velocity-sensitive dragging, so that
        the faster the mouse moves, the bigger the movement to the slider. This
        helps when making accurate adjustments if the slider's range is quite large.

        If false, the slider will just try to snap to wherever the mouse is.
    */
    void setVelocityBasedMode (const bool isVelocityBased);

    /** Returns true if velocity-based mode is active.
        @see setVelocityBasedMode
    */
    bool getVelocityBasedMode() const                       { return isVelocityBased; }

    /** Changes aspects of the scaling used when in velocity-sensitive mode.

        These apply when you've used setVelocityBasedMode() to turn on velocity mode,
        or if you're holding down ctrl.

        @param sensitivity      higher values than 1.0 increase the range of acceleration used
        @param threshold        the minimum number of pixels that the mouse needs to move for it
                                to be treated as a movement
        @param offset           values greater than 0.0 increase the minimum speed that will be used when
                                the threshold is reached
        @param userCanPressKeyToSwapMode    if true, then the user can hold down the ctrl or command
                                key to toggle velocity-sensitive mode
    */
    void setVelocityModeParameters (const double sensitivity = 1.0,
                                    const int threshold = 1,
                                    const double offset = 0.0,
                                    const bool userCanPressKeyToSwapMode = true);

    /** Returns the velocity sensitivity setting.
        @see setVelocityModeParameters
    */
    double getVelocitySensitivity() const                       { return velocityModeSensitivity; }

    /** Returns the velocity threshold setting.
        @see setVelocityModeParameters
    */
    int getVelocityThreshold() const                            { return velocityModeThreshold; }

    /** Returns the velocity offset setting.
        @see setVelocityModeParameters
    */
    double getVelocityOffset() const                            { return velocityModeOffset; }

    /** Returns the velocity user key setting.
        @see setVelocityModeParameters
    */
    bool getVelocityModeIsSwappable() const                     { return userKeyOverridesVelocity; }

    //==============================================================================
    /** Sets up a skew factor to alter the way values are distributed.

        You may want to use a range of values on the slider where more accuracy
        is required towards one end of the range, so this will logarithmically
        spread the values across the length of the slider.

        If the factor is < 1.0, the lower end of the range will fill more of the
        slider's length; if the factor is > 1.0, the upper end of the range
        will be expanded instead. A factor of 1.0 doesn't skew it at all.

        To set the skew position by using a mid-point, use the setSkewFactorFromMidPoint()
        method instead.

        @see getSkewFactor, setSkewFactorFromMidPoint
    */
    void setSkewFactor (const double factor);

    /** Sets up a skew factor to alter the way values are distributed.

        This allows you to specify the slider value that should appear in the
        centre of the slider's visible range.

        @see setSkewFactor, getSkewFactor
     */
    void setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint);

    /** Returns the current skew factor.

        See setSkewFactor for more info.

        @see setSkewFactor, setSkewFactorFromMidPoint
    */
    double getSkewFactor() const                                { return skewFactor; }

    //==============================================================================
    /** Used by setIncDecButtonsMode().
    */
    enum IncDecButtonMode
    {
        incDecButtonsNotDraggable,
        incDecButtonsDraggable_AutoDirection,
        incDecButtonsDraggable_Horizontal,
        incDecButtonsDraggable_Vertical
    };

    /** When the style is IncDecButtons, this lets you turn on a mode where the mouse
        can be dragged on the buttons to drag the values.

        By default this is turned off. When enabled, clicking on the buttons still works
        them as normal, but by holding down the mouse on a button and dragging it a little
        distance, it flips into a mode where the value can be dragged. The drag direction can
        either be set explicitly to be vertical or horizontal, or can be set to
        incDecButtonsDraggable_AutoDirection so that it depends on whether the buttons
        are side-by-side or above each other.
    */
    void setIncDecButtonsMode (const IncDecButtonMode mode);

    //==============================================================================
    /** The position of the slider's text-entry box.

        @see setTextBoxStyle
    */
    enum TextEntryBoxPosition
    {
        NoTextBox,              /**< Doesn't display a text box.  */
        TextBoxLeft,            /**< Puts the text box to the left of the slider, vertically centred.  */
        TextBoxRight,           /**< Puts the text box to the right of the slider, vertically centred.  */
        TextBoxAbove,           /**< Puts the text box above the slider, horizontally centred.  */
        TextBoxBelow            /**< Puts the text box below the slider, horizontally centred.  */
    };

    /** Changes the location and properties of the text-entry box.

        @param newPosition          where it should go (or NoTextBox to not have one at all)
        @param isReadOnly           if true, it's a read-only display
        @param textEntryBoxWidth    the width of the text-box in pixels. Make sure this leaves enough
                                    room for the slider as well!
        @param textEntryBoxHeight   the height of the text-box in pixels. Make sure this leaves enough
                                    room for the slider as well!

        @see setTextBoxIsEditable, getValueFromText, getTextFromValue
    */
    void setTextBoxStyle (const TextEntryBoxPosition newPosition,
                          const bool isReadOnly,
                          const int textEntryBoxWidth,
                          const int textEntryBoxHeight);

    /** Returns the status of the text-box.
        @see setTextBoxStyle
    */
    const TextEntryBoxPosition getTextBoxPosition() const                   { return textBoxPos; }

    /** Returns the width used for the text-box.
        @see setTextBoxStyle
    */
    int getTextBoxWidth() const                                             { return textBoxWidth; }

    /** Returns the height used for the text-box.
        @see setTextBoxStyle
    */
    int getTextBoxHeight() const                                            { return textBoxHeight; }

    /** Makes the text-box editable.

        By default this is true, and the user can enter values into the textbox,
        but it can be turned off if that's not suitable.

        @see setTextBoxStyle, getValueFromText, getTextFromValue
    */
    void setTextBoxIsEditable (const bool shouldBeEditable);

    /** Returns true if the text-box is read-only.
        @see setTextBoxStyle
    */
    bool isTextBoxEditable() const                                          { return editableText; }

    /** If the text-box is editable, this will give it the focus so that the user can
        type directly into it.

        This is basically the effect as the user clicking on it.
    */
    void showTextBox();

    /** If the text-box currently has focus and is being edited, this resets it and takes keyboard
        focus away from it.

        @param discardCurrentEditorContents     if true, the slider's value will be left
                                                unchanged; if false, the current contents of the
                                                text editor will be used to set the slider position
                                                before it is hidden.
    */
    void hideTextBox (const bool discardCurrentEditorContents);


    //==============================================================================
    /** Changes the slider's current value.

        This will trigger a callback to SliderListener::sliderValueChanged() for any listeners
        that are registered, and will synchronously call the valueChanged() method in case subclasses
        want to handle it.

        @param newValue                 the new value to set - this will be restricted by the
                                        minimum and maximum range, and will be snapped to the
                                        nearest interval if one has been set
        @param sendUpdateMessage        if false, a change to the value will not trigger a call to
                                        any SliderListeners or the valueChanged() method
        @param sendMessageSynchronously if true, then a call to the SliderListeners will be made
                                        synchronously; if false, it will be asynchronous
    */
    void setValue (double newValue,
                   const bool sendUpdateMessage = true,
                   const bool sendMessageSynchronously = false);

    /** Returns the slider's current value. */
    double getValue() const;

    /** Returns the Value object that represents the slider's current position.
        You can use this Value object to connect the slider's position to external values or setters,
        either by taking a copy of the Value, or by using Value::referTo() to make it point to
        your own Value object.
        @see Value, getMaxValue, getMinValueObject
    */
    Value& getValueObject()                                                 { return currentValue; }

    //==============================================================================
    /** Sets the limits that the slider's value can take.

        @param newMinimum   the lowest value allowed
        @param newMaximum   the highest value allowed
        @param newInterval  the steps in which the value is allowed to increase - if this
                            is not zero, the value will always be (newMinimum + (newInterval * an integer)).
    */
    void setRange (const double newMinimum,
                   const double newMaximum,
                   const double newInterval = 0);

    /** Returns the current maximum value.
        @see setRange
    */
    double getMaximum() const                                               { return maximum; }

    /** Returns the current minimum value.
        @see setRange
    */
    double getMinimum() const                                               { return minimum; }

    /** Returns the current step-size for values.
        @see setRange
    */
    double getInterval() const                                              { return interval; }

    //==============================================================================
    /** For a slider with two or three thumbs, this returns the lower of its values.

        For a two-value slider, the values are controlled with getMinValue() and getMaxValue().
        A slider with three values also uses the normal getValue() and setValue() methods to
        control the middle value.

        @see setMinValue, getMaxValue, TwoValueHorizontal, TwoValueVertical, ThreeValueHorizontal, ThreeValueVertical
    */
    double getMinValue() const;

    /** For a slider with two or three thumbs, this returns the lower of its values.
        You can use this Value object to connect the slider's position to external values or setters,
        either by taking a copy of the Value, or by using Value::referTo() to make it point to
        your own Value object.
        @see Value, getMinValue, getMaxValueObject
    */
    Value& getMinValueObject()                                              { return valueMin; }

    /** For a slider with two or three thumbs, this sets the lower of its values.

        This will trigger a callback to SliderListener::sliderValueChanged() for any listeners
        that are registered, and will synchronously call the valueChanged() method in case subclasses
        want to handle it.

        @param newValue                 the new value to set - this will be restricted by the
                                        minimum and maximum range, and will be snapped to the nearest
                                        interval if one has been set.
        @param sendUpdateMessage        if false, a change to the value will not trigger a call to
                                        any SliderListeners or the valueChanged() method
        @param sendMessageSynchronously if true, then a call to the SliderListeners will be made
                                        synchronously; if false, it will be asynchronous
        @param allowNudgingOfOtherValues  if false, this value will be restricted to being below the
                                        max value (in a two-value slider) or the mid value (in a three-value
                                        slider). If false, then if this value goes beyond those values,
                                        it will push them along with it.
        @see getMinValue, setMaxValue, setValue
    */
    void setMinValue (double newValue,
                      const bool sendUpdateMessage = true,
                      const bool sendMessageSynchronously = false,
                      const bool allowNudgingOfOtherValues = false);

    /** For a slider with two or three thumbs, this returns the higher of its values.

        For a two-value slider, the values are controlled with getMinValue() and getMaxValue().
        A slider with three values also uses the normal getValue() and setValue() methods to
        control the middle value.

        @see getMinValue, TwoValueHorizontal, TwoValueVertical, ThreeValueHorizontal, ThreeValueVertical
    */
    double getMaxValue() const;

    /** For a slider with two or three thumbs, this returns the higher of its values.
        You can use this Value object to connect the slider's position to external values or setters,
        either by taking a copy of the Value, or by using Value::referTo() to make it point to
        your own Value object.
        @see Value, getMaxValue, getMinValueObject
    */
    Value& getMaxValueObject()                                              { return valueMax; }

    /** For a slider with two or three thumbs, this sets the lower of its values.

        This will trigger a callback to SliderListener::sliderValueChanged() for any listeners
        that are registered, and will synchronously call the valueChanged() method in case subclasses
        want to handle it.

        @param newValue                 the new value to set - this will be restricted by the
                                        minimum and maximum range, and will be snapped to the nearest
                                        interval if one has been set.
        @param sendUpdateMessage        if false, a change to the value will not trigger a call to
                                        any SliderListeners or the valueChanged() method
        @param sendMessageSynchronously if true, then a call to the SliderListeners will be made
                                        synchronously; if false, it will be asynchronous
        @param allowNudgingOfOtherValues  if false, this value will be restricted to being above the
                                        min value (in a two-value slider) or the mid value (in a three-value
                                        slider). If false, then if this value goes beyond those values,
                                        it will push them along with it.
        @see getMaxValue, setMinValue, setValue
    */
    void setMaxValue (double newValue,
                      const bool sendUpdateMessage = true,
                      const bool sendMessageSynchronously = false,
                      const bool allowNudgingOfOtherValues = false);

    //==============================================================================
    /** Adds a listener to be called when this slider's value changes. */
    void addListener (SliderListener* const listener);

    /** Removes a previously-registered listener. */
    void removeListener (SliderListener* const listener);

    //==============================================================================
    /** This lets you choose whether double-clicking moves the slider to a given position.

        By default this is turned off, but it's handy if you want a double-click to act
        as a quick way of resetting a slider. Just pass in the value you want it to
        go to when double-clicked.

        @see getDoubleClickReturnValue
    */
    void setDoubleClickReturnValue (const bool isDoubleClickEnabled,
                                    const double valueToSetOnDoubleClick);

    /** Returns the values last set by setDoubleClickReturnValue() method.

        Sets isEnabled to true if double-click is enabled, and returns the value
        that was set.

        @see setDoubleClickReturnValue
    */
    double getDoubleClickReturnValue (bool& isEnabled) const;

    //==============================================================================
    /** Tells the slider whether to keep sending change messages while the user
        is dragging the slider.

        If set to true, a change message will only be sent when the user has
        dragged the slider and let go. If set to false (the default), then messages
        will be continuously sent as they drag it while the mouse button is still
        held down.
    */
    void setChangeNotificationOnlyOnRelease (const bool onlyNotifyOnRelease);

    /** This lets you change whether the slider thumb jumps to the mouse position
        when you click.

        By default, this is true. If it's false, then the slider moves with relative
        motion when you drag it.

        This only applies to linear bars, and won't affect two- or three- value
        sliders.
    */
    void setSliderSnapsToMousePosition (const bool shouldSnapToMouse);

    /** If enabled, this gives the slider a pop-up bubble which appears while the
        slider is being dragged.

        This can be handy if your slider doesn't have a text-box, so that users can
        see the value just when they're changing it.

        If you pass a component as the parentComponentToUse parameter, the pop-up
        bubble will be added as a child of that component when it's needed. If you
        pass 0, the pop-up will be placed on the desktop instead (note that it's a
        transparent window, so if you're using an OS that can't do transparent windows
        you'll have to add it to a parent component instead).
    */
    void setPopupDisplayEnabled (const bool isEnabled,
                                 Component* const parentComponentToUse);

    /** If this is set to true, then right-clicking on the slider will pop-up
        a menu to let the user change the way it works.

        By default this is turned off, but when turned on, the menu will include
        things like velocity sensitivity, and for rotary sliders, whether they
        use a linear or rotary mouse-drag to move them.
    */
    void setPopupMenuEnabled (const bool menuEnabled);

    /** This can be used to stop the mouse scroll-wheel from moving the slider.

        By default it's enabled.
    */
    void setScrollWheelEnabled (const bool enabled);

    /** Returns a number to indicate which thumb is currently being dragged by the
        mouse.

        This will return 0 for the main thumb, 1 for the minimum-value thumb, 2 for
        the maximum-value thumb, or -1 if none is currently down.
    */
    int getThumbBeingDragged() const                { return sliderBeingDragged; }

    //==============================================================================
    /** Callback to indicate that the user is about to start dragging the slider.

        @see SliderListener::sliderDragStarted
    */
    virtual void startedDragging();

    /** Callback to indicate that the user has just stopped dragging the slider.

        @see SliderListener::sliderDragEnded
    */
    virtual void stoppedDragging();

    /** Callback to indicate that the user has just moved the slider.

        @see SliderListener::sliderValueChanged
    */
    virtual void valueChanged();

    /** Callback to indicate that the user has just moved the slider.
        Note - the valueChanged() method has changed its format and now no longer has
        any parameters. Update your code to use the new version.
        This version has been left here with an int as its return value to cause
        a syntax error if you've got existing code that uses the old version.
    */
    virtual int valueChanged (double) { jassertfalse; return 0; }

    //==============================================================================
    /** Subclasses can override this to convert a text string to a value.

        When the user enters something into the text-entry box, this method is
        called to convert it to a value.

        The default routine just tries to convert it to a double.

        @see getTextFromValue
    */
    virtual double getValueFromText (const String& text);

    /** Turns the slider's current value into a text string.

        Subclasses can override this to customise the formatting of the text-entry box.

        The default implementation just turns the value into a string, using
        a number of decimal places based on the range interval. If a suffix string
        has been set using setTextValueSuffix(), this will be appended to the text.

        @see getValueFromText
    */
    virtual const String getTextFromValue (double value);

    /** Sets a suffix to append to the end of the numeric value when it's displayed as
        a string.

        This is used by the default implementation of getTextFromValue(), and is just
        appended to the numeric value. For more advanced formatting, you can override
        getTextFromValue() and do something else.
    */
    void setTextValueSuffix (const String& suffix);

    //==============================================================================
    /** Allows a user-defined mapping of distance along the slider to its value.

        The default implementation for this performs the skewing operation that
        can be set up in the setSkewFactor() method. Override it if you need
        some kind of custom mapping instead, but make sure you also implement the
        inverse function in valueToProportionOfLength().

        @param proportion       a value 0 to 1.0, indicating a distance along the slider
        @returns                the slider value that is represented by this position
        @see valueToProportionOfLength
    */
    virtual double proportionOfLengthToValue (double proportion);

    /** Allows a user-defined mapping of value to the position of the slider along its length.

        The default implementation for this performs the skewing operation that
        can be set up in the setSkewFactor() method. Override it if you need
        some kind of custom mapping instead, but make sure you also implement the
        inverse function in proportionOfLengthToValue().

        @param value            a valid slider value, between the range of values specified in
                                setRange()
        @returns                a value 0 to 1.0 indicating the distance along the slider that
                                represents this value
        @see proportionOfLengthToValue
    */
    virtual double valueToProportionOfLength (double value);

    /** Returns the X or Y coordinate of a value along the slider's length.

        If the slider is horizontal, this will be the X coordinate of the given
        value, relative to the left of the slider. If it's vertical, then this will
        be the Y coordinate, relative to the top of the slider.

        If the slider is rotary, this will throw an assertion and return 0. If the
        value is out-of-range, it will be constrained to the length of the slider.
    */
    float getPositionOfValue (const double value);

    //==============================================================================
    /** This can be overridden to allow the slider to snap to user-definable values.

        If overridden, it will be called when the user tries to move the slider to
        a given position, and allows a subclass to sanity-check this value, possibly
        returning a different value to use instead.

        @param attemptedValue       the value the user is trying to enter
        @param userIsDragging       true if the user is dragging with the mouse; false if
                                    they are entering the value using the text box
        @returns                    the value to use instead
    */
    virtual double snapValue (double attemptedValue, const bool userIsDragging);


    //==============================================================================
    /** This can be called to force the text box to update its contents.

        (Not normally needed, as this is done automatically).
    */
    void updateText();


    /** True if the slider moves horizontally. */
    bool isHorizontal() const;
    /** True if the slider moves vertically. */
    bool isVertical() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the slider.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x1001200,  /**< A colour to use to fill the slider's background. */
        thumbColourId               = 0x1001300,  /**< The colour to draw the thumb with. It's up to the look
                                                       and feel class how this is used. */
        trackColourId               = 0x1001310,  /**< The colour to draw the groove that the thumb moves along. */
        rotarySliderFillColourId    = 0x1001311,  /**< For rotary sliders, this colour fills the outer curve. */
        rotarySliderOutlineColourId = 0x1001312,  /**< For rotary sliders, this colour is used to draw the outer curve's outline. */

        textBoxTextColourId         = 0x1001400,  /**< The colour for the text in the text-editor box used for editing the value. */
        textBoxBackgroundColourId   = 0x1001500,  /**< The background colour for the text-editor box. */
        textBoxHighlightColourId    = 0x1001600,  /**< The text highlight colour for the text-editor box. */
        textBoxOutlineColourId      = 0x1001700   /**< The colour to use for a border around the text-editor box. */
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void labelTextChanged (Label*);
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseDoubleClick (const MouseEvent& e);
    /** @internal */
    void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
    /** @internal */
    void modifierKeysChanged (const ModifierKeys& modifiers);
    /** @internal */
    void buttonClicked (Button* button);
    /** @internal */
    void lookAndFeelChanged();
    /** @internal */
    void enablementChanged();
    /** @internal */
    void focusOfChildComponentChanged (FocusChangeType cause);
    /** @internal */
    void handleAsyncUpdate();
    /** @internal */
    void colourChanged();
    /** @internal */
    void valueChanged (Value& value);

private:
    SortedSet <void*> listeners;
    Value currentValue, valueMin, valueMax;
    double lastCurrentValue, lastValueMin, lastValueMax;
    double minimum, maximum, interval, doubleClickReturnValue;
    double valueWhenLastDragged, valueOnMouseDown, skewFactor, lastAngle;
    double velocityModeSensitivity, velocityModeOffset, minMaxDiff;
    int velocityModeThreshold;
    float rotaryStart, rotaryEnd;
    int numDecimalPlaces, mouseXWhenLastDragged, mouseYWhenLastDragged;
    int mouseDragStartX, mouseDragStartY;
    int sliderRegionStart, sliderRegionSize;
    int sliderBeingDragged;
    int pixelsForFullDragExtent;
    Rectangle sliderRect;
    String textSuffix;

    SliderStyle style;
    TextEntryBoxPosition textBoxPos;
    int textBoxWidth, textBoxHeight;
    IncDecButtonMode incDecButtonMode;

    bool editableText : 1, doubleClickToValue : 1;
    bool isVelocityBased : 1, userKeyOverridesVelocity : 1, rotaryStop : 1;
    bool incDecButtonsSideBySide : 1, sendChangeOnlyOnRelease : 1, popupDisplayEnabled : 1;
    bool menuEnabled : 1, menuShown : 1, mouseWasHidden : 1, incDecDragged : 1;
    bool scrollWheelEnabled : 1, snapsToMousePos : 1;
    Font font;
    Label* valueBox;
    Button* incButton;
    Button* decButton;
    ScopedPointer <Component> popupDisplay;
    Component* parentForPopupDisplay;

    float getLinearSliderPos (const double value);
    void restoreMouseIfHidden();
    void sendDragStart();
    void sendDragEnd();
    double constrainedValue (double value) const;
    void triggerChangeMessage (const bool synchronous);
    bool incDecDragDirectionIsHorizontal() const;

    Slider (const Slider&);
    const Slider& operator= (const Slider&);
};


#endif   // __JUCE_SLIDER_JUCEHEADER__
