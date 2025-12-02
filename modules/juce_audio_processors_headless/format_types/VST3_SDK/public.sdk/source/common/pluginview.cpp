//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Common Base Classes
// Filename    : public.sdk/source/common/pluginview.cpp
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

#include "pluginview.h"

namespace Steinberg {

//------------------------------------------------------------------------
//  CPluginView implementation
//------------------------------------------------------------------------
CPluginView::CPluginView (const ViewRect* _rect)
: rect (0, 0, 0, 0)
{
	if (_rect)
		rect = *_rect;
}

//------------------------------------------------------------------------
CPluginView::~CPluginView ()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API CPluginView::isPlatformTypeSupported (FIDString /*type*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CPluginView::attached (void* parent, FIDString /*type*/)
{
	systemWindow = parent;

	attachedToParent ();
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CPluginView::removed ()
{
	systemWindow = nullptr;

	removedFromParent ();
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CPluginView::onSize (ViewRect* newSize)
{
	if (newSize)
		rect = *newSize;
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CPluginView::getSize (ViewRect* size)
{
	if (size)
	{
		*size = rect;
		return kResultTrue;
	}
	return kInvalidArgument;
}

//------------------------------------------------------------------------
} // namespace Steinberg
