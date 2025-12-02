//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstparameters.h
// Created by  : Steinberg, 03/2008
// Description : VST Parameter Implementation
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "base/source/fobject.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"

#include <map>
#include <vector>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
/** Description of a Parameter.
\ingroup vstClasses
*/
class Parameter : public FObject
{
public:
//------------------------------------------------------------------------
	Parameter ();
	Parameter (const ParameterInfo&);
	Parameter (const TChar* title, ParamID tag, const TChar* units = nullptr,
	           ParamValue defaultValueNormalized = 0., int32 stepCount = 0,
	           int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId,
               const TChar* shortTitle = nullptr);
	~Parameter () override;

	/** Returns its read only info. */
	virtual const ParameterInfo& getInfo () const { return info; }

	/** Returns its writable info. */
	virtual ParameterInfo& getInfo () { return info; }

	/** Sets its associated UnitId. */
	virtual void setUnitID (UnitID id) { info.unitId = id; }
	/** Gets its associated UnitId. */
	virtual UnitID getUnitID () { return info.unitId; }

	/** Gets its normalized value [0.0, 1.0]. */
	ParamValue getNormalized () const { return valueNormalized; }
	/** Sets its normalized value [0.0, 1.0]. */
	virtual bool setNormalized (ParamValue v);

	/** Converts a normalized value to a string. */
	virtual void toString (ParamValue valueNormalized, String128 string) const;
	/** Converts a string to a normalized value. */
	virtual bool fromString (const TChar* string, ParamValue& valueNormalized) const;

	/** Converts a normalized value to plain value (e.g. 0.5 to 10000.0Hz). */
	virtual ParamValue toPlain (ParamValue valueNormalized) const;
	/** Converts a plain value to a normalized value (e.g. 10000 to 0.5). */
	virtual ParamValue toNormalized (ParamValue plainValue) const;

	/** Gets the current precision (used for string representation of float). */
	virtual int32 getPrecision () const { return precision; }
	/** Sets the precision for string representation of float value (for example 4.34 with 2 as
	 * precision). */
	virtual void setPrecision (int32 val) { precision = val; }

	OBJ_METHODS (Parameter, FObject)
//------------------------------------------------------------------------
protected:
	ParameterInfo info {};
	ParamValue valueNormalized {0.};
	int32 precision {4};
};

//------------------------------------------------------------------------
/** Description of a RangeParameter.
\ingroup vstClasses 
*/
class RangeParameter : public Parameter
{
public:
//------------------------------------------------------------------------
	RangeParameter (const ParameterInfo& paramInfo, ParamValue _minPlain, ParamValue _maxPlain);
	RangeParameter (const TChar* title, ParamID tag, const TChar* units = nullptr,
	                ParamValue minPlain = 0., ParamValue maxPlain = 1.,
	                ParamValue defaultValuePlain = 0., int32 stepCount = 0,
	                int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId,
                    const TChar* shortTitle = nullptr);

	/** Gets the minimum plain value, same as toPlain (0). */
	virtual ParamValue getMin () const { return minPlain; }
	/** Sets the minimum plain value. */
	virtual void setMin (ParamValue value) { minPlain = value; }
	/** Gets the maximum plain value, same as toPlain (1). */
	virtual ParamValue getMax () const { return maxPlain; }
	/** Sets the maximum plain value. */
	virtual void setMax (ParamValue value) { maxPlain = value; }

	/** Converts a normalized value to a string. */
	void toString (ParamValue _valueNormalized, String128 string) const SMTG_OVERRIDE;
	/** Converts a string to a normalized value. */
	bool fromString (const TChar* string, ParamValue& _valueNormalized) const SMTG_OVERRIDE;

	/** Converts a normalized value to plain value (e.g. 0.5 to 10000.0Hz). */
	ParamValue toPlain (ParamValue _valueNormalized) const SMTG_OVERRIDE;
	/** Converts a plain value to a normalized value (e.g. 10000 to 0.5). */
	ParamValue toNormalized (ParamValue plainValue) const SMTG_OVERRIDE;

	OBJ_METHODS (RangeParameter, Parameter)
//------------------------------------------------------------------------
protected:
	RangeParameter ();

	ParamValue minPlain;
	ParamValue maxPlain;
};

//------------------------------------------------------------------------
/** Description of a StringListParameter.
\ingroup vstClasses
*/
class StringListParameter : public Parameter
{
public:
//------------------------------------------------------------------------
	StringListParameter (const ParameterInfo& paramInfo);
	StringListParameter (const TChar* title, ParamID tag, const TChar* units = nullptr,
	                     int32 flags = ParameterInfo::kCanAutomate | ParameterInfo::kIsList,
	                     UnitID unitID = kRootUnitId, const TChar* shortTitle= nullptr);
	~StringListParameter () override;

	/** Appends a string and increases the stepCount. */
	virtual void appendString (const String128 string);
	/** Replaces the string at index. Index must be between 0 and stepCount+1 */
	virtual bool replaceString (int32 index, const String128 string);

	/** clear all added String */
	virtual void clear ();

	/** Converts a normalized value to a string. */
	void toString (ParamValue _valueNormalized, String128 string) const SMTG_OVERRIDE;
	/** Converts a string to a normalized value. */
	bool fromString (const TChar* string, ParamValue& _valueNormalized) const SMTG_OVERRIDE;

	/** Converts a normalized value to plain value (e.g. 0.5 to 10000.0Hz). */
	ParamValue toPlain (ParamValue _valueNormalized) const SMTG_OVERRIDE;
	/** Converts a plain value to a normalized value (e.g. 10000 to 0.5). */
	ParamValue toNormalized (ParamValue plainValue) const SMTG_OVERRIDE;

	OBJ_METHODS (StringListParameter, Parameter)
//------------------------------------------------------------------------
protected:
	using StringVector = std::vector<TChar*>;
	StringVector strings;
};

//------------------------------------------------------------------------
/** Collection of parameters.
\ingroup vstClasses
*/
class ParameterContainer
{
public:
//------------------------------------------------------------------------
	ParameterContainer ();
	~ParameterContainer ();

	/** Init param array. */
	void init (int32 initialSize = 10, int32 resizeDelta = 100);

	/** Creates and adds a new parameter from a ParameterInfo. */
	Parameter* addParameter (const ParameterInfo& info);

	/** Creates and adds a new parameter with given properties. */
	Parameter* addParameter (const TChar* title, const TChar* units = nullptr, int32 stepCount = 0,
	                         ParamValue defaultValueNormalized = 0.,
	                         int32 flags = ParameterInfo::kCanAutomate, int32 tag = -1,
	                         UnitID unitID = kRootUnitId, const TChar* shortTitle = nullptr);

	/** Adds a given parameter. */
	Parameter* addParameter (Parameter* p);

	/** Returns the count of parameters. */
	int32 getParameterCount () const { return params ? static_cast<int32> (params->size ()) : 0; }

	/** Gets parameter by index. */
	Parameter* getParameterByIndex (int32 index) const;

	/** Removes all parameters. */
	void removeAll ()
	{
		if (params)
			params->clear ();
		id2index.clear ();
	}

	/** Gets parameter by ID. */
	Parameter* getParameter (ParamID tag) const;

	/** Remove a specific parameter by ID. */
	bool removeParameter (ParamID tag);
	//------------------------------------------------------------------------
protected:
	using ParameterPtrVector = std::vector<IPtr<Parameter>>;
	using IndexMap = std::map<ParamID, ParameterPtrVector::size_type>;
	ParameterPtrVector* params {nullptr};
	IndexMap id2index;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
