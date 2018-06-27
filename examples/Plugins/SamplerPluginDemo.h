/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             SamplerPlugin
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Sampler audio plugin.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        SamplerAudioProcessor

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <tuple>
#include <iomanip>
#include <sstream>
#include <functional>
#include <mutex>

namespace IDs
{

#define DECLARE_ID(name) const juce::Identifier name (#name);

DECLARE_ID (DATA_MODEL)
DECLARE_ID (sampleReader)
DECLARE_ID (centreFrequencyHz)
DECLARE_ID (loopMode)
DECLARE_ID (loopPointsSeconds)

DECLARE_ID (MPE_SETTINGS)
DECLARE_ID (synthVoices)
DECLARE_ID (voiceStealingEnabled)
DECLARE_ID (legacyModeEnabled)
DECLARE_ID (mpeZoneLayout)
DECLARE_ID (legacyFirstChannel)
DECLARE_ID (legacyLastChannel)
DECLARE_ID (legacyPitchbendRange)

DECLARE_ID (VISIBLE_RANGE)
DECLARE_ID (totalRange)
DECLARE_ID (visibleRange)

#undef DECLARE_ID

} // namespace IDs

enum class LoopMode
{
    none,
    forward,
    pingpong
};

template <typename Movable>
class MoveOnlyFifo final
{
public:
    explicit MoveOnlyFifo (int size)
        : buffer (size),
          abstractFifo (size)
    {}

    MoveOnlyFifo()
        : MoveOnlyFifo (1024)
    {}

    template <typename Convertible>
    Convertible push (Convertible item) noexcept
    {
        auto writer = abstractFifo.write (1);

        if (writer.blockSize1 == 1)
        {
            buffer[writer.startIndex1] = move (item);
            item = {};
        }
        else if (writer.blockSize2 == 1)
        {
            buffer[writer.startIndex2] = move (item);
            item = {};
        }

        return item;
    }

    Movable pop() noexcept
    {
        auto reader = abstractFifo.read (1);

        if (reader.blockSize1 == 1)
            return move (buffer[reader.startIndex1]);

        if (reader.blockSize2 == 1)
            return move (buffer[reader.startIndex2]);

        return {};
    }

private:
    std::vector<Movable> buffer;
    AbstractFifo abstractFifo;
};

//==============================================================================
// Represents the constant parts of an audio sample: its name, sample rate,
// length, and the audio sample data itself.
// Samples might be pretty big, so we'll keep shared_ptrs to them most of the
// time, to reduce duplication and copying.
class Sample final
{
public:
    Sample (AudioFormatReader& source, double maxSampleLengthSecs)
        : sourceSampleRate (source.sampleRate),
          length (jmin (int (source.lengthInSamples),
                        int (maxSampleLengthSecs * sourceSampleRate))),
          data (jmin (2, int (source.numChannels)), length + 4)
    {
        if (length == 0)
            throw std::runtime_error ("Unable to load sample");

        source.read (&data, 0, length + 4, 0, true, true);
    }

    double getSampleRate() const                    { return sourceSampleRate; }
    int getLength() const                           { return length; }
    const AudioBuffer<float>& getBuffer() const     { return data; }

private:
    double sourceSampleRate;
    int length;
    AudioBuffer<float> data;
};

//==============================================================================
// A class which contains all the information related to sample-playback, such
// as sample data, loop points, and loop kind.
// It is expected that multiple sampler voices will maintain pointers to a
// single instance of this class, to avoid redundant duplication of sample
// data in memory.
class MPESamplerSound final
{
public:
    void setSample (std::unique_ptr<Sample> value)
    {
        sample = move (value);
        setLoopPointsInSeconds (loopPoints);
    }

    Sample* getSample() const
    {
        return sample.get();
    }

    void setLoopPointsInSeconds (Range<double> value)
    {
        loopPoints = sample == nullptr ? value
                                       : Range<double> (0, sample->getLength() / sample->getSampleRate())
                                                        .constrainRange (value);
    }

    Range<double> getLoopPointsInSeconds() const
    {
        return loopPoints;
    }

    void setCentreFrequencyInHz (double centre)
    {
        centreFrequencyInHz = centre;
    }

    double getCentreFrequencyInHz() const
    {
        return centreFrequencyInHz;
    }

    void setLoopMode (LoopMode type)
    {
        loopMode = type;
    }

    LoopMode getLoopMode() const
    {
        return loopMode;
    }

private:
    std::unique_ptr<Sample> sample;
    double centreFrequencyInHz { 440.0 };
    Range<double> loopPoints;
    LoopMode loopMode { LoopMode::none };
};

//==============================================================================
class MPESamplerVoice  : public MPESynthesiserVoice
{
public:
    explicit MPESamplerVoice (std::shared_ptr<const MPESamplerSound> sound)
        : samplerSound (move (sound))
    {
        jassert (samplerSound != nullptr);
    }

    void noteStarted() override
    {
        jassert (currentlyPlayingNote.isValid());
        jassert (currentlyPlayingNote.keyState == MPENote::keyDown
              || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);

        level    .setValue (currentlyPlayingNote.pressure.asUnsignedFloat());
        frequency.setValue (currentlyPlayingNote.getFrequencyInHertz());

        auto loopPoints = samplerSound->getLoopPointsInSeconds();
        loopBegin.setValue (loopPoints.getStart() * samplerSound->getSample()->getSampleRate());
        loopEnd  .setValue (loopPoints.getEnd()   * samplerSound->getSample()->getSampleRate());

        for (auto smoothed : { &level, &frequency, &loopBegin, &loopEnd })
            smoothed->reset (currentSampleRate, smoothingLengthInSeconds);

        currentSamplePos = 0.0;
        tailOff          = 0.0;
    }

    void noteStopped (bool allowTailOff) override
    {
        jassert (currentlyPlayingNote.keyState == MPENote::off);

        if (allowTailOff && tailOff == 0.0)
            tailOff = 1.0;
        else
            stopNote();
    }

    void notePressureChanged() override
    {
        level.setValue (currentlyPlayingNote.pressure.asUnsignedFloat());
    }

    void notePitchbendChanged() override
    {
        frequency.setValue (currentlyPlayingNote.getFrequencyInHertz());
    }

    void noteTimbreChanged()   override {}
    void noteKeyStateChanged() override {}

    void renderNextBlock (AudioBuffer<float>& outputBuffer,
                          int startSample,
                          int numSamples) override
    {
        jassert (samplerSound->getSample() != nullptr);

        auto loopPoints = samplerSound->getLoopPointsInSeconds();
        loopBegin.setValue (loopPoints.getStart() * samplerSound->getSample()->getSampleRate());
        loopEnd  .setValue (loopPoints.getEnd()   * samplerSound->getSample()->getSampleRate());

        auto& data = samplerSound->getSample()->getBuffer();

        auto inL = data.getReadPointer (0);
        auto inR = data.getNumChannels() > 1 ? data.getReadPointer (1) : nullptr;

        auto outL = outputBuffer.getWritePointer (0, startSample);

        if (outL == nullptr)
            return;

        auto outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample)
                                                      : nullptr;

        size_t writePos = 0;

        while (--numSamples >= 0 && renderNextSample (inL, inR, outL, outR, writePos))
            writePos += 1;
    }

    double getCurrentSamplePosition() const
    {
        return currentSamplePos;
    }

private:
    bool renderNextSample (const float* inL,
                           const float* inR,
                           float* outL,
                           float* outR,
                           size_t writePos)
    {
        auto currentLevel     = level.getNextValue();
        auto currentFrequency = frequency.getNextValue();
        auto currentLoopBegin = loopBegin.getNextValue();
        auto currentLoopEnd   = loopEnd.getNextValue();

        if (isTailingOff())
        {
            currentLevel *= tailOff;
            tailOff *= 0.9999;

            if (tailOff < 0.005)
            {
                stopNote();
                return false;
            }
        }

        auto pos      = (int) currentSamplePos;
        auto nextPos  = pos + 1;
        auto alpha    = (float) (currentSamplePos - pos);
        auto invAlpha = 1.0f - alpha;

        // just using a very simple linear interpolation here..
        auto l = static_cast<float> (currentLevel * (inL[pos] * invAlpha + inL[nextPos] * alpha));
        auto r = static_cast<float> ((inR != nullptr) ? currentLevel * (inR[pos] * invAlpha + inR[nextPos] * alpha)
                                                      : l);

        if (outR != nullptr)
        {
            outL[writePos] += l;
            outR[writePos] += r;
        }
        else
        {
            outL[writePos] += (l + r) * 0.5f;
        }

        std::tie (currentSamplePos, currentDirection) = getNextState (currentFrequency,
                                                                      currentLoopBegin,
                                                                      currentLoopEnd);

        if (currentSamplePos > samplerSound->getSample()->getLength())
        {
            stopNote();
            return false;
        }

        return true;
    }

    double getSampleValue() const;

    bool isTailingOff() const
    {
        return tailOff != 0.0;
    }

    void stopNote()
    {
        clearCurrentNote();
        currentSamplePos = 0.0;
    }

    enum class Direction
    {
        forward,
        backward
    };

    std::tuple<double, Direction> getNextState (double freq,
                                                double begin,
                                                double end) const
    {
        auto nextPitchRatio = freq / samplerSound->getCentreFrequencyInHz();

        auto nextSamplePos = currentSamplePos;
        auto nextDirection = currentDirection;

        // Move the current sample pos in the correct direction
        switch (currentDirection)
        {
            case Direction::forward:
                nextSamplePos += nextPitchRatio;
                break;

            case Direction::backward:
                nextSamplePos -= nextPitchRatio;
                break;
        }

        // Update current sample position, taking loop mode into account
        // If the loop mode was changed while we were travelling backwards, deal
        // with it gracefully.
        if (nextDirection == Direction::backward && nextSamplePos < begin)
        {
            nextSamplePos = begin;
            nextDirection = Direction::forward;

            return { nextSamplePos, nextDirection };
        }

        if (samplerSound->getLoopMode() == LoopMode::none)
            return { nextSamplePos, nextDirection };

        if (nextDirection == Direction::forward && end < nextSamplePos && !isTailingOff())
        {
            if (samplerSound->getLoopMode() == LoopMode::forward)
                nextSamplePos = begin;
            else if (samplerSound->getLoopMode() == LoopMode::pingpong)
            {
                nextSamplePos = end;
                nextDirection = Direction::backward;
            }
        }
        return { nextSamplePos, nextDirection };
    }

    std::shared_ptr<const MPESamplerSound> samplerSound;
    LinearSmoothedValue<double> level { 0 };
    LinearSmoothedValue<double> frequency { 0 };
    LinearSmoothedValue<double> loopBegin;
    LinearSmoothedValue<double> loopEnd;
    double currentSamplePos { 0 };
    double tailOff { 0 };
    Direction currentDirection { Direction::forward };
    double smoothingLengthInSeconds { 0.01 };
};

