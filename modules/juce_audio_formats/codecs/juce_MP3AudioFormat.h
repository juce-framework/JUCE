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

#if JUCE_USE_MP3AUDIOFORMAT || DOXYGEN

//==============================================================================
/**
    Software-based MP3 decoding format (doesn't currently provide an encoder).

    IMPORTANT DISCLAIMER: By choosing to enable the JUCE_USE_MP3AUDIOFORMAT flag and
    to compile the MP3 code into your software, you do so AT YOUR OWN RISK! By doing so,
    you are agreeing that Raw Material Software Limited is in no way responsible for any patent,
    copyright, or other legal issues that you may suffer as a result.

    The code in juce_MP3AudioFormat.cpp is NOT guaranteed to be free from infringements of 3rd-party
    intellectual property. If you wish to use it, please seek your own independent advice about the
    legality of doing so. If you are not willing to accept full responsibility for the consequences
    of using this code, then do not enable the JUCE_USE_MP3AUDIOFORMAT setting.

    @tags{Audio}
*/
class MP3AudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    MP3AudioFormat();
    ~MP3AudioFormat() override;

    //==============================================================================
    Array<int> getPossibleSampleRates() override;
    Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;
    bool isCompressed() override;
    StringArray getQualityOptions() override;

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream*, bool deleteStreamIfOpeningFails) override;

    AudioFormatWriter* createWriterFor (OutputStream*, double sampleRateToUse,
                                        unsigned int numberOfChannels, int bitsPerSample,
                                        const StringPairArray& metadataValues, int qualityOptionIndex) override;
    using AudioFormat::createWriterFor;
};

#endif

} // namespace juce
