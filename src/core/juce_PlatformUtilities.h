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

#ifndef __JUCE_PLATFORMUTILITIES_JUCEHEADER__
#define __JUCE_PLATFORMUTILITIES_JUCEHEADER__

#include "../text/juce_StringArray.h"
#include "../io/files/juce_File.h"


//==============================================================================
/**
    A collection of miscellaneous platform-specific utilities.

*/
class JUCE_API  PlatformUtilities
{
public:
    //==============================================================================
    /** Plays the operating system's default alert 'beep' sound. */
    static void beep();

    /** Tries to launch the system's default reader for a given file or URL. */
    static bool openDocument (const String& documentURL, const String& parameters);

    /** Tries to launch the system's default email app to let the user create an email.
    */
    static bool launchEmailWithAttachments (const String& targetEmailAddress,
                                            const String& emailSubject,
                                            const String& bodyText,
                                            const StringArray& filesToAttach);

#if JUCE_MAC || JUCE_IOS || DOXYGEN
    //==============================================================================
    /** MAC ONLY - Turns a Core CF String into a juce one. */
    static String cfStringToJuceString (CFStringRef cfString);

    /** MAC ONLY - Turns a juce string into a Core CF one. */
    static CFStringRef juceStringToCFString (const String& s);

    /** MAC ONLY - Turns a file path into an FSRef, returning true if it succeeds. */
    static bool makeFSRefFromPath (FSRef* destFSRef, const String& path);

    /** MAC ONLY - Turns an FSRef into a juce string path. */
    static String makePathFromFSRef (FSRef* file);

    /** MAC ONLY - Converts any decomposed unicode characters in a string into
        their precomposed equivalents.
    */
    static String convertToPrecomposedUnicode (const String& s);

    /** MAC ONLY - Gets the type of a file from the file's resources. */
    static OSType getTypeOfFile (const String& filename);

    /** MAC ONLY - Returns true if this file is actually a bundle. */
    static bool isBundle (const String& filename);

    /** MAC ONLY - Adds an item to the dock */
    static void addItemToDock (const File& file);

    /** MAC ONLY - Returns the current OS version number.
        E.g. if it's running on 10.4, this will be 4, 10.5 will return 5, etc.
    */
    static int getOSXMinorVersionNumber();
#endif


#if JUCE_WINDOWS || DOXYGEN
    //==============================================================================
    // Some registry helper functions:

    /** WIN32 ONLY - Returns a string from the registry.

        The path is a string for the entire path of a value in the registry,
        e.g. "HKEY_CURRENT_USER\Software\foo\bar"
    */
    static String getRegistryValue (const String& regValuePath,
                                    const String& defaultValue = String::empty);

    /** WIN32 ONLY - Sets a registry value as a string.

        This will take care of creating any groups needed to get to the given
        registry value.
    */
    static void setRegistryValue (const String& regValuePath,
                                  const String& value);

    /** WIN32 ONLY - Returns true if the given value exists in the registry. */
    static bool registryValueExists (const String& regValuePath);

    /** WIN32 ONLY - Deletes a registry value. */
    static void deleteRegistryValue (const String& regValuePath);

    /** WIN32 ONLY - Deletes a registry key (which is registry-talk for 'folder'). */
    static void deleteRegistryKey (const String& regKeyPath);

    /** WIN32 ONLY - Creates a file association in the registry.

        This lets you set the exe that should be launched by a given file extension.
        @param fileExtension        the file extension to associate, including the
                                    initial dot, e.g. ".txt"
        @param symbolicDescription  a space-free short token to identify the file type
        @param fullDescription      a human-readable description of the file type
        @param targetExecutable     the executable that should be launched
        @param iconResourceNumber   the icon that gets displayed for the file type will be
                                    found by looking up this resource number in the
                                    executable. Pass 0 here to not use an icon
    */
    static void registerFileAssociation (const String& fileExtension,
                                         const String& symbolicDescription,
                                         const String& fullDescription,
                                         const File& targetExecutable,
                                         int iconResourceNumber);

    /** WIN32 ONLY - This returns the HINSTANCE of the current module.

        In a normal Juce application this will be set to the module handle
        of the application executable.

        If you're writing a DLL using Juce and plan to use any Juce messaging or
        windows, you'll need to make sure you use the setCurrentModuleInstanceHandle()
        to set the correct module handle in your DllMain() function, because
        the win32 system relies on the correct instance handle when opening windows.
    */
    static void* JUCE_CALLTYPE getCurrentModuleInstanceHandle() noexcept;

    /** WIN32 ONLY - Sets a new module handle to be used by the library.

        @see getCurrentModuleInstanceHandle()
    */
    static void JUCE_CALLTYPE setCurrentModuleInstanceHandle (void* newHandle) noexcept;

    /** WIN32 ONLY - Gets the command-line params as a string.

        This is needed to avoid unicode problems with the argc type params.
    */
    static String JUCE_CALLTYPE getCurrentCommandLineParams();
#endif

