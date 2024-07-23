/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
    /** Manages a set of ChildProcesses and periodically checks their return value. Upon completion
        it calls listeners added with addChildProcessExitedListener().

        This class is mostly aimed for usage on Linux, where terminated child processes are only
        cleaned up if their return code is read after termination. In order to ensure this one needs
        to call ChildProcess::isFinished() until it returns false or
        ChildProcess::waitForProcessToFinish() until it returns true.

        This class will keep querying the return code on a Timer thread until the process
        terminates. This can be handy if one wants to start and stop multiple ChildProcesses on
        Linux that could take a long time to complete.

        Since this class uses a Timer to check subprocess status, it's generally only safe to
        access the returned ChildProcesses from the message thread.

        @see ChildProcessManagerSingleton

        @tags{Events}
    */
    class JUCE_API  ChildProcessManager final : private DeletedAtShutdown
    {
    public:
       #ifndef DOXYGEN
        JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (ChildProcessManager)
       #endif

        /** Creates a new ChildProcess and starts it with the provided arguments.

            The arguments are the same as the overloads to ChildProcess::start().

            The manager will keep the returned ChildProcess object alive until it terminates and its
            return value has been queried. Calling ChildProcess::kill() on the returned object will
            eventually cause its removal from the ChildProcessManager after it terminates.
        */
        template <typename... Args>
        std::shared_ptr<ChildProcess> createAndStartManagedChildProcess (Args&&... args)
        {
            auto p = std::make_shared<ChildProcess>();

            if (! p->start (std::forward<Args> (args)...))
                return nullptr;

            processes.insert (p);
            timer.startTimer (1000);

            return p;
        }

        /** Registers a callback function that is called for every ChildProcess that terminated.

            This registration is deleted when the returned ErasedScopedGuard is deleted.
        */
        auto addChildProcessExitedListener (std::function<void (ChildProcess*)> listener)
        {
            return listeners.addListener (std::move (listener));
        }

        /** Returns true if the ChildProcessManager contains any running ChildProcesses that it's
            monitoring.
        */
        auto hasRunningProcess() const
        {
            return timer.isTimerRunning();
        }

    private:
        ChildProcessManager() = default;
        ~ChildProcessManager() override  { clearSingletonInstance(); }

        void checkProcesses();

        std::set<std::shared_ptr<ChildProcess>> processes;
        detail::CallbackListenerList<ChildProcess*> listeners;
        TimedCallback timer { [this] { checkProcesses(); } };
    };

} // namespace juce
