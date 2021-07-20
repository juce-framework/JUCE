/*!
	@file		AudioUnitSDK/AUOutputElement.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUOutputElement_h
#define AudioUnitSDK_AUOutputElement_h

#include <AudioUnitSDK/AUBuffer.h>
#include <AudioUnitSDK/AUScopeElement.h>

namespace ausdk {

/*!
	@class	AUOutputElement
	@brief	Implements an audio unit output element.
*/
class AUOutputElement : public AUIOElement {
public:
	explicit AUOutputElement(AUBase& audioUnit);

	AUOutputElement(AUBase& audioUnit, const AudioStreamBasicDescription& format);

	AUSDK_DEPRECATED("Construct with a reference")
	explicit AUOutputElement(AUBase* audioUnit) : AUOutputElement(*audioUnit) {}

	// AUElement override
	OSStatus SetStreamFormat(const AudioStreamBasicDescription& desc) override;
	[[nodiscard]] bool NeedsBufferSpace() const override { return true; }
};

} // namespace ausdk

#endif // AudioUnitSDK_AUOutputElement_h
