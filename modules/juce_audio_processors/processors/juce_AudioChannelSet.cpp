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

AudioChannelSet::AudioChannelSet (uint32 c) : channels (c) {}

bool AudioChannelSet::operator== (const AudioChannelSet& other) const noexcept  { return channels == other.channels; }
bool AudioChannelSet::operator!= (const AudioChannelSet& other) const noexcept  { return channels != other.channels; }
bool AudioChannelSet::operator<  (const AudioChannelSet& other) const noexcept  { return channels <  other.channels; }

String AudioChannelSet::getChannelTypeName (AudioChannelSet::ChannelType type)
{
    if (type >= discreteChannel0)
        return String ("Discrete ") + String (type - discreteChannel0 + 1);

    switch (type)
    {
        case left:                return NEEDS_TRANS("Left");
        case right:               return NEEDS_TRANS("Right");
        case centre:              return NEEDS_TRANS("Centre");
        case subbass:             return NEEDS_TRANS("Subbass");
        case leftSurround:        return NEEDS_TRANS("Left Surround");
        case rightSurround:       return NEEDS_TRANS("Right Surround");
        case leftCentre:          return NEEDS_TRANS("Left Centre");
        case rightCentre:         return NEEDS_TRANS("Right Centre");
        case surround:            return NEEDS_TRANS("Surround");
        case leftRearSurround:    return NEEDS_TRANS("Left Rear Surround");
        case rightRearSurround:   return NEEDS_TRANS("Right Rear Surround");
        case topMiddle:           return NEEDS_TRANS("Top Middle");
        case topFrontLeft:        return NEEDS_TRANS("Top Front Left");
        case topFrontCentre:      return NEEDS_TRANS("Top Front Centre");
        case topFrontRight:       return NEEDS_TRANS("Top Front Right");
        case topRearLeft:         return NEEDS_TRANS("Top Rear Left");
        case topRearCentre:       return NEEDS_TRANS("Top Rear Centre");
        case topRearRight:        return NEEDS_TRANS("Top Rear Right");
        case wideLeft:            return NEEDS_TRANS("Wide Left");
        case wideRight:           return NEEDS_TRANS("Wide Right");
        case subbass2:            return NEEDS_TRANS("Subbass 2");
        case leftSurroundDirect:  return NEEDS_TRANS ("Left Surround Direct");
        case rightSurroundDirect: return NEEDS_TRANS ("Right Surround Direct");
        case ambisonicW:          return NEEDS_TRANS("Ambisonic W");
        case ambisonicX:          return NEEDS_TRANS("Ambisonic X");
        case ambisonicY:          return NEEDS_TRANS("Ambisonic Y");
        case ambisonicZ:          return NEEDS_TRANS("Ambisonic Z");
        default:                  break;
    }

    return "Unknown";
}

String AudioChannelSet::getAbbreviatedChannelTypeName (AudioChannelSet::ChannelType type)
{
    if (type >= discreteChannel0)
        return String (type - discreteChannel0 + 1);

    switch (type)
    {
        case left:                return "L";
        case right:               return "R";
        case centre:              return "C";
        case subbass:             return "Lfe";
        case leftSurround:        return "Ls";
        case rightSurround:       return "Rs";
        case leftCentre:          return "Lc";
        case rightCentre:         return "Rc";
        case surround:            return "S";
        case leftRearSurround:    return "Lrs";
        case rightRearSurround:   return "Rrs";
        case topMiddle:           return "Tm";
        case topFrontLeft:        return "Tfl";
        case topFrontCentre:      return "Tfc";
        case topFrontRight:       return "Tfr";
        case topRearLeft:         return "Trl";
        case topRearCentre:       return "Trc";
        case topRearRight:        return "Trr";
        case wideLeft:            return "Wl";
        case wideRight:           return "Wr";
        case subbass2:            return "Lfe2";
        case leftSurroundDirect:  return "Lsd";
        case rightSurroundDirect: return "Rsd";
        case ambisonicW:          return "W";
        case ambisonicX:          return "X";
        case ambisonicY:          return "Y";
        case ambisonicZ:          return "Z";
        default:                  break;
    }

    return "";
}

