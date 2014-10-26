/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_ANIMATEDAPPCOMPONENT_H_INCLUDED
#define JUCE_ANIMATEDAPPCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A base class for writing simple one-page graphical apps.

    A subclass can inherit from this and implement just a few methods such as
    paint() and mouse-handling. The base class provides some simple abstractions
    to take care of continuously repainting itself.
*/
class AnimatedAppComponent   : public Component,
                               private Timer
{
public:
    AnimatedAppComponent();

    /** Your subclass can call this to start a timer running which will
        call update() and repaint the component at the given frequency.
    */
    void setFramesPerSecond (int framesPerSecond);

    /** Returns the number of elapsed frames since the component started running. */
    int getFrameCounter() const noexcept;

    /**
    */
    int getMillisecondsSinceLastPaint() const noexcept;

    /**
    */
    void update() = 0;

private:
    //==============================================================================
    Time lastPaintTime;
    int elapsedFrames;

    void timerCallback() override;
    void paintOverChildren (Graphics&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedAppComponent)
};



#endif   // JUCE_ANIMATEDAPPCOMPONENT_H_INCLUDED