template <typename Contents>
class ReferenceCountingAdapter  : public ReferenceCountedObject
{
public:
    template <typename... Args>
    explicit ReferenceCountingAdapter (Args&&... args)
        : contents (std::forward<Args> (args)...)
    {}

    const Contents& get() const
    {
        return contents;
    }

    Contents& get()
    {
        return contents;
    }

private:
    Contents contents;
};

template <typename Contents, typename... Args>
std::unique_ptr<ReferenceCountingAdapter<Contents>>
make_reference_counted (Args&&... args)
{
    auto adapter = new ReferenceCountingAdapter<Contents> (std::forward<Args> (args)...);
    return std::unique_ptr<ReferenceCountingAdapter<Contents>> (adapter);
}

//==============================================================================
inline std::unique_ptr<AudioFormatReader> makeAudioFormatReader (AudioFormatManager& manager,
                                                                 const void* sampleData,
                                                                 size_t dataSize)
{
    return std::unique_ptr<AudioFormatReader> (manager.createReaderFor (new MemoryInputStream (sampleData,
                                                                                               dataSize,
                                                                                               false)));
}

inline std::unique_ptr<AudioFormatReader> makeAudioFormatReader (AudioFormatManager& manager,
                                                                 const File& file)
{
    return std::unique_ptr<AudioFormatReader> (manager.createReaderFor (file));
}

//==============================================================================
class AudioFormatReaderFactory
{
public:
    virtual ~AudioFormatReaderFactory() noexcept = default;
    virtual std::unique_ptr<AudioFormatReader> make (AudioFormatManager&) const = 0;
    virtual std::unique_ptr<AudioFormatReaderFactory> clone() const = 0;
};

//==============================================================================
class MemoryAudioFormatReaderFactory  : public AudioFormatReaderFactory
{
public:
    MemoryAudioFormatReaderFactory (const void* sampleData, size_t dataSize)
        : sampleData (sampleData),
          dataSize (dataSize)
    {}

    std::unique_ptr<AudioFormatReader> make (AudioFormatManager&manager ) const override
    {
        return makeAudioFormatReader (manager, sampleData, dataSize);
    }

    std::unique_ptr<AudioFormatReaderFactory> clone() const override
    {
        return std::unique_ptr<AudioFormatReaderFactory> (new MemoryAudioFormatReaderFactory (*this));
    }

private:
    const void* sampleData;
    size_t dataSize;
};

//==============================================================================
class FileAudioFormatReaderFactory  : public AudioFormatReaderFactory
{
public:
    explicit FileAudioFormatReaderFactory (File file)
        : file (std::move (file))
    {}

    std::unique_ptr<AudioFormatReader> make (AudioFormatManager& manager) const override
    {
        return makeAudioFormatReader (manager, file);
    }

    std::unique_ptr<AudioFormatReaderFactory> clone() const override
    {
        return std::unique_ptr<AudioFormatReaderFactory> (new FileAudioFormatReaderFactory (*this));
    }

private:
    File file;
};

namespace juce
{

bool operator== (const MPEZoneLayout& a, const MPEZoneLayout& b)
{
    if (a.getLowerZone() != b.getLowerZone())
        return false;

    if (a.getUpperZone() != b.getUpperZone())
        return false;

    return true;
}

bool operator!= (const MPEZoneLayout& a, const MPEZoneLayout& b)
{
    return ! (a == b);
}

template<>
struct VariantConverter<LoopMode>
{
    static LoopMode fromVar (const var& v)
    {
        return static_cast<LoopMode> (int (v));
    }

    static var toVar (LoopMode loopMode)
    {
        return static_cast<int> (loopMode);
    }
};

template <typename Wrapped>
struct GenericVariantConverter
{
    static Wrapped fromVar (const var& v)
    {
        auto cast = dynamic_cast<ReferenceCountingAdapter<Wrapped>*> (v.getObject());
        jassert (cast != nullptr);
        return cast->get();
    }

    static var toVar (Wrapped range)
    {
        return { make_reference_counted<Wrapped> (std::move (range)).release() };
    }
};

template <typename Numeric>
struct VariantConverter<Range<Numeric>>  : GenericVariantConverter<Range<Numeric>> {};

template<>
struct VariantConverter<MPEZoneLayout>  : GenericVariantConverter<MPEZoneLayout> {};

template<>
struct VariantConverter<std::shared_ptr<AudioFormatReaderFactory>>
    : GenericVariantConverter<std::shared_ptr<AudioFormatReaderFactory>>
{};

} // namespace juce

//==============================================================================
class VisibleRangeDataModel  : public ValueTree::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() noexcept = default;
        virtual void totalRangeChanged   (Range<double>) {}
        virtual void visibleRangeChanged (Range<double>) {}
    };

    VisibleRangeDataModel()
        : VisibleRangeDataModel (ValueTree (IDs::VISIBLE_RANGE))
    {}

    explicit VisibleRangeDataModel (const ValueTree& vt)
        : valueTree (vt),
          totalRange   (valueTree, IDs::totalRange,   nullptr),
          visibleRange (valueTree, IDs::visibleRange, nullptr)
    {
        jassert (valueTree.hasType (IDs::VISIBLE_RANGE));
        valueTree.addListener (this);
    }

    VisibleRangeDataModel (const VisibleRangeDataModel& other)
        : VisibleRangeDataModel (other.valueTree)
    {}

    VisibleRangeDataModel& operator= (const VisibleRangeDataModel& other)
    {
        auto copy (other);
        swap (copy);
        return *this;
    }

    Range<double> getTotalRange() const
    {
        return totalRange;
    }

    void setTotalRange (Range<double> value, UndoManager* undoManager)
    {
        totalRange.setValue (value, undoManager);
        setVisibleRange (visibleRange, undoManager);
    }

    Range<double> getVisibleRange() const
    {
        return visibleRange;
    }

    void setVisibleRange (Range<double> value, UndoManager* undoManager)
    {
        visibleRange.setValue (totalRange.get().constrainRange (value), undoManager);
    }

    void addListener (Listener& listener)
    {
        listenerList.add (&listener);
    }

    void removeListener (Listener& listener)
    {
        listenerList.remove (&listener);
    }

    void swap (VisibleRangeDataModel& other) noexcept
    {
        using std::swap;
        swap (other.valueTree, valueTree);
    }

private:
    void valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (property == IDs::totalRange)
        {
            totalRange.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.totalRangeChanged (totalRange); });
        }
        else if (property == IDs::visibleRange)
        {
            visibleRange.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.visibleRangeChanged (visibleRange); });
        }
    }

    void valueTreeChildAdded        (ValueTree&, ValueTree&)      override { jassertfalse; }
    void valueTreeChildRemoved      (ValueTree&, ValueTree&, int) override { jassertfalse; }
    void valueTreeChildOrderChanged (ValueTree&, int, int)        override { jassertfalse; }
    void valueTreeParentChanged     (ValueTree&)                  override { jassertfalse; }

    ValueTree valueTree;

    CachedValue<Range<double>> totalRange;
    CachedValue<Range<double>> visibleRange;

    ListenerList<Listener> listenerList;
};

