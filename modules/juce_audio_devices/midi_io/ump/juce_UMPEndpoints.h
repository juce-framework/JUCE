/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::universal_midi_packets
{

/**
    An interface class for types that are interested in receiving updates
    about changes to available MIDI endpoints.

    @tags{Audio}
*/
struct EndpointsListener
{
    EndpointsListener() = default;

    EndpointsListener (const EndpointsListener&) = default;
    EndpointsListener (EndpointsListener&&) noexcept = default;

    EndpointsListener& operator= (const EndpointsListener&) = default;
    EndpointsListener& operator= (EndpointsListener&&) noexcept = default;

    virtual ~EndpointsListener() = default;

    /** Called on each platform to notify listeners that some aspect of the MIDI configuration
        has changed, including device connection, disconnection, and property changes.
    */
    virtual void endpointsChanged() = 0;

    /** Called on Android to indicate that the service managing the virtual MIDI ports
        was started or stopped.
        Creating a virtual endpoint will fail if the service is not running, so you may wish to
        listen for this event, and to create the virtual ports after this function has been called.
        You can query the current state of the service using Endpoints::isVirtualUmpServiceActive()
        and Endpoints::isVirtualBytestreamServiceActive().
    */
    virtual void virtualMidiServiceActiveChanged() {}
};

/** MIDI implementation technologies

    @tags{Audio}
*/
enum class Backend
{
    alsa,       ///< linux
    android,    ///< android
    coremidi,   ///< macOS, iOS
    winmm,      ///< classic Windows MIDI
    winrt,      ///< Windows WinRT MIDI 1.0 (experimental)
    wms,        ///< Windows MIDI Services (experimental)
};

/**
    Endpoints known to the system.

    Use this to locate hardware and software devices that are capable of sending and
    receiving MIDI messages.

    Call makeSession() to create a session that manages connections to those devices.

    @tags{Audio}
*/
class Endpoints : public DeletedAtShutdown
{
public:
    /** Fetch endpoint ids. */
    std::vector<EndpointId> getEndpoints() const;

    /** Fetch endpoint ids into the provided buffer. */
    void getEndpoints (std::vector<EndpointId>&) const;

    /** Fetches information about a particular endpoint, or nullopt if the information
        is unavailable.

        Currently, this function may return nullopt on Android for UMP devices that are
        connected but not opened.

        If you're displaying endpoint names in a user-facing list, it's probably a good idea to
        use the Endpoint name if getEndpoint() returns a value. Otherwise, you can use the
        StaticDeviceInformation name if getStaticDeviceInformation() returns a value. If both
        functions return nullopt, this implies that the device is unknown/disconnected.
    */
    std::optional<Endpoint> getEndpoint (const EndpointId&) const;

    /** Fetches static information about a particular endpoint, or nullopt if the endpoint is
        unavailable, which might happen if the endpoint has been disconnected.
    */
    std::optional<StaticDeviceInfo> getStaticDeviceInfo (const EndpointId&) const;

    /** Adds a listener that will receive notifications when endpoints are added, removed,
        or otherwise changed.
    */
    void addListener (EndpointsListener&);

    /** Removes a listener that was previously added with addListener().
    */
    void removeListener (EndpointsListener&);

    /** Creates a session to manage connections to endpoints.
        It's possible for this function to fail, in which case Session::isAlive() will return false.
    */
    Session makeSession (const String& name) const;

    /** Returns the technology that is being used to communicate with MIDI devices on this platform.
        This is mainly relevant on Windows, where there are several different MIDI implementations.

        The intended use of this getter is to allow programs to detect whether the Windows MIDI
        Services (WMS) are active and running. If initialisation of WMS failed, the implementation
        will fall back to an earlier, more limited implementation. Programs may wish to detect this
        situation so that they can direct users to install the WMS SDK.

        If no MIDI technology is available, this returns nullopt.
    */
    std::optional<Backend> getBackend() const;

    /** Creating a virtual legacy port will only succeed if this function returns true.
        On platforms that support virtual MIDI connections (macOS, iOS, ALSA, Windows MIDI Services),
        this will normally return true.
        On platforms that don't support virtual MIDI (older Windows MIDI APIs), this will always
        return false.
        On Android, this will return false until the virtual MIDI service has started, and will
        return true while the service is running.
        You can listen for virtualMidiServiceActiveChanged() to take actions (e.g. creating or
        destroying virtual ports) when the service status changes.
    */
    bool isVirtualMidiBytestreamServiceActive() const;

    /** Creating a virtual UMP endpoint will only succeed if this function returns true.
        On recent platforms that support virtual MIDI connections (recent macOS and iOS, recent ALSA,
        Windows MIDI Services), this will normally return true.
        On platforms that don't support virtual MIDI (older Windows MIDI APIs), this will always
        return false.
        On Android, this will return false until the virtual MIDI service has started, and will
        return true while the service is running.
        You can listen for virtualMidiServiceActiveChanged() to take actions (e.g. creating or
        destroying virtual ports) when the service status changes.
    */
    bool isVirtualMidiUmpServiceActive() const;

    /** By default, Android MIDI services are initially disabled, but can be enabled by calling
        this function, passing true.
        This function is *not synchronous*, so the service will start or stop at some point after
        this function has returned.
        To find out when the service state changes, listen for virtualMidiServiceActiveChanged()
        and check the result of isVirtualMidiBytestreamServiceActive().

        On platforms other than Android, this call does nothing.
    */
    void setVirtualMidiBytestreamServiceActive (bool);

    /** By default, Android MIDI services are initially disabled, but can be enabled by calling
        this function, passing true.
        This function is *not synchronous*, so the service will start or stop at some point after
        this function has returned.
        To find out when the service state changes, listen for virtualMidiServiceActiveChanged()
        and check the result of isVirtualMidiUmpServiceActive().
        On Android versions that don't support virtual UMP (API level < 35), this call does nothing.

        On platforms other than Android, this call does nothing.
    */
    void setVirtualMidiUmpServiceActive (bool);

    JUCE_DECLARE_SINGLETON_INLINE (Endpoints, true)

    ~Endpoints() override;

    /** @internal */
    class Impl;

private:
    Endpoints();

    std::unique_ptr<Impl> impl;
};

}
