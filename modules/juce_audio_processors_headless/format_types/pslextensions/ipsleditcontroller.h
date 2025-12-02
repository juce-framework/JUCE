//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
//
// Filename    : ipsleditcontroller.h
// Created by  : PreSonus Software Ltd., 02/2017, last updated 10/2017
// Description : Plug-in Edit Controller Extension Interface
//
//************************************************************************************************
/*
	DISCLAIMER:
	The PreSonus Plug-In Extensions are host-specific extensions of existing proprietary technologies,
	provided to the community on an AS IS basis. They are not part of any official 3rd party SDK and
	PreSonus is not affiliated with the owner of the underlying technology in any way.
*/
//************************************************************************************************

#ifndef _ipsleditcontroller_h
#define _ipsleditcontroller_h

#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/falignpush.h"

namespace Steinberg {
namespace Vst {
class IEditController; }}

namespace Presonus {

/** Parameter extra flags. Used with IEditControllerExtra. */
enum ParamExtraFlags
{
	kParamFlagMicroEdit = 1<<0	///< parameter should be displayed in host micro view
};

/** Automation mode. Used with IEditControllerExtra. */
enum AutomationMode
{
	kAutomationNone = 0,	///< no automation data available
	kAutomationOff,			///< data available, but mode is set to off
	kAutomationRead,		///< data + read mode
	kAutomationTouch,		///< data + touch mode
	kAutomationLatch,		///< data + latch mode
	kAutomationWrite		///< data + write mode
};

/** Slave mode. Used with ISlaveControllerHandler. */
enum SlaveMode
{
	kSlaveModeNormal,	         ///< plug-in used in different location following given master
	kSlaveModeLowLatencyClone	 ///< plug-in used as hidden slave for low latency processing following given master 
};

//************************************************************************************************
// IEditControllerExtra
/**	Extension to Steinberg::Vst::IEditController with additonal flags and notifications
	not available in the standard edit controller interface. */
//************************************************************************************************

struct IEditControllerExtra: Steinberg::FUnknown
{	
	/** Get extra flags for given parameter (see ParamExtraFlags). */
	virtual Steinberg::int32 PLUGIN_API getParamExtraFlags (Steinberg::Vst::ParamID id) = 0;
	
	/** Set automation mode for given parameter (see AutomationMode). */
	virtual Steinberg::tresult PLUGIN_API setParamAutomationMode (Steinberg::Vst::ParamID id, Steinberg::int32 automationMode) = 0;
	
	static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IEditControllerExtra, 0x50553fd9, 0x1d2c4c24, 0xb410f484, 0xc5fb9f3f)

//************************************************************************************************
// ISlaveControllerHandler
/**	Extension to Steinberg::Vst::IEditController used to notify the plug-in about slave instances.

	The host might decide to use "cloned" (slave) instances in various scenarios, e.g. to process
	audio paths with different latencies simultaneously or to synchronize grouped plug-in instances
	between multiple mixer channels - see SlaveMode. In this case multiple plug-in instances are active
	at the same time even though it looks like one to the user, i.e. only the editor of the master
	instance is visible and can be used to change parameters. The edit controller implementation has
	to synchronize parameter changes between instances that aren't visible to the host internally. 
*/
//************************************************************************************************

struct ISlaveControllerHandler: Steinberg::FUnknown
{	
	/**	Add slave edit controller. Implementation must sync non-automatable parameters between
		this instance (master) and given slave instance internally, i.e. when the master (this)
		changes update all connected slaves.
	*/
	virtual Steinberg::tresult PLUGIN_API addSlave (Steinberg::Vst::IEditController* slave, Steinberg::int32 slaveMode) = 0;

	/** Remove slave edit controller. */
	virtual Steinberg::tresult PLUGIN_API removeSlave (Steinberg::Vst::IEditController* slave) = 0;
		
	static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (ISlaveControllerHandler, 0xd93894bd, 0x67454c29, 0x977ae2f5, 0xdb380434)

} // namespace Presonus

#include "pluginterfaces/base/falignpop.h"

#endif // _ipsleditcontroller_h