//==============================================================================
class MPESettingsDataModel  : public ValueTree::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() noexcept = default;
        virtual void synthVoicesChanged (int) {}
        virtual void voiceStealingEnabledChanged (bool) {}
        virtual void legacyModeEnabledChanged (bool) {}
        virtual void mpeZoneLayoutChanged (const MPEZoneLayout&) {}
        virtual void legacyFirstChannelChanged (int) {}
        virtual void legacyLastChannelChanged (int) {}
        virtual void legacyPitchbendRangeChanged (int) {}
    };

    MPESettingsDataModel()
        : MPESettingsDataModel (ValueTree (IDs::MPE_SETTINGS))
    {}

    explicit MPESettingsDataModel (const ValueTree& vt)
        : valueTree (vt),
          synthVoices          (valueTree, IDs::synthVoices,          nullptr, 15),
          voiceStealingEnabled (valueTree, IDs::voiceStealingEnabled, nullptr, false),
          legacyModeEnabled    (valueTree, IDs::legacyModeEnabled,    nullptr, true),
          mpeZoneLayout        (valueTree, IDs::mpeZoneLayout,        nullptr, {}),
          legacyFirstChannel   (valueTree, IDs::legacyFirstChannel,   nullptr, 1),
          legacyLastChannel    (valueTree, IDs::legacyLastChannel,    nullptr, 15),
          legacyPitchbendRange (valueTree, IDs::legacyPitchbendRange, nullptr, 48)
    {
        jassert (valueTree.hasType (IDs::MPE_SETTINGS));
        valueTree.addListener (this);
    }

    MPESettingsDataModel (const MPESettingsDataModel& other)
        : MPESettingsDataModel (other.valueTree)
    {}

    MPESettingsDataModel& operator= (const MPESettingsDataModel& other)
    {
        auto copy (other);
        swap (copy);
        return *this;
    }

    int getSynthVoices() const
    {
        return synthVoices;
    }

    void setSynthVoices (int value, UndoManager* undoManager)
    {
        synthVoices.setValue (Range<int> (1, 20).clipValue (value), undoManager);
    }

    bool getVoiceStealingEnabled() const
    {
        return voiceStealingEnabled;
    }

    void setVoiceStealingEnabled (bool value, UndoManager* undoManager)
    {
        voiceStealingEnabled.setValue (value, undoManager);
    }

    bool getLegacyModeEnabled() const
    {
        return legacyModeEnabled;
    }

    void setLegacyModeEnabled (bool value, UndoManager* undoManager)
    {
        legacyModeEnabled.setValue (value, undoManager);
    }

    MPEZoneLayout getMPEZoneLayout() const
    {
        return mpeZoneLayout;
    }

    void setMPEZoneLayout (MPEZoneLayout value, UndoManager* undoManager)
    {
        mpeZoneLayout.setValue (value, undoManager);
    }

    int getLegacyFirstChannel() const
    {
        return legacyFirstChannel;
    }

    void setLegacyFirstChannel (int value, UndoManager* undoManager)
    {
        legacyFirstChannel.setValue (Range<int> (1, legacyLastChannel).clipValue (value), undoManager);
    }

    int getLegacyLastChannel() const
    {
        return legacyLastChannel;
    }

    void setLegacyLastChannel (int value, UndoManager* undoManager)
    {
        legacyLastChannel.setValue (Range<int> (legacyFirstChannel, 15).clipValue (value), undoManager);
    }

    int getLegacyPitchbendRange() const
    {
        return legacyPitchbendRange;
    }

    void setLegacyPitchbendRange (int value, UndoManager* undoManager)
    {
        legacyPitchbendRange.setValue (Range<int> (0, 95).clipValue (value), undoManager);
    }

    void addListener (Listener& listener)
    {
        listenerList.add (&listener);
    }

    void removeListener (Listener& listener)
    {
        listenerList.remove (&listener);
    }

    void swap (MPESettingsDataModel& other) noexcept
    {
        using std::swap;
        swap (other.valueTree, valueTree);
    }

private:
    void valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (property == IDs::synthVoices)
        {
            synthVoices.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.synthVoicesChanged (synthVoices); });
        }
        else if (property == IDs::voiceStealingEnabled)
        {
            voiceStealingEnabled.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.voiceStealingEnabledChanged (voiceStealingEnabled); });
        }
        else if (property == IDs::legacyModeEnabled)
        {
            legacyModeEnabled.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyModeEnabledChanged (legacyModeEnabled); });
        }
        else if (property == IDs::mpeZoneLayout)
        {
            mpeZoneLayout.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.mpeZoneLayoutChanged (mpeZoneLayout); });
        }
        else if (property == IDs::legacyFirstChannel)
        {
            legacyFirstChannel.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyFirstChannelChanged (legacyFirstChannel); });
        }
        else if (property == IDs::legacyLastChannel)
        {
            legacyLastChannel.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyLastChannelChanged (legacyLastChannel); });
        }
        else if (property == IDs::legacyPitchbendRange)
        {
            legacyPitchbendRange.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyPitchbendRangeChanged (legacyPitchbendRange); });
        }
    }

    void valueTreeChildAdded        (ValueTree&, ValueTree&)      override { jassertfalse; }
    void valueTreeChildRemoved      (ValueTree&, ValueTree&, int) override { jassertfalse; }
    void valueTreeChildOrderChanged (ValueTree&, int, int)        override { jassertfalse; }
    void valueTreeParentChanged     (ValueTree&)                  override { jassertfalse; }

    ValueTree valueTree;

    CachedValue<int> synthVoices;
    CachedValue<bool> voiceStealingEnabled;
    CachedValue<bool> legacyModeEnabled;
    CachedValue<MPEZoneLayout> mpeZoneLayout;
    CachedValue<int> legacyFirstChannel;
    CachedValue<int> legacyLastChannel;
    CachedValue<int> legacyPitchbendRange;

    ListenerList<Listener> listenerList;
};

//==============================================================================
class DataModel  : public ValueTree::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() noexcept = default;
        virtual void sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory>) {}
        virtual void centreFrequencyHzChanged (double) {}
        virtual void loopModeChanged (LoopMode) {}
        virtual void loopPointsSecondsChanged (Range<double>) {}
    };

    explicit DataModel (AudioFormatManager& audioFormatManager)
        : DataModel (audioFormatManager, ValueTree (IDs::DATA_MODEL))
    {}

    DataModel (AudioFormatManager& audioFormatManager, const ValueTree& vt)
        : audioFormatManager (&audioFormatManager),
          valueTree (vt),
          sampleReader      (valueTree, IDs::sampleReader,      nullptr),
          centreFrequencyHz (valueTree, IDs::centreFrequencyHz, nullptr),
          loopMode          (valueTree, IDs::loopMode,          nullptr, LoopMode::none),
          loopPointsSeconds (valueTree, IDs::loopPointsSeconds, nullptr)
    {
        jassert (valueTree.hasType (IDs::DATA_MODEL));
        valueTree.addListener (this);
    }

    DataModel (const DataModel& other)
        : DataModel (*other.audioFormatManager, other.valueTree)
    {}

    DataModel& operator= (const DataModel& other)
    {
        auto copy (other);
        swap (copy);
        return *this;
    }

    std::unique_ptr<AudioFormatReader> getSampleReader() const
    {
        return sampleReader != nullptr ? sampleReader.get()->make (*audioFormatManager) : nullptr;
    }

    void setSampleReader (std::unique_ptr<AudioFormatReaderFactory> readerFactory,
                          UndoManager* undoManager)
    {
        sampleReader.setValue (move (readerFactory), undoManager);
        setLoopPointsSeconds (Range<double> (0, getSampleLengthSeconds()).constrainRange (loopPointsSeconds),
                              undoManager);
    }

    double getSampleLengthSeconds() const
    {
        if (auto r = getSampleReader())
            return r->lengthInSamples / r->sampleRate;

        return 1.0;
    }

    double getCentreFrequencyHz() const
    {
        return centreFrequencyHz;
    }

    void setCentreFrequencyHz (double value, UndoManager* undoManager)
    {
        centreFrequencyHz.setValue (Range<double> (20, 20000).clipValue (value),
                                    undoManager);
    }

    LoopMode getLoopMode() const
    {
        return loopMode;
    }

    void setLoopMode (LoopMode value, UndoManager* undoManager)
    {
        loopMode.setValue (value, undoManager);
    }

    Range<double> getLoopPointsSeconds() const
    {
        return loopPointsSeconds;
    }

    void setLoopPointsSeconds (Range<double> value, UndoManager* undoManager)
    {
        loopPointsSeconds.setValue (Range<double> (0, getSampleLengthSeconds()).constrainRange (value),
                                    undoManager);
    }

    MPESettingsDataModel mpeSettings()
    {
        return MPESettingsDataModel (valueTree.getOrCreateChildWithName (IDs::MPE_SETTINGS, nullptr));
    }

    void addListener (Listener& listener)
    {
        listenerList.add (&listener);
    }

    void removeListener (Listener& listener)
    {
        listenerList.remove (&listener);
    }

    void swap (DataModel& other) noexcept
    {
        using std::swap;
        swap (other.valueTree, valueTree);
    }

    AudioFormatManager& getAudioFormatManager() const
    {
        return *audioFormatManager;
    }

private:
    void valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (property == IDs::sampleReader)
        {
            sampleReader.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.sampleReaderChanged (sampleReader); });
        }
        else if (property == IDs::centreFrequencyHz)
        {
            centreFrequencyHz.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.centreFrequencyHzChanged (centreFrequencyHz); });
        }
        else if (property == IDs::loopMode)
        {
            loopMode.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.loopModeChanged (loopMode); });
        }
        else if (property == IDs::loopPointsSeconds)
        {
            loopPointsSeconds.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.loopPointsSecondsChanged (loopPointsSeconds); });
        }
    }

    void valueTreeChildAdded        (ValueTree&, ValueTree&)      override {}
    void valueTreeChildRemoved      (ValueTree&, ValueTree&, int) override { jassertfalse; }
    void valueTreeChildOrderChanged (ValueTree&, int, int)        override { jassertfalse; }
    void valueTreeParentChanged     (ValueTree&)                  override { jassertfalse; }

    AudioFormatManager* audioFormatManager;

    ValueTree valueTree;

    CachedValue<std::shared_ptr<AudioFormatReaderFactory>> sampleReader;
    CachedValue<double> centreFrequencyHz;
    CachedValue<LoopMode> loopMode;
    CachedValue<Range<double>> loopPointsSeconds;

    ListenerList<Listener> listenerList;
};

namespace
{
void initialiseComboBoxWithConsecutiveIntegers (Component& owner,
                                                ComboBox& comboBox,
                                                Label& label,
                                                int firstValue,
                                                int numValues,
                                                int valueToSelect)
{
    for (auto i = 0; i < numValues; ++i)
        comboBox.addItem (String (i + firstValue), i + 1);

    comboBox.setSelectedId (valueToSelect - firstValue + 1);

    label.attachToComponent (&comboBox, true);
    owner.addAndMakeVisible (comboBox);
}

constexpr int controlHeight     = 24;
constexpr int controlSeparation = 6;

} // namespace

