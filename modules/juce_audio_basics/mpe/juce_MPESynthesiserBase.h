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

#ifndef JUCE_MPESynthesiserBase_H_INCLUDED
#define JUCE_MPESynthesiserBase_H_INCLUDED


//==============================================================================
/**
    Derive from this class to create a basic audio generator capable of MPE.
    Implement the callbacks of MPEInstrument::Listener (noteAdded, notePressureChanged
    etc.) to let your audio generator know that MPE notes were triggered, modulated,
    or released. What to do inside them, and how that influences your audio generator,
    is up to you!

    This class uses an instance of MPEInstrument internally to handle the MPE
    note state logic.

    This class is a very low-level base class for an MPE instrument. If you need
    something more sophisticated, have a look at MPESynthesiser. This class extends
    MPESynthesiserBase by adding the concept of voices that can play notes,
    a voice stealing algorithm, and much more.

    @see MPESynthesiser, MPEInstrument
*/
struct JUCE_API  MPESynthesiserBase   : public MPEInstrument::Listener
{
public:
    //==============================================================================
    /** Constructor. */
    MPESynthesiserBase();

    /** Constructor.

        If you use this constructor, the synthesiser will take ownership of the
        provided instrument object, and will use it internally to handle the
        MPE note state logic.
        This is useful if you want to use an instance of your own class derived
        from MPEInstrument for the MPE logic.
    */
    MPESynthesiserBase (MPEInstrument* instrument);

    //==============================================================================
    /** Returns the synthesiser's internal MPE zone layout.
        This happens by value, to enforce thread-safety and class invariants.
    */
    MPEZoneLayout getZoneLayout() const noexcept;

    /** Re-sets the synthesiser's internal MPE zone layout to the one passed in.
        As a side effect, this will discard all currently playing notes,
        call noteReleased for all of them, and disable legacy mode (if previously enabled).
    */
    void setZoneLayout (MPEZoneLayout newLayout);

    //==============================================================================
    /** Tells the synthesiser what the sample rate is for the audio it's being
        used to render.
    */
    virtual void setCurrentPlaybackSampleRate (double sampleRate);

    /** Returns the current target sample rate at which rendering is being done.
        Subclasses may need to know this so that they can pitch things correctly.
    */
    double getSampleRate() const noexcept          { return sampleRate; }

    //==============================================================================
    /** Creates the next block of audio output.

        Call this to make sound. This will chop up the AudioBuffer into subBlock
        pieces separated by events in the MIDI buffer, and then call
        processNextSubBlock on each one of them. In between you will get calls
        to noteAdded/Changed/Finished, where you can update parameters that
        depend on those notes to use for your audio rendering.
    */
    template <typename floatType>
    void renderNextBlock (AudioBuffer<floatType>& outputAudio,
                          const MidiBuffer& inputMidi,
                          int startSample,
                          int numSamples);

    //==============================================================================
    /** Handle incoming MIDI events (called from renderNextBlock).

        The default implementation provided here simply forwards everything
        to MPEInstrument::processNextMidiEvent, where it is used to update the
        MPE notes, zones etc. MIDI messages not relevant for MPE are ignored.

        This method can be overridden if you need to do custom MIDI handling
        on top of MPE. The MPESynthesiser class overrides this to implement
        callbacks for MIDI program changes and non-MPE-related MIDI controller
        messages.
    */
    virtual void handleMidiEvent (const MidiMessage&);

    //==============================================================================
    /** Sets a minimum limit on the size to which audio sub-blocks will be divided when rendering.

        When rendering, the audio blocks that are passed into renderNextBlock() will be split up
        into smaller blocks that lie between all the incoming midi messages, and it is these smaller
        sub-blocks that are rendered with multiple calls to renderVoices().

        Obviously in a pathological case where there are midi messages on every sample, then
        renderVoices() could be called once per sample and lead to poor performance, so this
        setting allows you to set a lower limit on the block size.

        The default setting is 32, which means that midi messages are accurate to about < 1ms
        accuracy, which is probably fine for most purposes, but you may want to increase or
        decrease this value for your synth.
    */
    void setMinimumRenderingSubdivisionSize (int numSamples) noexcept;

    //==============================================================================
    /** Puts the synthesiser into legacy mode.

        @param pitchbendRange   The note pitchbend range in semitones to use when in legacy mode.
                                Must be between 0 and 96, otherwise behaviour is undefined.
                                The default pitchbend range in legacy mode is +/- 2 semitones.
        @param channelRange     The range of MIDI channels to use for notes when in legacy mode.
                                The default is to use all MIDI channels (1-16).

        To get out of legacy mode, set a new MPE zone layout using setZoneLayout.
    */
    void enableLegacyMode (int pitchbendRange = 2,
                           Range<int> channelRange = Range<int> (1, 17));

    /** Returns true if the instrument is in legacy mode, false otherwise. */
    bool isLegacyModeEnabled() const noexcept;

    /** Returns the range of MIDI channels (1-16) to be used for notes when in legacy mode. */
    Range<int> getLegacyModeChannelRange() const noexcept;

    /** Re-sets the range of MIDI channels (1-16) to be used for notes when in legacy mode. */
    void setLegacyModeChannelRange (Range<int> channelRange);

    /** Returns the pitchbend range in semitones (0-96) to be used for notes when in legacy mode. */
    int getLegacyModePitchbendRange() const noexcept;

    /** Re-sets the pitchbend range in semitones (0-96) to be used for notes when in legacy mode. */
    void setLegacyModePitchbendRange (int pitchbendRange);

protected:
    //==============================================================================
    /** Implement this method to render your audio inside.
        @see renderNextBlock
    */
    virtual void renderNextSubBlock (AudioBuffer<float>& outputAudio,
                                     int startSample,
                                     int numSamples) = 0;

    /** Implement this method if you want to render 64-bit audio as well;
        otherwise leave blank.
     */
    virtual void renderNextSubBlock (AudioBuffer<double>& /*outputAudio*/,
                                     int /*startSample*/,
                                     int /*numSamples*/) {}

protected:
    //==============================================================================
    /** @internal */
    ScopedPointer<MPEInstrument> instrument;
    /** @internal */
    CriticalSection renderAudioLock;

private:
    //==============================================================================
    double sampleRate;
    int minimumSubBlockSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESynthesiserBase)
};


#endif // JUCE_MPESynthesiserBase_H_INCLUDED
