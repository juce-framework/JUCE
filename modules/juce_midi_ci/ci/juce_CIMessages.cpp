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

namespace juce::midi_ci::Message
{

//==============================================================================
auto Header::tie() const
{
    return std::tuple (deviceID, category, version, source, destination);
}

bool Header::operator== (const Header& x) const { return tie() == x.tie(); }
bool Header::operator!= (const Header& x) const { return ! operator== (x); }

//==============================================================================
auto DiscoveryResponse::tie() const
{
    return std::tuple (device, capabilities, maximumSysexSize, outputPathID, functionBlock);
}

bool DiscoveryResponse::operator== (const DiscoveryResponse& x) const { return tie() == x.tie(); }
bool DiscoveryResponse::operator!= (const DiscoveryResponse& x) const { return ! operator== (x); }

//==============================================================================
auto Discovery::tie() const
{
    return std::tuple (device, capabilities, maximumSysexSize, outputPathID);
}

bool Discovery::operator== (const Discovery& x) const { return tie() == x.tie(); }
bool Discovery::operator!= (const Discovery& x) const { return ! operator== (x); }

//==============================================================================
auto EndpointInquiryResponse::tie() const
{
    return std::tuple (status, makeComparableRange (data));
}

bool EndpointInquiryResponse::operator== (const EndpointInquiryResponse& x) const { return tie() == x.tie(); }
bool EndpointInquiryResponse::operator!= (const EndpointInquiryResponse& x) const { return ! operator== (x); }

//==============================================================================
auto EndpointInquiry::tie() const
{
    return std::tuple (status);
}

bool EndpointInquiry::operator== (const EndpointInquiry& x) const { return tie() == x.tie(); }
bool EndpointInquiry::operator!= (const EndpointInquiry& x) const { return ! operator== (x); }

//==============================================================================
auto InvalidateMUID::tie() const
{
    return std::tuple (target);
}

bool InvalidateMUID::operator== (const InvalidateMUID& x) const { return tie() == x.tie(); }
bool InvalidateMUID::operator!= (const InvalidateMUID& x) const { return ! operator== (x); }

//==============================================================================
auto ACK::tie() const
{
    return std::tuple (originalCategory, statusCode, statusData, details, makeComparableRange (messageText));
}

bool ACK::operator== (const ACK& x) const { return tie() == x.tie(); }
bool ACK::operator!= (const ACK& x) const { return ! operator== (x); }

//==============================================================================
auto NAK::tie() const
{
    return std::tuple (originalCategory, statusCode, statusData, details, makeComparableRange (messageText));
}

bool NAK::operator== (const NAK& x) const { return tie() == x.tie(); }
bool NAK::operator!= (const NAK& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileInquiryResponse::tie() const
{
    return std::tuple (makeComparableRange (enabledProfiles), makeComparableRange (disabledProfiles));
}

bool ProfileInquiryResponse::operator== (const ProfileInquiryResponse& x) const { return tie() == x.tie(); }
bool ProfileInquiryResponse::operator!= (const ProfileInquiryResponse& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileInquiry::tie() const
{
    return std::tuple<>();
}

bool ProfileInquiry::operator== (const ProfileInquiry& x) const { return tie() == x.tie(); }
bool ProfileInquiry::operator!= (const ProfileInquiry& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileAdded::tie() const
{
    return std::tuple (profile);
}

bool ProfileAdded::operator== (const ProfileAdded& x) const { return tie() == x.tie(); }
bool ProfileAdded::operator!= (const ProfileAdded& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileRemoved::tie() const
{
    return std::tuple (profile);
}

bool ProfileRemoved::operator== (const ProfileRemoved& x) const { return tie() == x.tie(); }
bool ProfileRemoved::operator!= (const ProfileRemoved& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileDetailsResponse::tie() const
{
    return std::tuple (profile, target, makeComparableRange (data));
}

bool ProfileDetailsResponse::operator== (const ProfileDetailsResponse& x) const { return tie() == x.tie(); }
bool ProfileDetailsResponse::operator!= (const ProfileDetailsResponse& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileDetails::tie() const
{
    return std::tuple (profile, target);
}

bool ProfileDetails::operator== (const ProfileDetails& x) const { return tie() == x.tie(); }
bool ProfileDetails::operator!= (const ProfileDetails& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileOn::tie() const
{
    return std::tuple (profile, numChannels);
}

bool ProfileOn::operator== (const ProfileOn& x) const { return tie() == x.tie(); }
bool ProfileOn::operator!= (const ProfileOn& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileOff::tie() const
{
    return std::tuple (profile);
}

bool ProfileOff::operator== (const ProfileOff& x) const { return tie() == x.tie(); }
bool ProfileOff::operator!= (const ProfileOff& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileEnabledReport::tie() const
{
    return std::tuple (profile, numChannels);
}

bool ProfileEnabledReport::operator== (const ProfileEnabledReport& x) const { return tie() == x.tie(); }
bool ProfileEnabledReport::operator!= (const ProfileEnabledReport& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileDisabledReport::tie() const
{
    return std::tuple (profile, numChannels);
}

bool ProfileDisabledReport::operator== (const ProfileDisabledReport& x) const { return tie() == x.tie(); }
bool ProfileDisabledReport::operator!= (const ProfileDisabledReport& x) const { return ! operator== (x); }

//==============================================================================
auto ProfileSpecificData::tie() const
{
    return std::tuple (profile, makeComparableRange (data));
}

bool ProfileSpecificData::operator== (const ProfileSpecificData& x) const { return tie() == x.tie(); }
bool ProfileSpecificData::operator!= (const ProfileSpecificData& x) const { return ! operator== (x); }

//==============================================================================
auto PropertyExchangeCapabilitiesResponse::tie() const
{
    return std::tuple (numSimultaneousRequestsSupported, majorVersion, minorVersion);
}

bool PropertyExchangeCapabilitiesResponse::operator== (const PropertyExchangeCapabilitiesResponse& x) const { return tie() == x.tie(); }
bool PropertyExchangeCapabilitiesResponse::operator!= (const PropertyExchangeCapabilitiesResponse& x) const { return ! operator== (x); }

//==============================================================================
auto PropertyExchangeCapabilities::tie() const
{
    return std::tuple (numSimultaneousRequestsSupported, majorVersion, minorVersion);
}

bool PropertyExchangeCapabilities::operator== (const PropertyExchangeCapabilities& x) const { return tie() == x.tie(); }
bool PropertyExchangeCapabilities::operator!= (const PropertyExchangeCapabilities& x) const { return ! operator== (x); }

//==============================================================================
auto StaticSizePropertyExchange::tie() const
{
    return std::tuple (requestID, makeComparableRange (header));
}

//==============================================================================
auto DynamicSizePropertyExchange::tie() const
{
    return std::tuple (requestID,
                       makeComparableRange (header),
                       totalNumChunks,
                       thisChunkNum,
                       makeComparableRange (data));
}

//==============================================================================
bool PropertyGetDataResponse::operator== (const PropertyGetDataResponse& x) const { return tie() == x.tie(); }
bool PropertyGetDataResponse::operator!= (const PropertyGetDataResponse& x) const { return ! operator== (x); }

//==============================================================================
bool PropertyGetData::operator== (const PropertyGetData& x) const { return tie() == x.tie(); }
bool PropertyGetData::operator!= (const PropertyGetData& x) const { return ! operator== (x); }

//==============================================================================
bool PropertySetDataResponse::operator== (const PropertySetDataResponse& x) const { return tie() == x.tie(); }
bool PropertySetDataResponse::operator!= (const PropertySetDataResponse& x) const { return ! operator== (x); }

//==============================================================================
bool PropertySetData::operator== (const PropertySetData& x) const { return tie() == x.tie(); }
bool PropertySetData::operator!= (const PropertySetData& x) const { return ! operator== (x); }

//==============================================================================
bool PropertySubscribeResponse::operator== (const PropertySubscribeResponse& x) const { return tie() == x.tie(); }
bool PropertySubscribeResponse::operator!= (const PropertySubscribeResponse& x) const { return ! operator== (x); }

//==============================================================================
bool PropertySubscribe::operator== (const PropertySubscribe& x) const { return tie() == x.tie(); }
bool PropertySubscribe::operator!= (const PropertySubscribe& x) const { return ! operator== (x); }

//==============================================================================
bool PropertyNotify::operator== (const PropertyNotify& x) const { return tie() == x.tie(); }
bool PropertyNotify::operator!= (const PropertyNotify& x) const { return ! operator== (x); }

//==============================================================================
auto ProcessInquiryResponse::tie() const
{
    return std::tuple (supportedFeatures);
}

bool ProcessInquiryResponse::operator== (const ProcessInquiryResponse& x) const { return tie() == x.tie(); }
bool ProcessInquiryResponse::operator!= (const ProcessInquiryResponse& x) const { return ! operator== (x); }

//==============================================================================
auto ProcessInquiry::tie() const
{
    return std::tuple<>();
}

bool ProcessInquiry::operator== (const ProcessInquiry& x) const { return tie() == x.tie(); }
bool ProcessInquiry::operator!= (const ProcessInquiry& x) const { return ! operator== (x); }

//==============================================================================
auto ProcessMidiMessageReportResponse::tie() const
{
    return std::tuple (messageDataControl, requestedMessages, channelControllerMessages, noteDataMessages);
}

bool ProcessMidiMessageReportResponse::operator== (const ProcessMidiMessageReportResponse& x) const { return tie() == x.tie(); }
bool ProcessMidiMessageReportResponse::operator!= (const ProcessMidiMessageReportResponse& x) const { return ! operator== (x); }

//==============================================================================
auto ProcessMidiMessageReport::tie() const
{
    return std::tuple (messageDataControl, requestedMessages, channelControllerMessages, noteDataMessages);
}

bool ProcessMidiMessageReport::operator== (const ProcessMidiMessageReport& x) const { return tie() == x.tie(); }
bool ProcessMidiMessageReport::operator!= (const ProcessMidiMessageReport& x) const { return ! operator== (x); }

//==============================================================================
auto ProcessEndMidiMessageReport::tie() const
{
    return std::tuple<>();
}

bool ProcessEndMidiMessageReport::operator== (const ProcessEndMidiMessageReport& x) const { return tie() == x.tie(); }
bool ProcessEndMidiMessageReport::operator!= (const ProcessEndMidiMessageReport& x) const { return ! operator== (x); }

//==============================================================================
bool Parsed::operator== (const Parsed& other) const
{
    const auto tie = [] (const auto& x) { return std::tie (x.header, x.body); };
    return tie (*this) == tie (other);
}

bool Parsed::operator!= (const Parsed& other) const { return ! operator== (other); }

} // namespace juce::midi_ci::Message