//==============================================================================
class MPELegacySettingsComponent final  : public Component,
                                          public MPESettingsDataModel::Listener
{
public:
    explicit MPELegacySettingsComponent (const MPESettingsDataModel& model,
                                         UndoManager& um)
        : dataModel (model),
          undoManager (&um)
    {
        dataModel.addListener (*this);

        initialiseComboBoxWithConsecutiveIntegers (*this, legacyStartChannel, legacyStartChannelLabel, 1, 16, 1);
        initialiseComboBoxWithConsecutiveIntegers (*this, legacyEndChannel, legacyEndChannelLabel, 1, 16, 16);
        initialiseComboBoxWithConsecutiveIntegers (*this, legacyPitchbendRange, legacyPitchbendRangeLabel, 0, 96, 2);

        legacyStartChannel.onChange = [this]
        {
            if (isLegacyModeValid())
            {
                undoManager->beginNewTransaction();
                dataModel.setLegacyFirstChannel (getFirstChannel(), undoManager);
            }
        };

        legacyEndChannel.onChange = [this]
        {
            if (isLegacyModeValid())
            {
                undoManager->beginNewTransaction();
                dataModel.setLegacyLastChannel (getLastChannel(), undoManager);
            }
        };

        legacyPitchbendRange.onChange = [this]
        {
            if (isLegacyModeValid())
            {
                undoManager->beginNewTransaction();
                dataModel.setLegacyPitchbendRange (legacyPitchbendRange.getText().getIntValue(), undoManager);
            }
        };
    }

    int getMinHeight() const
    {
        return (controlHeight * 3) + (controlSeparation * 2);
    }

private:
    void resized() override
    {
        Rectangle<int> r (proportionOfWidth (0.65f), 0, proportionOfWidth (0.25f), getHeight());

        for (auto& comboBox : { &legacyStartChannel, &legacyEndChannel, &legacyPitchbendRange })
        {
            comboBox->setBounds (r.removeFromTop (controlHeight));
            r.removeFromTop (controlSeparation);
        }
    }

    bool isLegacyModeValid() const
    {
        if (! areLegacyModeParametersValid())
        {
            handleInvalidLegacyModeParameters();
            return false;
        }

        return true;
    }

    void legacyFirstChannelChanged (int value) override
    {
        legacyStartChannel.setSelectedId (value, dontSendNotification);
    }

    void legacyLastChannelChanged (int value) override
    {
        legacyEndChannel.setSelectedId (value, dontSendNotification);
    }

    void legacyPitchbendRangeChanged (int value) override
    {
        legacyPitchbendRange.setSelectedId (value + 1, dontSendNotification);
    }

    int getFirstChannel() const
    {
        return legacyStartChannel.getText().getIntValue();
    }

    int getLastChannel() const
    {
        return legacyEndChannel.getText().getIntValue();
    }

    bool areLegacyModeParametersValid() const
    {
        return getFirstChannel() <= getLastChannel();
    }

    void handleInvalidLegacyModeParameters() const
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Invalid legacy mode channel layout",
                                          "Cannot set legacy mode start/end channel:\n"
                                          "The end channel must not be less than the start channel!",
                                          "Got it");
    }

    MPESettingsDataModel dataModel;

    ComboBox legacyStartChannel, legacyEndChannel, legacyPitchbendRange;

    Label legacyStartChannelLabel   { {}, "First channel" },
          legacyEndChannelLabel     { {}, "Last channel" },
          legacyPitchbendRangeLabel { {}, "Pitchbend range (semitones)" };

    UndoManager* undoManager;
};

//==============================================================================
class MPENewSettingsComponent final  : public Component,
                                       public MPESettingsDataModel::Listener
{
public:
    MPENewSettingsComponent (const MPESettingsDataModel& model,
                             UndoManager& um)
        : dataModel (model),
          undoManager (&um)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (isLowerZoneButton);
        isLowerZoneButton.setToggleState (true, NotificationType::dontSendNotification);

        initialiseComboBoxWithConsecutiveIntegers (*this, memberChannels, memberChannelsLabel, 0, 16, 15);
        initialiseComboBoxWithConsecutiveIntegers (*this, masterPitchbendRange, masterPitchbendRangeLabel, 0, 96, 2);
        initialiseComboBoxWithConsecutiveIntegers (*this, notePitchbendRange, notePitchbendRangeLabel, 0, 96, 48);

        for (auto& button : { &setZoneButton, &clearAllZonesButton })
            addAndMakeVisible (button);

        setZoneButton.onClick = [this]
        {
            auto isLowerZone = isLowerZoneButton.getToggleState();
            auto numMemberChannels = memberChannels.getText().getIntValue();
            auto perNotePb = notePitchbendRange.getText().getIntValue();
            auto masterPb = masterPitchbendRange.getText().getIntValue();

            if (isLowerZone)
                zoneLayout.setLowerZone (numMemberChannels, perNotePb, masterPb);
            else
                zoneLayout.setUpperZone (numMemberChannels, perNotePb, masterPb);

            undoManager->beginNewTransaction();
            dataModel.setMPEZoneLayout (zoneLayout, undoManager);
        };

        clearAllZonesButton.onClick = [this]
        {
            zoneLayout.clearAllZones();
            undoManager->beginNewTransaction();
            dataModel.setMPEZoneLayout (zoneLayout, undoManager);
        };
    }

    int getMinHeight() const
    {
        return (controlHeight * 6) + (controlSeparation * 6);
    }

private:
    void resized() override
    {
        Rectangle<int> r (proportionOfWidth (0.65f), 0, proportionOfWidth (0.25f), getHeight());

        isLowerZoneButton.setBounds (r.removeFromTop (controlHeight));
        r.removeFromTop (controlSeparation);

        for (auto& comboBox : { &memberChannels, &masterPitchbendRange, &notePitchbendRange })
        {
            comboBox->setBounds (r.removeFromTop (controlHeight));
            r.removeFromTop (controlSeparation);
        }

        r.removeFromTop (controlSeparation);

        auto buttonLeft = proportionOfWidth (0.5f);

        setZoneButton.setBounds (r.removeFromTop (controlHeight).withLeft (buttonLeft));
        r.removeFromTop (controlSeparation);
        clearAllZonesButton.setBounds (r.removeFromTop (controlHeight).withLeft (buttonLeft));
    }

    void mpeZoneLayoutChanged (const MPEZoneLayout& value) override
    {
        zoneLayout = value;
    }

    MPESettingsDataModel dataModel;
    MPEZoneLayout zoneLayout;

    ComboBox memberChannels, masterPitchbendRange, notePitchbendRange;

    ToggleButton isLowerZoneButton  { "Lower zone" };

    Label memberChannelsLabel       { {}, "Nr. of member channels" },
          masterPitchbendRangeLabel { {}, "Master pitchbend range (semitones)" },
          notePitchbendRangeLabel   { {}, "Note pitchbend range (semitones)" };

    TextButton setZoneButton       { "Set zone" },
               clearAllZonesButton { "Clear all zones" };

    UndoManager* undoManager;
};

//==============================================================================
class MPESettingsComponent final  : public Component,
                                    public MPESettingsDataModel::Listener
{
public:
    MPESettingsComponent (const MPESettingsDataModel& model,
                          UndoManager& um)
        : dataModel (model),
          legacySettings (dataModel, um),
          newSettings (dataModel, um),
          undoManager (&um)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (newSettings);
        addChildComponent (legacySettings);

        initialiseComboBoxWithConsecutiveIntegers (*this, numberOfVoices, numberOfVoicesLabel, 1, 20, 15);
        numberOfVoices.onChange = [this]
        {
            undoManager->beginNewTransaction();
            dataModel.setSynthVoices (numberOfVoices.getText().getIntValue(), undoManager);
        };

        for (auto& button : { &legacyModeEnabledToggle, &voiceStealingEnabledToggle })
        {
            addAndMakeVisible (button);
        }

        legacyModeEnabledToggle.onClick = [this]
        {
            undoManager->beginNewTransaction();
            dataModel.setLegacyModeEnabled (legacyModeEnabledToggle.getToggleState(), undoManager);
        };

        voiceStealingEnabledToggle.onClick = [this]
        {
            undoManager->beginNewTransaction();
            dataModel.setVoiceStealingEnabled (voiceStealingEnabledToggle.getToggleState(), undoManager);
        };
    }

private:
    void resized() override
    {
        auto topHeight = jmax (legacySettings.getMinHeight(), newSettings.getMinHeight());
        auto r = getLocalBounds();
        r.removeFromTop (15);
        auto top = r.removeFromTop (topHeight);
        legacySettings.setBounds (top);
        newSettings.setBounds (top);

        r.removeFromLeft (proportionOfWidth (0.65f));
        r = r.removeFromLeft (proportionOfWidth (0.25f));

        auto toggleLeft = proportionOfWidth (0.25f);

        legacyModeEnabledToggle.setBounds (r.removeFromTop (controlHeight).withLeft (toggleLeft));
        r.removeFromTop (controlSeparation);
        voiceStealingEnabledToggle.setBounds (r.removeFromTop (controlHeight).withLeft (toggleLeft));
        r.removeFromTop (controlSeparation);
        numberOfVoices.setBounds (r.removeFromTop (controlHeight));
    }

    void legacyModeEnabledChanged (bool value) override
    {
        legacySettings.setVisible (value);
        newSettings.setVisible (! value);
        legacyModeEnabledToggle.setToggleState (value, dontSendNotification);
    }

    void voiceStealingEnabledChanged (bool value) override
    {
        voiceStealingEnabledToggle.setToggleState (value, dontSendNotification);
    }

    void synthVoicesChanged (int value) override
    {
        numberOfVoices.setSelectedId (value, dontSendNotification);
    }

    MPESettingsDataModel dataModel;
    MPELegacySettingsComponent legacySettings;
    MPENewSettingsComponent newSettings;

    ToggleButton legacyModeEnabledToggle    { "Enable Legacy Mode" },
                 voiceStealingEnabledToggle { "Enable synth voice stealing" };

    ComboBox numberOfVoices;
    Label numberOfVoicesLabel { {}, "Number of synth voices" };

    UndoManager* undoManager;
};

