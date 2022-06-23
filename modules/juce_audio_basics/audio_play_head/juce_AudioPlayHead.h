/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

    /** Describes a musical time signature.

        @see PositionInfo::getTimeSignature() PositionInfo::setTimeSignature()
    */
    struct JUCE_API TimeSignature
    {
        /** Time signature numerator, e.g. the 3 of a 3/4 time sig */
        int numerator   = 4;

        /** Time signature denominator, e.g. the 4 of a 3/4 time sig */
        int denominator = 4;

        bool operator== (const TimeSignature& other) const
        {
            const auto tie = [] (auto& x) { return std::tie (x.numerator, x.denominator); };
            return tie (*this) == tie (other);
        }

        bool operator!= (const TimeSignature& other) const
        {
            return ! operator== (other);
        }
    };

    /** Holds the begin and end points of a looped region.

        @see PositionInfo::getIsLooping() PositionInfo::setIsLooping() PositionInfo::getLoopPoints() PositionInfo::setLoopPoints()
    */
    struct JUCE_API LoopPoints
    {
        /** The current cycle start position in units of quarter-notes. */
        double ppqStart = 0;

        /** The current cycle end position in units of quarter-notes. */
        double ppqEnd = 0;

        bool operator== (const LoopPoints& other) const
        {
            const auto tie = [] (auto& x) { return std::tie (x.ppqStart, x.ppqEnd); };
            return tie (*this) == tie (other);
        }

        bool operator!= (const LoopPoints& other) const
        {
            return ! operator== (other);
        }
    };

    //==============================================================================
    /** This type is deprecated; prefer PositionInfo instead.

        Some position info may be unavailable, depending on the host or plugin format.
        Unfortunately, CurrentPositionInfo doesn't have any way of differentiating between
        default values and values that have been set explicitly.
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
    /**
        Describes the time at the start of the current audio callback.

        Not all hosts and plugin formats can provide all of the possible time
        information, so most of the getter functions in this class return
        an Optional that will only be engaged if the host provides the corresponding
        information. As a plugin developer, you should code defensively so that
        the plugin behaves sensibly even when the host fails to provide timing
        information.

        A default-constructed instance of this class will return nullopt from
        all functions that return an Optional.
    */
    class PositionInfo
    {
    public:
        /** Returns the number of samples that have elapsed. */
        Optional<int64_t> getTimeInSamples() const                      { return getOptional (flagTimeSamples, timeInSamples); }

        /** @see getTimeInSamples() */
        void setTimeInSamples (Optional<int64_t> timeInSamplesIn)       {        setOptional (flagTimeSamples, timeInSamples, timeInSamplesIn); }

        /** Returns the number of samples that have elapsed. */
        Optional<double> getTimeInSeconds() const                       { return getOptional (flagTimeSeconds, timeInSeconds); }

        /** @see getTimeInSamples() */
        void setTimeInSeconds (Optional<double> timeInSecondsIn)        {        setOptional (flagTimeSeconds, timeInSeconds, timeInSecondsIn); }

        /** Returns the bpm, if available. */
        Optional<double> getBpm() const                                 { return getOptional (flagTempo, tempoBpm); }

        /** @see getBpm() */
        void setBpm (Optional<double> bpmIn)                            {        setOptional (flagTempo, tempoBpm, bpmIn); }

        /** Returns the time signature, if available. */
        Optional<TimeSignature> getTimeSignature() const                { return getOptional (flagTimeSignature, timeSignature); }

        /** @see getTimeSignature() */
        void setTimeSignature (Optional<TimeSignature> timeSignatureIn) {        setOptional (flagTimeSignature, timeSignature, timeSignatureIn); }

        /** Returns host loop points, if available. */
        Optional<LoopPoints> getLoopPoints() const                      { return getOptional (flagLoopPoints, loopPoints); }

        /** @see getLoopPoints() */
        void setLoopPoints (Optional<LoopPoints> loopPointsIn)          {        setOptional (flagLoopPoints, loopPoints, loopPointsIn); }

        /** The number of bars since the beginning of the timeline.

            This value isn't available in all hosts or in all plugin formats.
        */
        Optional<int64_t> getBarCount() const                           { return getOptional (flagBarCount, barCount); }

        /** @see getBarCount() */
        void setBarCount (Optional<int64_t> barCountIn)                 {        setOptional (flagBarCount, barCount, barCountIn); }

        /** The position of the start of the last bar, in units of quarter-notes.

            This is the time from the start of the timeline to the start of the current
            bar, in ppq units.

            Note - this value may be unavailable on some hosts, e.g. Pro-Tools.
        */
        Optional<double> getPpqPositionOfLastBarStart() const           { return getOptional (flagLastBarStartPpq, lastBarStartPpq); }

        /** @see getPpqPositionOfLastBarStart() */
        void setPpqPositionOfLastBarStart (Optional<double> positionIn) {        setOptional (flagLastBarStartPpq, lastBarStartPpq, positionIn); }

        /** The video frame rate, if available. */
        Optional<FrameRate> getFrameRate() const                        { return getOptional (flagFrameRate, frame); }

        /** @see getFrameRate() */
        void setFrameRate (Optional<FrameRate> frameRateIn)             {        setOptional (flagFrameRate, frame, frameRateIn); }

        /** The current play position, in units of quarter-notes. */
        Optional<double> getPpqPosition() const                         { return getOptional (flagPpqPosition, positionPpq); }

        /** @see getPpqPosition() */
        void setPpqPosition (Optional<double> ppqPositionIn)            {        setOptional (flagPpqPosition, positionPpq, ppqPositionIn); }

        /** For timecode, the position of the start of the timeline, in seconds from 00:00:00:00. */
        Optional<double> getEditOriginTime() const                      { return getOptional (flagOriginTime, originTime); }

        /** @see getEditOriginTime() */
        void setEditOriginTime (Optional<double> editOriginTimeIn)      {        setOptional (flagOriginTime, originTime, editOriginTimeIn); }

        /** Get the host's callback time in nanoseconds, if available. */
        Optional<uint64_t> getHostTimeNs() const                        { return getOptional (flagHostTimeNs, hostTimeNs); }

        /** @see getHostTimeNs() */
        void setHostTimeNs (Optional<uint64_t> hostTimeNsIn)            {        setOptional (flagHostTimeNs, hostTimeNs, hostTimeNsIn); }

        /** True if the transport is currently playing. */
        bool getIsPlaying() const                                       { return getFlag (flagIsPlaying); }

        /** @see getIsPlaying() */
        void setIsPlaying (bool isPlayingIn)                            {        setFlag (flagIsPlaying, isPlayingIn); }

        /** True if the transport is currently recording.

            (When isRecording is true, then isPlaying will also be true).
        */
        bool getIsRecording() const                                     { return getFlag (flagIsRecording); }

        /** @see getIsRecording() */
        void setIsRecording (bool isRecordingIn)                        {        setFlag (flagIsRecording, isRecordingIn); }

        /** True if the transport is currently looping. */
        bool getIsLooping() const                                       { return getFlag (flagIsLooping); }

        /** @see getIsLooping() */
        void setIsLooping (bool isLoopingIn)                            {        setFlag (flagIsLooping, isLoopingIn); }

        bool operator== (const PositionInfo& other) const noexcept
        {
            const auto tie = [] (const PositionInfo& i)
            {
                return std::make_tuple (i.getTimeInSamples(),
                                        i.getTimeInSeconds(),
                                        i.getPpqPosition(),
                                        i.getEditOriginTime(),
                                        i.getPpqPositionOfLastBarStart(),
                                        i.getFrameRate(),
                                        i.getBarCount(),
                                        i.getTimeSignature(),
                                        i.getBpm(),
                                        i.getLoopPoints(),
                                        i.getHostTimeNs(),
                                        i.getIsPlaying(),
                                        i.getIsRecording(),
                                        i.getIsLooping());
            };

            return tie (*this) == tie (other);
        }

        bool operator!= (const PositionInfo& other) const noexcept
        {
            return ! operator== (other);
        }

    private:
        bool getFlag (int64_t flagToCheck) const
        {
            return (flagToCheck & flags) != 0;
        }

        void setFlag (int64_t flagToCheck, bool value)
        {
            flags = (value ? flags | flagToCheck : flags & ~flagToCheck);
        }

        template <typename Value>
        Optional<Value> getOptional (int64_t flagToCheck, Value value) const
        {
            return getFlag (flagToCheck) ? makeOptional (std::move (value)) : nullopt;
        }

        template <typename Value>
        void setOptional (int64_t flagToCheck, Value& value, Optional<Value> opt)
        {
            if (opt.hasValue())
                value = *opt;

            setFlag (flagToCheck, opt.hasValue());
        }

        enum
        {
            flagTimeSignature   = 1 << 0,
            flagLoopPoints      = 1 << 1,
            flagFrameRate       = 1 << 2,
            flagTimeSeconds     = 1 << 3,
            flagLastBarStartPpq = 1 << 4,
            flagPpqPosition     = 1 << 5,
            flagOriginTime      = 1 << 6,
            flagTempo           = 1 << 7,
            flagTimeSamples     = 1 << 8,
            flagBarCount        = 1 << 9,
            flagHostTimeNs      = 1 << 10,
            flagIsPlaying       = 1 << 11,
            flagIsRecording     = 1 << 12,
            flagIsLooping       = 1 << 13
        };

        TimeSignature timeSignature;
        LoopPoints loopPoints;
        FrameRate frame        = FrameRateType::fps23976;
        double timeInSeconds   = 0.0;
        double lastBarStartPpq = 0.0;
        double positionPpq     = 0.0;
        double originTime      = 0.0;
        double tempoBpm        = 0.0;
        int64_t timeInSamples  = 0;
        int64_t barCount       = 0;
        uint64_t hostTimeNs    = 0;
        int64_t flags          = 0;
    };

    //==============================================================================
    /** Deprecated, use getPosition() instead.

        Fills-in the given structure with details about the transport's
        position at the start of the current processing block. If this method returns
        false then the current play head position is not available and the given
        structure will be undefined.

        You can ONLY call this from your processBlock() method! Calling it at other
        times will produce undefined behaviour, as the host may not have any context
        in which a time would make sense, and some hosts will almost certainly have
        multithreading issues if it's not called on the audio thread.
    */
    [[deprecated ("Use getPosition instead. Not all hosts are able to provide all time position information; getPosition differentiates clearly between set and unset fields.")]]
    bool getCurrentPosition (CurrentPositionInfo& result)
    {
        if (const auto pos = getPosition())
        {
            result.resetToDefault();

            if (const auto sig = pos->getTimeSignature())
            {
                result.timeSigNumerator   = sig->numerator;
                result.timeSigDenominator = sig->denominator;
            }

            if (const auto loop = pos->getLoopPoints())
            {
                result.ppqLoopStart     = loop->ppqStart;
                result.ppqLoopEnd       = loop->ppqEnd;
            }

            if (const auto frame = pos->getFrameRate())
                result.frameRate = *frame;

            if (const auto timeInSeconds = pos->getTimeInSeconds())
                result.timeInSeconds = *timeInSeconds;

            if (const auto lastBarStartPpq = pos->getPpqPositionOfLastBarStart())
                result.ppqPositionOfLastBarStart = *lastBarStartPpq;

            if (const auto ppqPosition = pos->getPpqPosition())
                result.ppqPosition = *ppqPosition;

            if (const auto originTime = pos->getEditOriginTime())
                result.editOriginTime = *originTime;

            if (const auto bpm = pos->getBpm())
                result.bpm = *bpm;

            if (const auto timeInSamples = pos->getTimeInSamples())
                result.timeInSamples = *timeInSamples;

            return true;
        }

        return false;
    }

    /** Fetches details about the transport's position at the start of the current
        processing block. If this method returns nullopt then the current play head
        position is not available.

        A non-null return value just indicates that the host was able to provide
        *some* relevant timing information. Individual PositionInfo getters may
        still return nullopt.

        You can ONLY call this from your processBlock() method! Calling it at other
        times will produce undefined behaviour, as the host may not have any context
        in which a time would make sense, and some hosts will almost certainly have
        multithreading issues if it's not called on the audio thread.
    */
    virtual Optional<PositionInfo> getPosition() const = 0;

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
