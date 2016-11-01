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


/**
    Represents the touch surface of a block device.
*/
class TouchSurface
{
public:
    TouchSurface (Block&);

    /** Destructor. */
    virtual ~TouchSurface();

    //==============================================================================
    struct Touch
    {
        /** A touch index, which will stay constant for each finger as it is tracked. */
        int index;

        /** The X position of this touch on the device, in logical units starting from 0 (left).
            See Block::getWidth() for the maximum X value on the device.
        */
        float x;

        /** An approximation of the velocity at which the X value is changing.
            Measured in units/second. This is intended as a useful hint to help with gesture
            detection, but may be 0 if the device doesn't provide this data.
        */
        float xVelocity;

        /** The Y position of this touch on the device, in logical units starting from 0 (top).
            See Block::getHeight() to find the maximum Y on the device.
        */
        float y;

        /** An approximation of the velocity at which the Y value is changing.
            Measured in units/second. This is intended as a useful hint to help with gesture
            detection, but may be 0 if the device doesn't provide this data.
        */
        float yVelocity;

        /** The current pressure of this touch, in the range 0 (no pressure) to 1 (very hard) */
        float z;

        /** The rate at which pressure is currently changing.
            Measured in units/second. This is intended as a useful hint to help with gesture
            detection, but may be 0 if the device doesn't provide this data.
        */
        float zVelocity;

        /** The timestamp of this event, in milliseconds since the device was booted. */
        Block::Timestamp eventTimestamp;

        /** True if this is the first event for this finger. */
        bool isTouchStart;

        /** True if this is the final event as this finger is lifted off. */
        bool isTouchEnd;

        /** The ID of the block that generated this touch. */
        Block::UID blockUID;

        /** The initial X position of the touchStart event corresponding to this finger. */
        float startX;

        /** The initial Y position of the touchStart event corresponding to this finger. */
        float startY;
    };

    /** Receives callbacks when a touch moves or changes pressure. */
    struct Listener
    {
        virtual ~Listener();

        virtual void touchChanged (TouchSurface&, const Touch&) = 0;
    };

    /** Adds a listener to be called when there's a change to this surface. */
    void addListener (Listener*);

    /** Removes a previously-registered listener. */
    void removeListener (Listener*);

    /** For a Seaboard unit, this will return the number of keys.
        For other types of touch-surface, it will return 0.
    */
    virtual int getNumberOfKeywaves() const = 0;

    /** The block that owns this touch surface. */
    Block& block;

    /** Testing feature: allows you to inject touches into a touch-surface. */
    void callListenersTouchChanged (const TouchSurface::Touch& t)
    {
        listeners.call (&Listener::touchChanged, *this, t);
    }

    /** Forces touch-off messages for all active touches. */
    virtual void cancelAllActiveTouches() noexcept = 0;

protected:
    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TouchSurface)
};
