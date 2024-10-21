/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioFormat::AudioFormat (String name, StringArray extensions)
   : formatName (name), fileExtensions (extensions)
{
}

AudioFormat::AudioFormat (StringRef name, StringRef extensions)
   : formatName (name.text), fileExtensions (StringArray::fromTokens (extensions, false))
{
}

AudioFormat::~AudioFormat()
{
}

bool AudioFormat::canHandleFile (const File& f)
{
    for (auto& e : getFileExtensions())
        if (f.hasFileExtension (e))
            return true;

    return false;
}

const String& AudioFormat::getFormatName() const                { return formatName; }
StringArray AudioFormat::getFileExtensions() const              { return fileExtensions; }
bool AudioFormat::isCompressed()                                { return false; }
StringArray AudioFormat::getQualityOptions()                    { return {}; }

MemoryMappedAudioFormatReader* AudioFormat::createMemoryMappedReader (const File&)
{
    return nullptr;
}

MemoryMappedAudioFormatReader* AudioFormat::createMemoryMappedReader (FileInputStream* fin)
{
    delete fin;
    return nullptr;
}

bool AudioFormat::isChannelLayoutSupported (const AudioChannelSet& channelSet)
{
    if (channelSet == AudioChannelSet::mono())      return canDoMono();
    if (channelSet == AudioChannelSet::stereo())    return canDoStereo();

    return false;
}

AudioFormatWriter* AudioFormat::createWriterFor (OutputStream* streamToWriteTo,
                                                 double sampleRateToUse,
                                                 const AudioChannelSet& channelLayout,
                                                 int bitsPerSample,
                                                 const StringPairArray& metadataValues,
                                                 int qualityOptionIndex)
{
    if (isChannelLayoutSupported (channelLayout))
        return createWriterFor (streamToWriteTo, sampleRateToUse,
                                static_cast<unsigned int> (channelLayout.size()),
                                bitsPerSample, metadataValues, qualityOptionIndex);

    return nullptr;
}

} // namespace juce
