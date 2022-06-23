//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
//
// Filename    : pslvst2extensions.h
// Created by  : PreSonus Software Ltd., 05/2012, last updated 08/2017
// Description : PreSonus-specific VST2 API Extensions
//
//************************************************************************************************
/*
	DISCLAIMER:
	The PreSonus Plug-In Extensions are host-specific extensions of existing proprietary technologies,
	provided to the community on an AS IS basis. They are not part of any official 3rd party SDK and
	PreSonus is not affiliated with the owner of the underlying technology in any way.
*/
//************************************************************************************************

#ifndef _pslvst2extensions_h
#define _pslvst2extensions_h

namespace Presonus {

//////////////////////////////////////////////////////////////////////////////////////////////////
// CanDo Strings
//////////////////////////////////////////////////////////////////////////////////////////////////

/** Identifiers to be passed to VST2's canDo() method. */
namespace PlugCanDos
{
	/** Check if view can be resized by the host. */
	static const char* canDoViewResize = "supportsViewResize";

	/** Check if view can be embedded by the host. */
	static const char* canDoViewEmbedding = "supportsViewEmbedding";
	
	/** Check if view scaling for high-DPI is supported by the plug-in. */
	static const char* canDoViewDpiScaling = "supportsViewDpiScaling";

	/** Check if gain reduction reporting is supported by the plug-in. */
	static const char* canDoGainReductionInfo = "supportsGainReductionInfo";

	/** Check if slave effects are supported by plug-in. */
	static const char* canDoSlaveEffects = "supportsSlaveEffects";
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Opcodes
//////////////////////////////////////////////////////////////////////////////////////////////////

/** Vendor-specific opcodes a VST2 plug-in can implement to add non-standard features like 
	embedding its views as subview into the host, resizing from the host, high-DPI scaling, etc.
	
	Embedding corresponds to the Presonus::IPlugInViewEmbedding VST3 extended interface.
	
	Resizing works like VST3's checkSizeConstraint() and onSize() methods, VST3's canResize()
	is defined via canDoViewResize. 
	
	For "DPI-aware" host applications on the Windows platform a similar mimic to the
	Presonus::IPlugInViewScaling VST3 extended interface is defined here.

	Gain reduction reporting corresponds to the Presonus::IGainReductionInfo VST3 interface.

	Slave effect handling corresponds to the Presonus::ISlaveControllerHandler VST3 interface.
*/
enum Opcodes
{
	/** PreSonus vendor ID - distinguishes our calls from other VST2 extensions.
		Pass this vendor ID as "index" (aka "lArg1") parameter for vendor specific calls. */
	kVendorID = 'PreS',

	/** The host can suggest a new editor size, and the plug-in can modify the suggested 
		size to a suitable value if it cannot resize to the given values.
		The ptrArg is a ERect* to the input/output rect. This differs from the ERect**
		used by effEditGetRect, because here the rect is owned by the host, not the plug-in.
		The result is 0 on failure, 1 on success. */
	kEffEditCheckSizeConstraints = 'AeCc',

	/** The host can set a new size after negotiating the size via the above
		kEffEditCheckSizeConstraints, triggering the actual resizing.
		The ptrArg is a ERect* to the input/output rect. This differs from the ERect**
		used by effEditGetRect, because here the rect is owned by the host, not the plug-in.
		The result is 0 on failure, 1 on success. */
	kEffEditSetRect = 'AeSr',

	/** When the view is embedded, it may need to adjust its UI, e.g. by suppressing
		its built-in resizing facility because this is then controlled by the host.
		The ptrArg is a VstInt32*, pointing to 0 to disable or to 1 to enable embedding.
		Per default, embedding is disabled until the host calls this to indicate otherwise. */
	kEffEditSetEmbedded = 'AeEm',
	
	/** Inform the view about the current content scaling factor. The factor is passed in the opt argument.
		For more details, please check the documentation of Presonus::IPlugInViewScaling. */
	kEffEditSetContentScaleFactor = 'AeCs',

	/** Get current gain reduction for display. The ptrArg is a float* to be set to the dB value.
		For more details, please check the documentation of Presonus::IGainReductionInfo. */
	kEffGetGainReductionValueInDb = 'GRdB',

	/** Add slave effect. The ptrArg is a pointer to the slave AEffect, the 'opt' float transmits the mode (see enum SlaveMode).
		For more details, please check the documentation of Presonus::ISlaveControllerHandler. */
	kEffAddSlave = 'AdSl',

	/** Remove slave effect. The ptrArg is a pointer to the slave AEffect.
		For more details, please check the documentation of Presonus::ISlaveControllerHandler. */
	kEffRemoveSlave = 'RmSl'
};

} // namespace Presonus

#endif // _pslvst2extensions_h
