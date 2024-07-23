//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
//
// Filename    : ipslviewscaling.h
// Created by  : PreSonus Software Ltd., 03/2015
// Description : Plug-in View DPI Scaling Interface
//
//************************************************************************************************
/*
	DISCLAIMER:
	The PreSonus Plug-In Extensions are host-specific extensions of existing proprietary technologies,
	provided to the community on an AS IS basis. They are not part of any official 3rd party SDK and
	PreSonus is not affiliated with the owner of the underlying technology in any way.
*/
//************************************************************************************************

#ifndef _ipslviewscaling_h
#define _ipslviewscaling_h

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/falignpush.h"

namespace Presonus {

//************************************************************************************************
// IPlugInViewScaling
/**	Support for plug-in view content scaling, to be implemented by the VST3 IPlugView class. 
	
	On Windows, if a process is "DPI-aware" and the system DPI setting is different from the default
	value of 96 DPI, the application is responsible to scale the contents of its windows accordingly,
	including child windows provided by 3rd party plug-ins.
	
	This interface is used by the host to inform the plug-in about the current scaling factor.
	The scaling factor is used to convert user space coordinates aka DIPs (device-independent pixels)
	to physical pixels on screen.
	
	The plug-in has to be prepared to deal with the following scaling factors:
		
	 96 DPI = 100% scaling (factor = 1.0)
	120 DPI = 125% scaling (factor = 1.25)
	144 DPI = 150% scaling (factor = 1.5)
	192 DPI = 200% scaling (factor = 2.0)

	On Windows 8.1 or later DPI settings are per monitor. The scaling factor for a window can change
	when it is moved between screens.
*/
//************************************************************************************************

struct IPlugInViewScaling: Steinberg::FUnknown
{
	/**	Inform the view about the current content scaling factor. The scaling factor can change 
		if the window is moved between screens.
	*/
    virtual Steinberg::tresult PLUGIN_API setContentScaleFactor (float factor) = 0;

    static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IPlugInViewScaling, 0x65ed9690, 0x8ac44525, 0x8aadef7a, 0x72ea703f)

} // namespace Presonus

#include "pluginterfaces/base/falignpop.h"

#endif // _ipslviewscaling_h