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

class VirtualEndpoint::Impl final
{
public:
    class Native
    {
    public:
        virtual ~Native() = default;

        virtual EndpointId getId() const = 0;
        virtual bool setBlock (uint8_t, const Block&) = 0;
        virtual bool setName (const String&) = 0;
    };

    EndpointId getId() const
    {
        return identifier;
    }

    bool setBlock (uint8_t i, const Block& b)
    {
        if (native != nullptr)
            return native->setBlock (i, b);

        return false;
    }

    bool setName (const String& n)
    {
        if (native != nullptr)
            return native->setName (n);

        return false;
    }

    bool isAlive() const
    {
        return native != nullptr;
    }

    static VirtualEndpoint makeVirtualEndpoint (std::unique_ptr<Native> x)
    {
        if (x != nullptr)
            return VirtualEndpoint { rawToUniquePtr (new Impl (std::move (x))) };

        return {};
    }

private:
    explicit Impl (std::unique_ptr<Native> n)
        : native (std::move (n)),
          identifier (native->getId())
    {
    }

    std::unique_ptr<Native> native;
    EndpointId identifier;
};

VirtualEndpoint::VirtualEndpoint() = default;
VirtualEndpoint::~VirtualEndpoint() = default;
VirtualEndpoint::VirtualEndpoint (std::unique_ptr<Impl> x) : impl (std::move (x)) {}
VirtualEndpoint::VirtualEndpoint (VirtualEndpoint&&) noexcept = default;
VirtualEndpoint& VirtualEndpoint::operator= (VirtualEndpoint&&) noexcept = default;

EndpointId VirtualEndpoint::getId() const
{
    if (impl != nullptr)
        return impl->getId();

    return {};
}

bool VirtualEndpoint::setBlock (uint8_t i, const Block& b)
{
    if (impl != nullptr)
        return impl->setBlock (i, b);

    return false;
}

bool VirtualEndpoint::setName (const String& x)
{
    if (impl != nullptr)
        return impl->setName (x);

    return false;
}

bool VirtualEndpoint::isAlive() const
{
    if (impl != nullptr)
        return impl->isAlive();

    return false;
}

}
