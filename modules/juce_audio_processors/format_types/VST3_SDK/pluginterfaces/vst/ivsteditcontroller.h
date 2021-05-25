//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivsteditcontroller.h
// Created by  : Steinberg, 09/2005
// Description : VST Edit Controller Interfaces
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/ipluginbase.h"
#include "vsttypes.h"

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpush.h"
//------------------------------------------------------------------------

//------------------------------------------------------------------------
/** Class Category Name for Controller Component */
//------------------------------------------------------------------------
#ifndef kVstComponentControllerClass
#define kVstComponentControllerClass "Component Controller Class"
#endif

//------------------------------------------------------------------------
namespace Steinberg {
class IPlugView;
class IBStream;

//------------------------------------------------------------------------
namespace Vst {
//------------------------------------------------------------------------
/** Controller Parameter Info. */
//------------------------------------------------------------------------
struct ParameterInfo
{
//------------------------------------------------------------------------
	ParamID id;				///< unique identifier of this parameter (named tag too)
	String128 title;		///< parameter title (e.g. "Volume")
	String128 shortTitle;	///< parameter shortTitle (e.g. "Vol")
	String128 units;		///< parameter unit (e.g. "dB")
	int32 stepCount;		///< number of discrete steps (0: continuous, 1: toggle, discrete value otherwise 
							///< (corresponding to max - min, for example: 127 for a min = 0 and a max = 127) - see \ref vst3parameterIntro)
	ParamValue defaultNormalizedValue;	///< default normalized value [0,1] (in case of discrete value: defaultNormalizedValue = defDiscreteValue / stepCount)
	UnitID unitId;			///< id of unit this parameter belongs to (see \ref vst3UnitsIntro)

	int32 flags;			///< ParameterFlags (see below)
	enum ParameterFlags
	{
		kNoFlags		 = 0,		///< no flags wanted
		kCanAutomate	 = 1 << 0,	///< parameter can be automated
		kIsReadOnly		 = 1 << 1,	///< parameter cannot be changed from outside (implies that kCanAutomate is false)
		kIsWrapAround	 = 1 << 2,	///< attempts to set the parameter value out of the limits will result in a wrap around [SDK 3.0.2]
		kIsList			 = 1 << 3,	///< parameter should be displayed as list in generic editor or automation editing [SDK 3.1.0]

		kIsProgramChange = 1 << 15,	///< parameter is a program change (unitId gives info about associated unit 
									///< - see \ref vst3UnitPrograms)
		kIsBypass		 = 1 << 16	///< special bypass parameter (only one allowed): Plug-in can handle bypass
									///< (highly recommended to export a bypass parameter for effect Plug-in)
	};
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/** View Types used for IEditController::createView */
//------------------------------------------------------------------------
namespace ViewType {
const CString kEditor = "editor";
}

//------------------------------------------------------------------------
/** Flags used for IComponentHandler::restartComponent */
//------------------------------------------------------------------------
enum RestartFlags
{
	kReloadComponent			= 1 << 0,	///< The Component should be reloaded             [SDK 3.0.0]
	kIoChanged					= 1 << 1,	///< Input and/or Output Bus configuration has changed        [SDK 3.0.0]
	kParamValuesChanged			= 1 << 2,	///< Multiple parameter values have changed 
											///< (as result of a program change for example)  [SDK 3.0.0]
	kLatencyChanged				= 1 << 3,	///< Latency has changed (IAudioProcessor.getLatencySamples)  [SDK 3.0.0]
	kParamTitlesChanged			= 1 << 4,	///< Parameter titles or default values or flags have changed [SDK 3.0.0]
	kMidiCCAssignmentChanged	= 1 << 5,	///< MIDI Controller Assignments have changed     [SDK 3.0.1]
	kNoteExpressionChanged		= 1 << 6,	///< Note Expression has changed (info, count, PhysicalUIMapping, ...) [SDK 3.5.0]
	kIoTitlesChanged			= 1 << 7,	///< Input and/or Output bus titles have changed  [SDK 3.5.0]
	kPrefetchableSupportChanged = 1 << 8,	///< Prefetch support has changed (\see IPrefetchableSupport) [SDK 3.6.1]
	kRoutingInfoChanged			= 1 << 9	///< RoutingInfo has changed (\see IComponent)    [SDK 3.6.6]
};

//------------------------------------------------------------------------
/** Host callback interface for an edit controller.
\ingroup vstIHost vst300
- [host imp]
- [released: 3.0.0]
- [mandatory]

Allow transfer of parameter editing to component (processor) via host and support automation.
Cause the host to react on configuration changes (restartComponent)

\see IEditController */
//------------------------------------------------------------------------
class IComponentHandler: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** To be called before calling a performEdit (e.g. on mouse-click-down event). */
	virtual tresult PLUGIN_API beginEdit (ParamID id) = 0;

