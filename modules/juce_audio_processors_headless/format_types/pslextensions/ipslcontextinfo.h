//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
//
// Filename    : ipslcontextinfo.h
// Created by  : PreSonus Software Ltd., 08/2013, last updated 11/2016
// Description : Context Information Interface
//
//************************************************************************************************
/*
	DISCLAIMER:
	The PreSonus Plug-In Extensions are host-specific extensions of existing proprietary technologies,
	provided to the community on an AS IS basis. They are not part of any official 3rd party SDK and
	PreSonus is not affiliated with the owner of the underlying technology in any way.
*/
//************************************************************************************************

#ifndef _ipslcontextinfo_h
#define _ipslcontextinfo_h

#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/falignpush.h"

namespace Presonus {

//************************************************************************************************
// IContextInfoProvider
/**	Callback interface to access context information from the host. Implemented by the host
	as extension of Steinberg::Vst::IComponentHandler. 
	
	The host might not be able to report all available attributes at all times. Please check the
	return value of getContextInfoValue() and getContextInfoString(). It's not required to implement
	IContextInfoHandler on the plug-in side, but we recommend to do so. The host will then call 
	notifyContextInfoChange() during initialization to inform the plug-in about the initial state of
	the available attributes.
	
	Usage Example:
	
	IComponentHandler* handler;
	FUnknownPtr<IContextInfoProvider> contextInfoProvider (handler);
	
	void PLUGIN_API MyEditController::notifyContextInfoChange ()
	{
		int32 channelIndex = 0;
		contextInfoProvider->getContextInfoValue (channelIndex, ContextInfo::kIndex);

		TChar channelName[128] = {0};
		contextInfoProvider->getContextInfoString (channelName, 128, ContextInfo::kName);
	}
*/
//************************************************************************************************

struct IContextInfoProvider: Steinberg::FUnknown
{	
	/** Get context information by identifier. */
	virtual Steinberg::tresult PLUGIN_API getContextInfoValue (Steinberg::int32& value, Steinberg::FIDString id) = 0;

	/** Get context information by identifier. */
	virtual Steinberg::tresult PLUGIN_API getContextInfoString (Steinberg::Vst::TChar* string, Steinberg::int32 maxCharCount, Steinberg::FIDString id) = 0;
	
    static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IContextInfoProvider, 0x483e61ea, 0x17994494, 0x8199a35a, 0xebb35e3c)

//************************************************************************************************
// IContextInfoProvider2
/**	Extension to IContextInfoProvider enabling the plug-in to modify host context information.
	Values like volume or pan support both, numeric and string representation for get and set.*/
//************************************************************************************************

struct IContextInfoProvider2: IContextInfoProvider
{
	using IContextInfoProvider::getContextInfoValue;

	/** Get context information by identifier (floating-point). */
	virtual Steinberg::tresult PLUGIN_API getContextInfoValue (double& value, Steinberg::FIDString id) = 0;
		
	/** Set context information by identifier (floating-point). */
	virtual Steinberg::tresult PLUGIN_API setContextInfoValue (Steinberg::FIDString id, double value) = 0;
	
	/** Set context information by identifier (integer). */
	virtual Steinberg::tresult PLUGIN_API setContextInfoValue (Steinberg::FIDString id, Steinberg::int32 value) = 0;
	
	/** Set context information by identifier (string). */
	virtual Steinberg::tresult PLUGIN_API setContextInfoString (Steinberg::FIDString id, Steinberg::Vst::TChar* string) = 0;
	
    static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IContextInfoProvider2, 0x61e45968, 0x3d364f39, 0xb15e1733, 0x4944172b)

//************************************************************************************************
// IContextInfoHandler
/**	Notification interface for context information changes. Implemented by the plug-in as extension of
	Steinberg::Vst::IEditController. */
//************************************************************************************************

struct IContextInfoHandler: Steinberg::FUnknown
{
	/** Called by the host if context information has changed. */
	virtual void PLUGIN_API notifyContextInfoChange () = 0;
  
	static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IContextInfoHandler, 0xc3b17bc0, 0x2c174494, 0x80293402, 0xfbc4bbf8)

//************************************************************************************************
// IContextInfoHandler2
/**	Replacement of IContextInfoHandler passing additional information about what changed on the host-side. 
	This interface will be preferred if implemented by the plug-in. It is required to 
	receive certain notifications like volume, pan, etc. */
//************************************************************************************************

struct IContextInfoHandler2: Steinberg::FUnknown
{
	/**	Called by the host if context information has changed. 
		The identifier (id) is empty for the inital update. */
	virtual void PLUGIN_API notifyContextInfoChange (Steinberg::FIDString id) = 0;
  
