/*================================================================================================*/
/*
 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	This file is part of the Avid AAX SDK.
 *	
 *	The AAX SDK is subject to commercial or open-source licensing.
 *	
 *	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
 *	Agreement and Avid Privacy Policy.
 *	
 *	AAX SDK License: https://developer.avid.com/aax
 *	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
 *	
 *	Or: You may also use this code under the terms of the GPL v3 (see
 *	www.gnu.org/licenses).
 *	
 *	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 *	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 *	DISCLAIMED.
 *
 */
/*================================================================================================*/

#include "AAX_CEffectParameters.h"
#include "AAX_VController.h"
#include "AAX_VAutomationDelegate.h"
#include "AAX_VTransport.h"
#include "AAX_VPageTable.h"
#include "AAX_Assert.h"
#include "AAX_CParameterManager.h"
#include "AAX_CPacketDispatcher.h"
#include <cmath>
#include <cstring>


AAX_CParamID	cPreviewID = "PreviewID";
AAX_CParamID	cDefaultMasterBypassID = "MasterBypassID";

const AAX_CTypeID CONTROLS_CHUNK_ID = 'elck';
const char CONTROLS_CHUNK_DESCRIPTION[] = "Complete Controls State";

/*!
*	\internal Converts four character AAX_CTypeID to a string. Used by chunk methods.
*/
inline void ConvertOSTypeToCString(AAX_CTypeID osType, char *outStr)
{
	char *source = (char *)&osType;
	outStr[4] = 0;
#if defined(__BIG_ENDIAN__) && (0 != __BIG_ENDIAN__)
	outStr[0] = source[0];
	outStr[1] = source[1];
	outStr[2] = source[2];
	outStr[3] = source[3];
#else
	outStr[3] = source[0];
	outStr[2] = source[1];
	outStr[1] = source[2];
	outStr[0] = source[3];
#endif
}

int32_t	NormalizedToInt32(double	normalizedValue)
{
	//clamp the normalized value, just to make sure... 
	if (normalizedValue > 1)
		normalizedValue = 1;
	if (normalizedValue < 0)
		normalizedValue = 0;
	
	//Convert the double [0 to 1] to a full range int.
	return (int32_t)(std::floor(double(AAX_INT32_MIN) + normalizedValue * (double(AAX_INT32_MAX) - double(AAX_INT32_MIN)) + 0.5));
}

double	Int32ToNormalized(int32_t value)
{
	double normalizedValue = double(value) - double(AAX_INT32_MIN);
	normalizedValue = normalizedValue / (double(AAX_INT32_MAX) - double(AAX_INT32_MIN));
	return normalizedValue;
}

double BoolToNormalized (bool value )
{
	if ( value )
		return 1.0;
	return 0.0;
}


/*===================================================================================================*/
AAX_CEffectParameters::AAX_CEffectParameters (void)
: mNumPlugInChanges(0)
, mChunkSize(0)
, mChunkParser()
, mNumChunkedParameters(0)
, mPacketDispatcher()
, mParameterManager()
, mFilteredParameters()
, mController(NULL)
, mTransport(NULL)
, mAutomationDelegate(NULL)
{		
}

AAX_CEffectParameters::~AAX_CEffectParameters(void)
{
}

AAX_Result	AAX_CEffectParameters::Initialize(IACFUnknown* inController)
{
	AAX_Result err = AAX_SUCCESS;
	mController = new AAX_VController(inController);
	mAutomationDelegate = new AAX_VAutomationDelegate(inController);
	mTransport = new AAX_VTransport(inController);
	
	mParameterManager.Initialize(mAutomationDelegate);
	mPacketDispatcher.Initialize(mController, this);
		
	//Call into EffectInit() which is a pure virtual hook for 3Ps to override, where they add parameters and meters.
	err = EffectInit();
    if (err != AAX_SUCCESS)
    {
        return err;
	}
    
	//Filter out the MasterBypass control as the effect layer used to do.
	AAX_CString bypassID;
	GetMasterBypassParameter(&bypassID);
	FilterParameterIDOnSave((AAX_CParamID)bypassID.Get());
	
	// Subtract any controls that have been eliminated from the chunk with a FilterControlIdOnSave().  
	int32_t numControls = 0;
	if (AAX_SUCCESS == GetNumberOfParameters(&numControls)) {
		if (numControls > 0) {
			mNumChunkedParameters = numControls - int32_t(mFilteredParameters.size());
			AAX_ASSERT(mNumChunkedParameters >= 0);
		}
	}
	else {
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_Critical, "AAX_CEffectParameters::Initialize - error getting the number of parameters");
	}

	return err;
}