String AudioChannelSet::getSpeakerArrangementAsString() const
{
    StringArray speakerTypes;
    Array<AudioChannelSet::ChannelType> speakers = getChannelTypes();

    for (int i = 0; i < speakers.size(); ++i)
    {
        String name = getAbbreviatedChannelTypeName (speakers.getReference (i));

        if (name.isNotEmpty())
            speakerTypes.add (name);
    }

    return speakerTypes.joinIntoString (" ");
}

String AudioChannelSet::getDescription() const
{
    if (isDiscreteLayout())            return String ("Discrete #") + String (size());
    if (*this == disabled())           return "Disabled";
    if (*this == mono())               return "Mono";
    if (*this == stereo())             return "Stereo";
    if (*this == createLCR())          return "LCR";
    if (*this == createLRS())          return "LRS";
    if (*this == createLCRS())         return "LCRS";
    if (*this == quadraphonic())       return "Quadraphonic";
    if (*this == pentagonal())         return "Pentagonal";
    if (*this == hexagonal())          return "Hexagonal";
    if (*this == octagonal())          return "Octagonal";
    if (*this == ambisonic())          return "Ambisonic";
    if (*this == create5point0())      return "5.1 Surround";
    if (*this == create5point1())      return "5.1 Surround (+Lfe)";
    if (*this == create6point0())      return "6.1 Surround";
    if (*this == create6point0Music()) return "6.1 (Music) Surround";
    if (*this == create6point1())      return "6.1 Surround (+Lfe)";
    if (*this == create7point0())      return "7.1 Surround (Rear)";
    if (*this == create7point1())      return "7.1 Surround (Rear +Lfe)";
    if (*this == create7point1AC3())   return "7.1 AC3 Surround (Rear + Lfe)";
    if (*this == createFront7point0()) return "7.1 Surround (Front)";
    if (*this == createFront7point1()) return "7.1 Surround (Front +Lfe)";

    return "Unknown";
}

bool AudioChannelSet::isDiscreteLayout() const noexcept
{
    Array<AudioChannelSet::ChannelType> speakers = getChannelTypes();
    for (int i = 0; i < speakers.size(); ++i)
        if (speakers.getReference (i) > ambisonicZ)
            return true;

    return false;
}

int AudioChannelSet::size() const noexcept
{
    return channels.countNumberOfSetBits();
}

AudioChannelSet::ChannelType AudioChannelSet::getTypeOfChannel (int index) const noexcept
{
    int bit = channels.findNextSetBit(0);

    for (int i = 0; i < index && bit >= 0; ++i)
        bit = channels.findNextSetBit (bit + 1);

    return static_cast<ChannelType> (bit);
}

int AudioChannelSet::getChannelIndexForType (AudioChannelSet::ChannelType type) const noexcept
{
    int idx = 0;
    for (int bit = channels.findNextSetBit (0); bit >= 0; bit = channels.findNextSetBit (bit + 1))
    {
        if (static_cast<ChannelType> (bit) == type)
            return idx;

        idx++;
    }

    return -1;
}

Array<AudioChannelSet::ChannelType> AudioChannelSet::getChannelTypes() const
{
    Array<ChannelType> result;

    for (int bit = channels.findNextSetBit(0); bit >= 0; bit = channels.findNextSetBit (bit + 1))
        result.add (static_cast<ChannelType> (bit));

    return result;
}

void AudioChannelSet::addChannel (ChannelType newChannel)
{
    const int bit = static_cast<int> (newChannel);
    jassert (bit >= 0 && bit < 1024);
    channels.setBit (bit);
}