    /** Clears the floating point unit's flags.

        Only has an effect under win32, currently.
    */
    static void fpuReset();


#if JUCE_LINUX || JUCE_WINDOWS
    //==============================================================================
    /** Loads a dynamically-linked library into the process's address space.

        @param pathOrFilename   the platform-dependent name and search path
        @returns                a handle which can be used by getProcedureEntryPoint(), or
                                zero if it fails.
        @see freeDynamicLibrary, getProcedureEntryPoint
    */
    static void* loadDynamicLibrary (const String& pathOrFilename);

    /** Frees a dynamically-linked library.

        @param libraryHandle   a handle created by loadDynamicLibrary
        @see loadDynamicLibrary, getProcedureEntryPoint
    */
    static void freeDynamicLibrary (void* libraryHandle);

    /** Finds a procedure call in a dynamically-linked library.

        @param libraryHandle    a library handle returned by loadDynamicLibrary
        @param procedureName    the name of the procedure call to try to load
        @returns                a pointer to the function if found, or 0 if it fails
        @see loadDynamicLibrary
    */
    static void* getProcedureEntryPoint (void* libraryHandle,
                                         const String& procedureName);
#endif

private:
    PlatformUtilities();

    JUCE_DECLARE_NON_COPYABLE (PlatformUtilities);
};


//==============================================================================
#if JUCE_MAC || JUCE_IOS || DOXYGEN

 /** A handy C++ wrapper that creates and deletes an NSAutoreleasePool object using RAII. */
 class JUCE_API  ScopedAutoReleasePool
 {
 public:
     ScopedAutoReleasePool();
     ~ScopedAutoReleasePool();

 private:
     void* pool;

     JUCE_DECLARE_NON_COPYABLE (ScopedAutoReleasePool);
 };

 /** A macro that can be used to easily declare a local ScopedAutoReleasePool object for RAII-based obj-C autoreleasing. */
 #define JUCE_AUTORELEASEPOOL  const JUCE_NAMESPACE::ScopedAutoReleasePool JUCE_JOIN_MACRO (autoReleasePool_, __LINE__);

#else
 #define JUCE_AUTORELEASEPOOL
#endif


//==============================================================================
#if JUCE_LINUX

/** A handy class that uses XLockDisplay and XUnlockDisplay to lock the X server
    using an RAII approach.
*/
class ScopedXLock
{
public:
    /** Creating a ScopedXLock object locks the X display.
        This uses XLockDisplay() to grab the display that Juce is using.
    */
    ScopedXLock();

    /** Deleting a ScopedXLock object unlocks the X display.
        This calls XUnlockDisplay() to release the lock.
    */
    ~ScopedXLock();
};

#endif


//==============================================================================
#if JUCE_MAC

/**
    A wrapper class for picking up events from an Apple IR remote control device.

    To use it, just create a subclass of this class, implementing the buttonPressed()
    callback, then call start() and stop() to start or stop receiving events.
*/
class JUCE_API  AppleRemoteDevice
{
public:
    //==============================================================================
    AppleRemoteDevice();
    virtual ~AppleRemoteDevice();

    //==============================================================================
    /** The set of buttons that may be pressed.
        @see buttonPressed
    */
    enum ButtonType
    {
        menuButton = 0,     /**< The menu button (if it's held for a short time). */
        playButton,         /**< The play button. */
        plusButton,         /**< The plus or volume-up button. */
        minusButton,        /**< The minus or volume-down button. */
        rightButton,        /**< The right button (if it's held for a short time). */
        leftButton,         /**< The left button (if it's held for a short time). */
        rightButton_Long,   /**< The right button (if it's held for a long time). */
        leftButton_Long,    /**< The menu button (if it's held for a long time). */
        menuButton_Long,    /**< The menu button (if it's held for a long time). */
        playButtonSleepMode,
        switched
    };

    //==============================================================================
    /** Override this method to receive the callback about a button press.

        The callback will happen on the application's message thread.

        Some buttons trigger matching up and down events, in which the isDown parameter
        will be true and then false. Others only send a single event when the
        button is pressed.
    */
    virtual void buttonPressed (ButtonType buttonId, bool isDown) = 0;

    //==============================================================================
    /** Starts the device running and responding to events.

        Returns true if it managed to open the device.

        @param inExclusiveMode  if true, the remote will be grabbed exclusively for this app,
                                and will not be available to any other part of the system. If
                                false, it will be shared with other apps.
        @see stop
    */
    bool start (bool inExclusiveMode);

    /** Stops the device running.
        @see start
    */
    void stop();

    /** Returns true if the device has been started successfully.
    */
    bool isActive() const;

    /** Returns the ID number of the remote, if it has sent one.
    */
    int getRemoteId() const                     { return remoteId; }

    //==============================================================================
    /** @internal */
    void handleCallbackInternal();

private:
    void* device;
    void* queue;
    int remoteId;

    bool open (bool openInExclusiveMode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppleRemoteDevice);
};

#endif


#endif   // __JUCE_PLATFORMUTILITIES_JUCEHEADER__
