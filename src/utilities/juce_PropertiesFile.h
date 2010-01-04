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

#ifndef __JUCE_PROPERTIESFILE_JUCEHEADER__
#define __JUCE_PROPERTIESFILE_JUCEHEADER__

#include "../io/files/juce_File.h"
#include "../containers/juce_PropertySet.h"
#include "../events/juce_Timer.h"
#include "../events/juce_ChangeBroadcaster.h"


//==============================================================================
/** Wrapper on a file that stores a list of key/value data pairs.

    Useful for storing application settings, etc. See the PropertySet class for
    the interfaces that read and write values.

    Not designed for very large amounts of data, as it keeps all the values in
    memory and writes them out to disk lazily when they are changed.

    Because this class derives from ChangeBroadcaster, ChangeListeners can be registered
    with it, and these will be signalled when a value changes.

    @see PropertySet
*/
class JUCE_API  PropertiesFile  : public PropertySet,
                                  public ChangeBroadcaster,
                                  private Timer
{
public:
    //==============================================================================
    enum FileFormatOptions
    {
        ignoreCaseOfKeyNames            = 1,
        storeAsBinary                   = 2,
        storeAsCompressedBinary         = 4,
        storeAsXML                      = 8
    };

    //==============================================================================
    /**
        Creates a PropertiesFile object.

        @param file                         the file to use
        @param millisecondsBeforeSaving     if this is zero or greater, then after a value
                                            is changed, the object will wait for this amount
                                            of time and then save the file. If zero, the file
                                            will be written to disk immediately on being changed
                                            (which might be slow, as it'll re-write synchronously
                                            each time a value-change method is called). If it is
                                            less than zero, the file won't be saved until
                                            save() or saveIfNeeded() are explicitly called.
        @param options                      a combination of the flags in the FileFormatOptions
                                            enum, which specify the type of file to save, and other
                                            options.
    */
    PropertiesFile (const File& file,
                    const int millisecondsBeforeSaving,
                    const int options);

    /** Destructor.

        When deleted, the file will first call saveIfNeeded() to flush any changes to disk.
    */
    ~PropertiesFile();

    //==============================================================================
    /** This will flush all the values to disk if they've changed since the last
        time they were saved.

        Returns false if it fails to write to the file for some reason (maybe because
        it's read-only or the directory doesn't exist or something).

        @see save
    */
    bool saveIfNeeded();

    /** This will force a write-to-disk of the current values, regardless of whether
        anything has changed since the last save.

        Returns false if it fails to write to the file for some reason (maybe because
        it's read-only or the directory doesn't exist or something).

        @see saveIfNeeded
    */
    bool save();

    /** Returns true if the properties have been altered since the last time they were
        saved.
    */
    bool needsToBeSaved() const;

    //==============================================================================
    /** Returns the file that's being used. */
    const File getFile() const                              { return file; }

    //==============================================================================
    /** Handy utility to create a properties file in whatever the standard OS-specific
        location is for these things.

        This uses getDefaultAppSettingsFile() to decide what file to create, then
        creates a PropertiesFile object with the specified properties. See
        getDefaultAppSettingsFile() and the class's constructor for descriptions of
        what the parameters do.

        @see getDefaultAppSettingsFile
    */
    static PropertiesFile* createDefaultAppPropertiesFile (const String& applicationName,
                                                           const String& fileNameSuffix,
                                                           const String& folderName,
                                                           const bool commonToAllUsers,
                                                           const int millisecondsBeforeSaving,
                                                           const int propertiesFileOptions);

    /** Handy utility to choose a file in the standard OS-dependent location for application
        settings files.

        So on a Mac, this will return a file called:
        ~/Library/Preferences/[folderName]/[applicationName].[fileNameSuffix]

        On Windows it'll return something like:
        C:\\Documents and Settings\\username\\Application Data\\[folderName]\\[applicationName].[fileNameSuffix]

        On Linux it'll return
        ~/.[folderName]/[applicationName].[fileNameSuffix]

        If you pass an empty string as the folder name, it'll use the app name for this (or
        omit the folder name on the Mac).

        If commonToAllUsers is true, then this will return the same file for all users of the
        computer, regardless of the current user. If it is false, the file will be specific to
        only the current user. Use this to choose whether you're saving settings that are common
        or user-specific.
    */
    static const File getDefaultAppSettingsFile (const String& applicationName,
                                                 const String& fileNameSuffix,
                                                 const String& folderName,
                                                 const bool commonToAllUsers);

    //==============================================================================
    juce_UseDebuggingNewOperator


protected:
    virtual void propertyChanged();

private:
    //==============================================================================
    File file;
    int timerInterval;
    const int options;
    bool needsWriting;

    void timerCallback();

    PropertiesFile (const PropertiesFile&);
    const PropertiesFile& operator= (const PropertiesFile&);
};

#endif   // __JUCE_PROPERTIESFILE_JUCEHEADER__
