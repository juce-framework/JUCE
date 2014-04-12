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

#ifndef JUCE_SYNTHESISER_H_INCLUDED
#define JUCE_SYNTHESISER_H_INCLUDED


//==============================================================================
/**
    Describes one of the sounds that a Synthesiser can play.

    A synthesiser can contain one or more sounds, and a sound can choose which
    midi notes and channels can trigger it.

    The SynthesiserSound is a passive class that just describes what the sound is -
    the actual audio rendering for a sound is done by a SynthesiserVoice. This allows
    more than one SynthesiserVoice to play the same sound at the same time.

    @see Synthesiser, SynthesiserVoice
*/
class JUCE_API  SynthesiserSound    : public ReferenceCountedObject
{
protected:
    //==============================================================================
    SynthesiserSound();

public:
    /** Destructor. */
    virtual ~SynthesiserSound();

    //==============================================================================
    /** Returns true if this sound should be played when a given midi note is pressed.

        The Synthesiser will use this information when deciding which sounds to trigger
        for a given note.
    */
    virtual bool appliesToNote (const int midiNoteNumber) = 0;

    /** Returns true if the sound should be triggered by midi events on a given channel.

        The Synthesiser will use this information when deciding which sounds to trigger
        for a given note.
    */
    virtual bool appliesToChannel (const int midiChannel) = 0;

    /** The class is reference-counted, so this is a handy pointer class for it. */
    typedef ReferenceCountedObjectPtr<SynthesiserSound> Ptr;


private:
    //==============================================================================
    JUCE_LEAK_DETECTOR (SynthesiserSound)
};


//==============================================================================
/**
    Represents a voice that a Synthesiser can use to play a SynthesiserSound.

    A voice plays a single sound at a time, and a synthesiser holds an array of
    voices so that it can play polyphonically.

    @see Synthesiser, SynthesiserSound
*/
class JUCE_API  SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a voice. */
    SynthesiserVoice();

    /** Destructor. */
    virtual ~SynthesiserVoice();

    //==============================================================================
    /** Returns the midi note that this voice is currently playing.
        Returns a value less than 0 if no note is playing.
    */
    int getCurrentlyPlayingNote() const noexcept                        { return currentlyPlayingNote; }

    /** Returns the sound that this voice is currently playing.
        Returns nullptr if it's not playing.
    */
    SynthesiserSound::Ptr getCurrentlyPlayingSound() const noexcept     { return currentlyPlayingSound; }

    /** Must return true if this voice object is capable of playing the given sound.

        If there are different classes of sound, and different classes of voice, a voice can
        choose which ones it wants to take on.

        A typical implementation of this method may just return true if there's only one type
        of voice and sound, or it might check the type of the sound object passed-in and
        see if it's one that it understands.
    */
    virtual bool canPlaySound (SynthesiserSound*) = 0;

    /** Called to start a new note.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void startNote (int midiNoteNumber,
                            float velocity,
                            SynthesiserSound* sound,
                            int currentPitchWheelPosition) = 0;

    /** Called to stop a note.

        This will be called during the rendering callback, so must be fast and thread-safe.

        If allowTailOff is false or the voice doesn't want to tail-off, then it must stop all
        sound immediately, and must call clearCurrentNote() to reset the state of this voice
        and allow the synth to reassign it another sound.

        If allowTailOff is true and the voice decides to do a tail-off, then it's allowed to
        begin fading out its sound, and it can stop playing until it's finished. As soon as it
        finishes playing (during the rendering callback), it must make sure that it calls
        clearCurrentNote().
    */
    virtual void stopNote (bool allowTailOff) = 0;

    /** Called to let the voice know that the pitch wheel has been moved.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void pitchWheelMoved (int newPitchWheelValue) = 0;

    /** Called to let the voice know that a midi controller has been moved.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void controllerMoved (int controllerNumber, int newControllerValue) = 0;

    /** Called to let the voice know that the aftertouch has changed.
        This will be called during the rendering callback, so must be fast and thread-safe.
    */
    virtual void aftertouchChanged (int newAftertouchValue);

    //==============================================================================
    /** Renders the next block of data for this voice.

        The output audio data must be added to the current contents of the buffer provided.
        Only the region of the buffer between startSample and (startSample + numSamples)
        should be altered by this method.

        If the voice is currently silent, it should just return without doing anything.

        If the sound that the voice is playing finishes during the course of this rendered
        block, it must call clearCurrentNote(), to tell the synthesiser that it has finished.

        The size of the blocks that are rendered can change each time it is called, and may
        involve rendering as little as 1 sample at a time. In between rendering callbacks,
        the voice's methods will be called to tell it about note and controller events.
    */
    virtual void renderNextBlock (AudioSampleBuffer& outputBuffer,
                                  int startSample,
                                  int numSamples) = 0;

    /** Returns true if the voice is currently playing a sound which is mapped to the given
        midi channel.

        If it's not currently playing, this will return false.
    */
    bool isPlayingChannel (int midiChannel) const;

    /** Changes the voice's reference sample rate.

        The rate is set so that subclasses know the output rate and can set their pitch
        accordingly.

        This method is called by the synth, and subclasses can access the current rate with
        the currentSampleRate member.
    */
    void setCurrentPlaybackSampleRate (double newRate);

    /** Returns true if the key that triggered this voice is still held down.
        Note that the voice may still be playing after the key was released (e.g because the
        sostenuto pedal is down).
    */
    bool isKeyDown() const noexcept                             { return keyIsDown; }

    /** Returns true if the sostenuto pedal is currently active for this voice. */
    bool isSostenutoPedalDown() const noexcept                  { return sostenutoPedalDown; }

    /** Returns true if this voice started playing its current note before the other voice did. */
    bool wasStartedBefore (const SynthesiserVoice& other) const noexcept;

