/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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
    Represents the touch surface of a BLOCKS device.

    @tags{Blocks}
*/
class TouchSurface
{
public:
    TouchSurface (Block&);

    /** Destructor. */
    virtual ~TouchSurface();

    //==============================================================================
    /** Structure used to describe touch properties */
    struct Touch
    {
        /** A touch index, which will stay constant for each finger as it is tracked. */
        int index;

        /** The X position of this touch on the device, in logical units starting from 0 (left).

            See Block::getWidth() for the maximum X value on the device.
        */
        float x;

        /** An approximation of the velocity at which the X value is changing, measured in
            units/second. This is intended as a useful hint to help with gesture detection, but
            may be 0 if the device doesn't provide this data.
        */
        float xVelocity;

        /** The Y position of this touch on the device, in logical units starting from 0 (top).

            See Block::getHeight() to find the maximum Y on the device.
        */
        float y;

        /** An approximation of the velocity at which the Y value is changing, measured in
            units/second. This is intended as a useful hint to help with gesture detection, but
            may be 0 if the device doesn't provide this data.
        */
        float yVelocity;

        /** The current pressure of this touch, in the range 0.0 (no pressure) to 1.0 (very hard). */
        float z;

        /** The rate at which pressure is currently changing, measured in units/second. This is
            intended as a useful hint to help with gesture detection, but may be 0 if the device
            doesn't provide this data.
        */
        float zVelocity;

        /** The timestamp of this event, in milliseconds since the device was booted. */
        Block::Timestamp eventTimestamp;

        /** True if this is the first event for this finger/index. */
        bool isTouchStart;

        /** True if this is the final event as this finger/index is lifted off. */
        bool isTouchEnd;

        /** The ID of the block that generated this touch. */
        Block::UID blockUID;

        /** The initial X position of the touchStart event corresponding to this finger/index. */
        float startX;

        /** The initial Y position of the touchStart event corresponding to this finger/index. */
        float startY;
    };

    //==============================================================================
    /** Forces a touch-off message for all active touches. */
    virtual void cancelAllActiveTouches() noexcept = 0;

    /** For the on-screen seaboard view, this will return the number of keys.
        For other types of touch-surface, it will return 0. */
    virtual int getNumberOfKeywaves() const = 0;

    //==============================================================================
    /** Receives callbacks when a touch moves or changes pressure. */
    struct Listener
    {
        virtual ~Listener();

        virtual void touchChanged (TouchSurface&, const Touch&) = 0;
    };

    /** Testing feature: this allows you to inject touches onto a touch surface. */
    void callListenersTouchChanged (const TouchSurface::Touch& t)
    {
        listeners.call ([this, &t] (Listener& l) { l.touchChanged (*this, t); });
    }

    /** Adds a listener to be called when the surface is touched. */
    void addListener (Listener*);

    /** Removes a previously-registered listener. */
    void removeListener (Listener*);

    //==============================================================================
    /** The block that owns this touch surface. */
    Block& block;

protected:
    //==============================================================================
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TouchSurface)
};

} // namespace juce
