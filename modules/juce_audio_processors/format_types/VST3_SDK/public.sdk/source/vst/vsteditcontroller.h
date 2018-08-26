//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vsteditcontroller.h
// Created by  : Steinberg, 04/2005
// Description : VST Edit Controller Implementation
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

#include "public.sdk/source/vst/vstcomponentbase.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "public.sdk/source/common/pluginview.h"
#include "base/source/fstring.h"

#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"

#include <vector>
#include <map>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

class EditorView;

//------------------------------------------------------------------------
/** Default implementation for a VST 3 edit controller.
\ingroup vstClasses
Can be used as base class for a specific controller implementation */
//------------------------------------------------------------------------
class EditController: public ComponentBase,
					  public IEditController,
					  public IEditController2
{
public:
//------------------------------------------------------------------------
	EditController ();

	//---from IEditController-------
	tresult PLUGIN_API setComponentState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;
	int32 PLUGIN_API getParameterCount () SMTG_OVERRIDE;
	tresult PLUGIN_API getParameterInfo (int32 paramIndex, ParameterInfo& info) SMTG_OVERRIDE;
	tresult PLUGIN_API getParamStringByValue (ParamID tag, ParamValue valueNormalized, String128 string) SMTG_OVERRIDE;
	tresult PLUGIN_API getParamValueByString (ParamID tag, TChar* string, ParamValue& valueNormalized) SMTG_OVERRIDE;
	ParamValue PLUGIN_API normalizedParamToPlain (ParamID tag, ParamValue valueNormalized) SMTG_OVERRIDE;
	ParamValue PLUGIN_API plainParamToNormalized (ParamID tag, ParamValue plainValue) SMTG_OVERRIDE;
	ParamValue PLUGIN_API getParamNormalized (ParamID tag) SMTG_OVERRIDE;
	tresult PLUGIN_API setParamNormalized (ParamID tag, ParamValue value) SMTG_OVERRIDE;
	tresult PLUGIN_API setComponentHandler (IComponentHandler* handler) SMTG_OVERRIDE;
	IPlugView* PLUGIN_API createView (FIDString /*name*/) SMTG_OVERRIDE {return nullptr;}

	//---from IEditController2-------
	tresult PLUGIN_API setKnobMode (KnobMode mode) SMTG_OVERRIDE { hostKnobMode = mode; return kResultTrue; }
	tresult PLUGIN_API openHelp (TBool /*onlyCheck*/) SMTG_OVERRIDE {return kResultFalse;}
	tresult PLUGIN_API openAboutBox (TBool /*onlyCheck*/) SMTG_OVERRIDE {return kResultFalse;}

	//---from ComponentBase---------
	tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API terminate  () SMTG_OVERRIDE;

	//---Internal Methods-------
	virtual tresult beginEdit (ParamID tag);	///< to be called before a serie of performEdit
	virtual tresult performEdit (ParamID tag, ParamValue valueNormalized); ///< will inform the host about the value change
	virtual tresult endEdit (ParamID tag);		///< to be called after a serie of performEdit
	virtual tresult startGroupEdit ();			///< calls IComponentHandler2::startGroupEdit() if host supports it
	virtual tresult finishGroupEdit ();			///< calls IComponentHandler2::finishGroupEdit() if host supports it

	virtual void editorDestroyed (EditorView* /*editor*/) {}	///< called from EditorView if it was destroyed
	virtual void editorAttached (EditorView* /*editor*/) {}		///< called from EditorView if it was attached to a parent
	virtual void editorRemoved (EditorView* /*editor*/) {}		///< called from EditorView if it was removed from a parent

	static KnobMode getHostKnobMode () { return hostKnobMode; }	///< return host knob mode

	/** Gets for a given tag the parameter object. */
	virtual Parameter* getParameterObject (ParamID tag) { return parameters.getParameter (tag); }

	/** Gets for a given tag the parameter information. */
	virtual tresult getParameterInfoByTag (ParamID tag, ParameterInfo& info);

	/** Calls IComponentHandler2::setDirty (state) if host supports it. */
	virtual tresult setDirty (TBool state);

	/** Calls IComponentHandler2::requestOpenEditor (name) if host supports it. */
	virtual tresult requestOpenEditor (FIDString name = ViewType::kEditor);

	//---Accessor Methods-------
	IComponentHandler* getComponentHandler () const { return componentHandler; }

	//---Interface---------
	OBJ_METHODS (EditController, ComponentBase)
	DEFINE_INTERFACES
		DEF_INTERFACE (IEditController)
		DEF_INTERFACE (IEditController2)
	END_DEFINE_INTERFACES (ComponentBase)
	REFCOUNT_METHODS (ComponentBase)
//------------------------------------------------------------------------
protected:
	IComponentHandler* componentHandler;
	IComponentHandler2* componentHandler2;

	ParameterContainer parameters;

	static KnobMode hostKnobMode;
};