protected:
    //==============================================================================
    /** Returns the current target sample rate at which rendering is being done.

        This is available for subclasses so they can pitch things correctly.
    */
    double getSampleRate() const                                { return currentSampleRate; }

    /** Resets the state of this voice after a sound has finished playing.

        The subclass must call this when it finishes playing a note and becomes available
        to play new ones.

        It must either call it in the stopNote() method, or if the voice is tailing off,
        then it should call it later during the renderNextBlock method, as soon as it
        finishes its tail-off.

        It can also be called at any time during the render callback if the sound happens
        to have finished, e.g. if it's playing a sample and the sample finishes.
    */
    void clearCurrentNote();


private:
    //==============================================================================
    friend class Synthesiser;

    double currentSampleRate;
    int currentlyPlayingNote;
    uint32 noteOnTime;
    SynthesiserSound::Ptr currentlyPlayingSound;
    bool keyIsDown, sostenutoPedalDown;

    JUCE_LEAK_DETECTOR (SynthesiserVoice)
};


//==============================================================================
/**
    Base class for a musical device that can play sounds.

    To create a synthesiser, you'll need to create a subclass of SynthesiserSound
    to describe each sound available to your synth, and a subclass of SynthesiserVoice
    which can play back one of these sounds.

    Then you can use the addVoice() and addSound() methods to give the synthesiser a
    set of sounds, and a set of voices it can use to play them. If you only give it
    one voice it will be monophonic - the more voices it has, the more polyphony it'll
    have available.

    Then repeatedly call the renderNextBlock() method to produce the audio. Any midi
    events that go in will be scanned for note on/off messages, and these are used to
    start and stop the voices playing the appropriate sounds.

    While it's playing, you can also cause notes to be triggered by calling the noteOn(),
    noteOff() and other controller methods.

    Before rendering, be sure to call the setCurrentPlaybackSampleRate() to tell it
    what the target playback rate is. This value is passed on to the voices so that
    they can pitch their output correctly.
*/
class JUCE_API  Synthesiser
{
public:
    //==============================================================================
    /** Creates a new synthesiser.

        You'll need to add some sounds and voices before it'll make any sound..
    */
    Synthesiser();