AAX_Result	AAX_CEffectParameters::Uninitialize()
{
	mParameterManager.RemoveAllParameters();

	if ( mController )
	{
		delete mController;
		mController = NULL;
	}
	if ( mAutomationDelegate )
	{
		delete mAutomationDelegate;
		mAutomationDelegate = NULL;
	}
	if ( mTransport )
	{
		delete mTransport;
		mTransport = NULL;
	}
	
	return AAX_SUCCESS;
}

AAX_Result	AAX_CEffectParameters::NotificationReceived(AAX_CTypeID inNotificationType, const void * inNotificationData, uint32_t /*inNotificationDataSize*/)
{
	switch (inNotificationType) {
		case AAX_eNotificationEvent_ASPreviewState:
		{
			AAX_IParameter* parameter = mParameterManager.GetParameterByID(cPreviewID);
			if (parameter != 0)
			{
				int32_t previewState = *reinterpret_cast<const int32_t*>(inNotificationData);
				parameter->SetValueWithBool((previewState == 0) ? false : true);
			}
			return AAX_SUCCESS;
		}
	}
    
	return AAX_SUCCESS;
}


AAX_IController*	AAX_CEffectParameters::Controller()
{
	return mController;
}

const AAX_IController*	AAX_CEffectParameters::Controller() const
{
	return mController;
}

AAX_IAutomationDelegate*        AAX_CEffectParameters::AutomationDelegate()
{
    return mAutomationDelegate;
}

const AAX_IAutomationDelegate*  AAX_CEffectParameters::AutomationDelegate() const
{
    return mAutomationDelegate;
}

AAX_Result AAX_CEffectParameters::GetNumberOfParameters(int32_t* aNumControls)  const 
{
	*aNumControls = mParameterManager.NumParameters();
	return AAX_SUCCESS;
}

AAX_Result AAX_CEffectParameters::GetMasterBypassParameter( AAX_IString * oMasterBypassControl )  const 
{
	//<DMT> Having this return a default value when this class doesn't actually create this parameter cause all sorts of bugs.  This needs to return NULL.  However, if the default master bypass ID is used and that parameter exists, we will return that value.  Mainly this is to preserve behavior for people who've already ported their plug-ins using the older code.  When adding a parameter, please use cDefaultMasterBypassID instead calling this function.
	if (mParameterManager.GetParameterByID(cDefaultMasterBypassID) != 0)
		*oMasterBypassControl = AAX_CString(cDefaultMasterBypassID);
	else
		*oMasterBypassControl = AAX_CString("");

	return AAX_SUCCESS;
}

AAX_Result AAX_CEffectParameters::GetParameterIsAutomatable( AAX_CParamID iParameterID, AAX_CBoolean * itIs )  const 
{
	*itIs = false;
	const AAX_IParameter* parameter = mParameterManager.GetParameterByID(iParameterID);
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;

	*itIs = static_cast<AAX_CBoolean>(parameter->Automatable());
	return AAX_SUCCESS;
}

