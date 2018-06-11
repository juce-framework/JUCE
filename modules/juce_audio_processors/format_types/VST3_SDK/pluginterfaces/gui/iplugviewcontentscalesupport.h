//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK GUI Interfaces
// Filename    : pluginterfaces/gui/iplugviewcontentscalesupport.h
// Created by  : Steinberg, 06/2016
// Description : Plug-in User Interface Scaling
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpush.h"
//------------------------------------------------------------------------

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
/** Plug-in view content scale support
\ingroup pluginGUI vstIPlug vst366
- [plug impl]
- [extends IPlugView]
- [optional]

This interface communicates the content scale factor from the host to the plug-in view on
systems where plug-ins cannot get this information directly like Microsoft Windows.

The host calls setContentScaleFactor directly after the plug view was attached and when the scale
factor changes (system change or window moved to another screen with different scaling settings).
The host could call setContentScaleFactor in a different context, for example: scaling the
plug-in editor for better readability.
When a plug-in handles this (by returning kResultTrue), it needs to scale the width and height of
its view by the scale factor and inform the host via a IPlugFrame::resizeView(), the host will then
call IPlugView::onSize()
 */
class IPlugViewContentScaleSupport : public FUnknown
{
public:
//------------------------------------------------------------------------
	typedef float ScaleFactor;

	virtual tresult PLUGIN_API setContentScaleFactor (ScaleFactor factor) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IPlugViewContentScaleSupport, 0x65ED9690, 0x8AC44525, 0x8AADEF7A, 0x72EA703F)

//------------------------------------------------------------------------
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------
