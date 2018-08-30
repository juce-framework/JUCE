//------------------------------------------------------------------------------
//! \file        ARATestAudioSource.h
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

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

#include <vector>

class TestAnalysisResult;

namespace ARA
{
namespace PlugIn
{

/*******************************************************************************/
class ARATestAudioSource : public AudioSource
{
public:
    ARATestAudioSource (Document* document, ARAAudioSourceHostRef hostRef)
    : AudioSource (document, hostRef),
      _analysisResult (nullptr)
    {}

    ~ARATestAudioSource ();

    // may return nullptr if analysis has not completed yet
    const TestAnalysisResult* getAnalysisResult () const { return _analysisResult; }
    void setAnalysisResult (const TestAnalysisResult* analysisResult);

    // render thread sample access:
    // in order to keep this test code as simple as possible, our test audio source uses brute
    // force and caches all samples in-memory so that renderers can access it without threading issues
    // the document controller triggers filling this cache on the main thread, immediately after access is enabled.
    // actual plug-ins will use a multi-threaded setup to only cache sections of the audio source on demand -
    // a sophisticated file I/O threading implementation is needed for file-based processing regardless of ARA.
    void updateRenderSampleCache ();
    const float* getRenderSampleCacheForChannel (ARAChannelCount channel) const;
    void destroyRenderSampleCache ();

protected:
    const TestAnalysisResult* _analysisResult;

    std::vector<float> _sampleCache;
};

}    // namespace PlugIn
}    // namespace ARA
