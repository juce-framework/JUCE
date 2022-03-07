/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

static void removeScaleFactorListenerFromAllPeers (ComponentPeer::ScaleFactorListener& listener)
{
     for (int i = 0; i < ComponentPeer::getNumPeers(); ++i)
         ComponentPeer::getPeer (i)->removeScaleFactorListener (&listener);
}

NativeScaleFactorNotifier::NativeScaleFactorNotifier (Component* comp, std::function<void (float)> onScaleChanged)
    : ComponentMovementWatcher (comp),
      scaleChanged (std::move (onScaleChanged))
{
    componentPeerChanged();
}

NativeScaleFactorNotifier::~NativeScaleFactorNotifier()
{
    removeScaleFactorListenerFromAllPeers (*this);
}

void NativeScaleFactorNotifier::nativeScaleFactorChanged (double newScaleFactor)
{
    NullCheckedInvocation::invoke (scaleChanged, (float) newScaleFactor);
}

void NativeScaleFactorNotifier::componentPeerChanged()
{
    removeScaleFactorListenerFromAllPeers (*this);

    if (auto* x = getComponent())
        peer = x->getPeer();

    if (auto* x = peer)
    {
        x->addScaleFactorListener (this);
        nativeScaleFactorChanged (x->getPlatformScaleFactor());
    }
}

} // namespace juce