	/** Called between beginEdit and endEdit to inform the handler that a given parameter has a new value. */
	virtual tresult PLUGIN_API performEdit (ParamID id, ParamValue valueNormalized) = 0;

	/** To be called after calling a performEdit (e.g. on mouse-click-up event). */
	virtual tresult PLUGIN_API endEdit (ParamID id) = 0;

	/** Instructs host to restart the component. This should be called in the UI-Thread context!
	\param flags is a combination of RestartFlags */
	virtual tresult PLUGIN_API restartComponent (int32 flags) = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IComponentHandler, 0x93A0BEA3, 0x0BD045DB, 0x8E890B0C, 0xC1E46AC6)

//------------------------------------------------------------------------
/** Extended Host callback interface IComponentHandler2 for an edit controller
\ingroup vstIHost vst310
- [host imp]
- [extends IComponentHandler]
- [released: 3.1.0]
- [optional]

One part handles:
- Setting dirty state of Plug-in
- requesting the host to open the editor

The other part handles parameter group editing from Plug-in UI. It wraps a set of \ref IComponentHandler::beginEdit /
\ref IComponentHandler::performEdit / \ref IComponentHandler::endEdit functions (see \ref IComponentHandler)
which should use the same timestamp in the host when writing automation.
This allows for better synchronizing multiple parameter changes at once.

\section IComponentHandler2Example Examples of different use cases
\code
	//--------------------------------------
	// in case of multiple switch buttons (with associated ParamID 1 and 3)
	// on mouse down :
	hostHandler2->startGroupEdit ();
	hostHandler->beginEdit (1);
	hostHandler->beginEdit (3);
	hostHandler->performEdit (1, 1.0);
	hostHandler->performEdit (3, 0.0); // the opposite of paramID 1 for example
	....
	// on mouse up :
	hostHandler->endEdit (1);
	hostHandler->endEdit (3);
	hostHandler2->finishGroupEdit ();
	....
	....
	//--------------------------------------
	// in case of multiple faders (with associated ParamID 1 and 3)
	// on mouse down :
	hostHandler2->startGroupEdit ();
	hostHandler->beginEdit (1);
	hostHandler->beginEdit (3);
	hostHandler2->finishGroupEdit ();
	....
	// on mouse move :
	hostHandler2->startGroupEdit ();
	hostHandler->performEdit (1, x); // x the wanted value
	hostHandler->performEdit (3, x);
	hostHandler2->finishGroupEdit ();
	....
	// on mouse up :
	hostHandler2->startGroupEdit ();
	hostHandler->endEdit (1);
	hostHandler->endEdit (3);
	hostHandler2->finishGroupEdit ();
\endcode
\see IComponentHandler
\see IEditController*/
//------------------------------------------------------------------------
class IComponentHandler2: public FUnknown
{
public:
	//------------------------------------------------------------------------
	/** Tells host that the Plug-in is dirty (something besides parameters has changed since last save),
	if true the host should apply a save before quitting. */
	virtual tresult PLUGIN_API setDirty (TBool state) = 0;

	/** Tells host that it should open the Plug-in editor the next time it's possible.
	You should use this instead of showing an alert and blocking the program flow (especially on loading projects). */
	virtual tresult PLUGIN_API requestOpenEditor (FIDString name = ViewType::kEditor) = 0;

	//------------------------------------------------------------------------
	/** Starts the group editing (call before a \ref IComponentHandler::beginEdit),
	the host will keep the current timestamp at this call and will use it for all \ref IComponentHandler::beginEdit
	/ \ref IComponentHandler::performEdit / \ref IComponentHandler::endEdit calls until a \ref finishGroupEdit (). */
	virtual tresult PLUGIN_API startGroupEdit () = 0;

