/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_RESULT_JUCEHEADER__
#define __JUCE_RESULT_JUCEHEADER__

#include "../text/juce_String.h"


//==============================================================================
/**
    Represents the 'success' or 'failure' of an operation, and holds an associated
    error message to describe the error when there's a failure.

    E.g.
    @code
    Result myOperation()
    {
        if (doSomeKindOfFoobar())
            return Result::ok();
        else
            return Result::fail ("foobar didn't work!");
    }

    const Result result (myOperation());

    if (result.wasOk())
    {
        ...it's all good...
    }
    else
    {
        warnUserAboutFailure ("The foobar operation failed! Error message was: "
                                + result.getErrorMessage());
    }
    @endcode
*/
class JUCE_API  Result
{
public:
    //==============================================================================
    /** Creates and returns a 'successful' result. */
    static Result ok() noexcept;

    /** Creates a 'failure' result.
        If you pass a blank error message in here, a default "Unknown Error" message
        will be used instead.
    */
    static Result fail (const String& errorMessage) noexcept;

    //==============================================================================
    /** Returns true if this result indicates a success. */
    bool wasOk() const noexcept;

    /** Returns true if this result indicates a failure.
        You can use getErrorMessage() to retrieve the error message associated
        with the failure.
    */
    bool failed() const noexcept;

    /** Returns true if this result indicates a success.
        This is equivalent to calling wasOk().
    */
    operator bool() const noexcept;

    /** Returns true if this result indicates a failure.
        This is equivalent to calling failed().
    */
    bool operator!() const noexcept;

    /** Returns the error message that was set when this result was created.
        For a successful result, this will be an empty string;
    */
    const String& getErrorMessage() const noexcept;

    //==============================================================================
    Result (const Result& other);
    Result& operator= (const Result& other);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    Result (Result&& other) noexcept;
    Result& operator= (Result&& other) noexcept;
   #endif

    bool operator== (const Result& other) const noexcept;
    bool operator!= (const Result& other) const noexcept;

private:
    String errorMessage;

    explicit Result (const String&) noexcept;

    // These casts are private to prevent people trying to use the Result object in numeric contexts
    operator int() const;
    operator void*() const;
};


#endif   // __JUCE_RESULT_JUCEHEADER__