//==============================================================================
class LoopPointMarker  : public Component
{
public:
    using MouseCallback = std::function<void (LoopPointMarker&, const MouseEvent&)>;

    LoopPointMarker (String marker,
                     MouseCallback onMouseDown,
                     MouseCallback onMouseDrag,
                     MouseCallback onMouseUp)
        : text (std::move (marker)),
          onMouseDown (move (onMouseDown)),
          onMouseDrag (move (onMouseDrag)),
          onMouseUp (move (onMouseUp))
    {
        setMouseCursor (MouseCursor::LeftRightResizeCursor);
    }

private:
    void resized() override
    {
        auto height = 20;
        auto triHeight = 6;

        auto bounds = getLocalBounds();
        Path newPath;
        newPath.addRectangle (bounds.removeFromBottom (height));

        newPath.startNewSubPath (bounds.getBottomLeft().toFloat());
        newPath.lineTo (bounds.getBottomRight().toFloat());
        Point<float> apex (static_cast<float> (bounds.getX() + (bounds.getWidth() / 2)),
                           static_cast<float> (bounds.getBottom() - triHeight));
        newPath.lineTo (apex);
        newPath.closeSubPath();

        newPath.addLineSegment (Line<float> (apex, Point<float> (apex.getX(), 0)), 1);

        path = newPath;
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colours::deepskyblue);
        g.fillPath (path);

        auto height = 20;
        g.setColour (Colours::white);
        g.drawText (text, getLocalBounds().removeFromBottom (height), Justification::centred);
    }

    bool hitTest (int x, int y) override
    {
        return path.contains ((float) x, (float) y);
    }

    void mouseDown (const MouseEvent& e) override
    {
        onMouseDown (*this, e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        onMouseDrag (*this, e);
    }

    void mouseUp (const MouseEvent& e) override
    {
        onMouseUp (*this, e);
    }

    String text;
    Path path;
    MouseCallback onMouseDown;
    MouseCallback onMouseDrag;
    MouseCallback onMouseUp;
};

//==============================================================================
class Ruler  : public Component,
               public VisibleRangeDataModel::Listener
{
public:
    explicit Ruler (const VisibleRangeDataModel& model)
        : visibleRange (model)
    {
        visibleRange.addListener (*this);
        setMouseCursor (MouseCursor::LeftRightResizeCursor);
    }

private:
    void paint (Graphics& g) override
    {
        auto minDivisionWidth = 50.0f;
        auto maxDivisions     = getWidth() / minDivisionWidth;

        auto lookFeel = dynamic_cast<LookAndFeel_V4*> (&getLookAndFeel());
        auto bg = lookFeel->getCurrentColourScheme()
                           .getUIColour (LookAndFeel_V4::ColourScheme::UIColour::widgetBackground);

        g.setGradientFill (ColourGradient (bg.brighter(),
                                           0,
                                           0,
                                           bg.darker(),
                                           0,
                                           (float) getHeight(),
                                           false));

        g.fillAll();
        g.setColour (bg.brighter());
        g.drawHorizontalLine (0, 0.0f, (float) getWidth());
        g.setColour (bg.darker());
        g.drawHorizontalLine (1, 0.0f, (float) getWidth());
        g.setColour (Colours::lightgrey);

        auto minLog = std::ceil (std::log10 (visibleRange.getVisibleRange().getLength() / maxDivisions));
        auto precision = 2 + std::abs (minLog);
        auto divisionMagnitude = std::pow (10, minLog);
        auto startingDivision = std::ceil (visibleRange.getVisibleRange().getStart() / divisionMagnitude);

        for (auto div = startingDivision; div * divisionMagnitude < visibleRange.getVisibleRange().getEnd(); ++div)
        {
            auto time = div * divisionMagnitude;
            auto xPos = (time - visibleRange.getVisibleRange().getStart()) * getWidth()
                              / visibleRange.getVisibleRange().getLength();

            std::ostringstream out_stream;
            out_stream << std::setprecision (roundToInt (precision)) << roundToInt (time);

            g.drawText (out_stream.str(),
                        Rectangle<int> (Point<int> (roundToInt (xPos) + 3, 0),
                                        Point<int> (roundToInt (xPos + minDivisionWidth), getHeight())),
                        Justification::centredLeft,
                        false);

            g.drawVerticalLine (roundToInt (xPos), 2.0f, (float) getHeight());
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        visibleRangeOnMouseDown = visibleRange.getVisibleRange();
        timeOnMouseDown = visibleRange.getVisibleRange().getStart()
                       + (visibleRange.getVisibleRange().getLength() * e.getMouseDownX()) / getWidth();
    }

    void mouseDrag (const MouseEvent& e) override
    {
        // Work out the scale of the new range
        auto unitDistance = 100.0f;
        auto scaleFactor  = 1.0 / std::pow (2, e.getDistanceFromDragStartY() / unitDistance);

        // Now position it so that the mouse continues to point at the same
        // place on the ruler.
        auto visibleLength = std::max (0.12, visibleRangeOnMouseDown.getLength() * scaleFactor);
        auto rangeBegin = timeOnMouseDown - visibleLength * e.x / getWidth();
        const Range<double> range (rangeBegin, rangeBegin + visibleLength);
        visibleRange.setVisibleRange (range, nullptr);
    }

    void visibleRangeChanged (Range<double>) override
    {
        repaint();
    }

    VisibleRangeDataModel visibleRange;
    Range<double> visibleRangeOnMouseDown;
    double timeOnMouseDown;
};

//==============================================================================
class LoopPointsOverlay  : public Component,
                           public DataModel::Listener,
                           public VisibleRangeDataModel::Listener
{
public:
    LoopPointsOverlay (const DataModel& dModel,
                       const VisibleRangeDataModel& vModel,
                       UndoManager& undoManager)
        : dataModel (dModel),
          visibleRange (vModel),
          beginMarker ("B",
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseDown (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointDragged   (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseUp   (m, e); }),
          endMarker   ("E",
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseDown (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointDragged   (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseUp   (m, e); }),
          undoManager (&undoManager)
    {
        dataModel   .addListener (*this);
        visibleRange.addListener (*this);

        for (auto ptr : { &beginMarker, &endMarker })
            addAndMakeVisible (ptr);
    }

private:
    void resized() override
    {
        positionLoopPointMarkers();
    }

    void loopPointMouseDown (LoopPointMarker&, const MouseEvent&)
    {
        loopPointsOnMouseDown = dataModel.getLoopPointsSeconds();
        undoManager->beginNewTransaction();
    }

    void loopPointDragged (LoopPointMarker& marker, const MouseEvent& e)
    {
        auto x = xPositionToTime (e.getEventRelativeTo (this).position.x);
        const Range<double> newLoopRange (&marker == &beginMarker ? x : loopPointsOnMouseDown.getStart(),
                                          &marker == &endMarker   ? x : loopPointsOnMouseDown.getEnd());

        dataModel.setLoopPointsSeconds (newLoopRange, undoManager);
    }

    void loopPointMouseUp (LoopPointMarker& marker, const MouseEvent& e)
    {
        auto x = xPositionToTime (e.getEventRelativeTo (this).position.x);
        const Range<double> newLoopRange (&marker == &beginMarker ? x : loopPointsOnMouseDown.getStart(),
                                          &marker == &endMarker   ? x : loopPointsOnMouseDown.getEnd());

        dataModel.setLoopPointsSeconds (newLoopRange, undoManager);
    }

    void loopPointsSecondsChanged (Range<double>) override
    {
        positionLoopPointMarkers();
    }

    void visibleRangeChanged (Range<double>) override
    {
        positionLoopPointMarkers();
    }

    double timeToXPosition (double time) const
    {
        return (time - visibleRange.getVisibleRange().getStart()) * getWidth()
                     / visibleRange.getVisibleRange().getLength();
    }

    double xPositionToTime (double xPosition) const
    {
        return ((xPosition * visibleRange.getVisibleRange().getLength()) / getWidth())
                           + visibleRange.getVisibleRange().getStart();
    }

    void positionLoopPointMarkers()
    {
        auto halfMarkerWidth = 7;

        for (auto tup : { std::make_tuple (&beginMarker, dataModel.getLoopPointsSeconds().getStart()),
                          std::make_tuple (&endMarker,   dataModel.getLoopPointsSeconds().getEnd()) })
        {
            auto ptr  = std::get<0> (tup);
            auto time = std::get<1> (tup);
            ptr->setSize (halfMarkerWidth * 2, getHeight());
            ptr->setTopLeftPosition (roundToInt (timeToXPosition (time) - halfMarkerWidth), 0);
        }
    }

    DataModel dataModel;
    VisibleRangeDataModel visibleRange;
    Range<double> loopPointsOnMouseDown;
    LoopPointMarker beginMarker, endMarker;
    UndoManager* undoManager;
};

//==============================================================================
class PlaybackPositionOverlay  : public Component,
                                 public Timer,
                                 public VisibleRangeDataModel::Listener
{
public:
    using Provider = std::function<std::vector<float>()>;
    PlaybackPositionOverlay (const VisibleRangeDataModel& model,
                             Provider provider)
        : visibleRange (model),
          provider (move (provider))
    {
        visibleRange.addListener (*this);
        startTimer (16);
    }

private:
    void paint (Graphics& g) override
    {
        g.setColour (Colours::red);

        for (auto position : provider())
        {
            g.drawVerticalLine (roundToInt (timeToXPosition (position)), 0.0f, (float) getHeight());
        }
    }

    void timerCallback() override
    {
        repaint();
    }

    void visibleRangeChanged (Range<double>) override
    {
        repaint();
    }

    double timeToXPosition (double time) const
    {
        return (time - visibleRange.getVisibleRange().getStart()) * getWidth()
                     / visibleRange.getVisibleRange().getLength();
    }

    VisibleRangeDataModel visibleRange;
    Provider provider;
};

//==============================================================================
class WaveformView  : public Component,
                      public ChangeListener,
                      public DataModel::Listener,
                      public VisibleRangeDataModel::Listener
{
public:
    WaveformView (const DataModel& model,
                  const VisibleRangeDataModel& vr)
        : dataModel (model),
          visibleRange (vr),
          thumbnailCache (4),
          thumbnail (4, dataModel.getAudioFormatManager(), thumbnailCache)
    {
        dataModel   .addListener (*this);
        visibleRange.addListener (*this);
        thumbnail   .addChangeListener (this);
    }

private:
    void paint (Graphics& g) override
    {
        // Draw the waveforms
        g.fillAll (Colours::black);
        auto numChannels = thumbnail.getNumChannels();

        if (numChannels == 0)
        {
            g.setColour (Colours::white);
            g.drawFittedText ("No File Loaded", getLocalBounds(), Justification::centred, 1);
            return;
        }

        auto bounds = getLocalBounds();
        auto channelHeight = bounds.getHeight() / numChannels;

        for (auto i = 0; i != numChannels; ++i)
        {
            drawChannel (g, i, bounds.removeFromTop (channelHeight));
        }
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            repaint();
    }

    void sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory> value) override
    {
        if (value == nullptr)
            thumbnail.clear();
        else
        {
            auto reader = value->make (dataModel.getAudioFormatManager());
            thumbnail.setReader (reader.release(), currentHashCode);
            currentHashCode += 1;
        }
    }

    void visibleRangeChanged (Range<double>) override
    {
        repaint();
    }

    void drawChannel (Graphics& g, int channel, Rectangle<int> bounds)
    {
        g.setGradientFill (ColourGradient (Colours::lightblue,
                                           bounds.getTopLeft().toFloat(),
                                           Colours::darkgrey,
                                           bounds.getBottomLeft().toFloat(),
                                           false));
        thumbnail.drawChannel (g,
                               bounds,
                               visibleRange.getVisibleRange().getStart(),
                               visibleRange.getVisibleRange().getEnd(),
                               channel,
                               1.0f);
    }

    DataModel dataModel;
    VisibleRangeDataModel visibleRange;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    int64 currentHashCode = 0;
};

//==============================================================================
class WaveformEditor  : public Component,
                        public DataModel::Listener
{
public:
    WaveformEditor (const DataModel& model,
                    PlaybackPositionOverlay::Provider provider,
                    UndoManager& undoManager)
        : dataModel (model),
          waveformView (model, visibleRange),
          playbackOverlay (visibleRange, move (provider)),
          loopPoints (dataModel, visibleRange, undoManager),
          ruler (visibleRange)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (waveformView);
        addAndMakeVisible (playbackOverlay);
        addChildComponent (loopPoints);
        loopPoints.setAlwaysOnTop (true);

        waveformView.toBack();

        addAndMakeVisible (ruler);
    }

private:
    void resized() override
    {
        auto bounds = getLocalBounds();
        ruler          .setBounds (bounds.removeFromTop (25));
        waveformView   .setBounds (bounds);
        playbackOverlay.setBounds (bounds);
        loopPoints     .setBounds (bounds);
    }

    void loopModeChanged (LoopMode value) override
    {
        loopPoints.setVisible (value != LoopMode::none);
    }

    void sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory> value) override
    {
        auto lengthInSeconds = dataModel.getSampleLengthSeconds();
        visibleRange.setTotalRange   (Range<double> (0, lengthInSeconds), nullptr);
        visibleRange.setVisibleRange (Range<double> (0, lengthInSeconds), nullptr);
    }

    DataModel dataModel;
    VisibleRangeDataModel visibleRange;
    WaveformView waveformView;
    PlaybackPositionOverlay playbackOverlay;
    LoopPointsOverlay loopPoints;
    Ruler ruler;
};

