/***********************************************************************

	This file is part of the Avid AAX SDK.

	The AAX SDK is subject to commercial or open-source licensing.

	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
	Agreement and Avid Privacy Policy.

	AAX SDK License: https://developer.avid.com/aax
	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement

	Or: You may also use this code under the terms of the GPL v3 (see
	www.gnu.org/licenses).

	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
	DISCLAIMED.

	Copyright (c) 2004, 2024 Avid Technology, Inc. All rights reserved.

************************************************************************/


#ifndef __acfextras_h__
#define __acfextras_h__

#include "acfbasetypes.h"
#include "acfresult.h"

/*! 
	\file acfextras.h
	\brief acfUID comparison operators. Utility and error checking Macros.
*/

inline bool operator == (const acfUID& lhs, const acfUID& rhs)
{
    return ((lhs.Data1 == rhs.Data1) &&
            (lhs.Data2 == rhs.Data2) &&
            (lhs.Data3 == rhs.Data3) &&
			(lhs.Data4[0] == rhs.Data4[0]) &&
			(lhs.Data4[1] == rhs.Data4[1]) &&
			(lhs.Data4[2] == rhs.Data4[2]) &&
			(lhs.Data4[3] == rhs.Data4[3]) &&
			(lhs.Data4[4] == rhs.Data4[4]) &&
			(lhs.Data4[5] == rhs.Data4[5]) &&
			(lhs.Data4[6] == rhs.Data4[6]) &&
			(lhs.Data4[7] == rhs.Data4[7]));
}

inline bool operator != (const acfUID& lhs,
                         const acfUID& rhs)
{
    return !(lhs == rhs);
}


// Macro to enforce relationship between the interface id and
// its associated interface type arguments in QueryInterface.
// In the following example:
// IXInterfaceType * pInterface = 0;
// pUnk->QueryInterface(IID_IXInterfaceType, (void **)&pInterface);
//
// The QueryInterface call can be rewritten to be compiler
// safe as:
// pUnk->QueryInterface(IID_PPV_ARG(IXInterfaceType, &pInterface));
// Macro from "Essential COM", by Don Box.
#define IID_PPV_ARG(Type, Expr) IID_##Type, \
	reinterpret_cast<void **>(static_cast<Type **>(Expr))

// Namespace version of IID_PPV_ARG.
#define NS_IID_PPV_ARG(NameSpace, Type, Expr) \
	NameSpace::IID_##Type, \
	reinterpret_cast<void **>(static_cast<NameSpace::Type **>(Expr))

// NOTE: We may not want the less than operator to be inline...
inline bool operator < (const acfUID& lhs, const acfUID& rhs)
{
    if (lhs.Data1 < rhs.Data1)
        return true;
    else if (lhs.Data1 == rhs.Data1) 
    {
        if (lhs.Data2 < rhs.Data2)
            return true;
        else if (lhs.Data2 == rhs.Data2)
        {
            if (lhs.Data3 < rhs.Data3)
                return true;
            else if (lhs.Data3 == rhs.Data3)
            {
                if (lhs.Data4[0] < rhs.Data4[0])
                    return true;
                else if (lhs.Data4[0] == rhs.Data4[0])
                {
					if (lhs.Data4[1] < rhs.Data4[1])
						return true;
					else if (lhs.Data4[1] == rhs.Data4[1])
					{
						if (lhs.Data4[2] < rhs.Data4[2])
							return true;
						else if (lhs.Data4[2] == rhs.Data4[2])
						{
							if (lhs.Data4[3] < rhs.Data4[3])
								return true;
							else if (lhs.Data4[3] == rhs.Data4[3])
							{
								if (lhs.Data4[4] < rhs.Data4[4])
									return true;
								else if (lhs.Data4[4] == rhs.Data4[4])
								{
									if (lhs.Data4[5] < rhs.Data4[5])
										return true;
									else if (lhs.Data4[5] == rhs.Data4[5])
									{
										if (lhs.Data4[6] < rhs.Data4[6])
											return true;
										else if (lhs.Data4[6] == rhs.Data4[6])
										{
											if (lhs.Data4[7] < rhs.Data4[7])
												return true;
										}
									}
								}
							}
						}
					}
                }
            }
        }
    }

    return false;
}


// Preliminary exception handlers for ACF methods.

/*!
    \b BEGIN_ACF_METHOD
    \brief Opens a try block for ACF result codes
	\remarks BEGIN_ACF_METHOD must be followed by a  
	corresponding END_ACF_METHOD. BEGIN_ACF_METHOD is the try
	end of the exception block. These MACROS define a
	try and catch block for the handling ACFRESULT exceptions.
 */
#define BEGIN_ACF_METHOD\
    ACFRESULT avx2methodresult = ACF_OK;\
    try\
    {
/*!
    \b END_ACF_METHOD
    \brief closes a try block for ACF result codes
	\remarks END_ACF_METHOD must be preceeded by a  
	corresponding BEGIN_ACF_METHOD. END_ACF_METHOD is the catch
	end of the exception block.These MACROS define a
	try and catch block for the handling ACFRESULT exceptions.
 */
#define END_ACF_METHOD\
    }\
    catch (ACFRESULT & acfresultexception)\
    {\
        avx2methodresult = acfresultexception;\
    }\
    catch (...)\
    {\
        avx2methodresult = ACF_E_UNEXPECTED;\
    }\
    return avx2methodresult;
/*!
	\b acfcheck( result )
	\brief acfcheck is a convenience function used for validating the
	success of returning ACFRESULT codes.
	\remarks if the return code is a comprised of a
	ACFFAILED code this function will throw an ACFRESULT exception.
	For this reason, use of the function should be wrapped in a 
	try -- catch block that can handle the exception, such as the 
	BEGIN_ACF_METHOD and END_ACF_METHOD macros. 

*/
inline void acfcheck(ACFRESULT result)
{
    if (ACFFAILED(result))
		throw ACFRESULT(result);
}

#endif // __acfextras_h__
