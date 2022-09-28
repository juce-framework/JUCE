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
class ScopedDisplayLink
{
public:
    static CGDirectDisplayID getDisplayIdForScreen (NSScreen* screen)
    {
        return (CGDirectDisplayID) [[screen.deviceDescription objectForKey: @"NSScreenNumber"] unsignedIntegerValue];
    }

    ScopedDisplayLink (NSScreen* screenIn, std::function<void()> onCallbackIn)
        : displayId (getDisplayIdForScreen (screenIn)),
          link ([display = displayId]
          {
              CVDisplayLinkRef ptr = nullptr;
              const auto result = CVDisplayLinkCreateWithCGDisplay (display, &ptr);
              jassertquiet (result == kCVReturnSuccess);
              jassertquiet (ptr != nullptr);
              return ptr;
          }()),
          onCallback (std::move (onCallbackIn))
    {
        const auto callback = [] (CVDisplayLinkRef,
                                  const CVTimeStamp*,
                                  const CVTimeStamp*,
                                  CVOptionFlags,
                                  CVOptionFlags*,
                                  void* context) -> int
        {
            static_cast<const ScopedDisplayLink*> (context)->onCallback();
            return kCVReturnSuccess;
        };

        const auto callbackResult = CVDisplayLinkSetOutputCallback (link.get(), callback, this);
        jassertquiet (callbackResult == kCVReturnSuccess);

        const auto startResult = CVDisplayLinkStart (link.get());
        jassertquiet (startResult == kCVReturnSuccess);
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
    std::function<void()> onCallback;

    // Instances can't be copied or moved, because 'this' is passed as context to
    // CVDisplayLinkSetOutputCallback
    JUCE_DECLARE_NON_COPYABLE (ScopedDisplayLink)
    JUCE_DECLARE_NON_MOVEABLE (ScopedDisplayLink)
};

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

    using RefreshCallback = std::function<void()>;
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
                result.emplace_back (screen, [callbacks = std::move (callbacks)]
                {
                    for (const auto& callback : callbacks)
                        callback();
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
