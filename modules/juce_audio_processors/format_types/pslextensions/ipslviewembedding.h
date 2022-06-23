//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
//
// Filename    : ipslviewembedding.h
// Created by  : PreSonus Software Ltd., 05/2012
// Description : Plug-in View Embedding Interface
//
//************************************************************************************************
/*
	DISCLAIMER:
	The PreSonus Plug-In Extensions are host-specific extensions of existing proprietary technologies,
	provided to the community on an AS IS basis. They are not part of any official 3rd party SDK and
	PreSonus is not affiliated with the owner of the underlying technology in any way.
*/
//************************************************************************************************

#ifndef _ipslviewembedding_h
#define _ipslviewembedding_h

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/falignpush.h"

namespace Steinberg {
class IPlugView; }

namespace Presonus {

//************************************************************************************************
// IPlugInViewEmbedding
/** Support for plug-in view embedding, to be implemented by the VST3 controller class. */
//************************************************************************************************

class IPlugInViewEmbedding: public Steinberg::FUnknown
{
public:
	/** Check if view embedding is supported. */
    virtual Steinberg::TBool PLUGIN_API isViewEmbeddingSupported () = 0;

	/** Inform plug-in that its view will be embedded. */
    virtual Steinberg::tresult PLUGIN_API setViewIsEmbedded (Steinberg::IPlugView* view, Steinberg::TBool embedded) = 0;

    static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID (IPlugInViewEmbedding, 0xda57e6d1, 0x1f3242d1, 0xad9c1a82, 0xfdb95695)

} // namespace Presonus

#include "pluginterfaces/base/falignpop.h"

#endif // _ipslviewembedding_h