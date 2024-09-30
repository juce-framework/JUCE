/*================================================================================================*/
/*
 *	Copyright 2016-2017, 2023-2024 Avid Technology, Inc.
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
 */

/**  
 *	@file   AAX_Exception.h
 *	
 *	@brief	%AAX SDK exception classes and utilities
 */
/*================================================================================================*/


#ifndef AAXLibrary_AAX_Exception_h
#define AAXLibrary_AAX_Exception_h

#include "AAX_Assert.h"
#include "AAX_StringUtilities.h"
#include "AAX.h"

#include <exception>
#include <string>
#include <set>


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#pragma mark AAX::Exception
#endif
///////////////////////////////////////////////////////////////

namespace AAX
{
	namespace Exception {
		class Any;
	}
	
	/** Generic conversion of a string-like object to a std::string
	 */
	inline std::string AsString(const char* inStr);
	inline const std::string& AsString(const std::string& inStr); ///< \copydoc AAX::AsString(const char*)
	inline const std::string& AsString(const Exception::Any& inStr); ///< \copydoc AAX::AsString(const char*)
	
	
	/** \namespace AAX::Exception
	 
	 \brief %AAX exception classes
	 
	 \details
	 All %AAX exception classes inherit from \ref AAX::Exception::Any
	 */
	namespace Exception
	{
		/** Base class for %AAX exceptions
		 
		 This class is defined within the %AAX Library and is always handled within the %AAX plug-in.
		 Objects of this class are never passed between the plug-in and the %AAX host.
		 
		 The definition of this class may change between versions of the %AAX SDK. This class does not
		 include any form of version safety for cross-version compatibility.
		 
		 \warning Do not use multiple inheritance in any sub-classes within the
		 \ref AAX::Exception::Any inheritance tree
		 
		 \warning Never pass exceptions across the library boundary to the %AAX host
		 */
		class Any
		{
		public:
			virtual ~Any() {}
			
			/** Explicit conversion from a string-like object
			 */
			template <class C>
			explicit Any(const C& inWhat)
			: mDesc(AAX::AsString(inWhat))
			, mFunction()
			, mLine()
			, mWhat(AAX::Exception::Any::CreateWhat(mDesc, mFunction, mLine))
			{
			}
			
			/** Explicit conversion from a string-like object with function name and line number
			 */
			template <class C1, class C2, class C3>
			explicit Any(const C1& inWhat, const C2& inFunction, const C3& inLine)
			: mDesc(AAX::AsString(inWhat))
			, mFunction(AAX::AsString(inFunction))
			, mLine(AAX::AsString(inLine))
			, mWhat(AAX::Exception::Any::CreateWhat(mDesc, mFunction, mLine))
			{
			}

			// copy constructor
			Any(const Any& inOther)
			: mDesc(inOther.mDesc)
			, mFunction(inOther.mFunction)
			, mLine(inOther.mLine)
			, mWhat(inOther.mWhat)
			{
			}
			
			// assignment operator
			Any& operator=(const Any& inOther)
			{
				mDesc = inOther.mDesc;
				mFunction = inOther.mFunction;
				mLine = inOther.mLine;
				mWhat = inOther.mWhat;
				return *this;
			}
			
			AAX_DEFAULT_MOVE_CTOR(Any);
			AAX_DEFAULT_MOVE_OPER(Any);
			
		public: // AAX::Exception::Any
			
#ifndef AAX_CPP11_SUPPORT
			// implicit conversion to std::string (mostly for AsString())
			operator const std::string&(void) const { return mWhat; }
#endif
			
			const std::string& What() const { return mWhat; }
			const std::string& Desc() const { return mDesc; }
			const std::string& Function() const { return mFunction; }
			const std::string& Line() const { return mLine; }
			
		private:
			static std::string CreateWhat(const std::string& inDesc, const std::string& inFunc, const std::string& inLine)
			{
				std::string whatStr(inDesc);
				if (false == inFunc.empty()) { whatStr += (" func:" + inFunc); }
				if (false == inLine.empty()) { whatStr += (" line:" + inLine); }
				return whatStr;
			}
			