AudioChannelSet AudioChannelSet::disabled()           { return AudioChannelSet(); }
AudioChannelSet AudioChannelSet::mono()               { return AudioChannelSet (1u << centre); }
AudioChannelSet AudioChannelSet::stereo()             { return AudioChannelSet ((1u << left) | (1u << right)); }
AudioChannelSet AudioChannelSet::createLCR()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre)); }
AudioChannelSet AudioChannelSet::createLRS()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << surround)); }
AudioChannelSet AudioChannelSet::createLCRS()         { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << surround)); }
AudioChannelSet AudioChannelSet::quadraphonic()       { return AudioChannelSet ((1u << left) | (1u << right) | (1u << leftSurround) | (1u << rightSurround)); }
AudioChannelSet AudioChannelSet::pentagonal()         { return AudioChannelSet ((1u << left) | (1u << right) | (1u << leftRearSurround) | (1u << rightRearSurround) | (1u << centre)); }
AudioChannelSet AudioChannelSet::hexagonal()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << leftRearSurround) | (1u << rightRearSurround) | (1u << centre) | (1u << surround)); }
AudioChannelSet AudioChannelSet::octagonal()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << leftSurround) | (1u << rightSurround) | (1u << centre) | (1u << surround) | (1u << wideLeft) | (1u << wideRight)); }
AudioChannelSet AudioChannelSet::ambisonic()          { return AudioChannelSet ((1u << ambisonicW) | (1u << ambisonicX) | (1u << ambisonicY) | (1u << ambisonicZ)); }
AudioChannelSet AudioChannelSet::create5point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << leftSurround) | (1u << rightSurround)); }
AudioChannelSet AudioChannelSet::create5point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << leftSurround)  | (1u << rightSurround)); }
AudioChannelSet AudioChannelSet::create6point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << leftSurround)  | (1u << rightSurround) | (1u << surround)); }
AudioChannelSet AudioChannelSet::create6point0Music() { return AudioChannelSet ((1u << left) | (1u << right) | (1u << leftRearSurround)  | (1u << rightRearSurround) | (1u << leftSurround) | (1u << rightSurround)); }
AudioChannelSet AudioChannelSet::create6point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass)  | (1u << leftSurround)  | (1u << rightSurround) | (1u << surround)); }
AudioChannelSet AudioChannelSet::create7point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << leftSurround) | (1u << rightSurround) | (1u << leftRearSurround)  | (1u << rightRearSurround)); }
AudioChannelSet AudioChannelSet::create7point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << leftRearSurround)  | (1u << rightRearSurround) | (1u << leftSurround) | (1u << rightSurround)); }
AudioChannelSet AudioChannelSet::create7point1AC3()   { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << leftSurround)  | (1u << rightSurround) | (1u << leftSurroundDirect) | (1u << rightSurroundDirect)); }
AudioChannelSet AudioChannelSet::createFront7point0() { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << leftSurround)  | (1u << rightSurround) | (1u << leftCentre) | (1u << rightCentre)); }
AudioChannelSet AudioChannelSet::createFront7point1() { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << leftSurround)  | (1u << rightSurround) | (1u << leftCentre) | (1u << rightCentre)); }


AudioChannelSet AudioChannelSet::discreteChannels (int numChannels)
{
    AudioChannelSet s;
    s.channels.setRange (discreteChannel0, numChannels, true);
    return s;
}

AudioChannelSet AudioChannelSet::canonicalChannelSet (int numChannels)
{
    if (numChannels == 1) return AudioChannelSet::mono();
    if (numChannels == 2) return AudioChannelSet::stereo();
    if (numChannels == 3) return AudioChannelSet::createLCR();
    if (numChannels == 4) return AudioChannelSet::quadraphonic();
    if (numChannels == 5) return AudioChannelSet::create5point0();
    if (numChannels == 6) return AudioChannelSet::create5point1();
    if (numChannels == 7) return AudioChannelSet::create7point0();
    if (numChannels == 8) return AudioChannelSet::create7point1();

    return discreteChannels (numChannels);
}
