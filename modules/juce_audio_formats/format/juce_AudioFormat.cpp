/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
