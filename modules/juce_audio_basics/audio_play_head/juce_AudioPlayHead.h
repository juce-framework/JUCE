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

//==============================================================================
/**
    A subclass of AudioPlayHead can supply information about the position and
    status of a moving play head during audio playback.

    One of these can be supplied to an AudioProcessor object so that it can find
    out about the position of the audio that it is rendering.

    @see AudioProcessor::setPlayHead, AudioProcessor::getPlayHead

    @tags{Audio}
*/
class JUCE_API  AudioPlayHead
{
protected:
    //==============================================================================
    AudioPlayHead() = default;

public:
    virtual ~AudioPlayHead() = default;

    //==============================================================================
    /** Frame rate types. */
    enum FrameRateType
    {
        fps23976        = 0,
        fps24           = 1,
        fps25           = 2,
        fps2997         = 3,
        fps30           = 4,
        fps2997drop     = 5,
        fps30drop       = 6,
        fps60           = 7,
        fps60drop       = 8,
        fpsUnknown      = 99
    };

    /** More descriptive frame rate type. */
    class JUCE_API  FrameRate
    {
    public:
        /** Creates a frame rate with a base rate of 0. */
        FrameRate() = default;

        /** Creates a FrameRate instance from a FrameRateType. */
        FrameRate (FrameRateType type) : FrameRate (fromType (type)) {}

        /** Gets the FrameRateType that matches the state of this FrameRate.

            Returns fpsUnknown if this FrameRate cannot be represented by any of the
            other enum fields.
        */
        FrameRateType getType() const
        {
            switch (base)
            {
                case 24:    return pulldown ? fps23976 : fps24;
                case 25:    return fps25;
                case 30:    return pulldown ? (drop ? fps2997drop : fps2997)
                                            : (drop ? fps30drop   : fps30);
                case 60:    return drop ? fps60drop : fps60;
            }

            return fpsUnknown;
        }

        /** Returns the plain rate, without taking pulldown into account. */
        int getBaseRate() const                         { return base; }

        /** Returns true if drop-frame timecode is in use. */
        bool isDrop() const                             { return drop; }

        /** Returns true if the effective framerate is actually equal to the base rate divided by 1.001 */
        bool isPullDown() const                         { return pulldown; }

        /** Returns the actual rate described by this object, taking pulldown into account. */
        double getEffectiveRate() const                 { return pulldown ? (double) base / 1.001 : (double) base; }

        /** Returns a copy of this object with the specified base rate. */
        JUCE_NODISCARD FrameRate withBaseRate (int x) const            { return with (&FrameRate::base, x); }

        /** Returns a copy of this object with drop frames enabled or disabled, as specified. */
        JUCE_NODISCARD FrameRate withDrop (bool x = true) const        { return with (&FrameRate::drop, x); }

        /** Returns a copy of this object with pulldown enabled or disabled, as specified. */
        JUCE_NODISCARD FrameRate withPullDown (bool x = true) const    { return with (&FrameRate::pulldown, x); }

        /** Returns true if this instance is equal to other. */
        bool operator== (const FrameRate& other) const
        {
            const auto tie = [] (const FrameRate& x) { return std::tie (x.base, x.drop, x.pulldown); };
            return tie (*this) == tie (other);
        }

        /** Returns true if this instance is not equal to other. */
        bool operator!= (const FrameRate& other) const { return ! (*this == other); }

    private:
        static FrameRate fromType (FrameRateType type)
        {
            switch (type)
            {
                case fps23976:      return FrameRate().withBaseRate (24).withPullDown();
                case fps24:         return FrameRate().withBaseRate (24);
                case fps25:         return FrameRate().withBaseRate (25);
                case fps2997:       return FrameRate().withBaseRate (30).withPullDown();
                case fps30:         return FrameRate().withBaseRate (30);
                case fps2997drop:   return FrameRate().withBaseRate (30).withDrop().withPullDown();
                case fps30drop:     return FrameRate().withBaseRate (30).withDrop();
                case fps60:         return FrameRate().withBaseRate (60);
                case fps60drop:     return FrameRate().withBaseRate (60).withDrop();
                case fpsUnknown:    break;
            }

            return {};
        }

