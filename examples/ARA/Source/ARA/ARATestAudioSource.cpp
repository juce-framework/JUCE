//------------------------------------------------------------------------------
//! \file        ARATestAudioSource.cpp
//! \description audio source implementation for the ARA sample plug-in
//! \project     ARA SDK, examples
//------------------------------------------------------------------------------
// Copyright (c) 2012-2018, Celemony Software GmbH, All Rights Reserved.
//
// License & Disclaimer:
//
// This Software Development Kit (SDK) may not be distributed in parts or
// its entirety without prior written agreement of Celemony Software GmbH.
//
// This SDK must not be used to modify, adapt, reproduce or transfer any
// software and/or technology used in any Celemony and/or Third-party
// application and/or software module (hereinafter referred as "the Software");
// to modify, translate, reverse engineer, decompile, disassemble or create
// derivative works based on the Software (except to the extent applicable laws
// specifically prohibit such restriction) or to lease, assign, distribute or
// otherwise transfer rights to the Software.
// Neither the name of the Celemony Software GmbH nor the names of its
// contributors may be used to endorse or promote products derived from this
// SDK without specific prior written permission.
//
// This SDK is provided by Celemony Software GmbH "as is" and any express or
// implied warranties, including, but not limited to, the implied warranties of
// non-infringement, merchantability and fitness for a particular purpose are
// disclaimed.
// In no event shall Celemony Software GmbH be liable for any direct, indirect,
// incidental, special, exemplary, or consequential damages (including, but not
// limited to, procurement of substitute goods or services; loss of use, data,
// or profits; or business interruption) however caused and on any theory of
// liability, whether in contract, strict liability, or tort (including
// negligence or otherwise) arising in any way out of the use of this software,
// even if advised of the possibility of such damage.
// Where the liability of Celemony Software GmbH is ruled out or restricted,
// this will also apply for the personal liability of employees, representatives
// and vicarious agents.
// The above restriction of liability will not apply if the damages suffered
// are attributable to willful intent or gross negligence or in the case of
// physical injury.
//------------------------------------------------------------------------------

#include "ARATestAudioSource.h"
#include "TestAnalysis.h"

namespace ARA
{
namespace PlugIn
{

ARATestAudioSource::~ARATestAudioSource ()
{
    setAnalysisResult (nullptr);
}

void ARATestAudioSource::setAnalysisResult (const TestAnalysisResult* analysisResult)
{
    if (analysisResult != _analysisResult)
    {
        delete _analysisResult;
        _analysisResult = analysisResult;
    }
}

void ARATestAudioSource::updateRenderSampleCache ()
{
    ARA_INTERNAL_ASSERT (isSampleAccessEnabled ());

    // set up cache (this is a hack, so we're ignoring potential overflow of 32 bit with long files here...)
    size_t totalSamples = getChannelCount () * (size_t)getSampleCount ();
    _sampleCache.resize (totalSamples);

    // create temporary host audio reader and let it fill the cache
    // (we can safely ignore any errors while reading since host must clear buffers in that case,
    // as well as report the error to the user)
    HostAudioReader audioReader (this);
    std::vector<void*> dataPointers (getChannelCount ());
    for (ARAChannelCount c = 0; c < getChannelCount (); ++c)
        dataPointers[c] = &_sampleCache[c * (size_t)getSampleCount ()];
    audioReader.readAudioSamples (0, getSampleCount (), dataPointers.data ());
}

const float* ARATestAudioSource::getRenderSampleCacheForChannel (ARAChannelCount channel) const
{
    return &_sampleCache[channel * (size_t)getSampleCount ()];
}

void ARATestAudioSource::destroyRenderSampleCache ()
{
    _sampleCache.clear ();
    _sampleCache.resize (0);
}

}    // namespace PlugIn
}    // namespace ARA
