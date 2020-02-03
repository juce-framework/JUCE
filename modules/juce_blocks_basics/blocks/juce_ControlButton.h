/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    Represents a button on a block device.

    @tags{Blocks}
*/
class ControlButton
{
public:
    ControlButton (Block&);

    /** Destructor. */
    virtual ~ControlButton();

    /** These are all known types of control buttons.
        You can find out which buttons a device has by calling getButtons(),
        and can get the name of a button type with getButtonName().
    */
    enum ButtonFunction
    {
        mode,   /**< The side button on a lightpad block and the first button on a live/loop block. */

        volume, /**< The volume button on a live/loop block. */

        // common to all types of block
        up,     /**< The up button on a control block.   */
        down,   /**< The down button on a control block. */

        // live block buttons
        scale,   /**< The scale button on a live block.   */
        chord,   /**< The chord button on a live block.   */
        arp,     /**< The arp button on a live block.     */
        sustain, /**< The sustain button on a live block. */
        octave,  /**< The octave button on a live block.  */
        love,    /**< The love button on a live block.    */

        // loop block buttons
        click,       /**< The click button on a loop block.         */
        snap,        /**< The snap button on a loop block.          */
        back,        /**< The back button on a loop block.          */
        playOrPause, /**< The play or pause button on a loop block. */
        record,      /**< The record button on a loop block.        */
        learn,       /**< The learn button on a loop block.         */

        // developer block buttons
        button0, /**< Button 0 on a developer block. */
        button1, /**< Button 1 on a developer block. */
        button2, /**< Button 2 on a developer block. */
        button3, /**< Button 3 on a developer block. */
        button4, /**< Button 4 on a developer block. */
        button5, /**< Button 5 on a developer block. */
        button6, /**< Button 6 on a developer block. */
        button7, /**< Button 7 on a developer block. */

        // touch block buttons
        velocitySensitivity, /**< The velocity sensitivity button on a touch block. */
        glideSensitivity,    /**< The glide sensitivity button on a touch block.    */
        slideSensitivity,    /**< The slide sensitivity button on a touch block.    */
        pressSensitivity,    /**< The press sensitivity button on a touch block.    */
        liftSensitivity,     /**< The lift sensitivity button on a touch block.     */
        fixedVelocity,       /**< The fixed velocity button on a touch block.       */
        glideLock,           /**< The glide lock button on a touch block.           */
        pianoMode            /**< The piano mode button on a touch block.           */
    };

    /** Returns the button's type. */
    virtual ButtonFunction getType() const = 0;

    /** Returns the button's description. */
    virtual String getName() const = 0;

    /** Returns the position of this button on the device, in device units.
        For buttons that are on the side of the device, this may want to return a value that
        is beyond the physical block size.
    */
    virtual float getPositionX() const = 0;

    /** Returns the position of this button on the device, in device units.
        For buttons that are on the side of the device, this may want to return a value that
        is beyond the physical block size.
    */
    virtual float getPositionY() const = 0;

    /** Returns true if this button has a controllable light. */
    virtual bool hasLight() const = 0;

    /** If the button can light-up, this sets its colour. */
    virtual bool setLightColour (LEDColour newColour) = 0;

    /** A listener that can be attached to a ControlButton object so that it
        gets called when the button is pushed or released.
    */
    struct Listener
    {
        virtual ~Listener();

        /** Called when the button is pressed. */
        virtual void buttonPressed (ControlButton&, Block::Timestamp) = 0;
        /** Called when the button is released. */
        virtual void buttonReleased (ControlButton&, Block::Timestamp) = 0;
    };

    /** Adds a listener to the control button. */
    void addListener (Listener*);
    /** Removes a listener from the control button. */
    void removeListener (Listener*);

    /** The control block that this button belongs to. */
    Block& block;

protected:
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlButton)
};

} // namespace juce
