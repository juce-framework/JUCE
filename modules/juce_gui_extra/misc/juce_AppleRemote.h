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

#ifndef JUCE_APPLEREMOTE_H_INCLUDED
#define JUCE_APPLEREMOTE_H_INCLUDED


//==============================================================================
#if JUCE_MAC || DOXYGEN
/**
    Receives events from an Apple IR remote control device (Only available in OSX!).

    To use it, just create a subclass of this class, implementing the buttonPressed()
    callback, then call start() and stop() to start or stop receiving events.
*/
class JUCE_API  AppleRemoteDevice
{
public:
    //==============================================================================
    AppleRemoteDevice();
    virtual ~AppleRemoteDevice();

    //==============================================================================
    /** The set of buttons that may be pressed.
        @see buttonPressed
    */
    enum ButtonType
    {
        menuButton = 0,     /**< The menu button (if it's held for a short time). */
        playButton,         /**< The play button. */
        plusButton,         /**< The plus or volume-up button. */
        minusButton,        /**< The minus or volume-down button. */
        rightButton,        /**< The right button (if it's held for a short time). */
        leftButton,         /**< The left button (if it's held for a short time). */
        rightButton_Long,   /**< The right button (if it's held for a long time). */
        leftButton_Long,    /**< The menu button (if it's held for a long time). */
        menuButton_Long,    /**< The menu button (if it's held for a long time). */
        playButtonSleepMode,
        switched
    };

    //==============================================================================
    /** Override this method to receive the callback about a button press.

        The callback will happen on the application's message thread.

        Some buttons trigger matching up and down events, in which the isDown parameter
        will be true and then false. Others only send a single event when the
        button is pressed.
    */
    virtual void buttonPressed (ButtonType buttonId, bool isDown) = 0;

    //==============================================================================
    /** Starts the device running and responding to events.

        Returns true if it managed to open the device.

        @param inExclusiveMode  if true, the remote will be grabbed exclusively for this app,
                                and will not be available to any other part of the system. If
                                false, it will be shared with other apps.
        @see stop
    */
    bool start (bool inExclusiveMode);

    /** Stops the device running.
        @see start
    */
    void stop();

    /** Returns true if the device has been started successfully.
    */
    bool isActive() const;

    /** Returns the ID number of the remote, if it has sent one.
    */
    int getRemoteId() const                     { return remoteId; }

    //==============================================================================
    /** @internal */
    void handleCallbackInternal();

private:
    void* device;
    void* queue;
    int remoteId;

    bool open (bool openInExclusiveMode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppleRemoteDevice)
};

#endif
#endif   // JUCE_APPLEREMOTE_H_INCLUDED
