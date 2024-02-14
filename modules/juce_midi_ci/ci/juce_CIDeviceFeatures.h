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
    Flags indicating the features that are supported by a given CI device.

    @tags{Audio}
*/
class DeviceFeatures
{
public:
    /** Constructs a DeviceFeatures object with no flags enabled. */
    DeviceFeatures() = default;

    /** Constructs a DeviceFeatures object, taking flag values from the "Capability Inquiry
        Category Supported" byte in a CI Discovery message.
    */
    explicit DeviceFeatures (std::byte f) : flags ((uint8_t) f) {}

    /** Returns a new DeviceFeatures instance with profile configuration marked as supported. */
    [[nodiscard]] DeviceFeatures withProfileConfigurationSupported (bool x = true) const        { return withFlag (profileConfiguration, x); }
    /** Returns a new DeviceFeatures instance with property exchange marked as supported. */
    [[nodiscard]] DeviceFeatures withPropertyExchangeSupported     (bool x = true) const        { return withFlag (propertyExchange,     x); }
    /** Returns a new DeviceFeatures instance with process inquiry marked as supported. */
    [[nodiscard]] DeviceFeatures withProcessInquirySupported       (bool x = true) const        { return withFlag (processInquiry,       x); }

    /** @see withProfileConfigurationSupported() */
    [[nodiscard]] bool isProfileConfigurationSupported  () const      { return getFlag (profileConfiguration); }
    /** @see withPropertyExchangeSupported() */
    [[nodiscard]] bool isPropertyExchangeSupported      () const      { return getFlag (propertyExchange); }
    /** @see withProcessInquirySupported() */
    [[nodiscard]] bool isProcessInquirySupported        () const      { return getFlag (processInquiry); }

    /** Returns the feature flags formatted into a bitfield suitable for use as the "Capability
        Inquiry Category Supported" byte in a CI Discovery message.
    */
    std::byte getSupportedCapabilities() const { return std::byte { flags }; }

    /** Returns true if this and other both have the same flags set. */
    bool operator== (const DeviceFeatures& other) const { return flags == other.flags; }
    /** Returns true if any flags in this and other differ. */
    bool operator!= (const DeviceFeatures& other) const { return ! operator== (other); }

private:
    enum Flags
    {
        profileConfiguration = 1 << 2,
        propertyExchange     = 1 << 3,
        processInquiry       = 1 << 4,
    };

    [[nodiscard]] DeviceFeatures withFlag (Flags f, bool value) const
    {
        return withMember (*this, &DeviceFeatures::flags, (uint8_t) (value ? (flags | f) : (flags & ~f)));
    }

    bool getFlag (Flags f) const
    {
        return (flags & f) != 0;
    }

    uint8_t flags = 0;
};

} // namespace juce::midi_ci
