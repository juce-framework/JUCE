/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
#if JUCE_MAC || DOXYGEN
/**
    Receives events from an Apple IR remote control device (Only available in OSX!).

    To use it, just create a subclass of this class, implementing the buttonPressed()
    callback, then call start() and stop() to start or stop receiving events.

    @tags{GUI}
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

} // namespace juce