//==============================================================================
class MainSamplerView  : public Component,
                         public DataModel::Listener
{
public:
    MainSamplerView (const DataModel& model,
                     PlaybackPositionOverlay::Provider provider,
                     UndoManager& um)
        : dataModel (model),
          waveformEditor (dataModel, move (provider), um),
          undoManager (&um)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (waveformEditor);
        addAndMakeVisible (loadNewSampleButton);

        auto setReader = [this] (const FileChooser& fc)
        {
            undoManager->beginNewTransaction();
            auto readerFactory = new FileAudioFormatReaderFactory (fc.getResult());
            dataModel.setSampleReader (std::unique_ptr<AudioFormatReaderFactory> (readerFactory),
                                       undoManager);
        };

        loadNewSampleButton.onClick = [this, setReader]
        {
            fileChooser.launchAsync (FileBrowserComponent::FileChooserFlags::openMode |
                                     FileBrowserComponent::FileChooserFlags::canSelectFiles,
                                     setReader);
        };

        addAndMakeVisible (centreFrequency);
        centreFrequency.onValueChange = [this]
        {
            undoManager->beginNewTransaction();
            dataModel.setCentreFrequencyHz (centreFrequency.getValue(),
                                            centreFrequency.isMouseButtonDown() ? nullptr : undoManager);
        };

        centreFrequency.setRange (20, 20000, 1);
        centreFrequency.setSliderStyle (Slider::SliderStyle::IncDecButtons);
        centreFrequency.setIncDecButtonsMode (Slider::IncDecButtonMode::incDecButtonsDraggable_Vertical);

        auto radioGroupId = 1;

        for (auto buttonPtr : { &loopKindNone, &loopKindForward, &loopKindPingpong })
        {
            addAndMakeVisible (buttonPtr);
            buttonPtr->setRadioGroupId (radioGroupId, dontSendNotification);
            buttonPtr->setClickingTogglesState (true);
        }

        loopKindNone.onClick = [this]
        {
            if (loopKindNone.getToggleState())
            {
                undoManager->beginNewTransaction();
                dataModel.setLoopMode (LoopMode::none, undoManager);
            }
        };

        loopKindForward.onClick = [this]
        {
            if (loopKindForward.getToggleState())
            {
                undoManager->beginNewTransaction();
                dataModel.setLoopMode (LoopMode::forward, undoManager);
            }
        };

        loopKindPingpong.onClick = [this]
        {
            if (loopKindPingpong.getToggleState())
            {
                undoManager->beginNewTransaction();
                dataModel.setLoopMode (LoopMode::pingpong, undoManager);
            }
        };

        addAndMakeVisible (centreFrequencyLabel);
        addAndMakeVisible (loopKindLabel);
    }

private:
    void resized() override
    {
        auto bounds = getLocalBounds();

        auto topBar = bounds.removeFromTop (50);
        auto padding = 4;
        loadNewSampleButton .setBounds (topBar.removeFromRight (100).reduced (padding));
        centreFrequencyLabel.setBounds (topBar.removeFromLeft  (100).reduced (padding));
        centreFrequency     .setBounds (topBar.removeFromLeft  (100).reduced (padding));

        auto bottomBar = bounds.removeFromBottom (50);
        loopKindLabel   .setBounds (bottomBar.removeFromLeft (100).reduced (padding));
        loopKindNone    .setBounds (bottomBar.removeFromLeft (80) .reduced (padding));
        loopKindForward .setBounds (bottomBar.removeFromLeft (80) .reduced (padding));
        loopKindPingpong.setBounds (bottomBar.removeFromLeft (80) .reduced (padding));

        waveformEditor.setBounds (bounds);
    }

    void loopModeChanged (LoopMode value) override
    {
        switch (value)
        {
            case LoopMode::none:
                loopKindNone.setToggleState (true, dontSendNotification);
                break;
            case LoopMode::forward:
                loopKindForward.setToggleState (true, dontSendNotification);
                break;
            case LoopMode::pingpong:
                loopKindPingpong.setToggleState (true, dontSendNotification);
                break;
        }
    }

    void centreFrequencyHzChanged (double value) override
    {
        centreFrequency.setValue (value, dontSendNotification);
    }

    DataModel dataModel;
    WaveformEditor waveformEditor;
    TextButton loadNewSampleButton { "Load New Sample" };
    Slider centreFrequency;

    TextButton loopKindNone        { "None" },
               loopKindForward     { "Forward" },
               loopKindPingpong    { "Ping Pong" };

    Label centreFrequencyLabel     { {}, "Sample Centre Freq / Hz" },
          loopKindLabel            { {}, "Looping Mode" };


    FileChooser fileChooser { "Select a file to load...", File(),
                              dataModel.getAudioFormatManager().getWildcardForAllFormats() };

    UndoManager* undoManager;
};

//==============================================================================
struct ProcessorState
{
    int synthVoices;
    bool legacyModeEnabled;
    Range<int> legacyChannels;
    int legacyPitchbendRange;
    bool voiceStealingEnabled;
    MPEZoneLayout mpeZoneLayout;
    std::unique_ptr<AudioFormatReaderFactory> readerFactory;
    Range<double> loopPointsSeconds;
    double centreFrequencyHz;
    LoopMode loopMode;
};

//==============================================================================
// We store the current sampler sound in a shared_ptr. Although we never
// call mutating member functions on this shared_ptr, we do read from it on
// both the audio and gui threads. Such concurrent reads should be safe
// without using atomic methods, but we use a tiny wrapper to enforce atomic
// accesses anyway - if nothing else, this wrapper enforces and documents that
// we never mutate the shared_ptr in a way which could cause a data race.
template <typename Contents>
class AtomicSharedPtr final
{
public:
    AtomicSharedPtr() = default;
    explicit AtomicSharedPtr (std::shared_ptr<Contents> contents)
        : contents (move (contents))
    {}