		private:
			std::string mDesc;
			std::string mFunction;
			std::string mLine;
			std::string mWhat;
		};
		
		/** Exception class for \ref AAX_EError results
		 */
		class ResultError : public Any
		{
		public:
			explicit ResultError(AAX_Result inWhatResult)
			: Any(ResultError::FormatResult(inWhatResult))
			, mResult(inWhatResult)
			{
			}
			
			template <class C>
			explicit ResultError(AAX_Result inWhatResult, const C& inFunction)
			: Any(ResultError::FormatResult(inWhatResult), inFunction, (const char*)NULL)
			, mResult(inWhatResult)
			{
			}
			
			template <class C1, class C2>
			explicit ResultError(AAX_Result inWhatResult, const C1& inFunction, const C2& inLine)
			: Any(ResultError::FormatResult(inWhatResult), inFunction, inLine)
			, mResult(inWhatResult)
			{
			}

			// copy constructor
			ResultError(const ResultError& inOther)
			: Any(inOther)
			, mResult(inOther.mResult)
			{
			}
			
			static std::string FormatResult(AAX_Result inResult)
			{
				return std::string(AAX::AsStringResult(inResult) + " (" + AAX::AsStringInt32((int32_t)inResult) + ")");
			}
			
			AAX_Result Result() const { return mResult; }
			
		private:
			AAX_Result mResult;
		};
	}

	std::string AsString(const char* inStr)
	{
		return inStr ? std::string(inStr) : std::string();
	}

	const std::string& AsString(const std::string& inStr)
	{
		return inStr;
	}

	const std::string& AsString(const Exception::Any& inStr)
	{
		return inStr.What();
	}
}


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

/** Error checker convenience class for \ref AAX_Result
 
 Implicitly convertable to an \ref AAX_Result.
 
 Provides an overloaded \c operator=() which will throw an \ref AAX::Exception::ResultError if assigned
 a non-success result.
 
 \warning Never use this class outside of an exception catch scope
 
 If the host supports \ref AAX_TRACE tracing, a log is emitted when the exception is thrown. A stacktrace
 is added if the host's trace priority filter level is set to \ref kAAX_Trace_Priority_Lowest
 
 When an error is encountered, \ref AAX_CheckedResult throws an \ref AAX_CheckedResult::Exception
 exception and clears its internal result value.
 
 \code
 #include "AAX_Exception.h"
 AAX_Result SomeCheckedMethod()
 {
   AAX_Result result = AAX_SUCCESS;
   try {
     AAX_CheckedResult cr;
     cr = ResultFunc1();
     cr = ResultFunc2();
   }
   catch (const AAX_CheckedResult::Exception& ex)
   {
     // handle exception; do not rethrow
     result = ex.Result();
   }
   catch (...)
   {
     result = AAX_ERROR_UNKNOWN_EXCEPTION;
   }
 
   return result;
 }
 \endcode
 
 \note The AAX Library method which calls \c GetEffectDescriptions() on the plug-in includes an
 appropriate exception handler, so \ref AAX_CheckedResult objects may be used within a plug-in's
 describe code without additional catch scopes.
 
 \code
 #include "AAX_Exception.h"
 AAX_Result GetEffectDescriptions( AAX_ICollection * outCollection )
 {
   AAX_CheckedResult cr;
   cr = MyDescriptionSubroutine1();
   cr = outCollection->AddEffect(...);
   // etc.
   return cr;
 }
 \endcode
 
 It is assumed that the exception handler will resolve any error state and that the
 \ref AAX_CheckedResult may therefore continue to be used from a clean state following the
 exception catch block.
 
 If the previous error value is required then it can be retrieved using
 \ref AAX_CheckedResult::LastError().
 
 \code
 // in this example, the exception is handled and
 // success is returned from MyFunc1()
 AAX_Result MyFunc1()
 {
   AAX_CheckedResult cr;
 
   try {
     cr = MethodThatReturnsError();
   } catch (const AAX::Exception::ResultError& ex) {
     // exception is fully handled here
   }
   
   // cr now holds a success value
   return cr;
 }
 
 // in this example, MyFunc2() returns the first
 // non-successful value which was encountered
 AAX_Result MyFunc2()
 {
   AAX_CheckedResult cr;
 
   try {
     AAX_SWALLOW(cr = MethodThatMayReturnError1());
     AAX_SWALLOW(cr = MethodThatMayReturnError2());
     cr = MethodThatMayReturnError3();
   } catch (const AAX::Exception::ResultError& ex) {
     // exception might not be fully handled
   }
   
   // pass the last error on to the caller
   return cr.LastError();
 }
 \endcode
 
 It is possible to add one or more accepted non-success values to an \ref AAX_CheckedResult
 so that these values will not trigger exceptions:
 
 \code
 AAX_CheckedResult cr;
 try {
   cr.AddAcceptedResult(AcceptableErrCode);
   cr = MethodThatReturnsAcceptedError();
   cr = MethodThatReturnsAnotherError();
 } catch (const AAX::Exception::ResultError& ex) {
   // handle the exception
 }
 \endcode
 */
