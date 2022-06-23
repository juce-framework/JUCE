//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
//
// Filename    : ipslhostcommands.h
// Created by  : PreSonus Software Ltd., 11/2009
// Description : Host Command Interface
//
//************************************************************************************************
/*
	DISCLAIMER:
	The PreSonus Plug-In Extensions are host-specific extensions of existing proprietary technologies,
	provided to the community on an AS IS basis. They are not part of any official 3rd party SDK and
	PreSonus is not affiliated with the owner of the underlying technology in any way.
*/
//************************************************************************************************

#ifndef _ipslhostcommands_h
#define _ipslhostcommands_h

#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/falignpush.h"

namespace Steinberg {
class IPlugView; }

namespace Presonus {

struct ICommandList;

//************************************************************************************************
// IHostCommandHandler
/** Callback interface to access host-specific parameter commands to be integrated
	into a context menu inside the plug-in editor. Implemented as extension of 
	Steinberg::Vst::IComponentHandler.

	Please note that the intention of this set of interfaces is not to allow a generic menu
	implementation. This is the responsibility of a GUI toolkit. It basically provides
	a way to enumerate and execute commands anonymously, i.e. the plug-in does not have to
	know the exact sematics of the commands and the host does not break the consistency of
	the plug-in GUI. 

	Usage Example:

	IComponentHandler* handler;
	FUnknownPtr<IHostCommandHandler> commandHandler (handler);
	if(commandHandler)
		if(ICommandList* commandList = commandHandler->createParamCommands (kMyParamId))
		{
			FReleaser commandListReleaser (commandList);
			commandHandler->popupCommandMenu (commandList, xPos, yPos);
		}
*/
//************************************************************************************************

struct IHostCommandHandler: Steinberg::FUnknown
{
	/**	Create list of currently available host commands for given parameter. 
		The command list has a short lifecycle, it is recreated whenever
		a context menu should appear. The returned pointer can be null, otherwise
		it has to be released. */
	virtual ICommandList* PLUGIN_API createParamCommands (Steinberg::Vst::ParamID tag) = 0;
	
	/** Helper to popup a command menu at given position.
		Coordinates are relative to view or in screen coordintes if view is null.
		Can be used for testing purpose, if the plug-in does not have its own context menu implementation
		or if it wants to use the look & feel of the host menu. This method is not supposed
		to support command lists implemented by the plug-in. */
	virtual Steinberg::tresult PLUGIN_API popupCommandMenu (ICommandList* commandList, Steinberg::int32 xPos, Steinberg::int32 yPos, Steinberg::IPlugView* view = 0) = 0;

	static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IHostCommandHandler, 0xF92032CD, 0x7A84407C, 0xABE6F863, 0x058EA6C2)

//************************************************************************************************
// CommandInfo
/** Describes a single command. */
//************************************************************************************************

struct CommandInfo
{
	Steinberg::Vst::String128 title;	///< command title (possibly localized into active host language)
	Steinberg::int32 flags;				///< command flags
	
	enum CommandFlags
	{
		kCanExecute  = 1<<0,	///< used to display command enabled/disabled
		kIsSeparator = 1<<1,	///< not a command, it's a separator
		kIsChecked   = 1<<2		///< used to display command with a check mark
	};
};

//************************************************************************************************
// ICommandList
/** Describes a list of commands. */
//************************************************************************************************

struct ICommandList: Steinberg::FUnknown
{
	/** Returns the number of commands. */
	virtual Steinberg::int32 PLUGIN_API getCommandCount () = 0;
	
	/** Get command information for a given index. */
	virtual Steinberg::tresult PLUGIN_API getCommandInfo (Steinberg::int32 index, CommandInfo& info) = 0;
	
	/** Execute command at given index. */
	virtual Steinberg::tresult PLUGIN_API executeCommand (Steinberg::int32 index) = 0;

	static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (ICommandList, 0xC5A687DB, 0x82F344E9, 0xB378254A, 0x47C4D712)

} // namespace Presonus

#include "pluginterfaces/base/falignpop.h"

#endif // _ipslhostcommands_h