	/** Finishes the group editing started by a \ref startGroupEdit (call after a \ref IComponentHandler::endEdit). */
	virtual tresult PLUGIN_API finishGroupEdit () = 0;

	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IComponentHandler2, 0xF040B4B3, 0xA36045EC, 0xABCDC045, 0xB4D5A2CC)

//------------------------------------------------------------------------
/** Extended Host callback interface IComponentHandlerBusActivation for an edit controller.
\ingroup vstIHost vst368
- [host imp]
- [extends IComponentHandler]
- [released: 3.6.8]
- [optional]

Allows the Plug-in to request the host to activate or deactivate a specific bus, 
if the host accepts it will call later on IComponent::activateBus (see \ref IComponent::activateBus). 
Useful especially for Instrument with more than 1 outputs, where the user could request
from the Plug-in UI a given output bus activation.

\see \ref IComponentHandler */
//------------------------------------------------------------------------
class IComponentHandlerBusActivation : public FUnknown
{
public:
	//------------------------------------------------------------------------
	/** request the host to activate or deactivate a specific bus. */
	virtual tresult PLUGIN_API requestBusActivation (MediaType type, BusDirection dir, int32 index,
	                                                 TBool state) = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IComponentHandlerBusActivation, 0x067D02C1, 0x5B4E274D, 0xA92D90FD, 0x6EAF7240)

//------------------------------------------------------------------------
/** Edit controller component interface.
\ingroup vstIPlug vst300
- [plug imp]
- [released: 3.0.0]
- [mandatory]

The Controller part of an effect or instrument with parameter handling (export, definition, conversion...).
\see IComponent::getControllerClassId, IMidiMapping */
//------------------------------------------------------------------------
class IEditController: public IPluginBase
{
public:
//------------------------------------------------------------------------
	/** Receives the component state. */
	virtual tresult PLUGIN_API setComponentState (IBStream* state) = 0;

	/** Sets the controller state. */
	virtual tresult PLUGIN_API setState (IBStream* state) = 0;

	/** Gets the controller state. */
	virtual tresult PLUGIN_API getState (IBStream* state) = 0;

	// parameters -------------------------
	/** Returns the number of parameters exported. */
	virtual int32 PLUGIN_API getParameterCount () = 0;
	/** Gets for a given index the parameter information. */
	virtual tresult PLUGIN_API getParameterInfo (int32 paramIndex, ParameterInfo& info /*out*/) = 0;

	/** Gets for a given paramID and normalized value its associated string representation. */
	virtual tresult PLUGIN_API getParamStringByValue (ParamID id, ParamValue valueNormalized /*in*/, String128 string /*out*/) = 0;
	/** Gets for a given paramID and string its normalized value. */
	virtual tresult PLUGIN_API getParamValueByString (ParamID id, TChar* string /*in*/, ParamValue& valueNormalized /*out*/) = 0;

	/** Returns for a given paramID and a normalized value its plain representation
		(for example 90 for 90db - see \ref vst3AutomationIntro). */
	virtual ParamValue PLUGIN_API normalizedParamToPlain (ParamID id, ParamValue valueNormalized) = 0;
	/** Returns for a given paramID and a plain value its normalized value. (see \ref vst3AutomationIntro) */
	virtual ParamValue PLUGIN_API plainParamToNormalized (ParamID id, ParamValue plainValue) = 0;

	/** Returns the normalized value of the parameter associated to the paramID. */
	virtual ParamValue PLUGIN_API getParamNormalized (ParamID id) = 0;
	/** Sets the normalized value to the parameter associated to the paramID. The controller must never
	    pass this value-change back to the host via the IComponentHandler. It should update the according
		GUI element(s) only!*/
	virtual tresult PLUGIN_API setParamNormalized (ParamID id, ParamValue value) = 0;

	// handler ----------------------------
	/** Gets from host a handler. */
	virtual tresult PLUGIN_API setComponentHandler (IComponentHandler* handler) = 0;

