/*!
	@file		AudioUnitSDK/ComponentBase.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_ComponentBase_h
#define AudioUnitSDK_ComponentBase_h

// module
#include <AudioUnitSDK/AUUtility.h>

// OS
#include <AudioToolbox/AudioUnit.h>

// std
#include <array>
#include <mutex>
#include <new>

namespace ausdk {

/*!
	@class	ComponentBase
	@brief	Base class for implementing an `AudioComponentInstance`.
*/
class ComponentBase {
public:
	/// Construct given an AudioComponentInstance, typically from APFactory::Constuct.
	explicit ComponentBase(AudioComponentInstance inInstance);

	virtual ~ComponentBase() = default;

	ComponentBase(const ComponentBase&) = delete;
	ComponentBase(ComponentBase&&) = delete;
	ComponentBase& operator=(const ComponentBase&) = delete;
	ComponentBase& operator=(ComponentBase&&) = delete;

	/// Called from dispatchers after constructing an instance.
	void DoPostConstructor();

	/// Called from dispatchers before destroying an instance.
	void DoPreDestructor();

	/// Obtain the wrapped `AudioComponentInstance` (underlying type of `AudioUnit`, `AudioCodec`,
	/// and others).
	[[nodiscard]] AudioComponentInstance GetComponentInstance() const noexcept
	{
		return mComponentInstance;
	}

	/// Return the instance's `AudioComponentDescription`.
	[[nodiscard]] AudioComponentDescription GetComponentDescription() const;

	/// Component dispatch method.
	static OSStatus AP_Open(void* self, AudioComponentInstance compInstance);

	/// Component dispatch method.
	static OSStatus AP_Close(void* self);

	/// A mutex which is held during `Open`, since some AU's and the Component Manager itself
	/// are not thread-safe against globals.
	static std::recursive_mutex& InitializationMutex();

protected:
	// subclasses are free to to override these methods to add functionality
	virtual void PostConstructor() {}
	virtual void PreDestructor() {}
	// these methods, however, are reserved for override only within this SDK
	virtual void PostConstructorInternal() {}
	virtual void PreDestructorInternal() {}

private:
	AudioComponentInstance mComponentInstance;
};

/*!
	@class	AudioComponentPlugInInstance
	@brief	Object which implements an AudioComponentPlugInInterface for the framework, and
			which holds the C++ implementation object.
*/
struct AudioComponentPlugInInstance {
	// The AudioComponentPlugInInterface must remain first

	AudioComponentPlugInInterface mPlugInInterface;

	void* (*mConstruct)(void* memory, AudioComponentInstance ci);

	void (*mDestruct)(void* memory);

	std::array<void*, 2> mPad; // pad to a 16-byte boundary (in either 32 or 64 bit mode)
	UInt32
		mInstanceStorage; // the ACI implementation object is constructed into this memory
						  // this member is just a placeholder. it is aligned to a 16byte boundary
};

/*!
	@class	APFactory
	@tparam	APMethodLookup	A class (e.g. AUBaseLookup) which provides a method selector lookup
							function.
	@tparam	Implementor		The class which implements the full plug-in (AudioUnit) interface.
	@brief	Provides an AudioComponentFactoryFunction and a convenience wrapper for
			AudioComponentRegister.
*/
template <class APMethodLookup, class Implementor>
class APFactory {
public:
	static void* Construct(void* memory, AudioComponentInstance compInstance)
	{
		return new (memory) Implementor(compInstance); // NOLINT manual memory management
	}

	static void Destruct(void* memory) { static_cast<Implementor*>(memory)->~Implementor(); }

	// This is the AudioComponentFactoryFunction. It returns an AudioComponentPlugInInstance.
	// The actual implementation object is not created until Open().
	static AudioComponentPlugInInterface* Factory(const AudioComponentDescription* /* inDesc */)
	{
		auto* const acpi =                                     // NOLINT owning memory
			static_cast<AudioComponentPlugInInstance*>(malloc( // NOLINT manual memory management
				offsetof(AudioComponentPlugInInstance, mInstanceStorage) + sizeof(Implementor)));
		acpi->mPlugInInterface.Open = ComponentBase::AP_Open;
		acpi->mPlugInInterface.Close = ComponentBase::AP_Close;
		acpi->mPlugInInterface.Lookup = APMethodLookup::Lookup;
		acpi->mPlugInInterface.reserved = nullptr;
		acpi->mConstruct = Construct;
		acpi->mDestruct = Destruct;
		acpi->mPad[0] = nullptr;
		acpi->mPad[1] = nullptr;
		return &acpi->mPlugInInterface;
	}

	// This is for runtime registration (not for plug-ins loaded from bundles).
	static AudioComponent Register(
		UInt32 type, UInt32 subtype, UInt32 manuf, CFStringRef name, UInt32 vers, UInt32 flags = 0)
	{
		const AudioComponentDescription desc = { type, subtype, manuf, flags, 0 };
		return AudioComponentRegister(&desc, name, vers, Factory);
	}
};

#ifndef AUSDK_EXPORT
#if __GNUC__
#define AUSDK_EXPORT __attribute__((visibility("default"))) // NOLINT
#else
#warning export?
#endif
#endif


/// Macro to generate the factory function for the specified Audio Component. Factory is an
/// APFactory such as AUBaseFactory. Class is the name of the final ComponentBase class which
/// implements instances of the class.
#define AUSDK_COMPONENT_ENTRY(FactoryType, Class) /* NOLINT macro */                               \
	AUSDK_EXPORT                                                                                   \
	extern "C" void* Class##Factory(const AudioComponentDescription* inDesc);                      \
	extern "C" void* Class##Factory(const AudioComponentDescription* inDesc)                       \
	{                                                                                              \
		return FactoryType<Class>::Factory(inDesc); /* NOLINT parens */                            \
	}

} // namespace ausdk

#endif // AudioUnitSDK_ComponentBase_h
