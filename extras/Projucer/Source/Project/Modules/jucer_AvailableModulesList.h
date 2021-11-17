/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ModuleDescription.h"

//==============================================================================
class AvailableModulesList  : private AsyncUpdater
{
public:
    using ModuleIDAndFolder     = std::pair<String, File>;
    using ModuleIDAndFolderList = std::vector<ModuleIDAndFolder>;

    AvailableModulesList() = default;

    //==============================================================================
    void scanPaths (const Array<File>& paths)
    {
        auto job = createScannerJob (paths);
        auto& ref = *job;

        removePendingAndAddJob (std::move (job));
        scanPool.waitForJobToFinish (&ref, -1);
    }

    void scanPathsAsync (const Array<File>& paths)
    {
        removePendingAndAddJob (createScannerJob (paths));
    }

    //==============================================================================
    ModuleIDAndFolderList getAllModules() const
    {
        const ScopedLock readLock (lock);
        return modulesList;
    }

    ModuleIDAndFolder getModuleWithID (const String& id) const
    {
        const ScopedLock readLock (lock);

        for (auto& mod : modulesList)
            if (mod.first == id)
                return mod;

        return {};
    }

    //==============================================================================
    void removeDuplicates (const ModuleIDAndFolderList& other)
    {
        const ScopedLock readLock (lock);

        const auto predicate = [&] (const ModuleIDAndFolder& entry)
        {
            return std::find (other.begin(), other.end(), entry) != other.end();
        };

        modulesList.erase (std::remove_if (modulesList.begin(), modulesList.end(), predicate),
                           modulesList.end());
    }

    //==============================================================================
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void availableModulesChanged (AvailableModulesList* listThatHasChanged) = 0;
    };

    void addListener (Listener* listenerToAdd)          { listeners.add (listenerToAdd); }
    void removeListener (Listener* listenerToRemove)    { listeners.remove (listenerToRemove); }

private:
    //==============================================================================
    struct ModuleScannerJob  : public ThreadPoolJob
    {
        ModuleScannerJob (const Array<File>& paths,
                          std::function<void (const ModuleIDAndFolderList&)>&& callback)
            : ThreadPoolJob ("ModuleScannerJob"),
              pathsToScan (paths),
              completionCallback (std::move (callback))
        {
        }

        JobStatus runJob() override
        {
            ModuleIDAndFolderList list;

            for (auto& p : pathsToScan)
                addAllModulesInFolder (p, list);

            if (! shouldExit())
            {
                std::sort (list.begin(), list.end(), [] (const ModuleIDAndFolder& m1,
                                                         const ModuleIDAndFolder& m2)
                {
                    return m1.first.compareIgnoreCase (m2.first) < 0;
                });

                completionCallback (list);
            }

            return jobHasFinished;
        }

        static bool tryToAddModuleFromFolder (const File& path, ModuleIDAndFolderList& list)
        {
            ModuleDescription m (path);

            if (m.isValid()
                && std::find_if (list.begin(), list.end(),
                                 [&m] (const ModuleIDAndFolder& element) { return element.first == m.getID(); }) == std::end (list))
            {
                list.push_back ({ m.getID(), path });
                return true;
            }

            return false;
        }

        static void addAllModulesInFolder (const File& topLevelPath, ModuleIDAndFolderList& list)
        {
            struct FileAndDepth
            {
                File file;
                int depth;
            };

            std::queue<FileAndDepth> pathsToCheck;
            pathsToCheck.push ({ topLevelPath, 0 });

            while (! pathsToCheck.empty())
            {
                const auto path = pathsToCheck.front();
                pathsToCheck.pop();

                if (tryToAddModuleFromFolder (path.file, list) || path.depth == 3)
                    continue;

                for (const auto& iter : RangedDirectoryIterator (path.file, false, "*", File::findDirectories))
                {
                    if (auto* job = ThreadPoolJob::getCurrentThreadPoolJob())
                        if (job->shouldExit())
                            return;

                    pathsToCheck.push({ iter.getFile(), path.depth + 1 });
                }
            }
        }

        Array<File> pathsToScan;
        std::function<void (const ModuleIDAndFolderList&)> completionCallback;
    };

    //==============================================================================
    void handleAsyncUpdate() override
    {
        listeners.call ([this] (Listener& l) { l.availableModulesChanged (this); });
    }

    std::unique_ptr<ThreadPoolJob> createScannerJob (const Array<File>& paths)
    {
        return std::make_unique<ModuleScannerJob> (paths, [this] (ModuleIDAndFolderList scannedModulesList)
        {
            if (scannedModulesList == modulesList)
                return;

            {
                const ScopedLock swapLock (lock);
                modulesList.swap (scannedModulesList);
            }

            triggerAsyncUpdate();
        });
    }

    void removePendingAndAddJob (std::unique_ptr<ThreadPoolJob> jobToAdd)
    {
        scanPool.removeAllJobs (false, 100);
        scanPool.addJob (jobToAdd.release(), true);
    }

    //==============================================================================
    ThreadPool scanPool { 1 };

    ModuleIDAndFolderList modulesList;
    ListenerList<Listener> listeners;
    CriticalSection lock;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AvailableModulesList)
};
