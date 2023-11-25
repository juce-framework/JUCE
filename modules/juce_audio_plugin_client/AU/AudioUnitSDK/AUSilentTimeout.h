/*!
	@file		AudioUnitSDK/AUSilentTimeout.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUSilentTimeout_h
#define AudioUnitSDK_AUSilentTimeout_h

#include <CoreFoundation/CFBase.h> // for UInt32
#include <algorithm>

namespace ausdk {

/*!
	@class	AUSilentTimeout
	@brief	Utility to assist in propagating a silence flag from signal-processing
			input to output, factoring in a processing delay.
*/
class AUSilentTimeout {
public:
	AUSilentTimeout() = default;

	void Process(UInt32 inFramesToProcess, UInt32 inTimeoutLimit, bool& ioSilence)
	{
		if (ioSilence) {
			if (mResetTimer) {
				mTimeoutCounter = inTimeoutLimit;
				mResetTimer = false;
			}

			if (mTimeoutCounter > 0) {
				mTimeoutCounter -= std::min(inFramesToProcess, mTimeoutCounter);
				ioSilence = false;
			}
		} else {
			// signal to reset the next time we receive silence
			mResetTimer = true;
		}
	}

	void Reset() { mResetTimer = true; }

private:
	UInt32 mTimeoutCounter{ 0 };
	bool mResetTimer{ false };
};

} // namespace ausdk

#endif // AudioUnitSDK_AUSilentTimeout_h
