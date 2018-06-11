//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Common Base Classes
// Filename    : public.sdk/source/common/pluginview.h
// Created by  : Steinberg, 01/2004
// Description : Plug-In View Implementation
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/gui/iplugview.h"
#include "base/source/fobject.h"

namespace Steinberg {

//------------------------------------------------------------------------
/** Plug-In view default implementation.
\ingroup sdkBase
Can be used as base class for an IPlugView implementation. */
//------------------------------------------------------------------------
class CPluginView: public FObject,
	               public IPlugView
{
public:
//------------------------------------------------------------------------
	CPluginView (const ViewRect* rect = nullptr);
	virtual ~CPluginView ();

	/** Returns its current frame rectangle. */
	const ViewRect& getRect () const	{ return rect; }

	/** Sets a new frame rectangle. */
	void setRect (const ViewRect& r)	{ rect = r; }

	/** Checks if this view is attached to its parent view. */
	bool isAttached () const			{ return systemWindow != nullptr; }

	/** Calls when this view will be attached to its parent view. */
	virtual void attachedToParent () {}

	/** Calls when this view will be removed from its parent view. */
	virtual void removedFromParent () {}

	//---from IPlugView-------
	tresult PLUGIN_API isPlatformTypeSupported (FIDString type) SMTG_OVERRIDE;
	tresult PLUGIN_API attached (void* parent, FIDString type) SMTG_OVERRIDE;
	tresult PLUGIN_API removed () SMTG_OVERRIDE;

	tresult PLUGIN_API onWheel (float /*distance*/) SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API onKeyDown (char16 /*key*/, int16 /*keyMsg*/, int16 /*modifiers*/) SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API onKeyUp (char16 /*key*/, int16 /*keyMsg*/, int16 /*modifiers*/) SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API getSize (ViewRect* size) SMTG_OVERRIDE;
	tresult PLUGIN_API onSize (ViewRect* newSize) SMTG_OVERRIDE;

	tresult PLUGIN_API onFocus (TBool /*state*/) SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API setFrame (IPlugFrame* frame) SMTG_OVERRIDE { plugFrame = frame; return kResultTrue; }

	tresult PLUGIN_API canResize () SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API checkSizeConstraint (ViewRect* /*rect*/) SMTG_OVERRIDE { return kResultFalse; }

	//---Interface------
	OBJ_METHODS (CPluginView, FObject)
	DEFINE_INTERFACES
		DEF_INTERFACE (IPlugView)
	END_DEFINE_INTERFACES (FObject)
	REFCOUNT_METHODS(FObject)
//------------------------------------------------------------------------
protected:
	ViewRect rect;
	void* systemWindow;
	IPlugFrame* plugFrame;
};

}	// namespace
