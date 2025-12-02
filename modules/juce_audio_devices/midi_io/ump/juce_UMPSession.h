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

/** This type is passed when creating a virtual endpoint to request static or dynamic blocks. */
enum class BlocksAreStatic : uint8_t
{
    no,     ///< Indicates that block layouts will not change after construction.
    yes,    ///< Indicates that the block layout may be modified after construction.
};

/**
    Allows creating new connections to endpoints, and also creating new virtual endpoints.

    The session is internally reference counted, meaning that the system resources represented
    by the session won't be released until the Session object, along with every Input, Output,
    VirtualEndpoint, LegacyVirtualInput, and LegacyVirtualOutput that it created has been
    destroyed.

    Internally, sessions cache open connections, so that multiple Inputs
    or Outputs to the same endpoint will share resources associated with that connection.
    Therefore, in order to minimise resource usage, you should create as few sessions as possible,
    and re-use them when creating new connections. It may still make sense to have a separate
    session for logically discrete components of your project, e.g. a multi-window application
    where each window has its own document with an associated MIDI configuration.

    @tags{Audio}
*/
class Session
{
public:
    /** Returns the name that was provided when creating this session, or an empty string
        if the session is not alive.
    */
    String getName() const;

    /** Creates a connection to a particular endpoint.
        On failure, returns a disconnected connection (i.e. isAlive() returns false).
        Passing an EndpointId denoting an endpoint that can only receive messages will fail.

        Incoming messages will be automatically converted to the specified protocol, regardless
        of their actual protocol 'on the wire'.

        If the session is not alive, this will always fail and return an Input that is not alive.
    */
    Input connectInput (const EndpointId&, PacketProtocol);

    /** Creates a connection to a particular endpoint.
        On failure, returns a disconnected connection (i.e. isAlive() returns false).
        Passing an EndpointId denoting an endpoint that can only send messages will fail.

        If the session is not alive, this will always fail and return an Output that is not alive.
    */
    Output connectOutput (const EndpointId&);

    /** Returns a new VirtualEndpoint if virtual endpoints are supported and the configuration is
        valid. If creating the endpoint fails, this will return an invalid VirtualEndpoint.

        To actually send and receive messages through this endpoint, use connectInput()
        and connectOutput(), passing the result of VirtualEndpoint::getId(). This will create
        an Input or Output that can be used in the same way as for any other kind of device.

        If the function blocks are static, all blocks must be marked as active.
        If the function blocks are not static, then blocks may be initially inactive.
        The number of declared function blocks may not change while the device is active,
        so if you need a dynamic number of blocks, mark the block layout as non-static
        and mark any initially unused blocks as inactive.

        Only a maximum of 32 blocks are allowed on an endpoint. If you pass more than 32
        blocks, this function will fail.

        This function may also fail if virtual devices are not available on the current platform.

        Some platforms (older macOS, older Linux) support virtual MIDI 1.0 devices but not
        virtual UMP devices. On such platforms, this function will fail. You may wish to check
        for this case, and to call createLegacyVirtualInput() and/or createLegacyVirtualOutput()
        as a fall-back.

        If the session is not alive, this will always fail and return a VirtualEndpoint that is not
        alive.

        On Android, this function will only ever succeed on newer platforms (API level >= 35).
        You may need to call Endpoints::setVirtualMidiUmpServiceActive() to make the virtual UMP
        service available. You can listen for state changes in the virtual MIDI service using
        EndpointsListener::virtualMidiServiceActiveChanged(). A maximum of one VirtualEndpoint may
        be alive at any time on Android, and attempting to create additional virtual endpoints will
        fail.
    */
    VirtualEndpoint createVirtualEndpoint (const String& name,
                                           const DeviceInfo& deviceInfo,
                                           const String& productInstanceID,
                                           PacketProtocol protocol,
                                           Span<const Block> initialBlocks,
                                           BlocksAreStatic);

    /** Creates a MIDI 1.0-compatible input port.

        Where supported by platform APIs, this will explicitly create a single-group MIDI 1.0 port.

        To use the input, pass the result of LegacyVirtualInput::getId() to connectInput().

        There are some special cases to keep in mind:

        - Windows MIDI Services only allows creation of UMP endpoints, not MIDI 1.0 ports, so on
          that platform we create an endpoint with a single MIDI 1.0 Block.
        - Android requires that virtual ports are declared in the app manifest. JUCE declares a
          virtual bytestream device with a single input and output, and a virtual UMP device with
          a single bidirectional port. By default, these virtual devices are disabled, but they
          can be enabled by calling Endpoints::setVirtualMidiBytestreamServiceActive() and
          Endpoints::setVirtualMidiUmpServiceActive(). After each service becomes available, you
          may create exactly one VirtualEndpoint, one LegacyVirtualInput, and/or one
          LegacyVirtualOutput. Additional virtual ports are not supported.
    */
    LegacyVirtualInput createLegacyVirtualInput (const String& name);

    /** Creates a MIDI 1.0-compatible output port.

        Where supported by platform APIs, this will explicitly create a single-group MIDI 1.0 port.

        To use the output, pass the result of LegacyVirtualOutput::getId() to connectOutput().

        There are some special cases to keep in mind:

        - Windows MIDI Services only allows creation of UMP endpoints, not MIDI 1.0 ports, so on
          that platform we create an endpoint with a single MIDI 1.0 Block.
        - Android requires that virtual ports are declared in the app manifest. JUCE declares a
          virtual bytestream device with a single input and output, and a virtual UMP device with
          a single bidirectional port. By default, these virtual devices are disabled, but they
          can be enabled by calling Endpoints::setVirtualMidiBytestreamServiceActive() and
          Endpoints::setVirtualMidiUmpServiceActive(). After each service becomes available, you
          may create exactly one VirtualEndpoint, one LegacyVirtualInput, and/or one
          LegacyVirtualOutput. Additional virtual ports are not supported.
    */
    LegacyVirtualOutput createLegacyVirtualOutput (const String& name);

    /** True if this session was created successfully and is currently alive. */
    bool isAlive() const;

    explicit operator bool() const { return isAlive(); }

    Session (const Session&);
    Session (Session&&) noexcept;
    Session& operator= (const Session&);
    Session& operator= (Session&&) noexcept;
    ~Session();

    /** @internal */
    class Impl;

private:
    explicit Session (std::shared_ptr<Impl>);

    std::shared_ptr<Impl> impl;
};

}