    AtomicSharedPtr (const AtomicSharedPtr& other) = delete;
    AtomicSharedPtr& operator= (const AtomicSharedPtr& other) = delete;

    std::shared_ptr<Contents> load() const
    {
        return atomic_load (&contents);
    }

private:
    std::shared_ptr<Contents> contents;
};

//==============================================================================
class SamplerAudioProcessor  : public AudioProcessor
{
public:
    SamplerAudioProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true))
    {
        if (auto* asset = createAssetInputStream ("cello.wav"))
        {
            std::unique_ptr<InputStream> inputStream (asset);
            inputStream->readIntoMemoryBlock (mb);

            readerFactory.reset (new MemoryAudioFormatReaderFactory (mb.getData(), mb.getSize()));
        }

        // Set up initial sample, which we load from a binary resource
        AudioFormatManager manager;
        manager.registerBasicFormats();
        auto reader = readerFactory->make (manager);
        auto sound = samplerSound.load();
        auto sample = std::unique_ptr<Sample> (new Sample (*reader, 10.0));
        auto lengthInSeconds = sample->getLength() / sample->getSampleRate();
        sound->setLoopPointsInSeconds ({lengthInSeconds * 0.1, lengthInSeconds * 0.9 });
        sound->setSample (move (sample));

        // Start with the max number of voices
        for (auto i = 0; i != maxVoices; ++i)
            synthesiser.addVoice (new MPESamplerVoice (sound));
    }

    void prepareToPlay (double sampleRate, int) override
    {
        synthesiser.setCurrentPlaybackSampleRate (sampleRate);
    }

    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == AudioChannelSet::mono()
            || layouts.getMainOutputChannelSet() == AudioChannelSet::stereo();
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override
    {
        // This function will be called from the message thread. We lock the command
        // queue to ensure that no messages are processed for the duration of this
        // call.
        std::lock_guard<std::mutex> lock (commandQueueMutex);

        ProcessorState state;
        state.synthVoices          = synthesiser.getNumVoices();
        state.legacyModeEnabled    = synthesiser.isLegacyModeEnabled();
        state.legacyChannels       = synthesiser.getLegacyModeChannelRange();
        state.legacyPitchbendRange = synthesiser.getLegacyModePitchbendRange();
        state.voiceStealingEnabled = synthesiser.isVoiceStealingEnabled();
        state.mpeZoneLayout        = synthesiser.getZoneLayout();
        state.readerFactory        = readerFactory == nullptr ? nullptr : readerFactory->clone();

        auto sound = samplerSound.load();
        state.loopPointsSeconds = sound->getLoopPointsInSeconds();
        state.centreFrequencyHz = sound->getCentreFrequencyInHz();
        state.loopMode          = sound->getLoopMode();

        return new SamplerAudioProcessorEditor (*this, std::move (state));
    }

    bool hasEditor() const override                                       { return true; }

    //==============================================================================
    const String getName() const override                                 { return JucePlugin_Name; }
    bool acceptsMidi() const override                                     { return true; }
    bool producesMidi() const override                                    { return false; }
    bool isMidiEffect() const override                                    { return false; }
    double getTailLengthSeconds() const override                          { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                         { return 1; }
    int getCurrentProgram() override                                      { return 0; }
    void setCurrentProgram (int) override                                 {}
    const String getProgramName (int) override                            { return {}; }
    void changeProgramName (int, const String&) override                  {}

    //==============================================================================
    void getStateInformation (MemoryBlock&) override                      {}
    void setStateInformation (const void*, int) override                  {}

    //==============================================================================
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        // Try to acquire a lock on the command queue.
        // If we were successful, we pop all pending commands off the queue and
        // apply them to the processor.
        // If we weren't able to acquire the lock, it's because someone called
        // createEditor, which requires that the processor data model stays in
        // a valid state for the duration of the call.
        std::unique_lock<std::mutex> lock (commandQueueMutex, std::try_to_lock);

        if (lock.owns_lock())
        {
            while (auto command = incomingCommands.pop())
            {
                command->run (*this);
                // We push the command onto the outgoing buffer, as long as it has
                // room. If it doesn't have room for some reason, we'll delete
                // the command right here on this thread, which might take a while
                // and cause the audio to glitch, so I hope the buffer size is big
                // enough!
                outgoingCommands.push (move (command));
            }
        }

        synthesiser.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

        auto loadedSamplerSound = samplerSound.load();

        if (loadedSamplerSound->getSample() == nullptr)
            return;

        auto numVoices = synthesiser.getNumVoices();

        // Update the current playback positions
        for (auto i = 0; i != maxVoices; ++i)
        {
            auto* voicePtr = dynamic_cast<MPESamplerVoice*> (synthesiser.getVoice (i));

            if (i < numVoices && voicePtr != nullptr)
                playbackPositions[i] = static_cast<float> (voicePtr->getCurrentSamplePosition() / loadedSamplerSound->getSample()->getSampleRate());
            else
                playbackPositions[i] = 0.0f;
        }
    }

    // These should be called from the GUI thread, and will block until the
    // command buffer has enough room to accept a command.
    void setSample (std::unique_ptr<AudioFormatReaderFactory> fact, AudioFormatManager& formatManager)
    {
        class SetSampleCommand
        {
        public:
            SetSampleCommand (std::unique_ptr<AudioFormatReaderFactory> r,
                              std::unique_ptr<Sample> sample,
                              std::vector<std::unique_ptr<MPESamplerVoice>> newVoices)
                : readerFactory (move (r)),
                  sample (move (sample)),
                  newVoices (move (newVoices))
            {}

            void operator() (SamplerAudioProcessor& proc)
            {
                proc.readerFactory = move (readerFactory);
                auto samplerSound = proc.samplerSound.load();
                samplerSound->setSample (move (sample));
                auto numberOfVoices = proc.synthesiser.getNumVoices();
                proc.synthesiser.clearVoices();

                for (auto it = begin (newVoices); proc.synthesiser.getNumVoices() < numberOfVoices; ++it)
                {
                    proc.synthesiser.addVoice (it->release());
                }
            }

        private:
            std::unique_ptr<AudioFormatReaderFactory> readerFactory;
            std::unique_ptr<Sample> sample;
            std::vector<std::unique_ptr<MPESamplerVoice>> newVoices;
        };

        // Note that all allocation happens here, on the main message thread. Then,
        // we transfer ownership across to the audio thread.
        auto loadedSamplerSound = samplerSound.load();
        std::vector<std::unique_ptr<MPESamplerVoice>> newSamplerVoices;
        newSamplerVoices.reserve (maxVoices);

        for (auto i = 0; i != maxVoices; ++i)
            newSamplerVoices.emplace_back (new MPESamplerVoice (loadedSamplerSound));

        if (fact == nullptr)
        {
            pushCommand (SetSampleCommand (move (fact),
                                           nullptr,
                                           move (newSamplerVoices)));
        }
        else
        {
            auto reader = fact->make (formatManager);
            pushCommand (SetSampleCommand (move (fact),
                                           std::unique_ptr<Sample> (new Sample (*reader, 10.0)),
                                           move (newSamplerVoices)));
        }
    }

    void setCentreFrequency (double centreFrequency)
    {
        pushCommand ([centreFrequency] (SamplerAudioProcessor& proc)
                     {
                         auto loaded = proc.samplerSound.load();
                         if (loaded != nullptr)
                             loaded->setCentreFrequencyInHz (centreFrequency);
                     });
    }

    void setLoopMode (LoopMode loopMode)
    {
        pushCommand ([loopMode] (SamplerAudioProcessor& proc)
                     {
                         auto loaded = proc.samplerSound.load();
                         if (loaded != nullptr)
                             loaded->setLoopMode (loopMode);
                     });
    }

    void setLoopPoints (Range<double> loopPoints)
    {
        pushCommand ([loopPoints] (SamplerAudioProcessor& proc)
                     {
                         auto loaded = proc.samplerSound.load();
                         if (loaded != nullptr)
                             loaded->setLoopPointsInSeconds (loopPoints);
                     });
    }

    void setMPEZoneLayout (MPEZoneLayout layout)
    {
        pushCommand ([layout] (SamplerAudioProcessor& proc)
                     {
                         // setZoneLayout will lock internally, so we don't care too much about
                         // ensuring that the layout doesn't get copied or destroyed on the
                         // audio thread. If the audio glitches while updating midi settings
                         // it doesn't matter too much.
                         proc.synthesiser.setZoneLayout (layout);
                     });
    }

    void setLegacyModeEnabled (int pitchbendRange, Range<int> channelRange)
    {
        pushCommand ([pitchbendRange, channelRange] (SamplerAudioProcessor& proc)
                     {
                         proc.synthesiser.enableLegacyMode (pitchbendRange, channelRange);
                     });
    }

    void setVoiceStealingEnabled (bool voiceStealingEnabled)
    {
        pushCommand ([voiceStealingEnabled] (SamplerAudioProcessor& proc)
                     {
                         proc.synthesiser.setVoiceStealingEnabled (voiceStealingEnabled);
                     });
    }

    void setNumberOfVoices (int numberOfVoices)
    {
        // We don't want to call 'new' on the audio thread. Normally, we'd
        // construct things here, on the GUI thread, and then move them into the
        // command lambda. Unfortunately, C++11 doesn't have extended lambda
        // capture, so we use a custom struct instead.

        class SetNumVoicesCommand
        {
        public:
            SetNumVoicesCommand (std::vector<std::unique_ptr<MPESamplerVoice>> newVoices)
                : newVoices (move (newVoices))
            {}

            void operator() (SamplerAudioProcessor& proc)
            {
                if (newVoices.size() < proc.synthesiser.getNumVoices())
                    proc.synthesiser.reduceNumVoices (int (newVoices.size()));
                else
                {
                    for (auto it = begin (newVoices); proc.synthesiser.getNumVoices() < newVoices.size(); ++it)
                    {
                        proc.synthesiser.addVoice (it->release());
                    }
                }
            }

        private:
            std::vector<std::unique_ptr<MPESamplerVoice>> newVoices;
        };

        numberOfVoices = std::min (maxVoices, numberOfVoices);
        auto loadedSamplerSound = samplerSound.load();
        std::vector<std::unique_ptr<MPESamplerVoice>> newSamplerVoices;
        newSamplerVoices.reserve (numberOfVoices);

        for (auto i = 0; i != numberOfVoices; ++i)
            newSamplerVoices.emplace_back (new MPESamplerVoice (loadedSamplerSound));

        pushCommand (SetNumVoicesCommand (move (newSamplerVoices)));
    }

    // These accessors are just for an 'overview' and won't give the exact
    // state of the audio engine at a particular point in time.
    // If you call getNumVoices(), get the result '10', and then call
    // getPlaybackPosiiton(9), there's a chance the audio engine will have
    // been updated to remove some voices in the meantime, so the returned
    // value won't correspond to an existing voice.
    int getNumVoices() const                    { return synthesiser.getNumVoices(); }
    float getPlaybackPosition (int voice) const { return playbackPositions.at (voice); }

