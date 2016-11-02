/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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


/**
    Represents a button on a block device.
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
        mode,   // The button on the side of a lightpad block and the first button on a live/loop block

        volume, // on live + loop blocks

        // common to all types of block
        up,
        down,

        // live block buttons
        scale,
        chord,
        arp,
        sustain,
        octave,
        love,

        // loop block buttons
        click,
        snap,
        back,
        playOrPause,
        record,
        learn,

        // developer block buttons
        button0,
        button1,
        button2,
        button3,
        button4,
        button5,
        button6,
        button7
    };

    /** Returns the button's type. */
    virtual ButtonFunction getType() const = 0;

    /** Returns the button's description. */
    virtual juce::String getName() const = 0;

    /** Returns the position of this button on the device, in device units.
        For buttons that are on the side of the device, this may want to return a value that
        is beyond the phyiscal block size.
    */
    virtual float getPositionX() const = 0;

    /** Returns the position of this button on the device, in device units.
        For buttons that are on the side of the device, this may want to return a value that
        is beyond the phyiscal block size.
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

        /** */
        virtual void buttonPressed (ControlButton&, Block::Timestamp) = 0;
        virtual void buttonReleased (ControlButton&, Block::Timestamp) = 0;
    };

    /** */
    void addListener (Listener*);
    /** */
    void removeListener (Listener*);

    /** */
    Block& block;

protected:
    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlButton)
};
