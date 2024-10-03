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

namespace juce
{

VBlankAttachment::VBlankAttachment (Component* c, std::function<void()> callbackIn)
    : owner (c),
      callback ([cb = std::move (callbackIn)] (auto) { cb(); })
{
    jassert (owner != nullptr && callback);

    updateOwner();
    updatePeer();
}

VBlankAttachment::VBlankAttachment (Component* c, std::function<void (double)> callbackIn)
    : owner (c),
      callback (std::move (callbackIn))
{
    jassert (owner != nullptr && callback);

    updateOwner();
    updatePeer();
}

VBlankAttachment::VBlankAttachment (VBlankAttachment&& other)
    : VBlankAttachment (other.owner, std::move (other.callback))
{
    other.cleanup();
}

VBlankAttachment& VBlankAttachment::operator= (VBlankAttachment&& other)
{
    cleanup();

    owner = other.owner;
    callback = std::move (other.callback);
    updateOwner();
    updatePeer();

    other.cleanup();

    return *this;
}

VBlankAttachment::~VBlankAttachment()
{
    cleanup();
}

void VBlankAttachment::onVBlank (double timestampMs)
{
    NullCheckedInvocation::invoke (callback, timestampMs);
}

void VBlankAttachment::componentParentHierarchyChanged (Component&)
{
    updatePeer();
}

void VBlankAttachment::updateOwner()
{
    if (auto previousLastOwner = std::exchange (lastOwner, owner); previousLastOwner != owner)
    {
        if (previousLastOwner != nullptr)
            previousLastOwner->removeComponentListener (this);

        if (owner != nullptr)
            owner->addComponentListener (this);
    }
}

void VBlankAttachment::updatePeer()
{
    if (owner != nullptr)
    {
        if (auto* peer = owner->getPeer())
        {
            peer->addVBlankListener (this);

            if (lastPeer != peer && ComponentPeer::isValidPeer (lastPeer))
                lastPeer->removeVBlankListener (this);

            lastPeer = peer;
        }
    }
    else if (auto peer = std::exchange (lastPeer, nullptr); ComponentPeer::isValidPeer (peer))
    {
        peer->removeVBlankListener (this);
    }
}

void VBlankAttachment::cleanup()
{
    owner = nullptr;
    updateOwner();
    updatePeer();
}

} // namespace juce
