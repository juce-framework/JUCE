/*!
	@file		AudioUnitSDK/AUUtility.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUUtility_h
#define AudioUnitSDK_AUUtility_h

// OS
#if defined __has_include && __has_include(<CoreAudioTypes/CoreAudioTypes.h>)
#include <CoreAudioTypes/CoreAudioTypes.h>
#else
#include <CoreAudio/CoreAudioTypes.h>
#endif
#include <libkern/OSByteOrder.h>
#include <mach/mach_time.h>
#include <os/log.h>
#include <syslog.h>

// std
#include <bitset>
#include <cstddef>
#include <exception>
#include <mutex>
#include <string>
#include <system_error>
#include <vector>

// -------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark General

#ifdef AUSDK_NO_DEPRECATIONS
#define AUSDK_DEPRECATED(msg)
#else
#define AUSDK_DEPRECATED(msg) [[deprecated(msg)]] // NOLINT macro
#endif

#ifndef AUSDK_LOG_OBJECT
#define AUSDK_LOG_OBJECT OS_LOG_DEFAULT // NOLINT macro
#endif

// -------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Version

#define AUSDK_VERSION_MAJOR 1
#define AUSDK_VERSION_MINOR 1
#define AUSDK_VERSION_PATCH 0

// -------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Error-handling macros

#ifdef AUSDK_NO_LOGGING
#define AUSDK_LogError(...) /* NOLINT macro */
#else
#define AUSDK_LogError(...) /* NOLINT macro */                                                     \
	if (__builtin_available(macOS 10.11, *)) {                                                     \
		os_log_error(AUSDK_LOG_OBJECT, __VA_ARGS__);                                               \
	} else {                                                                                       \
		syslog(LOG_ERR, __VA_ARGS__);                                                              \
	}
#endif

#define AUSDK_Catch(result) /* NOLINT(cppcoreguidelines-macro-usage) */                            \
	catch (const ausdk::AUException& exc) { (result) = exc.mError; }                               \
	catch (const std::bad_alloc&) { (result) = kAudio_MemFullError; }                              \
	catch (const OSStatus& catch_err) { (result) = catch_err; }                                    \
	catch (const std::system_error& exc) { (result) = exc.code().value(); }                        \
	catch (...) { (result) = -1; }

#define AUSDK_Require(expr, error) /* NOLINT(cppcoreguidelines-macro-usage) */                     \
	do {                                                                                           \
		if (!(expr)) {                                                                             \
			return error;                                                                          \
		}                                                                                          \
	} while (0) /* NOLINT */

#define AUSDK_Require_noerr(expr) /* NOLINT(cppcoreguidelines-macro-usage) */                      \
	do {                                                                                           \
		if (const auto status_tmp_macro_detail_ = (expr); status_tmp_macro_detail_ != noErr) {     \
			return status_tmp_macro_detail_;                                                       \
		}                                                                                          \
	} while (0)

#pragma mark -

// -------------------------------------------------------------------------------------------------

namespace ausdk {

// -------------------------------------------------------------------------------------------------

/// A subclass of std::runtime_error that holds an OSStatus error.
class AUException : public std::runtime_error {
public:
	explicit AUException(OSStatus err)
		: std::runtime_error{ std::string("OSStatus ") + std::to_string(err) }, mError{ err }
	{
	}

	const OSStatus mError;
};

inline void ThrowExceptionIf(bool condition, OSStatus err)
{
	if (condition) {
		AUSDK_LogError("throwing %d", static_cast<int>(err));
		throw AUException{ err };
	}
}

[[noreturn]] inline void Throw(OSStatus err)
{
	AUSDK_LogError("throwing %d", static_cast<int>(err));
	throw AUException{ err };
}

inline void ThrowQuietIf(bool condition, OSStatus err)
{
	if (condition) {
		throw AUException{ err };
	}
}

[[noreturn]] inline void ThrowQuiet(OSStatus err) { throw AUException{ err }; }

// -------------------------------------------------------------------------------------------------

/// Wrap a std::recursive_mutex in a C++ Mutex (named requirement). Methods are virtual to support
/// customization.
class AUMutex {
public:
	AUMutex() = default;
	virtual ~AUMutex() = default;

	AUMutex(const AUMutex&) = delete;
	AUMutex(AUMutex&&) = delete;
	AUMutex& operator=(const AUMutex&) = delete;
	AUMutex& operator=(AUMutex&&) = delete;