//------------------------------------------------------------------------
/** View related to an edit controller.
\ingroup vstClasses  */
//------------------------------------------------------------------------
class EditorView : public CPluginView
{
public:
//------------------------------------------------------------------------
	EditorView (EditController* controller, ViewRect* size = nullptr);
	virtual ~EditorView ();

	/** Gets its controller part. */
	EditController* getController () { return controller; }

	//---from CPluginView-------------
	void attachedToParent () SMTG_OVERRIDE;
	void removedFromParent () SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:
	EditController* controller;
};

//------------------------------------------------------------------------
/** Unit element.
\ingroup vstClasses  */
//------------------------------------------------------------------------
class Unit : public FObject
{
public:
//------------------------------------------------------------------------
	Unit (const String128 name, UnitID unitId, UnitID parentUnitId = kRootUnitId,
	      ProgramListID programListId = kNoProgramListId);
	Unit (const UnitInfo& unit);

	/** Returns its info. */
	const UnitInfo& getInfo () const { return info; }

	/** Returns its Unit ID. */
	UnitID getID () const { return info.id; }

	/** Sets a new Unit ID. */
	void setID (UnitID newID) { info.id = newID; }

	/** Returns its Unit Name. */
	const TChar* getName () const { return info.name; }

	/** Sets a new Unit Name. */
	void setName (const String128 newName);

	/** Returns its ProgramList ID. */
	ProgramListID getProgramListID () const { return info.programListId; }

	/** Sets a new ProgramList ID. */
	void setProgramListID (ProgramListID newID) { info.programListId = newID; }

	OBJ_METHODS (Unit, FObject)
//------------------------------------------------------------------------
protected:
	Unit ();
	UnitInfo info;
};

//------------------------------------------------------------------------
/** ProgramList element.
\ingroup vstClasses  */
//------------------------------------------------------------------------
class ProgramList : public FObject
{
public:
//------------------------------------------------------------------------
	ProgramList (const String128 name, ProgramListID listId, UnitID unitId);
	ProgramList (const ProgramList& programList);

	const ProgramListInfo& getInfo () const { return info; }
	ProgramListID getID () const { return info.id; }
	const TChar* getName () const { return info.name; }
	int32 getCount () const { return info.programCount; }

	virtual tresult getProgramName (int32 programIndex, String128 name /*out*/);
	virtual tresult setProgramName (int32 programIndex, const String128 name /*in*/);
	virtual tresult getProgramInfo (int32 programIndex, CString attributeId,
	                                String128 value /*out*/);
	virtual tresult hasPitchNames (int32 programIndex)
	{
		(void)programIndex;
		return kResultFalse;
	}
	virtual tresult getPitchName (int32 programIndex, int16 midiPitch, String128 name /*out*/)
	{
		(void)programIndex;
		(void)midiPitch;
		(void)name;
		return kResultFalse;
	}

	/** Adds a program to the end of the list. returns the index of the program. */
	virtual int32 addProgram (const String128 name);

	/** Sets a program attribute value. */
	virtual bool setProgramInfo (int32 programIndex, CString attributeId, const String128 value);

	/** Creates and returns the program parameter. */
	virtual Parameter* getParameter ();

	OBJ_METHODS (ProgramList, FObject)
//------------------------------------------------------------------------
protected:
	typedef std::map<String, String> StringMap;
	typedef std::vector<String> StringVector;
	typedef std::vector<StringMap> ProgramInfoVector;
	ProgramListInfo info;
	UnitID unitId;
	StringVector programNames;
	ProgramInfoVector programInfos;
	Parameter* parameter;
};

//------------------------------------------------------------------------
/** ProgramListWithPitchNames element.
\ingroup vstClasses  */
//-----------------------------------------------------------------------------
class ProgramListWithPitchNames : public ProgramList
{
public:
	ProgramListWithPitchNames (const String128 name, ProgramListID listId, UnitID unitId);

	/** Sets a name for the given program index and a given pitch. */
	bool setPitchName (int32 programIndex, int16 pitch, const String128 pitchName);

	/** Removes the PitchName entry for the given program index and a given pitch. Returns true if
	 * it was found and removed. */
	bool removePitchName (int32 programIndex, int16 pitch);

