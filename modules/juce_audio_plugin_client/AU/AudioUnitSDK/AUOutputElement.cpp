/*!
	@file		AudioUnitSDK/AUOutputElement.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#include <AudioUnitSDK/AUBase.h>
#include <AudioUnitSDK/AUOutputElement.h>

namespace ausdk {

AUOutputElement::AUOutputElement(AUBase& audioUnit) : AUIOElement(audioUnit) { AllocateBuffer(); }

AUOutputElement::AUOutputElement(AUBase& audioUnit, const AudioStreamBasicDescription& format)
	: AUIOElement{ audioUnit, format }
{
	AllocateBuffer();
}

OSStatus AUOutputElement::SetStreamFormat(const AudioStreamBasicDescription& desc)
{
	const OSStatus result = AUIOElement::SetStreamFormat(desc); // inherited
	if (result == noErr) {
		AllocateBuffer();
	}
	return result;
}

} // namespace ausdk
