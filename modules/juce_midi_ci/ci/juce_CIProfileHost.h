/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci
{

/**
    Acting as a ResponderListener, instances of this class can formulate
    appropriate replies to profile transactions initiated by remote devices.

    ProfileHost instances also contains methods to inform remote devices about
    changes to local profile state.

    Stores the current state of profiles on the local device.

    @tags{Audio}
*/
class ProfileHost final : public ResponderDelegate
{
public:
    /** @internal

        Rather than constructing one of these objects yourself, you should configure
        a Device with profile support, and then use Device::getProfileHost()
        to retrieve a profile host that has been set up to work with that device.
    */
    ProfileHost (FunctionBlock fb, ProfileDelegate& d, BufferOutput& o)
        : functionBlock (fb), delegate (d), output (o) {}

    /** Adds support for a profile on the specified group/channel with a
        maximum number of channels that may be activated.
    */
    void addProfile     (ProfileAtAddress, int maxNumChannels = 1);

    /** Removes support for a profile on the specified group/channel.
    */
    void removeProfile  (ProfileAtAddress);

    /** Activates or deactivates a profile on the specified group/channel.

        The profile should previously have been added with addProfile().
        A positive value of numChannels will enable the profile, and a non-positive value
        will disable it. This includes group and function-block profiles; passing any positive
        value will enable the profile on the entire group or block.
    */
    void setProfileEnablement (ProfileAtAddress, int numChannels);

    /** Returns the profile states (supported/active) for all groups and channels.
    */
    const BlockProfileStates& getProfileStates() const { return states; }

    /** Returns the number of supported and active channels for the given
        profile on the specified group/channel.

        If the supported channels is 0, then the profile is not supported
        on the group/channel.

        If the active channels is 0, then the profile is inactive on the
        group/channel.
    */
    SupportedAndActive getState (ProfileAtAddress profileAtAddress) const
    {
        if (auto* state = states.getStateForDestination (profileAtAddress.address))
            return state->get (profileAtAddress.profile);

        return {};
    }

    /** @internal */
    bool tryRespond (ResponderOutput&, const Message::Parsed&) override;

private:
    class Visitor;

    void enableProfileImpl (ProfileAtAddress, int);
    void disableProfileImpl (ProfileAtAddress);

    template <typename Body>
    bool profileEnablementReceived (ResponderOutput&, const Body&);

    FunctionBlock functionBlock;
    ProfileDelegate& delegate;
    BufferOutput& output;
    BlockProfileStates states;
    bool isResponder = false;
    std::optional<ProfileAtAddress> currentEnablementMessage;
};

} // namespace juce::midi_ci
