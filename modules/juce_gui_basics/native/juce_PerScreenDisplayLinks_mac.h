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

//==============================================================================
/*
    Forwards NSNotificationCenter callbacks to a std::function<void()>.
*/
class FunctionNotificationCenterObserver
{
public:
    FunctionNotificationCenterObserver (NSNotificationName notificationName,
                                        id objectToObserve,
                                        std::function<void()> callback)
        : onNotification (std::move (callback)),
          observer (observerObject.get(), getSelector(), notificationName, objectToObserve)
    {}

private:
    struct ObserverClass
    {
        ObserverClass()
        {
            klass.addIvar<FunctionNotificationCenterObserver*> ("owner");

            klass.addMethod (getSelector(), [] (id self, SEL, NSNotification*)
            {
                getIvar<FunctionNotificationCenterObserver*> (self, "owner")->onNotification();
            });

            klass.registerClass();
        }

        NSObject* createInstance() const { return klass.createInstance(); }

    private:
        ObjCClass<NSObject> klass { "JUCEObserverClass_" };
    };

    static SEL getSelector()
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        return @selector (notificationFired:);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    std::function<void()> onNotification;

    NSUniquePtr<NSObject> observerObject
    {
        [this]
        {
            static ObserverClass observerClass;
            auto* result = observerClass.createInstance();
            object_setInstanceVariable (result, "owner", this);
            return result;
        }()
    };

    ScopedNotificationCenterObserver observer;

    // Instances can't be copied or moved, because 'this' is stored as a member of the ObserverClass
    // object.
    JUCE_DECLARE_NON_COPYABLE (FunctionNotificationCenterObserver)
    JUCE_DECLARE_NON_MOVEABLE (FunctionNotificationCenterObserver)
};

//==============================================================================
/*
    Manages the lifetime of a CVDisplayLinkRef for a single display, and automatically starts and
    stops it.
*/

// From macOS 15+, warnings suggest the CVDisplayLink functions can be replaced with
// NSView.displayLink(target:selector:), NSWindow.displayLink(target:selector:), or
// NSScreen.displayLink(target:selector:) all of which were only introduced in macOS 14+ however,
// it's not clear how these methods can be used to replace all use cases

JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

class ScopedDisplayLink
{
public:
    static CGDirectDisplayID getDisplayIdForScreen (NSScreen* screen)
    {
        return (CGDirectDisplayID) [[screen.deviceDescription objectForKey: @"NSScreenNumber"] unsignedIntegerValue];
    }

    ScopedDisplayLink (NSScreen* screenIn, std::function<void (double)> onCallbackIn)
        : displayId (getDisplayIdForScreen (screenIn)),
          link ([display = displayId]
          {
              CVDisplayLinkRef ptr = nullptr;
              [[maybe_unused]] const auto result = CVDisplayLinkCreateWithCGDisplay (display, &ptr);
              jassert (result == kCVReturnSuccess);
              jassert (ptr != nullptr);
              return ptr;
          }()),
          onCallback (std::move (onCallbackIn))
    {
        const auto callback = [] (CVDisplayLinkRef,
                                  const CVTimeStamp*,
                                  const CVTimeStamp* outputTime,
                                  CVOptionFlags,
                                  CVOptionFlags*,
                                  void* context) -> int
        {
            const auto outputTimeSec = (double) outputTime->videoTime / (double) outputTime->videoTimeScale;
            static_cast<const ScopedDisplayLink*> (context)->onCallback (outputTimeSec);
            return kCVReturnSuccess;
        };

        [[maybe_unused]] const auto callbackResult = CVDisplayLinkSetOutputCallback (link.get(), callback, this);
        jassert (callbackResult == kCVReturnSuccess);

        [[maybe_unused]] const auto startResult = CVDisplayLinkStart (link.get());
        jassert (startResult == kCVReturnSuccess);
    }

    ~ScopedDisplayLink() noexcept
    {
        if (link != nullptr)
            CVDisplayLinkStop (link.get());
    }

    CGDirectDisplayID getDisplayId() const { return displayId; }

    double getNominalVideoRefreshPeriodS() const
    {
        const auto nominalVideoRefreshPeriod = CVDisplayLinkGetNominalOutputVideoRefreshPeriod (link.get());

        if ((nominalVideoRefreshPeriod.flags & kCVTimeIsIndefinite) == 0)
            return (double) nominalVideoRefreshPeriod.timeValue / (double) nominalVideoRefreshPeriod.timeScale;

        return 0.0;
    }

private:
    struct DisplayLinkDestructor
    {
        void operator() (CVDisplayLinkRef ptr) const
        {
            if (ptr != nullptr)
                CVDisplayLinkRelease (ptr);
        }
    };