    /** Destructor. */
    virtual ~Synthesiser();

    //==============================================================================
    /** Deletes all voices. */
    void clearVoices();

    /** Returns the number of voices that have been added. */
    int getNumVoices() const noexcept                               { return voices.size(); }

    /** Returns one of the voices that have been added. */
    SynthesiserVoice* getVoice (int index) const;

    /** Adds a new voice to the synth.

        All the voices should be the same class of object and are treated equally.

        The object passed in will be managed by the synthesiser, which will delete
        it later on when no longer needed. The caller should not retain a pointer to the
        voice.
    */
    SynthesiserVoice* addVoice (SynthesiserVoice* newVoice);

    /** Deletes one of the voices. */
    void removeVoice (int index);

    //==============================================================================
    /** Deletes all sounds. */
    void clearSounds();

    /** Returns the number of sounds that have been added to the synth. */
    int getNumSounds() const noexcept                               { return sounds.size(); }

    /** Returns one of the sounds. */
    SynthesiserSound* getSound (int index) const noexcept           { return sounds [index]; }

    /** Adds a new sound to the synthesiser.

        The object passed in is reference counted, so will be deleted when the
        synthesiser and all voices are no longer using it.
    */
    SynthesiserSound* addSound (const SynthesiserSound::Ptr& newSound);

    /** Removes and deletes one of the sounds. */
    void removeSound (int index);

    //==============================================================================
    /** If set to true, then the synth will try to take over an existing voice if
        it runs out and needs to play another note.

        The value of this boolean is passed into findFreeVoice(), so the result will
        depend on the implementation of this method.
    */
    void setNoteStealingEnabled (bool shouldStealNotes);

    /** Returns true if note-stealing is enabled.
        @see setNoteStealingEnabled
    */
    bool isNoteStealingEnabled() const noexcept                     { return shouldStealNotes; }

    //==============================================================================
    /** Triggers a note-on event.

        The default method here will find all the sounds that want to be triggered by
        this note/channel. For each sound, it'll try to find a free voice, and use the
        voice to start playing the sound.

        Subclasses might want to override this if they need a more complex algorithm.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        The midiChannel parameter is the channel, between 1 and 16 inclusive.
    */
    virtual void noteOn (int midiChannel,
                         int midiNoteNumber,
                         float velocity);

    /** Triggers a note-off event.

        This will turn off any voices that are playing a sound for the given note/channel.

        If allowTailOff is true, the voices will be allowed to fade out the notes gracefully
        (if they can do). If this is false, the notes will all be cut off immediately.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        The midiChannel parameter is the channel, between 1 and 16 inclusive.
    */
    virtual void noteOff (int midiChannel,
                          int midiNoteNumber,
                          bool allowTailOff);

    /** Turns off all notes.

        This will turn off any voices that are playing a sound on the given midi channel.

        If midiChannel is 0 or less, then all voices will be turned off, regardless of
        which channel they're playing. Otherwise it represents a valid midi channel, from
        1 to 16 inclusive.

        If allowTailOff is true, the voices will be allowed to fade out the notes gracefully
        (if they can do). If this is false, the notes will all be cut off immediately.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.
    */
    virtual void allNotesOff (int midiChannel,
                              bool allowTailOff);

    /** Sends a pitch-wheel message to any active voices.

        This will send a pitch-wheel message to any voices that are playing sounds on
        the given midi channel.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        @param midiChannel          the midi channel, from 1 to 16 inclusive
        @param wheelValue           the wheel position, from 0 to 0x3fff, as returned by MidiMessage::getPitchWheelValue()
    */
    virtual void handlePitchWheel (int midiChannel,
                                   int wheelValue);

