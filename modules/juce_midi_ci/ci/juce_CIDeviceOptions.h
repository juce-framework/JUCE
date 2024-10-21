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

namespace juce::midi_ci
{

//==============================================================================
/**
    Configuration options for a Device.

    The options set here will remain constant over the lifetime of a Device.

    @tags{Audio}
*/
class DeviceOptions
{
public:
    static constexpr auto beginValidAscii = 32;  // inclusive
    static constexpr auto endValidAscii = 127;   // exclusive

    /** Creates a random product instance ID.
        This isn't really recommended - it's probably better to have a unique ID that remains
        persistent after a restart.
    */
    static std::array<char, 16> makeProductInstanceId (Random& random)
    {
        std::array<char, 16> result{};

        for (auto& c : result)
            c = (char) random.nextInt ({ beginValidAscii, endValidAscii });

        return result;
    }

    /** One or more DeviceMessageHandlers that should receive callbacks with any messages that the
        device wishes to send.
        Referenced DeviceMessageHandlers *must* outlive any Device constructed from these options.
    */
    [[nodiscard]] DeviceOptions withOutputs (std::vector<DeviceMessageHandler*> x) const
    {
        return withMember (*this, &DeviceOptions::outputs, x);
    }

    /** The function block layout of this device. */
    [[nodiscard]] DeviceOptions withFunctionBlock (FunctionBlock x) const
    {
        return withMember (*this, &DeviceOptions::functionBlock, x);
    }

    /** Basic information about the device used to determine manufacturer, model, etc.
        In order to populate this correctly, you'll need to register with the MIDI association -
        otherwise you might accidentally end up using IDs that are already assigned to other
        companies/individuals.
    */
    [[nodiscard]] DeviceOptions withDeviceInfo (const ump::DeviceInfo& x) const
    {
        return withMember (*this, &DeviceOptions::deviceInfo, x);
    }

    /** The features that you want to enable on the device.

        If you enable property exchange, you may wish to supply a PropertyDelegate using
        withPropertyDelegate().
        If you enable profile configuration, you may wish to supply a ProfileDelegate using
        withProfileDelegate().
        Process inquiry is not currently supported.
    */
    [[nodiscard]] DeviceOptions withFeatures (DeviceFeatures x) const
    {
        return withMember (*this, &DeviceOptions::features, x);
    }

    /** The maximum size of sysex messages to accept and to produce. */
    [[nodiscard]] DeviceOptions withMaxSysExSize (size_t x) const
    {
        return withMember (*this, &DeviceOptions::maxSysExSize, x);
    }

    /** Specifies a profile delegate that can be used to respond to particular profile events.
        The referenced ProfileDelegate *must* outlive the Device.
    */
    [[nodiscard]] DeviceOptions withProfileDelegate (ProfileDelegate* x) const
    {
        return withMember (*this, &DeviceOptions::profileDelegate, x);
    }

    /** Specifies a property delegate that can be used to respond to particular property events.
        The referenced PropertyDelegate *must* outlive the Device.
    */
    [[nodiscard]] DeviceOptions withPropertyDelegate (PropertyDelegate* x) const
    {
        return withMember (*this, &DeviceOptions::propertyDelegate, x);
    }

    /** Specifies a product instance ID that will be returned in endpoint response messages. */
    [[nodiscard]] DeviceOptions withProductInstanceId (const std::array<char, 16>& x) const
    {
        const auto null = std::find (x.begin(), x.end(), 0);

        if (! std::all_of (x.begin(), null, [] (char c) { return beginValidAscii <= c && c < endValidAscii; }))
        {
            // The product instance ID must be made up of ASCII characters
            jassertfalse;
            return *this;
        }

        if (std::any_of (null, x.end(), [] (auto c) { return c != 0; }))
        {
            // All characters after the null terminator must be 0
            jassertfalse;
            return *this;
        }

        return withMember (*this, &DeviceOptions::productInstanceId, x);
    }

    /** @see withOutputs() */
    [[nodiscard]] const auto& getOutputs()             const { return outputs; }
    /** @see withFunctionBlock() */
    [[nodiscard]] const auto& getFunctionBlock()       const { return functionBlock; }
    /** @see withDeviceInfo() */
    [[nodiscard]] const auto& getDeviceInfo()          const { return deviceInfo; }
    /** @see withFeatures() */
    [[nodiscard]] const auto& getFeatures()            const { return features; }
    /** @see withMaxSysExSize() */
    [[nodiscard]] const auto& getMaxSysExSize()        const { return maxSysExSize; }
    /** @see withProductInstanceId() */
    [[nodiscard]] const auto& getProductInstanceId()   const { return productInstanceId; }
    /** @see withProfileDelegate() */
    [[nodiscard]] const auto& getProfileDelegate()     const { return profileDelegate; }
    /** @see withPropertyDelegate() */
    [[nodiscard]] const auto& getPropertyDelegate()    const { return propertyDelegate; }

private:
    std::vector<DeviceMessageHandler*> outputs;
    FunctionBlock functionBlock;
    ump::DeviceInfo deviceInfo;
    DeviceFeatures features;
    size_t maxSysExSize = 512;
    std::array<char, 16> productInstanceId{};
    ProfileDelegate* profileDelegate = nullptr;
    PropertyDelegate* propertyDelegate = nullptr;
};

} // namespace juce::midi_ci
