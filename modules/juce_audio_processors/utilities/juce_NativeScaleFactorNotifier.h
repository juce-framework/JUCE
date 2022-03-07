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

/**
    Calls a function every time the native scale factor of a component's peer changes.

    This is used in the VST and VST3 wrappers to ensure that the editor's scale is kept in sync with
    the scale of its containing component.
*/
class NativeScaleFactorNotifier : private ComponentMovementWatcher,
                                  private ComponentPeer::ScaleFactorListener
{
public:
    /** Constructs an instance.

        While the instance is alive, it will listen for changes to the scale factor of the
        comp's peer, and will call onScaleChanged whenever this scale factor changes.

        @param comp             The component to observe
        @param onScaleChanged   A function that will be called when the backing scale factor changes
    */
    NativeScaleFactorNotifier (Component* comp, std::function<void (float)> onScaleChanged);
    ~NativeScaleFactorNotifier() override;

private:
    void nativeScaleFactorChanged (double newScaleFactor) override;
    void componentPeerChanged() override;

    using ComponentMovementWatcher::componentVisibilityChanged;
    void componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentMovedOrResized;
    void componentMovedOrResized (bool, bool) override {}

    ComponentPeer* peer = nullptr;
    std::function<void (float)> scaleChanged;

    JUCE_DECLARE_NON_COPYABLE (NativeScaleFactorNotifier)
    JUCE_DECLARE_NON_MOVEABLE (NativeScaleFactorNotifier)
};

} // namespace juce
