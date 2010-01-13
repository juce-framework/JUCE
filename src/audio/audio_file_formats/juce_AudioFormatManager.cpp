/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioFormatManager.h"
#include "juce_AiffAudioFormat.h"
#include "juce_WavAudioFormat.h"
#include "juce_FlacAudioFormat.h"
#include "juce_OggVorbisAudioFormat.h"
#include "../../io/files/juce_FileInputStream.h"


//==============================================================================
AudioFormatManager::AudioFormatManager()
    : defaultFormatIndex (0)
{
}

AudioFormatManager::~AudioFormatManager()
{
    clearFormats();
    clearSingletonInstance();
}

juce_ImplementSingleton (AudioFormatManager);


//==============================================================================
void AudioFormatManager::registerFormat (AudioFormat* newFormat,
                                         const bool makeThisTheDefaultFormat)
{
    jassert (newFormat != 0);

    if (newFormat != 0)
    {
#ifdef JUCE_DEBUG
        for (int i = getNumKnownFormats(); --i >= 0;)
        {
            if (getKnownFormat (i)->getFormatName() == newFormat->getFormatName())
            {
                jassertfalse // trying to add the same format twice!
            }
        }
#endif

        if (makeThisTheDefaultFormat)
            defaultFormatIndex = knownFormats.size();

        knownFormats.add (newFormat);
    }
}

void AudioFormatManager::registerBasicFormats()
{
#if JUCE_MAC
    registerFormat (new AiffAudioFormat(), true);
    registerFormat (new WavAudioFormat(), false);
#else
    registerFormat (new WavAudioFormat(), true);
    registerFormat (new AiffAudioFormat(), false);
#endif

#if JUCE_USE_FLAC
    registerFormat (new FlacAudioFormat(), false);
#endif

#if JUCE_USE_OGGVORBIS
    registerFormat (new OggVorbisAudioFormat(), false);
#endif
}

void AudioFormatManager::clearFormats()
{
    for (int i = getNumKnownFormats(); --i >= 0;)
        delete getKnownFormat(i);

    knownFormats.clear();
    defaultFormatIndex = 0;
}

int AudioFormatManager::getNumKnownFormats() const
{
    return knownFormats.size();
}

AudioFormat* AudioFormatManager::getKnownFormat (const int index) const
{
    return (AudioFormat*) knownFormats [index];
}

AudioFormat* AudioFormatManager::getDefaultFormat() const
{
    return getKnownFormat (defaultFormatIndex);
}

AudioFormat* AudioFormatManager::findFormatForFileExtension (const String& fileExtension) const
{
    String e (fileExtension);
    if (! e.startsWithChar (T('.')))
        e = T(".") + e;

    for (int i = 0; i < getNumKnownFormats(); ++i)
        if (getKnownFormat(i)->getFileExtensions().contains (e, true))
            return getKnownFormat(i);

    return 0;
}

const String AudioFormatManager::getWildcardForAllFormats() const
{
    StringArray allExtensions;

    int i;
    for (i = 0; i < getNumKnownFormats(); ++i)
        allExtensions.addArray (getKnownFormat (i)->getFileExtensions());

    allExtensions.trim();
    allExtensions.removeEmptyStrings();

    String s;
    for (i = 0; i < allExtensions.size(); ++i)
    {
        s << T('*');

        if (! allExtensions[i].startsWithChar (T('.')))
            s << T('.');

        s << allExtensions[i];

        if (i < allExtensions.size() - 1)
            s << T(';');
    }

    return s;
}

//==============================================================================
AudioFormatReader* AudioFormatManager::createReaderFor (const File& file)
{
    // you need to actually register some formats before the manager can
    // use them to open a file!
    jassert (knownFormats.size() > 0);

    for (int i = 0; i < getNumKnownFormats(); ++i)
    {
        AudioFormat* const af = getKnownFormat(i);

        if (af->canHandleFile (file))
        {
            InputStream* const in = file.createInputStream();

            if (in != 0)
            {
                AudioFormatReader* const r = af->createReaderFor (in, true);

                if (r != 0)
                    return r;
            }
        }
    }

    return 0;
}

AudioFormatReader* AudioFormatManager::createReaderFor (InputStream* audioFileStream)
{
    // you need to actually register some formats before the manager can
    // use them to open a file!
    jassert (knownFormats.size() > 0);

    ScopedPointer <InputStream> in (audioFileStream);

    if (in != 0)
    {
        const int64 originalStreamPos = in->getPosition();

        for (int i = 0; i < getNumKnownFormats(); ++i)
        {
            AudioFormatReader* const r = getKnownFormat(i)->createReaderFor (in, false);

            if (r != 0)
            {
                in.release();
                return r;
            }

            in->setPosition (originalStreamPos);

            // the stream that is passed-in must be capable of being repositioned so
            // that all the formats can have a go at opening it.
            jassert (in->getPosition() == originalStreamPos);
        }
    }

    return 0;
}


END_JUCE_NAMESPACE
