//-----------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST SDK
//
// Category    : Validator
// Filename    : public.sdk/source/vst/testsuite/iplugprovider.h
// Created by  : Steinberg, 04/2005
// Description : VST Test Suite
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2020, Steinberg Media Technologies GmbH, All Rights Reserved
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

#include "pluginterfaces/base/istringresult.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
/** Test Helper.
 * \ingroup TestClass
 *
 *	This class provides access to the component and the controller of a plug-in when running a unit
 *	test (see ITest).
 * 	You get this interface as the context argument in the ITestFactory::createTests method.
 */
//------------------------------------------------------------------------
class ITestPlugProvider : public FUnknown
{
public:
//------------------------------------------------------------------------
	/** get the component of the plug-in.
	 *
	 * The reference count of the component is increased in this function and you need to call
	 * releasePlugIn when done with the component.
	 */
	virtual IComponent* PLUGIN_API getComponent () = 0;
	/** get the controller of the plug-in.
	 *
	 * The reference count of the controller is increased in this function and you need to call
	 * releasePlugIn when done with the controller.
	 */
	virtual IEditController* PLUGIN_API getController () = 0;
	/** release the component and/or controller */
	virtual tresult PLUGIN_API releasePlugIn (IComponent* component,
	                                          IEditController* controller) = 0;
	/** get the sub categories of the plug-in */
	virtual tresult PLUGIN_API getSubCategories (IStringResult& result) const = 0;
	/** get the component UID of the plug-in */
	virtual tresult PLUGIN_API getComponentUID (FUID& uid) const = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (ITestPlugProvider, 0x86BE70EE, 0x4E99430F, 0x978F1E6E, 0xD68FB5BA)

//------------------------------------------------------------------------
/** Test Helper extension.
 * \ingroup TestClass
 */
class ITestPlugProvider2 : public ITestPlugProvider
{
public:
	/** get the plugin factory.
	 *
	 * The reference count of the returned factory object is not increased when calling this
	 * function.
	 */
	virtual IPluginFactory* PLUGIN_API getPluginFactory () = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (ITestPlugProvider2, 0xC7C75364, 0x7B8343AC, 0xA4495B0A, 0x3E5A46C7)

//------------------------------------------------------------------------
} // Vst
} // Steinberg