private:
    //==============================================================================
    class SamplerAudioProcessorEditor  : public AudioProcessorEditor,
                                         public FileDragAndDropTarget,
                                         public DataModel::Listener,
                                         public MPESettingsDataModel::Listener
    {
    public:
        SamplerAudioProcessorEditor (SamplerAudioProcessor& p, ProcessorState state)
            : AudioProcessorEditor (&p),
              processor (p),
              mainSamplerView (dataModel,
                               [&p]
                               {
                                   std::vector<float> ret;
                                   auto voices = p.getNumVoices();
                                   ret.reserve (voices);

                                   for (auto i = 0; i != voices; ++i)
                                       ret.emplace_back (p.getPlaybackPosition (i));

                                   return ret;
                               },
                               undoManager)
        {
            dataModel.addListener (*this);
            mpeSettings.addListener (*this);

            formatManager.registerBasicFormats();

            addAndMakeVisible (tabbedComponent);

            auto lookFeel = dynamic_cast<LookAndFeel_V4*> (&getLookAndFeel());
            auto bg = lookFeel->getCurrentColourScheme()
                               .getUIColour (LookAndFeel_V4::ColourScheme::UIColour::widgetBackground);

            tabbedComponent.addTab ("Sample Editor", bg, &mainSamplerView, false);
            tabbedComponent.addTab ("MPE Settings", bg, &settingsComponent, false);

            mpeSettings.setSynthVoices          (state.synthVoices,               nullptr);
            mpeSettings.setLegacyModeEnabled    (state.legacyModeEnabled,         nullptr);
            mpeSettings.setLegacyFirstChannel   (state.legacyChannels.getStart(), nullptr);
            mpeSettings.setLegacyLastChannel    (state.legacyChannels.getEnd(),   nullptr);
            mpeSettings.setLegacyPitchbendRange (state.legacyPitchbendRange,      nullptr);
            mpeSettings.setVoiceStealingEnabled (state.voiceStealingEnabled,      nullptr);
            mpeSettings.setMPEZoneLayout        (state.mpeZoneLayout,             nullptr);

            dataModel.setSampleReader (move (state.readerFactory),    nullptr);
            dataModel.setLoopPointsSeconds  (state.loopPointsSeconds, nullptr);
            dataModel.setCentreFrequencyHz  (state.centreFrequencyHz, nullptr);
            dataModel.setLoopMode           (state.loopMode,          nullptr);

            // Make sure that before the constructor has finished, you've set the
            // editor's size to whatever you need it to be.
            setResizable (true, true);
            setResizeLimits (640, 480, 2560, 1440);
            setSize (640, 480);
        }

    private:
        void resized() override
        {
            tabbedComponent.setBounds (getLocalBounds());
        }

        bool keyPressed (const KeyPress& key) override
        {
            if (key == KeyPress ('z', ModifierKeys::commandModifier, 0))
            {
                undoManager.undo();
                return true;
            }

            if (key == KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
            {
                undoManager.redo();
                return true;
            }

            return Component::keyPressed (key);
        }

        bool isInterestedInFileDrag (const StringArray& files) override
        {
            WildcardFileFilter filter (formatManager.getWildcardForAllFormats(), {}, "Known Audio Formats");
            return files.size() == 1 && filter.isFileSuitable (files[0]);
        }

        void filesDropped (const StringArray& files, int, int) override
        {
            jassert (files.size() == 1);
            undoManager.beginNewTransaction();
            auto r = new FileAudioFormatReaderFactory (files[0]);
            dataModel.setSampleReader (std::unique_ptr<AudioFormatReaderFactory> (r),
                                       &undoManager);

        }

        void sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory> value) override
        {
            processor.setSample (value == nullptr ? nullptr : value->clone(),
                                 dataModel.getAudioFormatManager());
        }

        void centreFrequencyHzChanged (double value) override
        {
            processor.setCentreFrequency (value);
        }

        void loopPointsSecondsChanged (Range<double> value) override
        {
            processor.setLoopPoints (value);
        }

        void loopModeChanged (LoopMode value) override
        {
            processor.setLoopMode (value);
        }

        void synthVoicesChanged (int value) override
        {
            processor.setNumberOfVoices (value);
        }

        void voiceStealingEnabledChanged (bool value) override
        {
            processor.setVoiceStealingEnabled (value);
        }

        void legacyModeEnabledChanged (bool value) override
        {
            if (value)
                setProcessorLegacyMode();
            else
                setProcessorMPEMode();
        }

        void mpeZoneLayoutChanged (const MPEZoneLayout&) override
        {
            setProcessorMPEMode();
        }

        void legacyFirstChannelChanged (int) override
        {
            setProcessorLegacyMode();
        }

        void legacyLastChannelChanged (int) override
        {
            setProcessorLegacyMode();
        }

        void legacyPitchbendRangeChanged (int) override
        {
            setProcessorLegacyMode();
        }

        void setProcessorLegacyMode()
        {
            processor.setLegacyModeEnabled (mpeSettings.getLegacyPitchbendRange(),
                                            Range<int> (mpeSettings.getLegacyFirstChannel(),
                                            mpeSettings.getLegacyLastChannel()));
        }

        void setProcessorMPEMode()
        {
            processor.setMPEZoneLayout (mpeSettings.getMPEZoneLayout());
        }

        SamplerAudioProcessor& processor;
        AudioFormatManager formatManager;
        DataModel dataModel { formatManager };
        UndoManager undoManager;
        MPESettingsDataModel mpeSettings { dataModel.mpeSettings() };
        TabbedComponent tabbedComponent { TabbedButtonBar::Orientation::TabsAtTop };
        MPESettingsComponent settingsComponent { dataModel.mpeSettings(), undoManager };
        MainSamplerView mainSamplerView;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessorEditor)
    };

    //==============================================================================
    // We want to send type-erased commands to the audio thread, but we also
    // want those commands to contain move-only resources, so that we can
    // construct resources on the gui thread, and then transfer ownership
    // cheaply to the audio thread. We can't do this with std::function
    // because it enforces that functions are copy-constructible.
    // Therefore, we use a very simple templated type-eraser here.
    struct Command
    {
        virtual ~Command() noexcept                    = default;
        virtual void run (SamplerAudioProcessor& proc) = 0;
    };

    template <typename Func>
    class TemplateCommand  : public Command,
                             public Func
    {
    public:
        template <typename FuncPrime>
        explicit TemplateCommand (FuncPrime&& funcPrime)
            : Func (std::forward<FuncPrime> (funcPrime))
        {}

        void run (SamplerAudioProcessor& proc) override
        {
            (*this) (proc);
        }
    };

    template <typename Func>
    static std::unique_ptr<Command> make_command (Func&& func)
    {
        return std::unique_ptr<TemplateCommand<Func>> (new TemplateCommand<Func> (std::forward<Func> (func)));
    }

    using CommandFifo = MoveOnlyFifo<std::unique_ptr<Command>>;

    class OutgoingBufferCleaner  : public Timer
    {
    public:
        explicit OutgoingBufferCleaner (CommandFifo& bufferToEmpty)
            : buffer (bufferToEmpty)
        {
            startTimer (500);
        }

    private:
        void timerCallback() override
        {
            while (auto command = buffer.pop())
                command = {};
        }

        CommandFifo& buffer;
    };

    // Spin, trying to post a command to the sampler sound, until there's
    // enough room in the command buffer to accept the new command.
    template <typename Func>
    void pushCommand (Func&& func)
    {
        auto command = make_command (std::forward<Func> (func));
        while (command)
            command = incomingCommands.push (move (command));
    }

    // We have an incoming and an outgoing command queue. The incoming commands
    // are used to update the sampler sound in a thread-safe way, without
    // blocking. Once we've consumed a command, we push it back onto the
    // outgoing command queue, which is cleaned up periodically by the
    // outgoingBufferCleaner.
    CommandFifo incomingCommands;
    CommandFifo outgoingCommands;
    OutgoingBufferCleaner outgoingBufferCleaner { outgoingCommands };

    MemoryBlock mb;
    std::unique_ptr<AudioFormatReaderFactory> readerFactory;
    AtomicSharedPtr<MPESamplerSound> samplerSound { std::make_shared<MPESamplerSound>() };
    MPESynthesiser synthesiser;

    // This mutex is used to ensure we don't modify the processor state during
    // a call to createEditor, which would cause the UI to become desynched
    // with the real state of the processor.
    std::mutex commandQueueMutex;

    static const int maxVoices { 20 };

    // This is used for visualising the current playback position of each voice.
    std::array<std::atomic<float>, maxVoices> playbackPositions;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessor)
};

const int SamplerAudioProcessor::maxVoices;