	// view -------------------------------
	/** Creates the editor view of the Plug-in, currently only "editor" is supported, see \ref ViewType.
		The life time of the editor view will never exceed the life time of this controller instance. */
	virtual IPlugView* PLUGIN_API createView (FIDString name) = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IEditController, 0xDCD7BBE3, 0x7742448D, 0xA874AACC, 0x979C759E)

//------------------------------------------------------------------------
/** Knob Mode */
//------------------------------------------------------------------------
enum KnobModes
{
	kCircularMode = 0,		///< Circular with jump to clicked position
	kRelativCircularMode,	///< Circular without jump to clicked position
	kLinearMode				///< Linear: depending on vertical movement
};

typedef int32 KnobMode;		///< Knob Mode

//------------------------------------------------------------------------
/** Edit controller component interface extension.
\ingroup vstIPlug vst310
- [plug imp]
- [extends IEditController]
- [released: 3.1.0]
- [optional]

Extension to inform the Plug-in about the host Knob Mode,
and to open the Plug-in about box or help documentation.

\see IEditController*/
//------------------------------------------------------------------------
class IEditController2: public FUnknown
{
public:
	/** Host could set the Knob Mode for the Plug-in. Return kResultFalse means not supported mode. \see KnobModes. */
	virtual tresult PLUGIN_API setKnobMode (KnobMode mode) = 0;

	/** Host could ask to open the Plug-in help (could be: opening a PDF document or link to a web page).
	    The host could call it with onlyCheck set to true for testing support of open Help. 
		Return kResultFalse means not supported function. */
	virtual tresult PLUGIN_API openHelp (TBool onlyCheck) = 0;

	/** Host could ask to open the Plug-in about box.
	    The host could call it with onlyCheck set to true for testing support of open AboutBox. 
		Return kResultFalse means not supported function. */
	virtual tresult PLUGIN_API openAboutBox (TBool onlyCheck) = 0;

	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IEditController2, 0x7F4EFE59, 0xF3204967, 0xAC27A3AE, 0xAFB63038)

//------------------------------------------------------------------------
/** MIDI Mapping Interface.
\ingroup vstIPlug vst301
- [plug imp]
- [extends IEditController]
- [released: 3.0.1]
- [optional]

MIDI controllers are not transmitted directly to a VST component. MIDI as hardware protocol has
restrictions that can be avoided in software. Controller data in particular come along with unclear
and often ignored semantics. On top of this they can interfere with regular parameter automation and
the host is unaware of what happens in the Plug-in when passing MIDI controllers directly.

So any functionality that is to be controlled by MIDI controllers must be exported as regular parameter.
The host will transform incoming MIDI controller data using this interface and transmit them as normal
parameter change. This allows the host to automate them in the same way as other parameters.
CtrlNumber could be typical MIDI controller value extended to some others values like pitch bend or
after touch (see \ref ControllerNumbers).
If the mapping has changed, the Plug-in should call IComponentHandler::restartComponent (kMidiCCAssignmentChanged)
to inform the host about this change. */
//------------------------------------------------------------------------
class IMidiMapping: public FUnknown
{
public:

	/** Gets an (preferred) associated ParamID for a given Input Event Bus index, channel and MIDI Controller.
	*	@param[in] busIndex - index of Input Event Bus
	*	@param[in] channel - channel of the bus
	*   @param[in] midiControllerNumber - see \ref ControllerNumbers for expected values (could be bigger than 127)
	*	@param[in] id - return the associated ParamID to the given midiControllerNumber
	*/
	virtual tresult PLUGIN_API getMidiControllerAssignment (int32 busIndex, int16 channel,
															CtrlNumber midiControllerNumber, ParamID& id/*out*/) = 0;

	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IMidiMapping, 0xDF0FF9F7, 0x49B74669, 0xB63AB732, 0x7ADBF5E5)

//------------------------------------------------------------------------
/** Parameter Editing from Host.
\ingroup vstIPlug vst350
- [plug imp]
- [extends IEditController]
- [released: 3.5.0]
- [optional]

If this interface is implemented by the edit controller and when performing edits from outside
the Plug-in (host / remote) of a not automatable and not read only flagged parameter (kind of helper parameter),
the host will start with a beginEditFromHost before calling setParamNormalized and end with an endEditFromHost.
Here the sequencing, the host will call:
- beginEditFromHost ()
- setParamNormalized ()
- setParamNormalized ()
- ...
- endEditFromHost ()
\see IEditController */
//------------------------------------------------------------------------
class IEditControllerHostEditing : public FUnknown
{
public:
	/** Called before a setParamNormalized sequence, a endEditFromHost will be call at the end of the editing action. */
	virtual tresult PLUGIN_API beginEditFromHost (ParamID paramID) = 0;

	/** Called after a beginEditFromHost and a sequence of setParamNormalized. */
	virtual tresult PLUGIN_API endEditFromHost (ParamID paramID) = 0;

	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IEditControllerHostEditing, 0xC1271208, 0x70594098, 0xB9DD34B3, 0x6BB0195E)

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------
