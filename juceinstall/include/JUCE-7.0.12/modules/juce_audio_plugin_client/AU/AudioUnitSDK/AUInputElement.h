/*!
	@file		AudioUnitSDK/AUInputElement.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUInputElement_h
#define AudioUnitSDK_AUInputElement_h

#include <AudioUnitSDK/AUBuffer.h>
#include <AudioUnitSDK/AUScopeElement.h>

namespace ausdk {

/*!
	@class	AUInputElement
	@brief	Implements an audio unit input element, managing the source of input from a callback
			or connection.
*/
class AUInputElement : public AUIOElement {
public:
	using AUIOElement::AUIOElement;

	// AUElement override
	OSStatus SetStreamFormat(const AudioStreamBasicDescription& fmt) override;
	[[nodiscard]] bool NeedsBufferSpace() const override { return IsCallback(); }
	void SetConnection(const AudioUnitConnection& conn);
	void SetInputCallback(AURenderCallback proc, void* refCon);
	[[nodiscard]] bool IsActive() const noexcept { return mInputType != EInputType::NoInput; }
	[[nodiscard]] bool IsCallback() const noexcept
	{
		return mInputType == EInputType::FromCallback;
	}
	[[nodiscard]] bool HasConnection() const noexcept
	{
		return mInputType == EInputType::FromConnection;
	}
	OSStatus PullInput(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp,
		AudioUnitElement inElement, UInt32 nFrames);
	OSStatus PullInputWithBufferList(AudioUnitRenderActionFlags& ioActionFlags,
		const AudioTimeStamp& inTimeStamp, AudioUnitElement inElement, UInt32 nFrames,
		AudioBufferList& inBufferList);

protected:
	void Disconnect();

private:
	enum class EInputType { NoInput, FromConnection, FromCallback };
	EInputType mInputType{ EInputType::NoInput };

	// if from callback:
	AURenderCallback mInputProc{ nullptr };
	void* mInputProcRefCon{ nullptr };

	// if from connection:
	AudioUnitConnection mConnection{};
};

inline OSStatus AUInputElement::PullInputWithBufferList(AudioUnitRenderActionFlags& ioActionFlags,
	const AudioTimeStamp& inTimeStamp, AudioUnitElement inElement, UInt32 nFrames,
	AudioBufferList& inBufferList)
{
	OSStatus theResult = noErr;

	if (HasConnection()) {
		// only support connections for V2 audio units
		theResult = AudioUnitRender(mConnection.sourceAudioUnit, &ioActionFlags, &inTimeStamp,
			mConnection.sourceOutputNumber, nFrames, &inBufferList);
	} else {
		// kFromCallback:
		theResult = (mInputProc)(mInputProcRefCon, &ioActionFlags, &inTimeStamp, inElement, nFrames,
			&inBufferList);
	}

	if (mInputType == EInputType::NoInput) { // defense: the guy upstream could have disconnected
											 // it's a horrible thing to do, but may happen!
		return kAudioUnitErr_NoConnection;
	}

#if !TARGET_OS_IPHONE || DEBUG
	if (theResult == noErr) { // if there's already an error, there's no point (and maybe some harm)
							  // in validating.
		if (ABL::IsBogusAudioBufferList(inBufferList) & 1) {
			return kAudioUnitErr_InvalidPropertyValue;
		}
	}
#endif
	return theResult;
}

} // namespace ausdk

#endif // AudioUnitSDK_AUInputElement_h
