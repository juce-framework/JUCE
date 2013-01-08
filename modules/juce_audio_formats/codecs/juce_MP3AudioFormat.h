/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#if JUCE_USE_MP3AUDIOFORMAT || DOXYGEN

//==============================================================================
/**
    Software-based MP3 decoding format (doesn't currently provide an encoder).

    IMPORTANT DISCLAIMER: By choosing to enable the JUCE_USE_MP3AUDIOFORMAT flag and
    to compile the MP3 code into your software, you do so AT YOUR OWN RISK! By doing so,
    you are agreeing that Raw Material Software is in no way responsible for any patent,
    copyright, or other legal issues that you may suffer as a result.

    The code in juce_MP3AudioFormat.cpp is NOT guaranteed to be free from infringements of 3rd-party
    intellectual property. If you wish to use it, please seek your own independent advice about the
    legality of doing so. If you are not willing to accept full responsibility for the consequences
    of using this code, then do not enable the JUCE_USE_MP3AUDIOFORMAT setting.
*/
class MP3AudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    MP3AudioFormat();
    ~MP3AudioFormat();

    //==============================================================================
    Array<int> getPossibleSampleRates();
    Array<int> getPossibleBitDepths();
    bool canDoStereo();
    bool canDoMono();
    bool isCompressed();
    StringArray getQualityOptions();

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream*, bool deleteStreamIfOpeningFails);

    AudioFormatWriter* createWriterFor (OutputStream*, double sampleRateToUse,
                                        unsigned int numberOfChannels, int bitsPerSample,
                                        const StringPairArray& metadataValues, int qualityOptionIndex);
};

#endif