	virtual void lock() { mImpl.lock(); }
	virtual void unlock() { mImpl.unlock(); }
	virtual bool try_lock() { return mImpl.try_lock(); }

private:
	std::recursive_mutex mImpl;
};

// -------------------------------------------------------------------------------------------------

/// Implement optional locking at AudioUnit non-realtime entry points (required only for a small
/// number of plug-ins which must synchronize against external entry points).
class AUEntryGuard {
public:
	explicit AUEntryGuard(AUMutex* maybeMutex) : mMutex{ maybeMutex }
	{
		if (mMutex != nullptr) {
			mMutex->lock();
		}
	}

	~AUEntryGuard()
	{
		if (mMutex != nullptr) {
			mMutex->unlock();
		}
	}

	AUEntryGuard(const AUEntryGuard&) = delete;
	AUEntryGuard(AUEntryGuard&&) = delete;
	AUEntryGuard& operator=(const AUEntryGuard&) = delete;
	AUEntryGuard& operator=(AUEntryGuard&&) = delete;

private:
	AUMutex* mMutex;
};

// -------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark ASBD

/// Utility functions relating to AudioStreamBasicDescription.
namespace ASBD {

constexpr bool IsInterleaved(const AudioStreamBasicDescription& format) noexcept
{
	return (format.mFormatFlags & kLinearPCMFormatFlagIsNonInterleaved) == 0u;
}

constexpr UInt32 NumberInterleavedChannels(const AudioStreamBasicDescription& format) noexcept
{
	return IsInterleaved(format) ? format.mChannelsPerFrame : 1;
}

constexpr UInt32 NumberChannelStreams(const AudioStreamBasicDescription& format) noexcept
{
	return IsInterleaved(format) ? 1 : format.mChannelsPerFrame;
}

constexpr bool IsCommonFloat32(const AudioStreamBasicDescription& format) noexcept
{
	return (
		format.mFormatID == kAudioFormatLinearPCM && format.mFramesPerPacket == 1 &&
		format.mBytesPerPacket == format.mBytesPerFrame
		// so far, it's a valid PCM format
		&& (format.mFormatFlags & kLinearPCMFormatFlagIsFloat) != 0 &&
		(format.mChannelsPerFrame == 1 ||
			(format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) != 0) &&
		((format.mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian) &&
		format.mBitsPerChannel == 32 // NOLINT
		&& format.mBytesPerFrame == NumberInterleavedChannels(format) * sizeof(float));
}

constexpr AudioStreamBasicDescription CreateCommonFloat32(
	Float64 sampleRate, UInt32 numChannels, bool interleaved = false) noexcept
{
	constexpr auto sampleSize = sizeof(Float32);

	AudioStreamBasicDescription asbd{};
	asbd.mFormatID = kAudioFormatLinearPCM;
	asbd.mFormatFlags = kAudioFormatFlagIsFloat |
						static_cast<AudioFormatFlags>(kAudioFormatFlagsNativeEndian) |
						kAudioFormatFlagIsPacked;
	asbd.mBitsPerChannel = 8 * sampleSize; // NOLINT magic number
	asbd.mChannelsPerFrame = numChannels;
	asbd.mFramesPerPacket = 1;
	asbd.mSampleRate = sampleRate;
	if (interleaved) {
		asbd.mBytesPerPacket = asbd.mBytesPerFrame = numChannels * sampleSize;
	} else {
		asbd.mBytesPerPacket = asbd.mBytesPerFrame = sampleSize;
		asbd.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
	}
	return asbd;
}

constexpr bool MinimalSafetyCheck(const AudioStreamBasicDescription& x) noexcept
{
	// This function returns false if there are sufficiently unreasonable values in any field.
	// It is very conservative so even some very unlikely values will pass.
	// This is just meant to catch the case where the data from a file is corrupted.

	return (x.mSampleRate >= 0.) && (x.mSampleRate < 3e6) // NOLINT SACD sample rate is 2.8224 MHz
		   && (x.mBytesPerPacket < 1000000)               // NOLINT
		   && (x.mFramesPerPacket < 1000000)              // NOLINT
		   && (x.mBytesPerFrame < 1000000)                // NOLINT
		   && (x.mChannelsPerFrame > 0) && (x.mChannelsPerFrame <= 1024) // NOLINT
		   && (x.mBitsPerChannel <= 1024)                                // NOLINT
		   && (x.mFormatID != 0) &&
		   !(x.mFormatID == kAudioFormatLinearPCM &&
			   (x.mFramesPerPacket != 1 || x.mBytesPerPacket != x.mBytesPerFrame));
}

inline bool IsEqual(
	const AudioStreamBasicDescription& lhs, const AudioStreamBasicDescription& rhs) noexcept
{
	return memcmp(&lhs, &rhs, sizeof(AudioStreamBasicDescription)) == 0;
}

} // namespace ASBD

// -------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark ACL

/// Utility functions relating to AudioChannelLayout.
namespace ACL {

constexpr bool operator==(const AudioChannelLayout& lhs, const AudioChannelLayout& rhs) noexcept
{
	if (lhs.mChannelLayoutTag != rhs.mChannelLayoutTag) {
		return false;
	}
	if (lhs.mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap) {
		return lhs.mChannelBitmap == rhs.mChannelBitmap;
	}
	if (lhs.mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions) {
		if (lhs.mNumberChannelDescriptions != rhs.mNumberChannelDescriptions) {
			return false;
		}
		for (auto i = 0u; i < lhs.mNumberChannelDescriptions; ++i) {
			const auto& lhdesc = lhs.mChannelDescriptions[i]; // NOLINT array subscript
			const auto& rhdesc = rhs.mChannelDescriptions[i]; // NOLINT array subscript

			if (lhdesc.mChannelLabel != rhdesc.mChannelLabel) {
				return false;
			}
			if (lhdesc.mChannelLabel == kAudioChannelLabel_UseCoordinates) {
				if (memcmp(&lhdesc, &rhdesc, sizeof(AudioChannelDescription)) != 0) {
					return false;
				}
			}
		}
	}
	return true;
}

} // namespace ACL

// -------------------------------------------------------------------------------------------------

/// Utility wrapper for the variably-sized AudioChannelLayout struct.
class AUChannelLayout {
public:
	AUChannelLayout() : AUChannelLayout(0, kAudioChannelLayoutTag_UseChannelDescriptions, 0) {}

