/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
/**
    A base class for writing simple one-page graphical apps.

    A subclass can inherit from this and implement just a few methods such as
    paint() and mouse-handling. The base class provides some simple abstractions
    to take care of continuously repainting itself.

    @tags{GUI}
*/
class JUCE_API  AnimatedAppComponent   : public Component,
                                         private Timer
{
public:
    AnimatedAppComponent();

    /** Your subclass can call this to start a timer running which will
        call update() and repaint the component at the given frequency.
    */
    void setFramesPerSecond (int framesPerSecond);

    /** You can use this function to synchronise animation updates with the current display's vblank
        events. When this mode is enabled the value passed to setFramesPerSecond() is ignored.
    */
    void setSynchroniseToVBlank (bool syncToVBlank);

    /** Called periodically, at the frequency specified by setFramesPerSecond().
        This is a the best place to do things like advancing animation parameters,
        checking the mouse position, etc.
    */
    virtual void update() = 0;

    /** Returns the number of times that update() has been called since the component
        started running.
    */
    int getFrameCounter() const noexcept        { return totalUpdates; }

    /** When called from update(), this returns the number of milliseconds since the
        last update call.
        This might be useful for accurately timing animations, etc.
    */
    int getMillisecondsSinceLastUpdate() const noexcept;

private:
    //==============================================================================
    void updateSync();

    Time lastUpdateTime = Time::getCurrentTime();
    int totalUpdates = 0;
    int framesPerSecond = 60;
    bool useVBlank = false;
    VBlankAttachment vBlankAttachment;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedAppComponent)
};

} // namespace juce