	//---from ProgramList---------
	int32 addProgram (const String128 name) SMTG_OVERRIDE;
	tresult hasPitchNames (int32 programIndex) SMTG_OVERRIDE;
	tresult getPitchName (int32 programIndex, int16 midiPitch,
	                      String128 name /*out*/) SMTG_OVERRIDE;

	OBJ_METHODS (ProgramListWithPitchNames, ProgramList)
protected:
	typedef std::map<int16, String> PitchNameMap;
	typedef std::vector<PitchNameMap> PitchNamesVector;
	PitchNamesVector pitchNames;
};

//------------------------------------------------------------------------
/** Advanced implementation (support IUnitInfo) for a VST 3 edit controller.
\ingroup vstClasses
- [extends EditController]
*/
//------------------------------------------------------------------------
class EditControllerEx1 : public EditController, public IUnitInfo
{
public:
	EditControllerEx1 ();
	virtual ~EditControllerEx1 ();

	/** Adds a given unit. */
	bool addUnit (Unit* unit);

	/** Adds a given program list. */
	bool addProgramList (ProgramList* list);

	/** Returns the ProgramList associated to a given listId. */
	ProgramList* getProgramList (ProgramListID listId) const;

	/** Notifies the host about program list changes. */
	tresult notifyProgramListChange (ProgramListID listId, int32 programIndex = kAllProgramInvalid);

	//---from IUnitInfo------------------
	int32 PLUGIN_API getUnitCount () SMTG_OVERRIDE { return static_cast<int32> (units.size ()); }
	tresult PLUGIN_API getUnitInfo (int32 unitIndex, UnitInfo& info /*out*/) SMTG_OVERRIDE;

	int32 PLUGIN_API getProgramListCount () SMTG_OVERRIDE;
	tresult PLUGIN_API getProgramListInfo (int32 listIndex,
	                                       ProgramListInfo& info /*out*/) SMTG_OVERRIDE;
	tresult PLUGIN_API getProgramName (ProgramListID listId, int32 programIndex,
	                                   String128 name /*out*/) SMTG_OVERRIDE;
	tresult PLUGIN_API getProgramInfo (ProgramListID listId, int32 programIndex,
	                                   CString attributeId /*in*/,
	                                   String128 attributeValue /*out*/) SMTG_OVERRIDE;

	tresult PLUGIN_API hasProgramPitchNames (ProgramListID listId,
	                                         int32 programIndex) SMTG_OVERRIDE;
	tresult PLUGIN_API getProgramPitchName (ProgramListID listId, int32 programIndex,
	                                        int16 midiPitch, String128 name /*out*/) SMTG_OVERRIDE;

	virtual tresult setProgramName (ProgramListID listId, int32 programIndex,
	                                const String128 name /*in*/);

	// units selection --------------------
	UnitID PLUGIN_API getSelectedUnit () SMTG_OVERRIDE { return selectedUnit; }
	tresult PLUGIN_API selectUnit (UnitID unitId) SMTG_OVERRIDE
	{
		selectedUnit = unitId;
		return kResultTrue;
	}

	tresult PLUGIN_API getUnitByBus (MediaType /*type*/, BusDirection /*dir*/, int32 /*busIndex*/,
	                                 int32 /*channel*/, UnitID& /*unitId*/ /*out*/) SMTG_OVERRIDE
	{
		return kResultFalse;
	}
	tresult PLUGIN_API setUnitProgramData (int32 /*listOrUnitId*/, int32 /*programIndex*/,
	                                       IBStream* /*data*/) SMTG_OVERRIDE
	{
		return kResultFalse;
	}

	/** Notifies the host about the selected Unit. */
	virtual tresult notifyUnitSelection ();

	//---from IDependent------------------
	void PLUGIN_API update (FUnknown* changedUnknown, int32 message) SMTG_OVERRIDE;

	//---Interface---------
	OBJ_METHODS (EditControllerEx1, EditController)
	DEFINE_INTERFACES
		DEF_INTERFACE (IUnitInfo)
	END_DEFINE_INTERFACES (EditController)
	REFCOUNT_METHODS (EditController)

protected:
	typedef std::vector<IPtr<ProgramList>> ProgramListVector;
	typedef std::map<ProgramListID, ProgramListVector::size_type> ProgramIndexMap;
	typedef std::vector<IPtr<Unit>> UnitVector;
	UnitVector units;
	ProgramListVector programLists;
	ProgramIndexMap programIndexMap;
	UnitID selectedUnit;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
