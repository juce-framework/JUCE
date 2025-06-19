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

//==============================================================================
/**
    An interface class for entities that are interested in disconnection notifications.

    @tags{Audio}
*/
struct DisconnectionListener
{
    DisconnectionListener() = default;

    DisconnectionListener (const DisconnectionListener&) = default;
    DisconnectionListener (DisconnectionListener&&) noexcept = default;

    DisconnectionListener& operator= (const DisconnectionListener&) = default;
    DisconnectionListener& operator= (DisconnectionListener&&) noexcept = default;

    virtual ~DisconnectionListener() = default;

    /** Called to notify that the system has destroyed/disconnected a resource. */
    virtual void disconnected() = 0;
};

}
