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

#ifndef __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__
#define __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__

#include "../utilities/juce_PropertiesFile.h"
#include "../utilities/juce_DeletedAtShutdown.h"
#include "../core/juce_Singleton.h"


//==============================================================================
/**
    Manages a collection of properties.

    This is a slightly higher-level wrapper for PropertiesFile, which can be used
    as a singleton.

    It holds two different PropertiesFile objects internally, one for user-specific
    settings (stored in your user directory), and one for settings that are common to
    all users (stored in a folder accessible to all users).

    The class manages the creation of these files on-demand, allowing access via the
    getUserSettings() and getCommonSettings() methods. It also has a few handy
    methods like testWriteAccess() to check that the files can be saved.

    If you're using one of these as a singleton, then your app's start-up code should
    first of all call setStorageParameters() to tell it the parameters to use to create
    the properties files.

    @see PropertiesFile
*/
class JUCE_API  ApplicationProperties   : public DeletedAtShutdown
{
public:
    //==============================================================================
    /**
        Creates an ApplicationProperties object.

        Before using it, you must call setStorageParameters() to give it the info
        it needs to create the property files.
    */
    ApplicationProperties() throw();

    /** Destructor.
    */
    ~ApplicationProperties();

    //==============================================================================
    juce_DeclareSingleton (ApplicationProperties, false)

    //==============================================================================
    /** Gives the object the information it needs to create the appropriate properties files.

        See the comments for PropertiesFile::createDefaultAppPropertiesFile() for more
        info about how these parameters are used.
    */
    void setStorageParameters (const String& applicationName,
                               const String& fileNameSuffix,
                               const String& folderName,
                               const int millisecondsBeforeSaving,
                               const int propertiesFileOptions) throw();

    /** Tests whether the files can be successfully written to, and can show
        an error message if not.

        Returns true if none of the tests fail.

        @param testUserSettings     if true, the user settings file will be tested
        @param testCommonSettings   if true, the common settings file will be tested
        @param showWarningDialogOnFailure   if true, the method will show a helpful error
                                    message box if either of the tests fail
    */
    bool testWriteAccess (const bool testUserSettings,
                          const bool testCommonSettings,
                          const bool showWarningDialogOnFailure);

    //==============================================================================
    /** Returns the user settings file.

        The first time this is called, it will create and load the properties file.

        Note that when you search the user PropertiesFile for a value that it doesn't contain,
        the common settings are used as a second-chance place to look. This is done via the
        PropertySet::setFallbackPropertySet() method - by default the common settings are set
        to the fallback for the user settings.

        @see getCommonSettings
    */
    PropertiesFile* getUserSettings() throw();

    /** Returns the common settings file.

        The first time this is called, it will create and load the properties file.

        @param returnUserPropsIfReadOnly  if this is true, and the common properties file is
                            read-only (e.g. because the user doesn't have permission to write
                            to shared files), then this will return the user settings instead,
                            (like getUserSettings() would do). This is handy if you'd like to
                            write a value to the common settings, but if that's no possible,
                            then you'd rather write to the user settings than none at all.
                            If returnUserPropsIfReadOnly is false, this method will always return
                            the common settings, even if any changes to them can't be saved.
        @see getUserSettings
    */
    PropertiesFile* getCommonSettings (const bool returnUserPropsIfReadOnly) throw();

    //==============================================================================
    /** Saves both files if they need to be saved.

        @see PropertiesFile::saveIfNeeded
    */
    bool saveIfNeeded();

    /** Flushes and closes both files if they are open.

        This flushes any pending changes to disk with PropertiesFile::saveIfNeeded()
        and closes both files. They will then be re-opened the next time getUserSettings()
        or getCommonSettings() is called.
    */
    void closeFiles();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    ScopedPointer <PropertiesFile> userProps, commonProps;

    String appName, fileSuffix, folderName;
    int msBeforeSaving, options;
    int commonSettingsAreReadOnly;

    ApplicationProperties (const ApplicationProperties&);
    const ApplicationProperties& operator= (const ApplicationProperties&);

    void openFiles() throw();
};


#endif   // __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__