class AAX_CheckedResult
{
public:
	typedef AAX::Exception::ResultError Exception;
	
	/* non-virtual destructor */ ~AAX_CheckedResult() {}
	
	/// \brief Construct an \ref AAX_CheckedResult in a success state
	AAX_CheckedResult()
	: mCurResult(AAX_SUCCESS)
	, mLastError(AAX_SUCCESS)
	, mAcceptedResults()
	{
		Initialize();
	}
	
	/// \brief Implicit conversion constructor from \ref AAX_Result
	/// \details Implicit conversion is OK in order to support AAX_CheckedResult cr = SomeFunc()
	AAX_CheckedResult(AAX_Result inResult)
	: mCurResult(inResult)
	, mLastError(AAX_SUCCESS)
	, mAcceptedResults()
	{
		Initialize();
		Check();
	}
	
	/** \brief Add an expected result which will not result in a throw
	 
	 It is acceptable for some methods to return certain non-success values such as
	 \ref AAX_RESULT_PACKET_STREAM_NOT_EMPTY or \ref AAX_RESULT_NEW_PACKET_POSTED
	 */
	void AddAcceptedResult(AAX_Result inResult)
	{
		mAcceptedResults.insert(inResult);
	}
	
	void ResetAcceptedResults()
	{
		mAcceptedResults.clear();
		mAcceptedResults.insert(AAX_SUCCESS);
	}
	
	/// \brief Assignment to \ref AAX_Result
	AAX_CheckedResult& operator=(AAX_Result inResult)
	{
		mCurResult = inResult;
		Check();
		return *this;
	}
	
	/// \brief bitwise-or assignment to \ref AAX_Result
	/// \details Sometimes used in legacy code to aggregate results into a single AAX_Result value
	AAX_CheckedResult& operator|=(AAX_Result inResult)
	{
		return this->operator=(inResult);
	}
	
	/// \brief Conversion to \ref AAX_Result
	operator AAX_Result() const
	{
		return mCurResult;
	}
	
	/// \brief Clears the current result state
	/// \details Does not affect the set of accepted results
	void Clear()
	{
		mCurResult = AAX_SUCCESS;
		mLastError = AAX_SUCCESS;
	}
	
	/// \brief Get the last non-success result which was stored in this object, or AAX_SUCCESS
	/// if no non-success result was ever stored in this object
	AAX_Result LastError() const
	{
		return mLastError;
	}
	
private:
	void Initialize()
	{
		ResetAcceptedResults();
	}
	
	void Check()
	{
		const AAX_Result err = mCurResult;
		if (0 == mAcceptedResults.count(err))
		{
			AAX_CheckedResult::Exception ex(err);
			
			// error state is now captured in ex
			mCurResult = AAX_SUCCESS;
			mLastError = err;
			
			AAX_TRACE_RELEASE(kAAX_Trace_Priority_Normal, "AAX_CheckedResult - throwing %s", ex.What().c_str());
			AAX_STACKTRACE(kAAX_Trace_Priority_Lowest, ""); // stacktrace is only printed for debug plug-in builds
			throw ex;
		}
	}
	
private:
	AAX_Result mCurResult;
	AAX_Result mLastError;
	std::set<AAX_Result> mAcceptedResults;
};


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#pragma mark AAX exception macros
#endif
///////////////////////////////////////////////////////////////

