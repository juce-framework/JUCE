#include "AraTestAudioSource.h"

namespace ARA
{
namespace PlugIn
{

void AraTestAudioSource::onEnableAudioSampleAccess ()
{
	// set up cache
	ARASampleCount totalSamples = getChannelCount () * getSampleCount ();
	_audioSourceCache.clear ();
	if (_audioSourceCache.size () < (size_t)totalSamples)
		_audioSourceCache.resize ((size_t)totalSamples);

	// create temporary host audio reader and let it fill the cache
	HostAudioReader reader (this);

	std::vector<void*> vDataOut (getChannelCount ());
	for (int c = 0; c < getChannelCount (); c++)
		vDataOut[c] = &_audioSourceCache[c * (size_t)getSampleCount ()];
	reader.readAudioSamples (0, getSampleCount (), vDataOut.data ());
}

const float* AraTestAudioSource::getChannelBuffer (int channel) const
{
	return &_audioSourceCache[channel * (size_t)getSampleCount ()];
}

}	// namespace PlugIn
}	// namespace Ara