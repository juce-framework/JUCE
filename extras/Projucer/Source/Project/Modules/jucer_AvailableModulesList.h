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

#include "jucer_ModuleDescription.h"

#include <future>

//==============================================================================
class AvailableModulesList final : private AsyncUpdater
{
public:
    using ModuleIDAndFolder     = std::pair<String, File>;
    using ModuleIDAndFolderList = std::vector<ModuleIDAndFolder>;

    AvailableModulesList() = default;

    //==============================================================================
    void scanPaths (const Array<File>& paths)
    {
        scanPathsAsync (paths);
        scanner = {};
    }

    void scanPathsAsync (const Array<File>& paths)
    {
        scanner = std::async (std::launch::async, [this, paths]
        {
            ModuleIDAndFolderList list;

            for (auto& p : paths)
                addAllModulesInFolder (p, list);

            std::sort (list.begin(), list.end(), [] (const ModuleIDAndFolder& m1,
                                                     const ModuleIDAndFolder& m2)
            {
                return m1.first.compareIgnoreCase (m2.first) < 0;
            });

            {
                const ScopedLock swapLock (lock);

                if (list == modulesList)
                    return;

                modulesList.swap (list);
            }

            triggerAsyncUpdate();
        });
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
    static bool tryToAddModuleFromFolder (const File& path, ModuleIDAndFolderList& list)
    {
        ModuleDescription m (path);

        if (m.isValid()
            && std::none_of (list.begin(), list.end(),
                             [&m] (const ModuleIDAndFolder& element) { return element.first == m.getID(); }))
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

                pathsToCheck.push ({ iter.getFile(), path.depth + 1 });
            }
        }
    }

    //==============================================================================
    void handleAsyncUpdate() override
    {
        listeners.call ([this] (Listener& l) { l.availableModulesChanged (this); });
    }

    //==============================================================================
    ModuleIDAndFolderList modulesList;
    ListenerList<Listener> listeners;
    CriticalSection lock;
    std::future<void> scanner;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AvailableModulesList)
};
