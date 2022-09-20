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
    A simple component that can be used to show a scrolling waveform of audio data.

    This is a handy way to get a quick visualisation of some audio data. Just create
    one of these, set its size and oversampling rate, and then feed it with incoming
    data by calling one of its pushBuffer() or pushSample() methods.

    You can override its paint method for more customised views, but it's only designed
    as a quick-and-dirty class for simple tasks, so please don't send us feature requests
    for fancy additional features that you'd like it to support! If you're building a
    real-world app that requires more powerful waveform display, you'll probably want to
    create your own component instead.

    @tags{Audio}
*/
class JUCE_API AudioVisualiserComponent  : public Component,
                                           private Timer
{
public:
    /** Creates a visualiser with the given number of channels. */
    AudioVisualiserComponent (int initialNumChannels);

    /** Destructor. */
    ~AudioVisualiserComponent() override;

    /** Changes the number of channels that the visualiser stores. */
    void setNumChannels (int numChannels);

    /** Changes the number of samples that the visualiser keeps in its history.
        Note that this value refers to the number of averaged sample blocks, and each
        block is calculated as the peak of a number of incoming audio samples. To set
        the number of incoming samples per block, use setSamplesPerBlock().
     */
    void setBufferSize (int bufferSize);

    /** */
    void setSamplesPerBlock (int newNumInputSamplesPerBlock) noexcept;

    /** */
    int getSamplesPerBlock() const noexcept                         { return inputSamplesPerBlock; }

    /** Clears the contents of the buffers. */
    void clear();

    /** Pushes a buffer of channels data.
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    void pushBuffer (const AudioBuffer<float>& bufferToPush);

    /** Pushes a buffer of channels data.
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    void pushBuffer (const AudioSourceChannelInfo& bufferToPush);

    /** Pushes a buffer of channels data.
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    void pushBuffer (const float** channelData, int numChannels, int numSamples);

    /** Pushes a single sample (per channel).
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    void pushSample (const float* samplesForEachChannel, int numChannels);

    /** Sets the colours used to paint the */
    void setColours (Colour backgroundColour, Colour waveformColour) noexcept;

    /** Sets the frequency at which the component repaints itself. */
    void setRepaintRate (int frequencyInHz);

    /** Draws a channel of audio data in the given bounds.
        The default implementation just calls getChannelAsPath() and fits this into the given
        area. You may want to override this to draw things differently.
    */
    virtual void paintChannel (Graphics&, Rectangle<float> bounds,
                               const Range<float>* levels, int numLevels, int nextSample);

    /** Creates a path which contains the waveform shape of a given set of range data.
        The path is normalised so that -1 and +1 are its upper and lower bounds, and it
        goes from 0 to numLevels on the X axis.
    */
    void getChannelAsPath (Path& result, const Range<float>* levels, int numLevels, int nextSample);

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;

private:
    struct ChannelInfo;

    OwnedArray<ChannelInfo> channels;
    int numSamples, inputSamplesPerBlock;
    Colour backgroundColour, waveformColour;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioVisualiserComponent)
};

} // namespace juce
