//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vsteditcontroller.cpp
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

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "base/source/updatehandler.h"
#include "pluginterfaces/base/ustring.h"

#include <stdio.h>

namespace Steinberg {
namespace Vst {

KnobMode EditController::hostKnobMode = kCircularMode;

//------------------------------------------------------------------------
// EditController Implementation
//------------------------------------------------------------------------
EditController::EditController () : componentHandler (nullptr), componentHandler2 (nullptr)
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::initialize (FUnknown* context)
{
	return ComponentBase::initialize (context);
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::terminate ()
{
	parameters.removeAll ();

	if (componentHandler)
	{
		componentHandler->release ();
		componentHandler = nullptr;
	}

	if (componentHandler2)
	{
		componentHandler2->release ();
		componentHandler2 = nullptr;
	}

	return ComponentBase::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::setComponentState (IBStream* /*state*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::setState (IBStream* /*state*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::getState (IBStream* /*state*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
int32 PLUGIN_API EditController::getParameterCount ()
{
	return parameters.getParameterCount ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::getParameterInfo (int32 paramIndex, ParameterInfo& info)
{
	Parameter* parameter = parameters.getParameterByIndex (paramIndex);
	if (parameter)
	{
		info = parameter->getInfo ();
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::getParamStringByValue (ParamID tag, ParamValue valueNormalized,
                                                          String128 string)
{
	Parameter* parameter = getParameterObject (tag);
	if (parameter)
	{
		parameter->toString (valueNormalized, string);
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::getParamValueByString (ParamID tag, TChar* string,
                                                          ParamValue& valueNormalized)
{
	Parameter* parameter = getParameterObject (tag);
	if (parameter)
	{
		if (parameter->fromString (string, valueNormalized))
		{
			return kResultTrue;
		}
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
ParamValue PLUGIN_API EditController::normalizedParamToPlain (ParamID tag,
                                                              ParamValue valueNormalized)
{
	Parameter* parameter = getParameterObject (tag);
	if (parameter)
	{
		return parameter->toPlain (valueNormalized);
	}
	return valueNormalized;
}

//------------------------------------------------------------------------
ParamValue PLUGIN_API EditController::plainParamToNormalized (ParamID tag, ParamValue plainValue)
{
	Parameter* parameter = getParameterObject (tag);
	if (parameter)
	{
		return parameter->toNormalized (plainValue);
	}
	return plainValue;
}

//------------------------------------------------------------------------
ParamValue PLUGIN_API EditController::getParamNormalized (ParamID tag)
{
	Parameter* parameter = getParameterObject (tag);
	if (parameter)
	{
		return parameter->getNormalized ();
	}
	return 0.;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::setParamNormalized (ParamID tag, ParamValue value)
{
	Parameter* parameter = getParameterObject (tag);
	if (parameter)
	{
		parameter->setNormalized (value);
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::setComponentHandler (IComponentHandler* newHandler)
{
	if (componentHandler == newHandler)
	{
		return kResultTrue;
	}

	if (componentHandler)
	{
		componentHandler->release ();
	}

	componentHandler = newHandler;
	if (componentHandler)
	{
		componentHandler->addRef ();
	}

	// try to get the extended version
	if (componentHandler2)
	{
		componentHandler2->release ();
		componentHandler2 = nullptr;
	}

	if (newHandler)
	{
		newHandler->queryInterface (IComponentHandler2::iid, (void**)&componentHandler2);
	}
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult EditController::beginEdit (ParamID tag)
{
	if (componentHandler)
	{
		return componentHandler->beginEdit (tag);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult EditController::performEdit (ParamID tag, ParamValue valueNormalized)
{
	if (componentHandler)
	{
		return componentHandler->performEdit (tag, valueNormalized);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult EditController::endEdit (ParamID tag)
{
	if (componentHandler)
	{
		return componentHandler->endEdit (tag);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult EditController::startGroupEdit ()
{
	if (componentHandler2)
	{
		return componentHandler2->startGroupEdit ();
	}
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult EditController::finishGroupEdit ()
{
	if (componentHandler2)
	{
		return componentHandler2->finishGroupEdit ();
	}
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult EditController::getParameterInfoByTag (ParamID tag, ParameterInfo& info)
{
	Parameter* parameter = getParameterObject (tag);
	if (parameter)
	{
		info = parameter->getInfo ();
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult EditController::setDirty (TBool state)
{
	if (componentHandler2)
	{
		return componentHandler2->setDirty (state);
	}
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult EditController::requestOpenEditor (FIDString name)
{
	if (componentHandler2)
	{
		return componentHandler2->requestOpenEditor (name);
	}
	return kNotImplemented;
}

//------------------------------------------------------------------------
// EditorView Implementation
//------------------------------------------------------------------------
EditorView::EditorView (EditController* controller, ViewRect* size)
: CPluginView (size), controller (controller)
{
	if (controller)
	{
		controller->addRef ();
	}
}

//------------------------------------------------------------------------
EditorView::~EditorView ()
{
	if (controller)
	{
		controller->editorDestroyed (this);
		controller->release ();
	}
}

//------------------------------------------------------------------------
void EditorView::attachedToParent ()
{
	if (controller)
	{
		controller->editorAttached (this);
	}
}

//------------------------------------------------------------------------
void EditorView::removedFromParent ()
{
	if (controller)
	{
		controller->editorRemoved (this);
	}
}

//------------------------------------------------------------------------
// EditControllerEx1 implementation
//------------------------------------------------------------------------
EditControllerEx1::EditControllerEx1 () : selectedUnit (kRootUnitId)
{
	UpdateHandler::instance ();
}

//------------------------------------------------------------------------
EditControllerEx1::~EditControllerEx1 ()
{
	for (ProgramListVector::const_iterator it = programLists.begin (), end = programLists.end ();
	     it != end; ++it)
	{
		if (*it)
			(*it)->removeDependent (this);
	}
}

//------------------------------------------------------------------------
bool EditControllerEx1::addUnit (Unit* unit)
{
	units.push_back (IPtr<Unit> (unit, false));
	return true;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::getUnitInfo (int32 unitIndex, UnitInfo& info /*out*/)
{
	Unit* unit = units.at (unitIndex);
	if (unit)
	{
		info = unit->getInfo ();
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult EditControllerEx1::notifyUnitSelection ()
{
	tresult result = kResultFalse;
	FUnknownPtr<IUnitHandler> unitHandler (componentHandler);
	if (unitHandler)
		result = unitHandler->notifyUnitSelection (selectedUnit);
	return result;
}

//------------------------------------------------------------------------
bool EditControllerEx1::addProgramList (ProgramList* list)
{
	programIndexMap[list->getID ()] = programLists.size ();
	programLists.push_back (IPtr<ProgramList> (list, false));
	list->addDependent (this);
	return true;
}

//------------------------------------------------------------------------
ProgramList* EditControllerEx1::getProgramList (ProgramListID listId) const
{
	ProgramIndexMap::const_iterator it = programIndexMap.find (listId);
	return it == programIndexMap.end () ? nullptr : programLists[it->second];
}

//------------------------------------------------------------------------
tresult EditControllerEx1::notifyProgramListChange (ProgramListID listId, int32 programIndex)
{
	tresult result = kResultFalse;
	FUnknownPtr<IUnitHandler> unitHandler (componentHandler);
	if (unitHandler)
		result = unitHandler->notifyProgramListChange (listId, programIndex);
	return result;
}

//------------------------------------------------------------------------
int32 PLUGIN_API EditControllerEx1::getProgramListCount ()
{
	return static_cast<int32> (programLists.size ());
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::getProgramListInfo (int32 listIndex,
                                                          ProgramListInfo& info /*out*/)
{
	if (listIndex < 0 || listIndex >= static_cast<int32> (programLists.size ()))
		return kResultFalse;
	info = programLists[listIndex]->getInfo ();
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::getProgramName (ProgramListID listId, int32 programIndex,
                                                      String128 name /*out*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find (listId);
	if (it != programIndexMap.end ())
	{
		return programLists[it->second]->getProgramName (programIndex, name);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult EditControllerEx1::setProgramName (ProgramListID listId, int32 programIndex,
                                           const String128 name /*in*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find (listId);
	if (it != programIndexMap.end ())
	{
		return programLists[it->second]->setProgramName (programIndex, name);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::getProgramInfo (ProgramListID listId, int32 programIndex,
                                                      CString attributeId /*in*/,
                                                      String128 attributeValue /*out*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find (listId);
	if (it != programIndexMap.end ())
	{
		return programLists[it->second]->getProgramInfo (programIndex, attributeId, attributeValue);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::hasProgramPitchNames (ProgramListID listId,
                                                            int32 programIndex)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find (listId);
	if (it != programIndexMap.end ())
	{
		return programLists[it->second]->hasPitchNames (programIndex);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::getProgramPitchName (ProgramListID listId, int32 programIndex,
                                                           int16 midiPitch, String128 name /*out*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find (listId);
	if (it != programIndexMap.end ())
	{
		return programLists[it->second]->getPitchName (programIndex, midiPitch, name);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
void PLUGIN_API EditControllerEx1::update (FUnknown* changedUnknown, int32 /*message*/)
{
	ProgramList* programList = FCast<ProgramList> (changedUnknown);
	if (programList)
	{
		FUnknownPtr<IUnitHandler> unitHandler (componentHandler);
		if (unitHandler)
			unitHandler->notifyProgramListChange (programList->getID (), kAllProgramInvalid);
	}
}

//------------------------------------------------------------------------
// Unit implementation
//------------------------------------------------------------------------
Unit::Unit ()
{
	memset (&info, 0, sizeof (UnitInfo));
}

//------------------------------------------------------------------------
Unit::Unit (const String128 name, UnitID unitId, UnitID parentUnitId, ProgramListID programListId)
{
	setName (name);
	info.id = unitId;
	info.parentUnitId = parentUnitId;
	info.programListId = programListId;
}

//------------------------------------------------------------------------
Unit::Unit (const UnitInfo& info) : info (info)
{
}

//------------------------------------------------------------------------
void Unit::setName (const String128 newName)
{
	UString128 (newName).copyTo (info.name, 128);
}

//------------------------------------------------------------------------
// ProgramList implementation
//------------------------------------------------------------------------
ProgramList::ProgramList (const String128 name, ProgramListID listId, UnitID unitId)
: unitId (unitId), parameter (nullptr)
{
	UString128 (name).copyTo (info.name, 128);
	info.id = listId;
	info.programCount = 0;
}

//------------------------------------------------------------------------
ProgramList::ProgramList (const ProgramList& programList)
: info (programList.info)
, unitId (programList.unitId)
, programNames (programList.programNames)
, parameter (nullptr)
{
}

//------------------------------------------------------------------------
int32 ProgramList::addProgram (const String128 name)
{
	++info.programCount;
	programNames.push_back (name);
	programInfos.push_back (ProgramInfoVector::value_type ());
	return static_cast<int32> (programNames.size ()) - 1;
}

//------------------------------------------------------------------------
bool ProgramList::setProgramInfo (int32 programIndex, CString attributeId, const String128 value)
{
	if (programIndex >= 0 && programIndex < static_cast<int32> (programNames.size ()))
	{
		programInfos.at (programIndex).insert (std::make_pair (attributeId, value));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
tresult ProgramList::getProgramInfo (int32 programIndex, CString attributeId,
                                     String128 value /*out*/)
{
	if (programIndex >= 0 && programIndex < static_cast<int32> (programNames.size ()))
	{
		StringMap::const_iterator it = programInfos[programIndex].find (attributeId);
		if (it != programInfos[programIndex].end ())
		{
			if (!it->second.isEmpty ())
			{
				it->second.copyTo16 (value, 0, 128);
				return kResultTrue;
			}
		}
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult ProgramList::getProgramName (int32 programIndex, String128 name /*out*/)
{
	if (programIndex >= 0 && programIndex < static_cast<int32> (programNames.size ()))
	{
		programNames.at (programIndex).copyTo16 (name, 0, 128);
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult ProgramList::setProgramName (int32 programIndex, const String128 name /*in*/)
{
	if (programIndex >= 0 && programIndex < static_cast<int32> (programNames.size ()))
	{
		programNames.at (programIndex) = name;
		if (parameter)
		{
			static_cast<StringListParameter*> (parameter)->replaceString (programIndex, name);
		}
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
Parameter* ProgramList::getParameter ()
{
	if (parameter == nullptr)
	{
		StringListParameter* listParameter = new StringListParameter (
		    info.name, info.id, 0,
		    ParameterInfo::kCanAutomate | ParameterInfo::kIsList | ParameterInfo::kIsProgramChange,
		    unitId);
		for (StringVector::const_iterator it = programNames.begin (), end = programNames.end ();
		     it != end; ++it)
		{
			listParameter->appendString (*it);
		}
		parameter = listParameter;
	}
	return parameter;
}

//------------------------------------------------------------------------
// ProgramListWithPitchNames implementation
//-----------------------------------------------------------------------------
ProgramListWithPitchNames::ProgramListWithPitchNames (const String128 name, ProgramListID listId,
                                                      UnitID unitId)
: ProgramList (name, listId, unitId)
{
}

//-----------------------------------------------------------------------------
int32 ProgramListWithPitchNames::addProgram (const String128 name)
{
	int32 index = ProgramList::addProgram (name);
	if (index >= 0)
		pitchNames.push_back (PitchNamesVector::value_type ());
	return index;
}

//-----------------------------------------------------------------------------
bool ProgramListWithPitchNames::setPitchName (int32 programIndex, int16 pitch,
                                              const String128 pitchName)
{
	if (programIndex < 0 || programIndex >= getCount ())
		return false;

	bool nameChanged = true;
	std::pair<PitchNameMap::iterator, bool> res =
	    pitchNames[programIndex].insert (std::make_pair (pitch, pitchName));
	if (!res.second)
	{
		if (res.first->second == pitchName)
			nameChanged = false;
		else
			res.first->second = pitchName;
	}

	if (nameChanged)
		changed ();
	return true;
}

//-----------------------------------------------------------------------------
bool ProgramListWithPitchNames::removePitchName (int32 programIndex, int16 pitch)
{
	bool result = false;
	if (programIndex >= 0 && programIndex < getCount ())
	{
		result = pitchNames.at (programIndex).erase (pitch) != 0;
	}
	if (result)
		changed ();
	return result;
}

//-----------------------------------------------------------------------------
tresult ProgramListWithPitchNames::hasPitchNames (int32 programIndex)
{
	if (programIndex >= 0 && programIndex < getCount ())
		return pitchNames.at (programIndex).empty () ? kResultFalse : kResultTrue;
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult ProgramListWithPitchNames::getPitchName (int32 programIndex, int16 midiPitch,
                                                 String128 name /*out*/)
{
	if (programIndex >= 0 && programIndex < getCount ())
	{
		PitchNameMap::const_iterator it = pitchNames[programIndex].find (midiPitch);
		if (it != pitchNames[programIndex].end ())
		{
			it->second.copyTo16 (name, 0, 128);
			return kResultTrue;
		}
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
