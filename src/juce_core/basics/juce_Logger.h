/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_LOGGER_JUCEHEADER__
#define __JUCE_LOGGER_JUCEHEADER__

#include "../text/juce_String.h"


//==============================================================================
/**
    Acts as an application-wide logging class.

    A subclass of Logger can be created and passed into the Logger::setCurrentLogger
    method and this will then be used by all calls to writeToLog.

    The logger class also contains methods for writing messages to the debugger's
    output stream.

    @see FileLogger
*/
class JUCE_API  Logger
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~Logger();

    //==============================================================================
    /** Sets the current logging class to use.

        Note that the object passed in won't be deleted when no longer needed.
        A null pointer can be passed-in to disable any logging.

        If deleteOldLogger is set to true, the existing logger will be
        deleted (if there is one).
    */
    static void JUCE_CALLTYPE setCurrentLogger (Logger* const newLogger,
                                                const bool deleteOldLogger = false);

    /** Writes a string to the current logger.

        This will pass the string to the logger's logMessage() method if a logger
        has been set.

        @see logMessage
    */
    static void JUCE_CALLTYPE writeToLog (const String& message);


    //==============================================================================
    /** Writes a message to the standard error stream.

        This can be called directly, or by using the DBG() macro in
        juce_PlatformDefs.h (which will avoid calling the method in non-debug builds).
    */
    static void JUCE_CALLTYPE outputDebugString (const String& text) throw();

    /** Writes a message to the standard error stream.

        This can be called directly, or by using the DBG_PRINTF() macro in
        juce_PlatformDefs.h (which will avoid calling the method in non-debug builds).
    */
    static void JUCE_CALLTYPE outputDebugPrintf (const tchar* format, ...) throw();


protected:
    //==============================================================================
    Logger();

    /** This is overloaded by subclasses to implement custom logging behaviour.

        @see setCurrentLogger
    */
    virtual void logMessage (const String& message) = 0;
};


#endif   // __JUCE_LOGGER_JUCEHEADER__