/*!
 \def AAX_SWALLOW(X)
 
 \brief Executes \a X in a try/catch block that catches \ref AAX_CheckedResult exceptions
 
 \details
 Catches exceptions thrown from \ref AAX_CheckedResult only - other exceptions require an explicit catch.
 
 \code
 AAX_CheckedResult cr;
 cr = NecessaryFunc1();
 AAX_SWALLOW(cr = FailableFunc());
 cr = NecessaryFunc2();
 \endcode
 */
#define AAX_SWALLOW(...) \
  try { if(true) { ( __VA_ARGS__ ); } } \
  catch (const AAX_CheckedResult::Exception& AAX_PREPROCESSOR_CONCAT(ex,__LINE__)) { \
    AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "%s line %d (%s) exception caught: %s (swallowed)", __FILE__, __LINE__, __FUNCTION__, AAX_PREPROCESSOR_CONCAT(ex,__LINE__).What().c_str()); \
  } do {} while (false)

/*!
 \def AAX_SWALLOW_MULT(X)
 
 \brief Executes \a X in a try/catch block that catches \ref AAX_CheckedResult exceptions
 
 \details
 Version of \ref AAX_SWALLOW for multi-line input.
 
 Catches exceptions thrown from \ref AAX_CheckedResult only - other exceptions require an
 explicit catch.
 
 \code
 AAX_CheckedResult cr;
 cr = NecessaryFunc();
 AAX_SWALLOW_MULT(
   cr = FailableFunc1();
   cr = FailableFunc2(); // may not execute
   cr = FailableFunc3(); // may not execute
 );
 cr = NecessaryFunc2();
 \endcode
 */
#define AAX_SWALLOW_MULT(...) \
try { if(true) { __VA_ARGS__ } } \
catch (const AAX_CheckedResult::Exception& AAX_PREPROCESSOR_CONCAT(ex,__LINE__)) { \
AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "%s line %d (%s) exception caught: %s (swallowed)", __FILE__, __LINE__, __FUNCTION__, AAX_PREPROCESSOR_CONCAT(ex,__LINE__).What().c_str()); \
} do {} while (false)

/*!
 \def AAX_CAPTURE(X,Y)
 
 \brief Executes \a Y in a try/catch block that catches
 \ref AAX::Exception::ResultError exceptions and captures the result
 
 \details
 Catches exceptions thrown from \ref AAX_CheckedResult and other
 \ref AAX::Exception::ResultError exceptions.
 
 \a X must be an \ref AAX_Result
 
 \code
 AAX_Result result = AAX_SUCCESS;
 AAX_CAPTURE(result, ResultErrorThrowingFunc());
 // result now holds the error code thrown by ThrowingFunc()
 
 AAX_CheckedResult cr;
 AAX_CAPTURE(result, cr = FailableFunc());
 \endcode
 */
#define AAX_CAPTURE(X, ...) \
try { if(true) { ( __VA_ARGS__ ); } } \
catch (const AAX::Exception::ResultError& AAX_PREPROCESSOR_CONCAT(ex,__LINE__)) { \
AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "%s line %d (%s) exception caught: %s (captured)", __FILE__, __LINE__, __FUNCTION__, AAX_PREPROCESSOR_CONCAT(ex,__LINE__).What().c_str()); \
(X) = AAX_PREPROCESSOR_CONCAT(ex,__LINE__).Result(); \
} do {} while (false)

/*!
 \def AAX_CAPTURE_MULT(X,Y)
 
 \brief Executes \a Y in a try/catch block that catches
 \ref AAX::Exception::ResultError exceptions and captures the result
 
 \details
 Version of \ref AAX_CAPTURE for multi-line input.
 
 Catches exceptions thrown from \ref AAX_CheckedResult and other
 \ref AAX::Exception::ResultError exceptions.
 
 \a X must be an \ref AAX_Result or an implicitly convertable type
 
 \code
 AAX_Result result = AAX_SUCCESS;
 AAX_CAPTURE_MULT(result,
   MaybeThrowingFunc1();
   MaybeThrowingFunc2();
   
   // can use AAX_CheckedResult within AAX_CAPTURE_MULT
   AAX_CheckedResult cr;
   cr = FailableFunc1();
   cr = FailableFunc2();
   cr = FailableFunc3();
 );
 
 // result now holds the value of the last thrown error
 return result;
 \endcode
 */
