/*!
	@file		AudioUnitSDK/ComponentBase.cpp
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
// self
#include <AudioUnitSDK/AUUtility.h>
#include <AudioUnitSDK/ComponentBase.h>

// std
#include <mutex>

namespace ausdk {

static OSStatus CB_GetComponentDescription(
	AudioComponentInstance inInstance, AudioComponentDescription* outDesc);

std::recursive_mutex& ComponentBase::InitializationMutex()
{
	__attribute__ ((no_destroy)) static std::recursive_mutex global;
	return global;
}

ComponentBase::ComponentBase(AudioComponentInstance inInstance) : mComponentInstance(inInstance)
{
	(void)GetComponentDescription();
}

void ComponentBase::DoPostConstructor()
{
	PostConstructorInternal();
	PostConstructor();
}

void ComponentBase::DoPreDestructor()
{
	PreDestructor();
	PreDestructorInternal();
}

OSStatus ComponentBase::AP_Open(void* self, AudioComponentInstance compInstance)
{
	OSStatus result = noErr;
	const auto acpi = static_cast<AudioComponentPlugInInstance*>(self);
	try {
		const std::lock_guard guard{ InitializationMutex() };

		auto* const cb =
			static_cast<ComponentBase*>((*acpi->mConstruct)(&acpi->mInstanceStorage, compInstance));
		cb->DoPostConstructor(); // allows base class to do additional initialization
		// once the derived class is fully constructed
		result = noErr;
	}
	AUSDK_Catch(result)
	if (result != noErr) {
		delete acpi; // NOLINT
	}
	return result;
}

OSStatus ComponentBase::AP_Close(void* self)
{
	OSStatus result = noErr;
	try {
		const auto acpi = static_cast<AudioComponentPlugInInstance*>(self);
		if (const auto acImp =
				reinterpret_cast<ComponentBase*>(&acpi->mInstanceStorage)) { // NOLINT
			acImp->DoPreDestructor();
			(*acpi->mDestruct)(&acpi->mInstanceStorage);
			free(self); // NOLINT manual memory management
		}
	}
	AUSDK_Catch(result)
	return result;
}

AudioComponentDescription ComponentBase::GetComponentDescription() const
{
	AudioComponentDescription desc = {};

	if (CB_GetComponentDescription(mComponentInstance, &desc) == noErr) {
		return desc;
	}

	return {};
}

static OSStatus CB_GetComponentDescription(
	AudioComponentInstance inInstance, AudioComponentDescription* outDesc)
{
	const AudioComponent comp = AudioComponentInstanceGetComponent(inInstance);
	if (comp != nullptr) {
		return AudioComponentGetDescription(comp, outDesc);
	}

	return kAudio_ParamError;
}

} // namespace ausdk
