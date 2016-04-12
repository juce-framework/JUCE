/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_RUNTIMEPERMISSIONS_H_INCLUDED
#define JUCE_RUNTIMEPERMISSIONS_H_INCLUDED

//==============================================================================
/**
    Class to handle app runtime permissions for certain functionality on some platforms.

    The use of this class is currently only required if the app should run on
    Android API level 23 and higher.

    On lower API levels, the permissions are specified in the app manifest. On iOS,
    runtime permission requests are handled automatically by the Apple APIs and not
    manually in the app code. On Windows, OS X, and Linux, runtime permissions are not
    used at all. In all these cases, request() will simply call through to the
    callback with no overhead and pass true (making it safe to use on all platforms).

    For example, to enable audio recording on Android in your cross-platform app,
    you could modify your code as follows:

    Old code:

        audioDeviceManager.initialise (2, 2, nullptr, true, String(), nullptr);

    New code:

        RuntimePermissions::request (
            RuntimePermissions::audioRecording,
            [this] (bool wasGranted)
            {
                 if (! wasGranted)
                 {
                     // e.g. display an error or initialise with 0 input channels
                     return;
                 }

                 audioDeviceManager.initialise (2, 2, nullptr, true, String(), nullptr);
            }
        );
*/
class JUCE_API  RuntimePermissions
{
public:
    //==============================================================================
    enum PermissionID
    {
        /** Permission to access the microphone (required on Android).
            You need to request this, for example, to initialise an AudioDeviceManager with
            a non-zero number of input channels, and to open the default audio input device.
        */
        recordAudio = 1,

        /** Permission to scan for and pair to Bluetooth MIDI devices (required on Android).
            You need to request this before calling BluetoothMidiDevicePairingDialogue::open(),
            otherwise no devices will be found.
        */
        bluetoothMidi = 2,
    };

    //==============================================================================
    /** Function type of runtime permission request callbacks. */
   #if JUCE_COMPILER_SUPPORTS_LAMBDAS
    typedef std::function<void (bool)> Callback;
   #else
    typedef void (*Callback) (bool);
   #endif

    //==============================================================================
    /** Call this method to request a runtime permission.

        @param permission  The PermissionID of the permission you want to request.

        @param callback    The callback to be called after the request has been granted
                           or denied; the argument passed will be true if the permission
                           has been granted and false otherwise.

        If no runtime request is required or possible to obtain the permission, the
        callback will be called immediately. The argument passed in will be true
        if the permission is granted or no permission is required on this platform,
        and false otherwise.

        If a runtime request is required to obtain the permission, the callback
        will be called asynchronously after the OS has granted or denied the requested
        permission (typically by displaying a dialog box to the user and waiting until
        the user has responded).
    */
    static void request (PermissionID permission, Callback callback);

    /** Returns whether a runtime request is required to obtain the permission
        on the current platform.
    */
    static bool isRequired (PermissionID permission);

    /** Returns true if the app has been already granted this permission, either
        via a previous runtime request or otherwise, or no permission is necessary.

        Note that this can be false even if isRequired returns false. In this case,
        the permission can not be obtained at all at runtime.
    */
    static bool isGranted (PermissionID permission);
};


#endif   // JUCE_RUNTIMEPERMISSIONS_H_INCLUDED
