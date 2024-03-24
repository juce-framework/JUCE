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

/**
    Calls a function every time the native scale factor of a component's peer changes.

    This is used in the VST and VST3 wrappers to ensure that the editor's scale is kept in sync with
    the scale of its containing component.

    @tags{GUI}
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
