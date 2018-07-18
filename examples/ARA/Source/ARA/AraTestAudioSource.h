//------------------------------------------------------------------------------
//! \file        AraTestPlaybackRenderer.h
//! \description AudioSource implementation for ARA sample plug-in,
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

#include <ARA_Library/PlugIn/AraPlug.h>

namespace ARA
{
namespace PlugIn
{

class AraTestAudioSource : public AudioSource
{
public:
	AraTestAudioSource (Document* document, ARAAudioSourceHostRef hostRef)
	: AudioSource (document, hostRef)
	{}

	// render thread sample access:
	// in order to keep this test code as simple as possible, our test audio source uses brute
	// force and caches all samples in-memory so that renderers can access it without threading issues.
	// furthermore, it fills this cache on the main main thread, immediately after access is enabled.
	// proper code will use a multi-threaded setup to only cache sections of the audio source on demand.
	// such a sophisticated threading implementation is needed for file-based processing regardless of ARA.
	void onEnableAudioSampleAccess ();

	const float* getChannelBuffer (int channel) const;

protected:
	std::vector<float> _audioSourceCache;
};

}	// namespace PlugIn
}	// namespace Ara