AAX_Result AAX_CEffectParameters::GetParameterNumberOfSteps( AAX_CParamID iParameterID, int32_t * aNumSteps )  const 
{
	const AAX_IParameter* parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	const uint32_t numSteps = parameter->GetNumberOfSteps();
	if (numSteps > 0x7FFFFFFF)
		return AAX_ERROR_SIGNED_INT_OVERFLOW;
	
	*aNumSteps = static_cast<int32_t>(numSteps);
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::GetParameterValueString ( AAX_CParamID iParameterID, AAX_IString * oValueString, int32_t iMaxLength )  const 
{
	//<DMT> Right now, this one isn't called from DAE.  It instead calls GetParameterStringFromValue().
	const AAX_IParameter*			parameter = mParameterManager.GetParameterByID(iParameterID);
	AAX_CString				str;

	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;

	parameter->GetValueString(iMaxLength, &str);
	*oValueString = str;
	return AAX_SUCCESS;	
}


AAX_Result AAX_CEffectParameters::GetParameterValueFromString ( AAX_CParamID iParameterID, double * oValuePtr, const AAX_IString & iValueString )  const 
{
	const AAX_IParameter*			parameter = mParameterManager.GetParameterByID(iParameterID);
	const AAX_CString				valueStr(iValueString);
	double normValue;
	
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;

	if (!parameter->GetNormalizedValueFromString(valueStr, &normValue))
		return AAX_ERROR_INVALID_STRING_CONVERSION;

	*oValuePtr = normValue;
	return AAX_SUCCESS;
}
	
AAX_Result AAX_CEffectParameters::GetParameterStringFromValue ( AAX_CParamID iParameterID, double value, AAX_IString * valueString, int32_t maxLength )  const 
{
	const AAX_IParameter *	parameter = mParameterManager.GetParameterByID(iParameterID);
	AAX_CString				valueStr;
	
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	if (!parameter->GetStringFromNormalizedValue(value, maxLength, valueStr))
		return AAX_ERROR_INVALID_STRING_CONVERSION;
	
	*valueString = valueStr;
	return AAX_SUCCESS;
}

AAX_Result AAX_CEffectParameters::GetParameterName( AAX_CParamID iParameterID, AAX_IString * oName )  const 
{
	const AAX_IParameter*			parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter != 0)
	{
		*oName = parameter->Name();
	}
	else
		*oName = "";

	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::GetParameterNameOfLength ( AAX_CParamID iParameterID, AAX_IString * oName, int32_t iNameLength )  const 
{
	AAX_Result	aResult = AAX_ERROR_INVALID_STRING_CONVERSION;
	
	if (NULL != oName)
	{
		aResult = AAX_SUCCESS;
		
		const AAX_IParameter* parameter = mParameterManager.GetParameterByID( iParameterID );
		if (parameter != 0)
		{
			//<DMT> Try to get a shortened name from the parameter first.  (If there aren't any short names, this function will return the full name to be shortened here.)
			const AAX_CString&  shortName = parameter->ShortenedName(iNameLength);
			
			//<DMT> Legacy name shortening algorithm kept here to keep similar behavior with ported plug-ins.
			std::vector<std::string>	controlNameStrings;
			char	aTempName[256];
			aTempName[0] = 0;
			strncpy ( aTempName, shortName.CString(), 255 );
			int	tempLength = int(strlen ( aTempName ));
			char *	curLoc = aTempName;
			char *	endLoc = curLoc + tempLength;
			while ( curLoc < endLoc )
			{
				controlNameStrings.push_back( curLoc );
				char *returnLoc = strchr( curLoc, '\n' );
				if ( returnLoc )
				{
					*returnLoc = 0;
					curLoc = returnLoc + 1;
				}
				else curLoc = endLoc;
			}

			int32_t	maxSize = 0;
			std::vector<std::string>::iterator	iter = controlNameStrings.begin();
			for ( ; iter != controlNameStrings.end(); ++iter )
			{
				if ( (int32_t) iter->size() > maxSize && (int32_t) iter->size() <= iNameLength )
				{
					maxSize = int32_t(iter->size());
					oName->Set(iter->c_str());
				}
			}
			
			if ( maxSize == 0 && controlNameStrings.size () > 0 )
			{
				if (0 < iNameLength)
				{
					shortName.SubString(0, static_cast<uint32_t>(iNameLength), oName);
				}
				else
				{
					oName->Set("");
				}
			}
		}
		else
		{
			oName->Set("");
		}
	}

	return aResult;		
}	


AAX_Result AAX_CEffectParameters::GetParameterNormalizedValue ( AAX_CParamID iParameterID, double * oValuePtr )  const 
{
	const AAX_IParameter*			parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;

	*oValuePtr = parameter->GetNormalizedValue();
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::GetParameterDefaultNormalizedValue( AAX_CParamID iParameterID, double * aValuePtr )  const 
{
	const AAX_IParameter* parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;

	*aValuePtr = parameter->GetNormalizedDefaultValue();
	return AAX_SUCCESS;
}

AAX_Result AAX_CEffectParameters::SetParameterDefaultNormalizedValue(AAX_CParamID iParameterID, double theDefaultValue)
{
	AAX_IParameter* parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;

	parameter->SetNormalizedDefaultValue(theDefaultValue);
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::SetParameterNormalizedValue (AAX_CParamID iParameterID, double aValue)
{
	// Right now, let's try calling SetNormalizedValue() and see how the automated parameter
	// wrapper works out.  This basically forwards this call into the parameter manager's 
	// SetValue() call, which should then do the token dispatching.  That may call back into
	// this object for now, but eventually, it should all be self contained in the parameter manager
	// so we can remove this wrapper in the future.
	AAX_IParameter*			parameter = mParameterManager.GetParameterByID(iParameterID);
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	parameter->SetNormalizedValue ( aValue );
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::SetParameterNormalizedRelative(AAX_CParamID iParameterID, double aValue)
{
	// This assumes that controls are NOT meant to wrap.  
	// If that's the desired action, then override this puppy.
	
	double oldValue;
	double newValue;

	AAX_Result result = this->GetParameterNormalizedValue(iParameterID, &oldValue);
	if (result != AAX_SUCCESS)
		return result;
	
	newValue = aValue+oldValue;
	
	//Clamp the value.
	if (newValue > 1.0)
		newValue = 1.0;
	if (newValue < 0.0)
		newValue = 0.0;
	
	return this->SetParameterNormalizedValue(iParameterID, newValue);
}


AAX_Result AAX_CEffectParameters::TouchParameter(AAX_CParamID iParameterID)
{
	AAX_IParameter *	parameter = mParameterManager.GetParameterByID( iParameterID );
	if ( parameter )
		parameter->Touch ();
	else return AAX_ERROR_INVALID_PARAMETER_ID;

	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::ReleaseParameter(AAX_CParamID iParameterID)
{
	AAX_IParameter *	parameter = mParameterManager.GetParameterByID( iParameterID );
	if ( parameter )
		parameter->Release ();
	else return AAX_ERROR_INVALID_PARAMETER_ID;
	
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::UpdateParameterTouch ( AAX_CParamID /*iParameterID*/, AAX_CBoolean /*iTouchState*/ )
{
	return AAX_SUCCESS;
}



AAX_Result AAX_CEffectParameters::UpdateParameterNormalizedValue(AAX_CParamID iParameterID, double aValue, AAX_EUpdateSource /*iSource*/ )
{
	AAX_Result	result = AAX_SUCCESS;
	
	// We will be using a custom entry point in AAX_IParameter.
	AAX_IParameter*					parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	const double prevValue = parameter->GetNormalizedValue();
	
	// Store the value into the parameter.
	parameter->UpdateNormalizedValue(aValue);
	
	// Now the control has changed
	result = mPacketDispatcher.SetDirty(iParameterID);
	
	if (prevValue != aValue)
		++mNumPlugInChanges;
	
	return result;
}

AAX_Result AAX_CEffectParameters::GenerateCoefficients()
{
	AAX_Result	result = AAX_SUCCESS;
	
	result = mPacketDispatcher.Dispatch();
	
	return result;
}

AAX_Result AAX_CEffectParameters::ResetFieldData (AAX_CFieldIndex /*inFieldIndex*/, void* oData, uint32_t inDataSize) const
{
	//Default implementation is just to zero out all data.
	memset(oData, 0, inDataSize);
	return AAX_SUCCESS;
}


AAX_Result	AAX_CEffectParameters::UpdateParameterNormalizedRelative(AAX_CParamID iParameterID, double aValue)
{
	// This assumes that controls are NOT meant to wrap.  
	// If that's the desired action, then override this puppy.
	double oldValue;
	double newValue;
	
	AAX_Result	result = this->GetParameterNormalizedValue( iParameterID, &oldValue );
	if (result != AAX_SUCCESS)
		return result;
	
	newValue = aValue + oldValue;
	
	//Clamp the value to the normalized range.
	if (newValue > 1.0)
		newValue = 1.0;
	if (newValue < 0.0)
		newValue = 0.0;
	
	return this->UpdateParameterNormalizedValue( iParameterID, newValue, AAX_eUpdateSource_Unspecified );
}

AAX_Result AAX_CEffectParameters::GetNumberOfChunks ( int32_t * numChunks )  const 
{
	*numChunks = 1;	//just the standard control chunk.
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::GetChunkIDFromIndex ( int32_t index, AAX_CTypeID * chunkID )  const 
{
	if (index != 0)
	{
		*chunkID = AAX_CTypeID(0);
		return AAX_ERROR_INVALID_CHUNK_INDEX;
	}
	
	*chunkID = CONTROLS_CHUNK_ID;
	return AAX_SUCCESS;	
}


AAX_Result AAX_CEffectParameters::GetChunkSize ( AAX_CTypeID chunkID, uint32_t * oSize )  const 
{
	if (chunkID != CONTROLS_CHUNK_ID)
	{
		*oSize = 0;
		return AAX_ERROR_INVALID_CHUNK_ID;
	}
	
	BuildChunkData();
	mChunkSize = mChunkParser.GetChunkDataSize();
	
	if (mChunkSize < 0)
	{
		return AAX_ERROR_INCORRECT_CHUNK_SIZE;
	}
	
	*oSize = static_cast<uint32_t>(mChunkSize);
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::GetChunk ( AAX_CTypeID chunkID, AAX_SPlugInChunk * chunk )  const 
{	
	//Check the chunkID
	if (chunkID != CONTROLS_CHUNK_ID)
		return AAX_ERROR_INVALID_CHUNK_ID;
	
	//Build the chunk.
	BuildChunkData();
	int32_t currentChunkSize = mChunkParser.GetChunkDataSize();		//Verify that the chunk data size hasn't changed since the last GetChunkSize call.
	if (mChunkSize != currentChunkSize || mChunkSize == 0)
	{
		return AAX_ERROR_INCORRECT_CHUNK_SIZE;	//If mChunkSize doesn't match the currently built chunk, then its likely that the previous call to GetChunkSize() didn't return the correct size.
	}
	
	//Set the version on the chunk data structure.  The other manID, prodID, PlugID, and fSize are populated already, coming from AAXCollection.
	chunk->fVersion = mChunkParser.GetChunkVersion();
	memset(chunk->fName, 0, 32);		//Just in case, lets make sure unused chars are null.
	strncpy(reinterpret_cast<char *>(chunk->fName), CONTROLS_CHUNK_DESCRIPTION, 31);
	return mChunkParser.GetChunkData(chunk);
}


AAX_Result AAX_CEffectParameters::SetChunk ( AAX_CTypeID chunkID, const AAX_SPlugInChunk * chunk )
{
	if (chunkID != CONTROLS_CHUNK_ID) 
		return AAX_ERROR_INVALID_CHUNK_ID;

	mChunkParser.LoadChunk(chunk);
	
	int32_t numControls = 0, controlIndex = 0;
	AAX_Result err = AAX_SUCCESS;
	err = this->GetNumberOfParameters(&numControls);
	if (AAX_SUCCESS != err) {
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_Critical, "AAX_CEffectParameters::SetChunk - error getting the number of parameters");
		return err;
	}
	
	for (controlIndex = 0; controlIndex < numControls; controlIndex++)	
	{	
		AAX_IParameter* parameter;
		parameter = mParameterManager.GetParameter(controlIndex);
		if (parameter != 0)
		{
			AAX_CParamID	parameterID = parameter->Identifier();
			if (mFilteredParameters.find(parameterID) == mFilteredParameters.end())
			{
				bool boolValue;
				int32_t intValue;
				float floatValue;
				double doubleValue;
				AAX_CString stringValue;
				
				if (parameter->GetValueAsFloat(&floatValue)) 
				{
					if (mChunkParser.FindDouble(parameterID, &doubleValue)) 
						parameter->SetValueWithFloat((float)doubleValue);
				} 
				else if (parameter->GetValueAsInt32(&intValue)) 
				{
					if (mChunkParser.FindInt32(parameterID, &intValue)) 
						parameter->SetValueWithInt32(intValue);
				}
				else if (parameter->GetValueAsBool(&boolValue))
				{
					if (mChunkParser.FindInt32(parameterID, &intValue)) 
						parameter->SetValueWithBool(intValue != 0);
				}
				else if (parameter->GetValueAsDouble(&doubleValue))
				{
					if (mChunkParser.FindDouble(parameterID, &doubleValue)) 
						parameter->SetValueWithDouble(doubleValue);
				}
				else if (parameter->GetValueAsString(&stringValue))
				{
					if (mChunkParser.FindString(parameterID, &stringValue))
						parameter->SetValueWithString(stringValue);
				}
			}
		}
	} 
	
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::CompareActiveChunk ( const AAX_SPlugInChunk * aChunkP, AAX_CBoolean * aIsEqualP )  const 
{
	if (aChunkP->fChunkID != CONTROLS_CHUNK_ID) 
	{
		// If we don't know what the chunk is then we don't want to be turning on the compare light unnecessarily.
		*aIsEqualP = true;
		return AAX_SUCCESS; 
	}

	// Now we assume they aren't equal until we make it through all the controls.
	*aIsEqualP = false;

	mChunkParser.LoadChunk(aChunkP);
	int32_t numControls = 0;
	AAX_Result err = AAX_SUCCESS;
	err = this->GetNumberOfParameters(&numControls);
	if (AAX_SUCCESS != err) {
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_Critical, "AAX_CEffectParameters::CompareActiveChunk - error getting the number of parameters");
		return err;
	}

	for (int32_t controlIndex = 0; controlIndex < numControls; controlIndex++) 
	{
		const AAX_IParameter* parameter = mParameterManager.GetParameter(controlIndex);
		AAX_ASSERT(NULL != parameter);
		if (!parameter)
			continue;
		
		AAX_CParamID parameterID = parameter->Identifier();

		if (mFilteredParameters.find(parameterID) == mFilteredParameters.end())
		{
			bool boolValue;
			int32_t intValue, chunkIntValue;
			float floatValue;
			double doubleValue, chunkDoubleValue;
			AAX_CString stringValue, chunkStringValue;
			
			if (parameter->GetValueAsFloat(&floatValue)) 
			{
				// It seems like float parameters stores their value in chunks as double. Do we really need this behavior?
				if ( !mChunkParser.FindDouble(parameterID, &chunkDoubleValue) ||
					floatValue != static_cast<float>(chunkDoubleValue) )
					return AAX_SUCCESS;
			} 
			else if (parameter->GetValueAsInt32(&intValue)) 
			{
				if ( !mChunkParser.FindInt32(parameterID, &chunkIntValue) ||
					intValue != chunkIntValue )
					return AAX_SUCCESS;
			}
			else if (parameter->GetValueAsBool(&boolValue))
			{
				if ( !mChunkParser.FindInt32(parameterID, &chunkIntValue) )
					return AAX_SUCCESS;

				if ( (chunkIntValue != 0) != boolValue )
					return AAX_SUCCESS;
			}
			else if (parameter->GetValueAsDouble(&doubleValue))
			{
				if ( !mChunkParser.FindDouble(parameterID, &chunkDoubleValue) ||
					(float)doubleValue != (float)chunkDoubleValue )
					return AAX_SUCCESS;
			}
			else if (parameter->GetValueAsString(&stringValue))
			{
				if ( !mChunkParser.FindString(parameterID, &chunkStringValue) ||
					stringValue != chunkStringValue )
					return AAX_SUCCESS;
			}
		}
	}

	//[8/3/2005, Bobby Lombardi, Impact: 8, Staley, x315]
	//After speaking with Chris T some more about the proposed fixes, Product Marketing 
	//would like to support the fix that bases the compare light activity on quantizing 
	//to the specific plug-in's control value units. For example, if a Gain control is 
	//in .1 increments of dB, the compare light activity should be triggered each change 
	//of a .1 dB, regardless of whether the control value skips over .1/tenth dB values.

	// 1/17/2011. This can be done by correct using the Precision parameter in 
	//   TaperDelegate and DisplayDelegate templates.

	*aIsEqualP = true;
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::GetNumberOfChanges ( int32_t * aValueP )  const 
{
	if(mNumPlugInChanges >= 0)
	{
		*aValueP = mNumPlugInChanges;
	}

	return (AAX_SUCCESS);
}


AAX_Result AAX_CEffectParameters::GetParameterType ( AAX_CParamID iParameterID, AAX_EParameterType * aControlType )  const 
{
	const AAX_IParameter* parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	*aControlType = parameter->GetType();
	return AAX_SUCCESS;
}


AAX_Result AAX_CEffectParameters::GetParameterOrientation( AAX_CParamID iParameterID, AAX_EParameterOrientation * aControlOrientation )  const 
{
	const AAX_IParameter*	parameter = mParameterManager.GetParameterByID( iParameterID );
	if (parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;

	*aControlOrientation = parameter->GetOrientation();
	return AAX_SUCCESS;
}
					
AAX_Result AAX_CEffectParameters::GetParameter(AAX_CParamID iParameterID, AAX_IParameter** parameter) 
{
	*parameter = mParameterManager.GetParameterByID( iParameterID );
	if (*parameter == 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;
	return AAX_SUCCESS;	
}

AAX_Result AAX_CEffectParameters::GetParameterIndex( AAX_CParamID iParameterID, int32_t * oControlIndex )  const 
{
	*oControlIndex = mParameterManager.GetParameterIndex( iParameterID );
	if (*oControlIndex < 0)
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	return AAX_SUCCESS;	
}

AAX_Result AAX_CEffectParameters::GetParameterIDFromIndex( int32_t iControlIndex, AAX_IString * oParameterID )  const 
{
	const AAX_IParameter * parameter = mParameterManager.GetParameter( iControlIndex );
	if ( parameter )
		*oParameterID = parameter->Identifier();
	else
	{
		*oParameterID = "";
		return AAX_ERROR_INVALID_PARAMETER_ID;
	}
	
	return AAX_SUCCESS;	
}

AAX_Result AAX_CEffectParameters::GetParameterValueInfo ( AAX_CParamID /*iParameterID*/, int32_t /*iSelector*/, int32_t* oValue) const
{
	// plugins should override this method if they wish to use
	// the parameter properties
	*oValue = 0;
	return AAX_ERROR_UNIMPLEMENTED;
}

////////////////////////////////////Internal Functions, no longer the interface.

																	
void	AAX_CEffectParameters::BuildChunkData() const
{
	mChunkParser.Clear();
	
	int32_t numControls = 0;
	if (AAX_SUCCESS != this->GetNumberOfParameters(&numControls)) {
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_Critical, "AAX_CEffectParameters::BuildChunkData - error getting the number of parameters");
		return;
	}

	for (int32_t controlIndex = 0; controlIndex < numControls; controlIndex++) 
	{
		const AAX_IParameter* parameter = mParameterManager.GetParameter(controlIndex);
		AAX_ASSERT(NULL != parameter);
		if (!parameter)
			continue;
		
		const char* parameterID = parameter->Identifier();
		
		if (mFilteredParameters.find(parameterID) == mFilteredParameters.end())
		{
			bool boolValue;
			int32_t intValue;
			float floatValue;
			double doubleValue;
			AAX_CString stringValue;
			
			if (parameter->GetValueAsFloat(&floatValue)) 
			{
				mChunkParser.AddDouble(parameterID, floatValue);
			} 
			else if (parameter->GetValueAsInt32(&intValue)) 
			{
				mChunkParser.AddInt32(parameterID, intValue);
			}
			else if (parameter->GetValueAsBool(&boolValue))
			{
				mChunkParser.AddInt32(parameterID, int32_t(boolValue));
			}
			else if (parameter->GetValueAsDouble(&doubleValue))
			{
				mChunkParser.AddDouble(parameterID, doubleValue);
			}
			else if (parameter->GetValueAsString(&stringValue))
			{
				mChunkParser.AddString(parameterID, stringValue);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------
void		AAX_CEffectParameters::FilterParameterIDOnSave( AAX_CParamID parameterID )
{
	if (parameterID != 0)
		mFilteredParameters.insert(parameterID);
}

//----------------------------------------------------------------------------------------------
//	METHOD: TimerWakeup
//----------------------------------------------------------------------------------------------
AAX_Result	AAX_CEffectParameters::TimerWakeup( )
{	
	return AAX_SUCCESS; //Default implementation doesn't do anything.  
}

//----------------------------------------------------------------------------------------------
//	METHOD: GetCurveData
//----------------------------------------------------------------------------------------------
AAX_Result	AAX_CEffectParameters::GetCurveData( /* AAX_ECurveType */ AAX_CTypeID /*iCurveType*/, const float * /*iValues*/, uint32_t /*iNumValues*/, float * /*oValues*/ ) const
{
	// Default implementation doesn't do anything.  Just returns unimplemented.  
	// Could clear these values, but that takes up unnecessary cycles and there isn't an obvious clear state for every curve type.
	return AAX_ERROR_UNIMPLEMENTED;
}

//----------------------------------------------------------------------------------------------
//	METHOD: GetCurveDataMeterIds
//----------------------------------------------------------------------------------------------
AAX_Result	AAX_CEffectParameters::GetCurveDataMeterIds( /* AAX_ECurveType */ AAX_CTypeID /*iCurveType*/, uint32_t* /*oXMeterId*/, uint32_t* /*oYMeterId*/ )  const
{
	// Default implementation doesn't do anything.  Just returns unimplemented.  
	return AAX_ERROR_UNIMPLEMENTED;
}

//----------------------------------------------------------------------------------------------
//	METHOD: GetCurveDataDisplayRange
//----------------------------------------------------------------------------------------------
AAX_Result	AAX_CEffectParameters::GetCurveDataDisplayRange(  /* AAX_ECurveType */ AAX_CTypeID /*iCurveType*/, float* /*oXMin*/, float* /*oXMax*/, float* /*oYMin*/, float* /*oYMax*/ ) const
{
	// Default implementation doesn't do anything.  Just returns unimplemented.  
	return AAX_ERROR_UNIMPLEMENTED;
}

//----------------------------------------------------------------------------------------------
//	METHOD: UpdatePageTable
//----------------------------------------------------------------------------------------------
AAX_Result AAX_CEffectParameters::UpdatePageTable(uint32_t inTableType, int32_t inTablePageSize, IACFUnknown* /*iHostUnknown*/, IACFUnknown* ioPageTableUnknown) const
{
	AAX_Result result = AAX_SUCCESS;
	AAX_VPageTable hostPageTable(ioPageTableUnknown);
	if (hostPageTable.IsSupported())
	{
		result = this->UpdatePageTable(inTableType, inTablePageSize, hostPageTable);
	}
	return result; // success result is fine here if the host did not provide valid IACFUnknowns - in that case we just assume that the host does not support this feature
}

//----------------------------------------------------------------------------------------------
//	METHOD: GetCustomData
//----------------------------------------------------------------------------------------------
AAX_Result	AAX_CEffectParameters::GetCustomData( AAX_CTypeID /*iDataBlockID*/, uint32_t /*inDataSize*/, void* /*oData*/, uint32_t* oDataWritten) const
{
	*oDataWritten = 0;
	return AAX_SUCCESS;
}


//----------------------------------------------------------------------------------------------
//	METHOD: SetCustomData
//----------------------------------------------------------------------------------------------
AAX_Result	AAX_CEffectParameters::SetCustomData( AAX_CTypeID /*iDataBlockID*/, uint32_t /*inDataSize*/, const void* /*iData*/ )
{
	return AAX_SUCCESS;
}

//----------------------------------------------------------------------------------------------
//	METHOD: RenderAudio_Hybrid
//----------------------------------------------------------------------------------------------
AAX_Result AAX_CEffectParameters::RenderAudio_Hybrid(AAX_SHybridRenderInfo* /*ioRenderInfo*/)
{
	return AAX_ERROR_UNIMPLEMENTED;
}

//----------------------------------------------------------------------------------------------
//	METHOD: AAX_UpdateMIDINodes
//----------------------------------------------------------------------------------------------
AAX_Result AAX_CEffectParameters::UpdateMIDINodes( AAX_CFieldIndex /*inFieldIndex*/, AAX_CMidiPacket& /*iPacket*/  )
{
	return AAX_SUCCESS;	
}

AAX_Result AAX_CEffectParameters::UpdateControlMIDINodes( AAX_CTypeID /*nodeID*/, AAX_CMidiPacket& /*iPacket*/ )
{
	return AAX_SUCCESS;	
}

AAX_ITransport* AAX_CEffectParameters::Transport()
{
	return mTransport;
}

const AAX_ITransport* AAX_CEffectParameters::Transport() const
{
	return mTransport;
}

bool AAX_CEffectParameters::IsParameterTouched ( AAX_CParamID inParameterID ) const
{
	AAX_CBoolean touched = false;
	if ( mAutomationDelegate )
	{
		if ( mAutomationDelegate->GetTouchState ( inParameterID, &touched ) != AAX_SUCCESS )
			touched = false;
	}
	
	return (touched != false);
}

bool AAX_CEffectParameters::IsParameterLinkReady ( AAX_CParamID inParameterID, AAX_EUpdateSource inSource ) const
{
	AAX_CBoolean linkReady = false;
	if ( inSource != AAX_eUpdateSource_Parameter &&
		 inSource != AAX_eUpdateSource_Chunk &&
		 inSource != AAX_eUpdateSource_Delay )
	{
		if ( mAutomationDelegate )
		{
			if ( mAutomationDelegate->GetTouchState ( inParameterID, &linkReady ) != AAX_SUCCESS )
				linkReady = false;
		}
	}
	
	return (linkReady != false);
}


AAX_Result AAX_CEffectParameters::SetTaperDelegate ( AAX_CParamID iParameterID, AAX_ITaperDelegateBase & inTaperDelegate, bool inPreserveValue )
{
	AAX_IParameter * parameter = mParameterManager.GetParameterByID ( iParameterID );
	if ( parameter == 0 )
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	parameter->SetTaperDelegate ( inTaperDelegate, inPreserveValue );
	
	if ( ! inPreserveValue )
		mPacketDispatcher.SetDirty ( iParameterID );

	return AAX_SUCCESS;
}

AAX_Result AAX_CEffectParameters::SetDisplayDelegate ( AAX_CParamID iParameterID, AAX_IDisplayDelegateBase & inDisplayDelegate )
{
	AAX_IParameter * parameter = mParameterManager.GetParameterByID ( iParameterID );
	if ( parameter == 0 )
		return AAX_ERROR_INVALID_PARAMETER_ID;
	
	parameter->SetDisplayDelegate ( inDisplayDelegate );
	return AAX_SUCCESS;
}
