//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vsteditcontroller.cpp
// Created by  : Steinberg, 04/2005
// Description : VST Edit Controller Implementation
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "base/source/updatehandler.h"
#include "pluginterfaces/base/funknownimpl.h"
#include "pluginterfaces/base/ustring.h"

#include <cstdio>

namespace Steinberg {
namespace Vst {

KnobMode EditController::hostKnobMode = kCircularMode;

//------------------------------------------------------------------------
// EditController Implementation
//------------------------------------------------------------------------
EditController::EditController ()
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

	componentHandler.reset ();
	componentHandler2.reset ();

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
	if (Parameter* parameter = parameters.getParameterByIndex (paramIndex))
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
	if (Parameter* parameter = getParameterObject (tag))
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
	if (Parameter* parameter = getParameterObject (tag))
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
	if (Parameter* parameter = getParameterObject (tag))
	{
		return parameter->toPlain (valueNormalized);
	}
	return valueNormalized;
}

//------------------------------------------------------------------------
ParamValue PLUGIN_API EditController::plainParamToNormalized (ParamID tag, ParamValue plainValue)
{
	if (Parameter* parameter = getParameterObject (tag))
	{
		return parameter->toNormalized (plainValue);
	}
	return plainValue;
}

//------------------------------------------------------------------------
ParamValue PLUGIN_API EditController::getParamNormalized (ParamID tag)
{
	if (Parameter* parameter = getParameterObject (tag))
	{
		return parameter->getNormalized ();
	}
	return 0.;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditController::setParamNormalized (ParamID tag, ParamValue value)
{
	if (Parameter* parameter = getParameterObject (tag))
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

	componentHandler = newHandler;
	componentHandler2.reset ();

    // try to get the extended version
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
	if (Parameter* parameter = getParameterObject (tag))
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

#ifndef NO_PLUGUI
//------------------------------------------------------------------------
// EditorView Implementation
//------------------------------------------------------------------------
EditorView::EditorView (EditController* _controller, ViewRect* size)
: CPluginView (size), controller (_controller)
{
}

//------------------------------------------------------------------------
EditorView::~EditorView ()
{
	if (controller)
	{
		controller->editorDestroyed (this);
		controller = nullptr;
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
#endif // NO_PLUGUI

//------------------------------------------------------------------------
// EditControllerEx1 implementation
//------------------------------------------------------------------------
EditControllerEx1::EditControllerEx1 ()
{
	UpdateHandler::instance ();
}

//------------------------------------------------------------------------
EditControllerEx1::~EditControllerEx1 ()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::terminate ()
{
	units.clear ();

	for (const auto& programList : programLists)
	{
		if (programList)
			programList->removeDependent (this);
	}
	programLists.clear ();
	programIndexMap.clear ();

	return EditController::terminate ();
}

//------------------------------------------------------------------------
bool EditControllerEx1::addUnit (Unit* unit)
{
	units.emplace_back (unit, false);
	return true;
}

//------------------------------------------------------------------------
tresult PLUGIN_API EditControllerEx1::getUnitInfo (int32 unitIndex, UnitInfo& info /*out*/)
{
	if (unitIndex < 0 || unitIndex >= static_cast<int32> (units.size ()))
		return kResultFalse;
	if (Unit* unit = units.at (unitIndex))
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
	if (auto unitHandler = U::cast<IUnitHandler> (componentHandler))
		result = unitHandler->notifyUnitSelection (selectedUnit);
	return result;
}

//------------------------------------------------------------------------
bool EditControllerEx1::addProgramList (ProgramList* list)
{
	programIndexMap[list->getID ()] = programLists.size ();
	programLists.emplace_back (list, false);
	list->addDependent (this);
	return true;
}

//------------------------------------------------------------------------
ProgramList* EditControllerEx1::getProgramList (ProgramListID listId) const
{
	auto it = programIndexMap.find (listId);
	return it == programIndexMap.end () ? nullptr : programLists[it->second];
}

//------------------------------------------------------------------------
tresult EditControllerEx1::notifyProgramListChange (ProgramListID listId, int32 programIndex)
{
	tresult result = kResultFalse;
	if (auto unitHandler = U::cast<IUnitHandler> (componentHandler))
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
	auto* programList = FCast<ProgramList> (changedUnknown);
	if (programList)
	{
		if (auto unitHandler = U::cast<IUnitHandler> (componentHandler))
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
: unitId (unitId)
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
{
}

//------------------------------------------------------------------------
int32 ProgramList::addProgram (const String128 name)
{
	++info.programCount;
	programNames.emplace_back (name);
	programInfos.emplace_back ();

	if (parameter)
	{
		static_cast<StringListParameter*> (parameter)->appendString (name);
	}

	return static_cast<int32> (programNames.size ()) - 1;
}

//------------------------------------------------------------------------
void ProgramList::clearPrograms ()
{
	info.programCount = 0;
	programNames.clear ();
	programInfos.clear ();

	if (parameter)
	{
		static_cast<StringListParameter*> (parameter)->clear ();
	}
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
			if (!it->second.empty ())
			{
				memset (value, 0, sizeof (String128));
				it->second.copy (value, 128);
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
		memset (name, 0, sizeof (String128));
		programNames.at (programIndex).copy (name, 128);
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
		auto* listParameter = new StringListParameter (
			info.name, info.id, nullptr,
			ParameterInfo::kCanAutomate | ParameterInfo::kIsList | ParameterInfo::kIsProgramChange,
			unitId);
		for (const auto& programName : programNames)
		{
			listParameter->appendString (programName.data ());
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
		pitchNames.emplace_back ();
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
		return (pitchNames.at (programIndex).empty () == true) ? kResultFalse : kResultTrue;
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
			memset (name, 0, sizeof (String128));
			it->second.copy (name, 128);
			return kResultTrue;
		}
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