        template <typename Member, typename Value>
        FrameRate with (Member&& member, Value&& value) const
        {
            auto copy = *this;
            copy.*member = std::forward<Value> (value);
            return copy;
        }

        int base = 0;
        bool drop = false, pulldown = false;
    };

    //==============================================================================
    /** This structure is filled-in by the AudioPlayHead::getCurrentPosition() method.
    */
    struct JUCE_API  CurrentPositionInfo
    {
        /** The tempo in BPM */
        double bpm = 120.0;

        /** Time signature numerator, e.g. the 3 of a 3/4 time sig */
        int timeSigNumerator = 4;
        /** Time signature denominator, e.g. the 4 of a 3/4 time sig */
        int timeSigDenominator = 4;

        /** The current play position, in samples from the start of the timeline. */
        int64 timeInSamples = 0;
        /** The current play position, in seconds from the start of the timeline. */
        double timeInSeconds = 0;

        /** For timecode, the position of the start of the timeline, in seconds from 00:00:00:00. */
        double editOriginTime = 0;

        /** The current play position, in units of quarter-notes. */
        double ppqPosition = 0;

        /** The position of the start of the last bar, in units of quarter-notes.

            This is the time from the start of the timeline to the start of the current
            bar, in ppq units.

            Note - this value may be unavailable on some hosts, e.g. Pro-Tools. If
            it's not available, the value will be 0.
        */
        double ppqPositionOfLastBarStart = 0;

        /** The video frame rate, if applicable. */
        FrameRate frameRate = FrameRateType::fps23976;

        /** True if the transport is currently playing. */
        bool isPlaying = false;

        /** True if the transport is currently recording.

            (When isRecording is true, then isPlaying will also be true).
        */
        bool isRecording = false;

        /** The current cycle start position in units of quarter-notes.
            Note that not all hosts or plugin formats may provide this value.
            @see isLooping
        */
        double ppqLoopStart = 0;

        /** The current cycle end position in units of quarter-notes.
            Note that not all hosts or plugin formats may provide this value.
            @see isLooping
        */
        double ppqLoopEnd = 0;

        /** True if the transport is currently looping. */
        bool isLooping = false;

        //==============================================================================
        bool operator== (const CurrentPositionInfo& other) const noexcept
        {
            const auto tie = [] (const CurrentPositionInfo& i)
            {
                return std::tie (i.timeInSamples,
                                 i.ppqPosition,
                                 i.editOriginTime,
                                 i.ppqPositionOfLastBarStart,
                                 i.frameRate,
                                 i.isPlaying,
                                 i.isRecording,
                                 i.bpm,
                                 i.timeSigNumerator,
                                 i.timeSigDenominator,
                                 i.ppqLoopStart,
                                 i.ppqLoopEnd,
                                 i.isLooping);
            };

            return tie (*this) == tie (other);
        }

        bool operator!= (const CurrentPositionInfo& other) const noexcept
        {
            return ! operator== (other);
        }

        void resetToDefault()
        {
            *this = CurrentPositionInfo{};
        }
    };

    //==============================================================================
    /** Fills-in the given structure with details about the transport's
        position at the start of the current processing block. If this method returns
        false then the current play head position is not available and the given
        structure will be undefined.

        You can ONLY call this from your processBlock() method! Calling it at other
        times will produce undefined behaviour, as the host may not have any context
        in which a time would make sense, and some hosts will almost certainly have
        multithreading issues if it's not called on the audio thread.
    */
    virtual bool getCurrentPosition (CurrentPositionInfo& result) = 0;

    /** Returns true if this object can control the transport. */
    virtual bool canControlTransport()                         { return false; }

    /** Starts or stops the audio. */
    virtual void transportPlay (bool shouldStartPlaying)       { ignoreUnused (shouldStartPlaying); }

    /** Starts or stops recording the audio. */
    virtual void transportRecord (bool shouldStartRecording)   { ignoreUnused (shouldStartRecording); }

    /** Rewinds the audio. */
    virtual void transportRewind()                             {}
};

} // namespace juce
