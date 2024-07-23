//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/classfactoryhelpers.h
// Created by  : Steinberg, 03/2017
// Description : Class factory
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2023, Steinberg Media Technologies GmbH, All Rights Reserved
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

//------------------------------------------------------------------------------
// Helper Macros. Not intended for direct use.
// Use:
//	META_CLASS(className),
//	META_CLASS_IFACE(className,Interface),
//	META_CLASS_SINGLE(className,Interface)
// instead.
//------------------------------------------------------------------------------
#define META_CREATE_FUNC(funcName) static FUnknown* funcName ()

#define CLASS_CREATE_FUNC(className)                                               \
	namespace Meta {                                                               \
	META_CREATE_FUNC (make##className) { return (NEW className)->unknownCast (); } \
	}

#define SINGLE_CREATE_FUNC(className)                                                     \
	namespace Meta {                                                                      \
	META_CREATE_FUNC (make##className) { return className::instance ()->unknownCast (); } \
	}

#define _META_CLASS(className)                                                         \
	namespace Meta {                                                                   \
	static Steinberg::MetaClass meta##className ((#className), Meta::make##className); \
	}

#define _META_CLASS_IFACE(className, Interface)                                                  \
	namespace Meta {                                                                             \
	static Steinberg::MetaClass meta##Interface##className ((#className), Meta::make##className, \
	                                                        Interface##_iid);                    \
	}

/** TODO
 */
#define META_CLASS(className)     \
	CLASS_CREATE_FUNC (className) \
	_META_CLASS (className)

/** TODO
 */
#define META_CLASS_IFACE(className, Interface) \
	CLASS_CREATE_FUNC (className)              \
	_META_CLASS_IFACE (className, Interface)

/** TODO
 */
#define META_CLASS_SINGLE(className, Interface) \
	SINGLE_CREATE_FUNC (className)              \
	_META_CLASS_IFACE (className, Interface)