    /** Sends a midi controller message to any active voices.

        This will send a midi controller message to any voices that are playing sounds on
        the given midi channel.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        @param midiChannel          the midi channel, from 1 to 16 inclusive
        @param controllerNumber     the midi controller type, as returned by MidiMessage::getControllerNumber()
        @param controllerValue      the midi controller value, between 0 and 127, as returned by MidiMessage::getControllerValue()
    */
    virtual void handleController (int midiChannel,
                                   int controllerNumber,
                                   int controllerValue);

    /** Sends an aftertouch message.

        This will send an aftertouch message to any voices that are playing sounds on
        the given midi channel and note number.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(), but may be called explicitly too.

        @param midiChannel          the midi channel, from 1 to 16 inclusive
        @param midiNoteNumber       the midi note number, 0 to 127
        @param aftertouchValue      the aftertouch value, between 0 and 127,
                                    as returned by MidiMessage::getAftertouchValue()
    */
    virtual void handleAftertouch (int midiChannel, int midiNoteNumber, int aftertouchValue);

    /** Handles a sustain pedal event. */
    virtual void handleSustainPedal (int midiChannel, bool isDown);

    /** Handles a sostenuto pedal event. */
    virtual void handleSostenutoPedal (int midiChannel, bool isDown);

    /** Can be overridden to handle soft pedal events. */
    virtual void handleSoftPedal (int midiChannel, bool isDown);

    //==============================================================================
    /** Tells the synthesiser what the sample rate is for the audio it's being used to render.

        This value is propagated to the voices so that they can use it to render the correct
        pitches.
    */
    void setCurrentPlaybackSampleRate (double sampleRate);

    /** Creates the next block of audio output.

        This will process the next numSamples of data from all the voices, and add that output
        to the audio block supplied, starting from the offset specified. Note that the
        data will be added to the current contents of the buffer, so you should clear it
        before calling this method if necessary.

        The midi events in the inputMidi buffer are parsed for note and controller events,
        and these are used to trigger the voices. Note that the startSample offset applies
        both to the audio output buffer and the midi input buffer, so any midi events
        with timestamps outside the specified region will be ignored.
    */
    void renderNextBlock (AudioSampleBuffer& outputAudio,
                          const MidiBuffer& inputMidi,
                          int startSample,
                          int numSamples);

protected:
    //==============================================================================
    /** This is used to control access to the rendering callback and the note trigger methods. */
    CriticalSection lock;

    OwnedArray<SynthesiserVoice> voices;
    ReferenceCountedArray<SynthesiserSound> sounds;

    /** The last pitch-wheel values for each midi channel. */
    int lastPitchWheelValues [16];

    /** Searches through the voices to find one that's not currently playing, and which
        can play the given sound.

        Returns nullptr if all voices are busy and stealing isn't enabled.

        This can be overridden to implement custom voice-stealing algorithms.
    */
    virtual SynthesiserVoice* findFreeVoice (SynthesiserSound* soundToPlay,
                                             const bool stealIfNoneAvailable) const;

    /** Chooses a voice that is most suitable for being re-used.
        The default method returns the one that has been playing for the longest, but
        you may want to override this and do something more cunning instead.
    */
    virtual SynthesiserVoice* findVoiceToSteal (SynthesiserSound* soundToPlay) const;

    /** Starts a specified voice playing a particular sound.

        You'll probably never need to call this, it's used internally by noteOn(), but
        may be needed by subclasses for custom behaviours.
    */
    void startVoice (SynthesiserVoice* voice,
                     SynthesiserSound* sound,
                     int midiChannel,
                     int midiNoteNumber,
                     float velocity);

    /** Can be overridden to do custom handling of incoming midi events. */
    virtual void handleMidiEvent (const MidiMessage&);

private:
    //==============================================================================
    double sampleRate;
    uint32 lastNoteOnCounter;
    bool shouldStealNotes;
    BigInteger sustainPedalsDown;

    void stopVoice (SynthesiserVoice*, bool allowTailOff);

   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // Note the new parameters for this method.
    virtual int findFreeVoice (const bool) const { return 0; }
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Synthesiser)
};


#endif   // JUCE_SYNTHESISER_H_INCLUDED