    CGDirectDisplayID displayId;
    std::unique_ptr<std::remove_pointer_t<CVDisplayLinkRef>, DisplayLinkDestructor> link;
    std::function<void (double)> onCallback;

    // Instances can't be copied or moved, because 'this' is passed as context to
    // CVDisplayLinkSetOutputCallback
    JUCE_DECLARE_NON_COPYABLE (ScopedDisplayLink)
    JUCE_DECLARE_NON_MOVEABLE (ScopedDisplayLink)
};

JUCE_END_IGNORE_DEPRECATION_WARNINGS

//==============================================================================
/*
    Holds a ScopedDisplayLink for each screen. When the screen configuration changes, the
    ScopedDisplayLinks will be recreated automatically to match the new configuration.
*/
class PerScreenDisplayLinks
{
public:
    PerScreenDisplayLinks()
    {
        refreshScreens();
    }

    using RefreshCallback = std::function<void (double)>;
    using Factory = std::function<RefreshCallback (CGDirectDisplayID)>;

    /*
        Automatically unregisters a CVDisplayLink callback factory when ~Connection() is called.
    */
    class Connection
    {
    public:
        Connection() = default;

        Connection (PerScreenDisplayLinks& linksIn, std::list<Factory>::const_iterator it)
            : links (&linksIn), iter (it) {}

        ~Connection() noexcept
        {
            if (links != nullptr)
                links->unregisterFactory (iter);
        }

        Connection (const Connection&) = delete;
        Connection& operator= (const Connection&) = delete;

        Connection (Connection&& other) noexcept
            : links (std::exchange (other.links, nullptr)), iter (other.iter) {}

        Connection& operator= (Connection&& other) noexcept
        {
            Connection { std::move (other) }.swap (*this);
            return *this;
        }

    private:
        void swap (Connection& other) noexcept
        {
            std::swap (other.links, links);
            std::swap (other.iter, iter);
        }

        PerScreenDisplayLinks* links = nullptr;
        std::list<Factory>::const_iterator iter;
    };

    /*  Stores the provided factory for as long as the returned Connection remains alive.

        Whenever the screen configuration changes, the factory function will be called for each
        screen. The RefreshCallback returned by the factory will be called every time that screen's
        display link callback fires.
    */
    [[nodiscard]] Connection registerFactory (Factory factory)
    {
        const ScopedLock lock (mutex);
        factories.push_front (std::move (factory));
        refreshScreens();
        return { *this, factories.begin() };
    }

    double getNominalVideoRefreshPeriodSForScreen (CGDirectDisplayID display) const
    {
        const ScopedLock lock (mutex);

        for (const auto& link : links)
            if (link.getDisplayId() == display)
                return link.getNominalVideoRefreshPeriodS();

        return 0.0;
    }

private:
    void unregisterFactory (std::list<Factory>::const_iterator iter)
    {
        const ScopedLock lock (mutex);
        factories.erase (iter);
        refreshScreens();
    }

    void refreshScreens()
    {
        auto newLinks = [&]
        {
            std::list<ScopedDisplayLink> result;

            for (NSScreen* screen in [NSScreen screens])
            {
                std::vector<RefreshCallback> callbacks;

                for (auto& factory : factories)
                    callbacks.push_back (factory (ScopedDisplayLink::getDisplayIdForScreen (screen)));

                // This is the callback that will actually fire in response to this screen's display
                // link callback.
                result.emplace_back (screen, [cbs = std::move (callbacks)] (double timestampSec)
                {
                    for (const auto& callback : cbs)
                        callback (timestampSec);
                });
            }

            return result;
        }();

        const ScopedLock lock (mutex);
        links = std::move (newLinks);
    }

    CriticalSection mutex;
    // This is a list rather than a vector so that the iterators are stable, even when items are
    // added/removed from the list. This is important because Connection objects store an iterator
    // internally, and may be created/destroyed arbitrarily.
    std::list<Factory> factories;
    // This is a list rather than a vector because ScopedDisplayLink is non-moveable.
    std::list<ScopedDisplayLink> links;

    FunctionNotificationCenterObserver screenParamsObserver { NSApplicationDidChangeScreenParametersNotification,
                                                              nullptr,
                                                              [this] { refreshScreens(); } };
};

} // namespace juce
