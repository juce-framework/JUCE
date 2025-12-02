//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Common Base Classes
// Filename    : public.sdk/source/common/pluginview.h
// Created by  : Steinberg, 01/2004
// Description : Plug-In View Implementation
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/gui/iplugview.h"
#include "base/source/fobject.h"

namespace Steinberg {

//------------------------------------------------------------------------
/** Plug-In view default implementation.
\ingroup sdkBase
Can be used as base class for an IPlugView implementation.
*/
class CPluginView : public FObject, public IPlugView
{
public:
//------------------------------------------------------------------------
	CPluginView (const ViewRect* rect = nullptr);
	~CPluginView () SMTG_OVERRIDE;

	/** Returns its current frame rectangle. */
	const ViewRect& getRect () const { return rect; }

	/** Sets a new frame rectangle. */
	void setRect (const ViewRect& r) { rect = r; }

	/** Checks if this view is attached to its parent view. */
	bool isAttached () const { return systemWindow != nullptr; }

	/** Calls when this view will be attached to its parent view. */
	virtual void attachedToParent () {}

	/** Calls when this view will be removed from its parent view. */
	virtual void removedFromParent () {}

	//---from IPlugView-------
	tresult PLUGIN_API isPlatformTypeSupported (FIDString type) SMTG_OVERRIDE;
	tresult PLUGIN_API attached (void* parent, FIDString type) SMTG_OVERRIDE;
	tresult PLUGIN_API removed () SMTG_OVERRIDE;

	tresult PLUGIN_API onWheel (float /*distance*/) SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API onKeyDown (char16 /*key*/, int16 /*keyMsg*/,
	                              int16 /*modifiers*/) SMTG_OVERRIDE
	{
		return kResultFalse;
	}
	tresult PLUGIN_API onKeyUp (char16 /*key*/, int16 /*keyMsg*/, int16 /*modifiers*/) SMTG_OVERRIDE
	{
		return kResultFalse;
	}
	tresult PLUGIN_API getSize (ViewRect* size) SMTG_OVERRIDE;
	tresult PLUGIN_API onSize (ViewRect* newSize) SMTG_OVERRIDE;

	tresult PLUGIN_API onFocus (TBool /*state*/) SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API setFrame (IPlugFrame* frame) SMTG_OVERRIDE
	{
		plugFrame = frame;
		return kResultTrue;
	}

	tresult PLUGIN_API canResize () SMTG_OVERRIDE { return kResultFalse; }
	tresult PLUGIN_API checkSizeConstraint (ViewRect* /*rect*/) SMTG_OVERRIDE
	{
		return kResultFalse;
	}

	//---Interface------
	OBJ_METHODS (CPluginView, FObject)
	DEFINE_INTERFACES
		DEF_INTERFACE (IPlugView)
	END_DEFINE_INTERFACES (FObject)
	REFCOUNT_METHODS (FObject)
//------------------------------------------------------------------------
protected:
	ViewRect rect;
	void* systemWindow {nullptr};
	IPtr<IPlugFrame> plugFrame;
};

//------------------------------------------------------------------------
} // namespace Steinberg
