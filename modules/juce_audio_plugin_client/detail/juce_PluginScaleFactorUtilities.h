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

#pragma once

namespace juce::detail
{

/** Keeps track of scale factors specified by the host and/or queried by the
    the plugin.
*/
class StoredScaleFactor
{
public:
    /** Sets a scale factor that originated from the host.
        This scale will take precedence over other scale factors.
    */
    StoredScaleFactor withHost     (float x) const { return withMember (*this, &StoredScaleFactor::host,     x); }

    /** Sets a scale factor that originated from the plugin.
        This scale will only be used if there's no host-provided scale.
        Defaults to 1.0f.
    */
    StoredScaleFactor withInternal (float x) const { return withMember (*this, &StoredScaleFactor::internal, x); }

    /** Returns the host-provided scale, if any, or the internal scale otherwise. */
    float get() const { return host.value_or (internal); }

    /** Returns true if this object holds a host-originated scale, or false otherwise. */
    bool isHostScale() const { return host.has_value(); }

private:
    std::optional<float> host;
    float internal = 1.0f;
};

class PluginScaleFactorManagerListener
{
public:
    virtual ~PluginScaleFactorManagerListener() = default;
    virtual void peerBoundsDidUpdate() = 0;
};

class PluginScaleFactorManager : private Timer,
                                 private ComponentListener
{
public:
    ~PluginScaleFactorManager() override
    {
        stopTimer();
    }

    void startObserving (Component& comp)
    {
        observed = &comp;
        observed->addComponentListener (this);
        applyScaleFactor (scale);

       #if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
        if (! scale.isHostScale())
            startTimer (500);
       #endif
    }

    void stopObserving ([[maybe_unused]] Component& comp)
    {
        stopTimer();
        jassert (&comp == observed.getComponent());
        observed->removeComponentListener (this);
        observed = nullptr;
    }

    void addListener (PluginScaleFactorManagerListener& l)
    {
        listeners.add (&l);
    }

    void removeListener (PluginScaleFactorManagerListener& l)
    {
        listeners.remove (&l);
    }

    void setHostScale (float x)
    {
        stopTimer();
        applyScaleFactor (scale.withHost (x));
    }

    std::optional<float> getHostScale() const
    {
        return scale.isHostScale() ? std::optional (scale.get()) : std::nullopt;
    }

    Rectangle<int> convertToHostBounds (Rectangle<float> pluginRect) const
    {
        jassert (observed != nullptr);
        return (observed->localAreaToGlobal (pluginRect) * getPlatformAndDesktopScale()).withZeroOrigin().toNearestIntEdges();
    }

    Rectangle<float> convertFromHostBounds (Rectangle<int> hostViewRect) const
    {
        jassert (observed != nullptr);
        return observed->getLocalArea (nullptr, hostViewRect.toFloat() / getPlatformAndDesktopScale()).withZeroOrigin();
    }

   #if JUCE_WINDOWS
    static double getScaleFactorForWindow (HWND h)
    {
        return (double) GetDpiForWindow (h) / USER_DEFAULT_SCREEN_DPI;
    }
   #endif

private:
    void componentParentHierarchyChanged (Component&) override
    {
        if (auto* peer = observed->getPeer())
            peer->setCustomPlatformScaleFactor (getHostScale());
    }

    float getScaleFactorForWindow() const
    {
        if (auto* comp = observed.getComponent())
            if (auto* peer = comp->getPeer())
                return (float) peer->getPlatformScaleFactor();

        return 1.0f;
    }

    void timerCallback() override
    {
        if (const auto estimatedScale = getScaleFactorForWindow(); estimatedScale > 0.0f)
            applyScaleFactor (scale.withInternal (estimatedScale));
    }

    void applyScaleFactor (StoredScaleFactor newFactor)
    {
        const auto previous = std::exchange (scale, newFactor).get();
        const auto current = scale.get();
        const auto scalesEqual = approximatelyEqual (current, previous);

        if (observed == nullptr)
            return;

        if (scale.isHostScale())
            if (auto* peer = observed->getPeer())
                peer->setCustomPlatformScaleFactor (current);

        if (scalesEqual)
            return;

       #if JUCE_LINUX || JUCE_BSD
        const MessageManagerLock mmLock;
       #endif

        if (auto* peer = observed->getPeer())
        {
            peer->updateBounds();
            listeners.call ([] (auto& l) { l.peerBoundsDidUpdate(); });
        }
    }

    float getPlatformAndDesktopScale() const
    {
        jassert (observed != nullptr);
        return (observed->getDesktopScaleFactor() * std::invoke ([&]
        {
            if (auto* peer = observed->getPeer())
                return (float) peer->getPlatformScaleFactor();

            return scale.get();
        }));
    }

    ListenerList<PluginScaleFactorManagerListener> listeners;
    Component::SafePointer<Component> observed;
    StoredScaleFactor scale;
};

} // namespace juce::detail
