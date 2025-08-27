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

class VBlankThread : private Thread,
                     private AsyncUpdater
{
public:
    using VBlankListener = ComponentPeer::VBlankListener;

    VBlankThread (ComSmartPtr<IDXGIOutput> out,
                  HMONITOR mon,
                  VBlankListener& listener)
        : Thread (SystemStats::getJUCEVersion() + ": VBlankThread"),
          output (out),
          monitor (mon)
    {
        listeners.push_back (listener);
        startThread (Priority::highest);
    }

    ~VBlankThread() override
    {
        cancelPendingUpdate();

        state |= flagExit;

        stopThread (-1);
    }

    void updateMonitor()
    {
        monitor = getMonitorFromOutput (output);
    }

    HMONITOR getMonitor() const noexcept { return monitor; }

    void addListener (VBlankListener& listener)
    {
        listeners.push_back (listener);
    }

    bool removeListener (const VBlankListener& listener)
    {
        auto it = std::find_if (listeners.cbegin(),
                                listeners.cend(),
                                [&listener] (const auto& l) { return &(l.get()) == &listener; });

        if (it != listeners.cend())
        {
            listeners.erase (it);
            return true;
        }

        return false;
    }

    bool hasNoListeners() const noexcept
    {
        return listeners.empty();
    }

    bool hasListener (const VBlankListener& listener) const noexcept
    {
        return std::any_of (listeners.cbegin(),
                            listeners.cend(),
                            [&listener] (const auto& l) { return &(l.get()) == &listener; });
    }

    static HMONITOR getMonitorFromOutput (ComSmartPtr<IDXGIOutput> output)
    {
        if (output == nullptr)
            return nullptr;

        DXGI_OUTPUT_DESC desc = {};
        return (FAILED (output->GetDesc (&desc)) || ! desc.AttachedToDesktop)
                   ? nullptr
                   : desc.Monitor;
    }

private:
    //==============================================================================
    void run() override
    {
        for (;;)
        {
            if (output->WaitForVBlank() == S_OK)
            {
                const auto now = Time::getMillisecondCounterHiRes();

                if (now - lastVBlankEvent.exchange (now) < 1.0)
                    sleep (1);

                const auto stateToRead = state.fetch_or (flagPaintPending);

                if ((stateToRead & flagExit) != 0)
                    return;

                if ((stateToRead & flagPaintPending) != 0)
                    continue;

                triggerAsyncUpdate();
            }
            else
            {
                sleep (1);
            }
        }
    }

    void handleAsyncUpdate() override
    {
        const auto timestampSec = lastVBlankEvent / 1000.0;

        for (auto& listener : listeners)
            listener.get().onVBlank (timestampSec);

        state &= ~flagPaintPending;
    }

    enum Flags
    {
        flagExit = 1 << 0,
        flagPaintPending = 1 << 1,
    };

    //==============================================================================
    ComSmartPtr<IDXGIOutput> output;
    HMONITOR monitor = nullptr;
    std::vector<std::reference_wrapper<VBlankListener>> listeners;

    std::atomic<double> lastVBlankEvent{};
    std::atomic<int> state{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VBlankThread)
    JUCE_DECLARE_NON_MOVEABLE (VBlankThread)
};

//==============================================================================
class VBlankDispatcher final : public DeletedAtShutdown
{
public:
    void updateDisplay (ComponentPeer::VBlankListener& listener, HMONITOR monitor)
    {
        if (monitor == nullptr)
        {
            removeListener (listener);
            return;
        }

        auto threadWithListener = threads.end();
        auto threadWithMonitor  = threads.end();

        for (auto it = threads.begin(); it != threads.end(); ++it)
        {
            if ((*it)->hasListener (listener))
                threadWithListener = it;

            if ((*it)->getMonitor() == monitor)
                threadWithMonitor = it;

            if (threadWithListener != threads.end()
                && threadWithMonitor != threads.end())
            {
                if (threadWithListener == threadWithMonitor)
                    return;

                (*threadWithMonitor)->addListener (listener);

                // This may invalidate iterators, so be careful!
                removeListener (threadWithListener, listener);
                return;
            }
        }

        if (threadWithMonitor != threads.end())
        {
            (*threadWithMonitor)->addListener (listener);
            return;
        }

        if (threadWithListener != threads.end())
            removeListener (threadWithListener, listener);

        for (const auto& adapter : directX->adapters.getAdapterArray())
        {
            for (UINT i = 0;; ++i)
            {
                ComSmartPtr<IDXGIOutput> output;
                const auto result = adapter->dxgiAdapter->EnumOutputs (i, output.resetAndGetPointerAddress());

                if (result == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
                    break;

                if (result == DXGI_ERROR_NOT_FOUND)
                    break;

                if (VBlankThread::getMonitorFromOutput (output) != monitor)
                    continue;

                threads.emplace_back (std::make_unique<VBlankThread> (output, monitor, listener));
                break;
            }
        }
    }

    void removeListener (const ComponentPeer::VBlankListener& listener)
    {
        for (auto it = threads.begin(); it != threads.end(); ++it)
            if (removeListener (it, listener))
                return;
    }

    void reconfigureDisplays()
    {
        directX->adapters.updateAdapters();

        for (auto& thread : threads)
            thread->updateMonitor();

        threads.erase (std::remove_if (threads.begin(),
                                       threads.end(),
                                       [] (const auto& thread) { return thread->getMonitor() == nullptr; }),
                       threads.end());
    }

    JUCE_DECLARE_SINGLETON_SINGLETHREADED_INLINE (VBlankDispatcher, false)

private:
    //==============================================================================
    using Threads = std::vector<std::unique_ptr<VBlankThread>>;

    VBlankDispatcher()
    {
        reconfigureDisplays();
    }

    ~VBlankDispatcher() override
    {
        threads.clear();
        clearSingletonInstance();
    }

    // This may delete the corresponding thread and invalidate iterators,
    // so be careful!
    bool removeListener (Threads::iterator it, const ComponentPeer::VBlankListener& listener)
    {
        if ((*it)->removeListener (listener))
        {
            if ((*it)->hasNoListeners())
                threads.erase (it);

            return true;
        }

        return false;
    }

    //==============================================================================
    Threads threads;
    SharedResourcePointer<DirectX> directX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VBlankDispatcher)
    JUCE_DECLARE_NON_MOVEABLE (VBlankDispatcher)
};

} // namespace juce