	/// Can construct from a layout tag.
	explicit AUChannelLayout(AudioChannelLayoutTag inTag) : AUChannelLayout(0, inTag, 0) {}

	AUChannelLayout(uint32_t inNumberChannelDescriptions, AudioChannelLayoutTag inChannelLayoutTag,
		AudioChannelBitmap inChannelBitMap)
		: mStorage(
			  kHeaderSize + (inNumberChannelDescriptions * sizeof(AudioChannelDescription)), {})
	{
		auto* const acl = reinterpret_cast<AudioChannelLayout*>(mStorage.data()); // NOLINT

		acl->mChannelLayoutTag = inChannelLayoutTag;
		acl->mChannelBitmap = inChannelBitMap;
		acl->mNumberChannelDescriptions = inNumberChannelDescriptions;
	}

	/// Implicit conversion from AudioChannelLayout& is allowed.
	AUChannelLayout(const AudioChannelLayout& acl) // NOLINT
		: mStorage(kHeaderSize + (acl.mNumberChannelDescriptions * sizeof(AudioChannelDescription)))
	{
		memcpy(mStorage.data(), &acl, mStorage.size());
	}

	bool operator==(const AUChannelLayout& other) const noexcept
	{
		return ACL::operator==(Layout(), other.Layout());
	}

	bool operator!=(const AUChannelLayout& y) const noexcept { return !(*this == y); }

	[[nodiscard]] bool IsValid() const noexcept { return NumberChannels() > 0; }

	[[nodiscard]] const AudioChannelLayout& Layout() const noexcept { return *LayoutPtr(); }

	[[nodiscard]] const AudioChannelLayout* LayoutPtr() const noexcept
	{
		return reinterpret_cast<const AudioChannelLayout*>(mStorage.data()); // NOLINT
	}

	/// After default construction, this method will return
	/// kAudioChannelLayoutTag_UseChannelDescriptions with 0 channel descriptions.
	[[nodiscard]] AudioChannelLayoutTag Tag() const noexcept { return Layout().mChannelLayoutTag; }

	[[nodiscard]] uint32_t NumberChannels() const noexcept { return NumberChannels(*LayoutPtr()); }

	[[nodiscard]] uint32_t Size() const noexcept { return static_cast<uint32_t>(mStorage.size()); }

