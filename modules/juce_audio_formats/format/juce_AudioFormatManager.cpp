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

AudioFormatManager::AudioFormatManager() {}
AudioFormatManager::~AudioFormatManager() {}

//==============================================================================
void AudioFormatManager::registerFormat (AudioFormat* newFormat, bool makeThisTheDefaultFormat)
{
    jassert (newFormat != nullptr);

    if (newFormat != nullptr)
    {
       #if JUCE_DEBUG
        for (auto* af : knownFormats)
        {
            if (af->getFormatName() == newFormat->getFormatName())
                jassertfalse; // trying to add the same format twice!
        }
       #endif

        if (makeThisTheDefaultFormat)
            defaultFormatIndex = getNumKnownFormats();

        knownFormats.add (newFormat);
    }
}

void AudioFormatManager::registerBasicFormats()
{
    registerFormat (new WavAudioFormat(), true);
    registerFormat (new AiffAudioFormat(), false);

   #if JUCE_USE_FLAC
    registerFormat (new FlacAudioFormat(), false);
   #endif

   #if JUCE_USE_OGGVORBIS
    registerFormat (new OggVorbisAudioFormat(), false);
   #endif

   #if JUCE_MAC || JUCE_IOS
    registerFormat (new CoreAudioFormat(), false);
   #endif

   #if JUCE_USE_MP3AUDIOFORMAT
    registerFormat (new MP3AudioFormat(), false);
   #endif

   #if JUCE_USE_WINDOWS_MEDIA_FORMAT
    registerFormat (new WindowsMediaAudioFormat(), false);
   #endif
}

void AudioFormatManager::clearFormats()
{
    knownFormats.clear();
    defaultFormatIndex = 0;
}

int AudioFormatManager::getNumKnownFormats() const                  { return knownFormats.size(); }
AudioFormat* AudioFormatManager::getKnownFormat (int index) const   { return knownFormats[index]; }
AudioFormat* AudioFormatManager::getDefaultFormat() const           { return getKnownFormat (defaultFormatIndex); }

AudioFormat* AudioFormatManager::findFormatForFileExtension (const String& fileExtension) const
{
    if (! fileExtension.startsWithChar ('.'))
        return findFormatForFileExtension ("." + fileExtension);

    for (auto* af : knownFormats)
        if (af->getFileExtensions().contains (fileExtension, true))
            return af;

    return nullptr;
}

String AudioFormatManager::getWildcardForAllFormats() const
{
    StringArray extensions;

    for (auto* af : knownFormats)
        extensions.addArray (af->getFileExtensions());

    extensions.trim();
    extensions.removeEmptyStrings();

    for (auto& e : extensions)
        e = (e.startsWithChar ('.') ? "*" : "*.") + e;

    extensions.removeDuplicates (true);
    return extensions.joinIntoString (";");
}

//==============================================================================
AudioFormatReader* AudioFormatManager::createReaderFor (const File& file)
{
    // you need to actually register some formats before the manager can
    // use them to open a file!
    jassert (getNumKnownFormats() > 0);

    for (auto* af : knownFormats)
        if (af->canHandleFile (file))
            if (auto in = file.createInputStream())
                if (auto* r = af->createReaderFor (in.release(), true))
                    return r;

    return nullptr;
}

AudioFormatReader* AudioFormatManager::createReaderFor (std::unique_ptr<InputStream> audioFileStream)
{
    // you need to actually register some formats before the manager can
    // use them to open a file!
    jassert (getNumKnownFormats() > 0);

    if (audioFileStream != nullptr)
    {
        auto originalStreamPos = audioFileStream->getPosition();

        for (auto* af : knownFormats)
        {
            if (auto* r = af->createReaderFor (audioFileStream.get(), false))
            {
                audioFileStream.release();
                return r;
            }

            audioFileStream->setPosition (originalStreamPos);

            // the stream that is passed-in must be capable of being repositioned so
            // that all the formats can have a go at opening it.
            jassert (audioFileStream->getPosition() == originalStreamPos);
        }
    }

    return nullptr;
}

} // namespace juce