#define AAX_CAPTURE_MULT(X, ...) \
try { if(true) { __VA_ARGS__ } } \
catch (const AAX_CheckedResult::Exception& AAX_PREPROCESSOR_CONCAT(ex,__LINE__)) { \
AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "%s line %d (%s) exception caught: %s (captured)", __FILE__, __LINE__, __FUNCTION__, AAX_PREPROCESSOR_CONCAT(ex,__LINE__).What().c_str()); \
(X) = AAX_PREPROCESSOR_CONCAT(ex,__LINE__).Result(); \
} do {} while (false)


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

/** RAII failure count convenience class for use with \ref AAX_CAPTURE() or
 \ref AAX_CAPTURE_MULT()
 
 Pass this object as the first argument in a series of \ref AAX_CAPTURE() calls to
 count the number of failures that occur and to re-throw the last error if zero of
 the attempted calls succeed.
 
 \code
 // example A: throw if all operations fail
 AAX_AggregateResult agg;
 AAX_CAPTURE( agg, RegisterThingA(); );
 AAX_CAPTURE( agg, RegisterThingB(); );
 AAX_CAPTURE( agg, RegisterThingC(); );
 \endcode
 
 In this example, when <TT>agg</TT> goes out of scope it checks whether any of A,
 B, or C succeeded. If none succeeded then the last error that was encountered is
 raised via an \ref AAX_CheckedResult::Exception. If at least one of the calls
 succeeded then any failures are swallowed and execution continues as normal.
 This approach can be useful in cases where you want to run every operation in a
 group and you only want a failure to be returned if all of the operations failed.
 
 \code
 // example B: throw if any operation fails
 AAX_AggregateResult agg;
 AAX_CAPTURE( agg, ImportantOperationW(); );
 AAX_CAPTURE( agg, ImportantOperationX(); );
 AAX_CAPTURE( agg, ImportantOperationY(); );
 AAX_CheckedResult err = agg;
 \endcode
 
 In this example, the last error encountered by <TT>agg</TT> is converted to an
 \ref AAX_CheckedResult. This will result in an \ref AAX_CheckedResult::Exception
 even if at least one of the attempted operations succeeded. This approach can be
 useful in cases where you want all operations in a group to be executed before
 an error is raised for any failure within the group.
 */
class AAX_AggregateResult
{
public:
	AAX_AggregateResult() = default;
	
	~AAX_AggregateResult()
	{
		if (0 == mNumSucceeded && 0 < mNumFailed) {
			try {
				// do normal logging
				this->Check();
			}
			catch(...)
			{
				// can't throw from a destructor
			}
		}
	}
	
	/// Overloaded <TT>operator=()</TT> for conversion from \ref AAX_Result
	AAX_AggregateResult& operator=(AAX_Result inResult)
	{
		if (AAX_SUCCESS == inResult)
		{
			++mNumSucceeded;
		}
		else
		{
			mLastFailure = inResult;
			++mNumFailed;
		}
		
		return *this;
	}

	/// Implicit conversion to AAX_Result clears the state
	operator AAX_Result()
	{
		AAX_Result const err = this->LastFailure();
		this->Clear();
		return err;
	}
	
	void Check() const { AAX_CheckedResult tempErr(mLastFailure); }
	void Clear() {
		mLastFailure = AAX_SUCCESS;
		mNumFailed = 0;
		mNumSucceeded = 0;
	}

	AAX_Result LastFailure() const { return mLastFailure; }
	int NumFailed() const { return mNumFailed; }
	int NumSucceeded() const { return mNumSucceeded; }
	int NumAttempted() const { return mNumFailed+mNumSucceeded; }
	
private:
	AAX_Result mLastFailure{AAX_SUCCESS};
	int mNumFailed{0};
	int mNumSucceeded{0};
};

#endif
