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

#ifndef JUCE_AUDIOCHANNELSET_H_INCLUDED
#define JUCE_AUDIOCHANNELSET_H_INCLUDED


//==============================================================================
/**
    Represents a set of audio channel types.

    For example, you might have a set of left + right channels, which is a stereo
    channel set. It is a collection of values from the AudioChannelSet::ChannelType
    enum, where each type may only occur once within the set.

    @see AudioProcessorBus
*/
class JUCE_API  AudioChannelSet
{
public:
    /** Creates an empty channel set.
        You can call addChannel to add channels to the set.
    */
    AudioChannelSet() noexcept  {}

    /** Creates a zero-channel set which can be used to indicate that a
        bus is disabled. */
    static AudioChannelSet disabled();

    /** Creates a one-channel mono set. */
    static AudioChannelSet mono();

    /** Creates a set containing a left and right channel. */
    static AudioChannelSet stereo();

    /** Creates a set containing a left, right and centre channels. */
    static AudioChannelSet createLCR();

    /** Creates a set containing a left, right and centre channels. */
    static AudioChannelSet createLRS();

    /** Creates a set containing a left, right, centre and surround channels. */
    static AudioChannelSet createLCRS();

    /** Creates a set for quadraphonic surround setup. */
    static AudioChannelSet quadraphonic();

    /** Creates a set for pentagonal surround setup. */
    static AudioChannelSet pentagonal();

    /** Creates a set for hexagonal surround setup. */
    static AudioChannelSet hexagonal();

    /** Creates a set for octagonal surround setup. */
    static AudioChannelSet octagonal();

    /** Creates a set for ambisonic surround setups. */
    static AudioChannelSet ambisonic();

    /** Creates a set for a 5.0 surround setup. */
    static AudioChannelSet create5point0();

    /** Creates a set for a 5.1 surround setup. */
    static AudioChannelSet create5point1();

    /** Creates a set for a 6.0 Cine surround setup. */
    static AudioChannelSet create6point0();

    /** Creates a set for a 6.0 Music surround setup. */
    static AudioChannelSet create6point0Music();

    /** Creates a set for a 6.1 surround setup. */
    static AudioChannelSet create6point1();

    /** Creates a set for a 7.0 surround setup. */
    static AudioChannelSet create7point0();

    /** Creates a set for a 7.1 surround setup. */
    static AudioChannelSet create7point1();

    /** Creates a set for a 7.1 AC3 C surround setup. */
    static AudioChannelSet create7point1AC3();

    /** Creates a set for a 7.0 surround setup (with side instead of rear speakers). */
    static AudioChannelSet createFront7point0();

    /** Creates a set for a 7.1 surround setup (with side instead of rear speakers). */
    static AudioChannelSet createFront7point1();

    /** Creates a set of untyped discrete channels. */
    static AudioChannelSet discreteChannels (int numChannels);

    /** Create a canonical channel set for a given number of channels.
        For example, numChannels = 1 will return mono, numChannels = 2 will return stereo, etc. */
    static AudioChannelSet canonicalChannelSet (int numChannels);

    //==============================================================================
    /** Represents different audio channel types. */
    enum ChannelType
    {
        unknown             = 0,

        left                = 1,
        right               = 2,
        centre              = 3,

        subbass             = 4,
        leftSurround        = 5,
        rightSurround       = 6,
        leftCentre          = 7,
        rightCentre         = 8,
        surround            = 9,
        leftSurroundDirect  = 10,     // also known as "side left"
        rightSurroundDirect = 11,     // also known as "side right"
        topMiddle           = 12,
        topFrontLeft        = 13,
        topFrontCentre      = 14,
        topFrontRight       = 15,
        topRearLeft         = 16,
        topRearCentre       = 17,
        topRearRight        = 18,
        subbass2            = 19,
        leftRearSurround    = 20,
        rightRearSurround   = 21,
        wideLeft            = 22,
        wideRight           = 23,


        ambisonicW          = 24,
        ambisonicX          = 25,
        ambisonicY          = 26,
        ambisonicZ          = 27,


        discreteChannel0    = 64  /**< Non-typed individual channels are indexed upwards from this value. */
    };

    /** Returns the name of a given channel type. For example, this method may return "Surround Left". */
    static String getChannelTypeName (ChannelType);

    /** Returns the abbreviated name of a channel type. For example, this method may return "Ls". */
    static String getAbbreviatedChannelTypeName (ChannelType);

    //==============================================================================
    /** Adds a channel to the set. */
    void addChannel (ChannelType newChannelType);

    /** Returns the number of channels in the set. */
    int size() const noexcept;

    /** Returns the number of channels in the set. */
    bool isDisabled() const noexcept                    { return size() == 0; }

    /** Returns an array of all the types in this channel set. */
    Array<ChannelType> getChannelTypes() const;

    /** Returns the type of one of the channels in the set, by index. */
    ChannelType getTypeOfChannel (int channelIndex) const noexcept;

    /** Returns the index for a particular channel-type.
        Will return -1 if the this set does not contain a channel of this type. */
    int getChannelIndexForType (ChannelType type) const noexcept;

    /** Returns a string containing a whitespace-separated list of speaker types
        corresponding to each channel. For example in a 5.1 arrangement,
        the string may be "L R C Lfe Ls Rs". If the speaker arrangement is unknown,
        the returned string will be empty.*/
    String getSpeakerArrangementAsString() const;

    /** Returns the description of the current layout. For example, this method may return
        "Quadraphonic". Note that the returned string may not be unique. */
    String getDescription() const;

    /** Returns if this is a channel layout made-up of discrete channels. */
    bool isDiscreteLayout() const noexcept;

    //==============================================================================
    bool operator== (const AudioChannelSet&) const noexcept;
    bool operator!= (const AudioChannelSet&) const noexcept;
    bool operator<  (const AudioChannelSet&) const noexcept;
private:
    BigInteger channels;

    explicit AudioChannelSet (uint32);
};



#endif   // JUCE_AUDIOCHANNELSET_H_INCLUDED