	static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IContextInfoHandler2, 0x31e29a7a, 0xe55043ad, 0x8b95b9b8, 0xda1fbe1e)

//************************************************************************************************
// Context Information Attributes
//************************************************************************************************

namespace ContextInfo
{
	/** Channel types. */
	enum ChannelType
	{
		kTrack = 0,		///< audio track
		kBus,			///< audio bus
		kFX,			///< FX channel
		kSynth,			///< output of virtual instrument
		kIn,			///< input from audio driver
		kOut			///< output to audio driver (main or sub-out)
	};
	
	/** Channel index mode. */
	enum ChannelIndexMode
	{
		kFlatIndex = 0,	///< channel indices are contiguous (example: track 1, track 2, bus 3, bus 4)
		kPerTypeIndex	///< channel indices restarts at zero for each type (example: track 1, track 2, bus 1, bus 2)
	};

	// per instance
	const Steinberg::FIDString kID = "id";							///< (R) channel identifier, use to compare identity (string)
	const Steinberg::FIDString kName = "name";						///< (R/W) channel name, can be displayed to the user (string)
	const Steinberg::FIDString kType = "type";						///< (R) channel type (int32, see ChannelType enumeration)
	const Steinberg::FIDString kMain = "main";						///< (R) channel is main output (int32, 0: false, 1: true)
	const Steinberg::FIDString kIndex = "index";					///< (R) channel index (int32, starts at zero)
	const Steinberg::FIDString kColor = "color";					///< (R/W) channel color (int32: RGBA)
	const Steinberg::FIDString kVisibility = "visibility";			///< (R) channel visibility (int32, 0: false, 1: true)
	const Steinberg::FIDString kSelected = "selected";				///< (R/W) selection state, channel is selected exlusively and scrolled into view on write (int32, 0: false, 1: true)
	const Steinberg::FIDString kMultiSelect = "multiselect";		///< (W) select channel without unselecting others (int32, 0: false, 1: true)
	const Steinberg::FIDString kFocused = "focused";				///< (R) focus for user input when multiple channels are selected (int32, 0: false, 1: true)

	const Steinberg::FIDString kRegionName = "regionName";			///< (R) name of region/event for region/event-based effects (string)
	const Steinberg::FIDString kRegionSelected = "regionSelected";	///< (R) selection state of region/event for region/event-based effects (int32, 0: false, 1: true)

	// per instance (requires IContextInfoHandler2 on plug-in side)
	const Steinberg::FIDString kVolume = "volume";					///< (R/W) volume factor [float, 0. = -oo dB, 1. = 0dB, etc.], also available as string
	const Steinberg::FIDString kMaxVolume = "maxVolume";			///< (R) maximum volume factor [float, 1. = 0dB], also available as string
	const Steinberg::FIDString kPan = "pan";						///< (R/W) stereo panning [float, < 0.5 = (L), 0.5 = (C), > 0.5 = (R)], also available as string
	const Steinberg::FIDString kMute = "mute";						///< (R/W) mute (int32, 0: false, 1: true)
	const Steinberg::FIDString kSolo = "solo";						///< (R/W) solo (int32, 0: false, 1: true)	
	const Steinberg::FIDString kSendCount = "sendcount";			///< (R) send count [int]
	const Steinberg::FIDString kSendLevel = "sendlevel";			///< (R/W) send level factor, index is appended to id (e.g. "sendlevel0" for first), also available as string
	const Steinberg::FIDString kMaxSendLevel = "maxSendlevel";		///< (R) maximum send level factor, also available as string

	// global
	const Steinberg::FIDString kActiveDocumentID = "activeDocumentID";	///< (R) active document identifier, use to get identity of the active document (string)
	const Steinberg::FIDString kDocumentID = "documentID";				///< (R) document identifier, use to compare identity (string)
	const Steinberg::FIDString kDocumentName = "documentName";			///< (R) document name, can be displayed to user (string)
	const Steinberg::FIDString kDocumentFolder = "documentFolder";		///< (R) document folder (string)	
	const Steinberg::FIDString kAudioFolder = "audioFolder";			///< (R) folder for audio files (string)
	const Steinberg::FIDString kIndexMode = "indexMode";				///< (R) channel index mode (default is flat, see ChannelIndexMode enumeration) 
}

} // namespace Presonus

#include "pluginterfaces/base/falignpop.h"

#endif // _ipslcontextinfo_h