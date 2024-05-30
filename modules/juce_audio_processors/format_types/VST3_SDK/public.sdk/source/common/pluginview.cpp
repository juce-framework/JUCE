//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Common Base Classes
// Filename    : public.sdk/source/common/pluginview.cpp
// Created by  : Steinberg, 01/2004
// Description : Plug-In View Implementation
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
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
    setFrame (nullptr);
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

} // end of namespace
