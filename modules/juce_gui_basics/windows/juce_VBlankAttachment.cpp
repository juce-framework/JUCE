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

namespace juce
{

VBlankAttachment::VBlankAttachment (Component* c, std::function<void()> callbackIn)
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

void VBlankAttachment::onVBlank()
{
    callback();
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