	static uint32_t NumberChannels(const AudioChannelLayout& inLayout) noexcept
	{
		if (inLayout.mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions) {
			return inLayout.mNumberChannelDescriptions;
		}
		if (inLayout.mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap) {
			return static_cast<uint32_t>(
				std::bitset<32>(inLayout.mChannelBitmap).count()); // NOLINT magic #
		}
		return AudioChannelLayoutTag_GetNumberOfChannels(inLayout.mChannelLayoutTag);
	}

private:
	constexpr static size_t kHeaderSize = offsetof(AudioChannelLayout, mChannelDescriptions[0]);
	std::vector<std::byte> mStorage;
};

// -------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark AudioBufferList

/// Utility functions relating to AudioBufferList.
namespace ABL {

// if the return result is odd, there was a null buffer.
inline uint32_t IsBogusAudioBufferList(const AudioBufferList& abl)
{
	const AudioBuffer *buf = abl.mBuffers, *const bufEnd = buf + abl.mNumberBuffers;
	uint32_t sum =
		0; // defeat attempts by the compiler to optimize away the code that touches the buffers
	uint32_t anyNull = 0;
	for (; buf < bufEnd; ++buf) {
		const uint32_t* const p = static_cast<const uint32_t*>(buf->mData);
		if (p == nullptr) {
			anyNull = 1;
			continue;
		}
		const auto dataSize = buf->mDataByteSize;
		if (dataSize >= sizeof(*p)) {
			const size_t frameCount = dataSize / sizeof(*p);
			sum += p[0];
			sum += p[frameCount - 1];
		}
	}
	return anyNull | (sum & ~1u);
}

} // namespace ABL

// -------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark HostTime

/// Utility functions relating to Mach absolute time.
namespace HostTime {

/// Returns the current host time
inline uint64_t Current() { return mach_absolute_time(); }

/// Returns the frequency of the host timebase, in ticks per second.
inline double Frequency()
{
	struct mach_timebase_info timeBaseInfo {
	}; // NOLINT
	mach_timebase_info(&timeBaseInfo);
	//	the frequency of that clock is: (sToNanosDenominator / sToNanosNumerator) * 10^9
	return static_cast<double>(timeBaseInfo.denom) / static_cast<double>(timeBaseInfo.numer) *
		   1.0e9; // NOLINT
}

} // namespace HostTime

// -------------------------------------------------------------------------------------------------

/// Basic RAII wrapper for CoreFoundation types
template <typename T>
class Owned {
	explicit Owned(T obj, bool fromget) noexcept : mImpl{ obj }
	{
		if (fromget) {
			retainRef();
		}
	}

public:
	static Owned from_get(T obj) noexcept { return Owned{ obj, true }; }

	static Owned from_create(T obj) noexcept { return Owned{ obj, false }; }
	static Owned from_copy(T obj) noexcept { return Owned{ obj, false }; }

	Owned() noexcept = default;
	~Owned() noexcept { releaseRef(); }

	Owned(const Owned& other) noexcept : mImpl{ other.mImpl } { retainRef(); }

	Owned(Owned&& other) noexcept : mImpl{ std::exchange(other.mImpl, nullptr) } {}

	Owned& operator=(const Owned& other) noexcept
	{
		if (this != &other) {
			releaseRef();
			mImpl = other.mImpl;
			retainRef();
		}
		return *this;
	}

	Owned& operator=(Owned&& other) noexcept
	{
		std::swap(mImpl, other.mImpl);
		return *this;
	}

	T operator*() const noexcept { return get(); }
	T get() const noexcept { return mImpl; }

	/// As with `unique_ptr<T>::release()`, releases ownership of the reference to the caller (not
	/// to be confused with decrementing the reference count as with `CFRelease()`).
	T release() noexcept { return std::exchange(mImpl, nullptr); }

	/// This is a from_get operation.
	Owned& operator=(T cfobj) noexcept
	{
		if (mImpl != cfobj) {
			releaseRef();
			mImpl = cfobj;
			retainRef();
		}
		return *this;
	}

private:
	void retainRef() noexcept
	{
		if (mImpl != nullptr) {
			CFRetain(mImpl);
		}
	}

	void releaseRef() noexcept
	{
		if (mImpl != nullptr) {
			CFRelease(mImpl);
		}
	}

	T mImpl{ nullptr };
};

// -------------------------------------------------------------------------------------------------

constexpr bool safe_isprint(char in_char) noexcept { return (in_char >= ' ') && (in_char <= '~'); }

inline std::string make_string_from_4cc(uint32_t in_4cc) noexcept
{
#if !TARGET_RT_BIG_ENDIAN
	in_4cc = OSSwapInt32(in_4cc); // NOLINT
#endif

	char* const string = reinterpret_cast<char*>(&in_4cc); // NOLINT
	for (size_t i = 0; i < sizeof(in_4cc); ++i) {
		if (!safe_isprint(string[i])) { // NOLINT
			string[i] = '.';            // NOLINT
		}
	}
	return std::string{ string, sizeof(in_4cc) };
}

} // namespace ausdk

#endif // AudioUnitSDK_AUUtility_h
