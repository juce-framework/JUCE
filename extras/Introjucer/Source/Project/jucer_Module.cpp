/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "jucer_Module.h"
#include "jucer_ProjectType.h"
#include "jucer_ProjectExporter.h"
#include "jucer_ProjectSaver.h"
#include "jucer_AudioPluginModule.h"
#include "jucer_JuceLibraryModule.h"


//==============================================================================
ModuleList::ModuleList()
{
    rescan();
}

ModuleList& ModuleList::getInstance()
{
    static ModuleList list;
    return list;
}

struct ModuleSorter
{
    static int compareElements (const ModuleList::Module* m1, const ModuleList::Module* m2)
    {
        return m1->uid.compareIgnoreCase (m2->uid);
    }
};

void ModuleList::rescan()
{
    modules.clear();

    moduleFolder = StoredSettings::getInstance()->getLastKnownJuceFolder().getChildFile ("modules");

    DirectoryIterator iter (moduleFolder, false, "*", File::findDirectories);

    while (iter.next())
    {
        const File moduleDef (iter.getFile().getChildFile ("juce_module_info"));

        if (moduleDef.exists())
        {
            ScopedPointer<JuceLibraryModule> m (new JuceLibraryModule (moduleDef));
            jassert (m->isValid());

            if (m->isValid())
            {
                Module* info = new Module();
                modules.add (info);

                info->uid = m->getID();
                info->name = m->moduleInfo ["name"];
                info->description = m->moduleInfo ["description"];
                info->file = moduleDef;
            }
        }
    }

    ModuleSorter sorter;
    modules.sort (sorter);
}

LibraryModule* ModuleList::Module::create() const
{
    return new JuceLibraryModule (file);
}

LibraryModule* ModuleList::loadModule (const String& uid) const
{
    const Module* const m = findModuleInfo (uid);

    return m != nullptr ? m->create() : nullptr;
}

const ModuleList::Module* ModuleList::findModuleInfo (const String& uid) const
{
    for (int i = modules.size(); --i >= 0;)
        if (modules.getUnchecked(i)->uid == uid)
            return modules.getUnchecked(i);

    return nullptr;
}

void ModuleList::getDependencies (const String& moduleID, StringArray& dependencies) const
{
    ScopedPointer<LibraryModule> lib (loadModule (moduleID));
    JuceLibraryModule* m = dynamic_cast<JuceLibraryModule*> (static_cast <LibraryModule*> (lib));

    if (m != nullptr)
    {
        const var depsArray (m->moduleInfo ["dependencies"]);
        const Array<var>* const deps = depsArray.getArray();

        for (int i = 0; i < deps->size(); ++i)
        {
            const var& d = deps->getReference(i);

            String uid (d ["id"].toString());
            String version (d ["version"].toString());

            if (! dependencies.contains (uid, true))
            {
                dependencies.add (uid);
                getDependencies (uid, dependencies);
            }
        }
    }
}

void ModuleList::createDependencies (const String& moduleID, OwnedArray<LibraryModule>& modules) const
{
    ScopedPointer<LibraryModule> lib (loadModule (moduleID));
    JuceLibraryModule* m = dynamic_cast<JuceLibraryModule*> (static_cast <LibraryModule*> (lib));

    if (m != nullptr)
    {
        var depsArray (m->moduleInfo ["dependencies"]);
        const Array<var>* const deps = depsArray.getArray();

        for (int i = 0; i < deps->size(); ++i)
        {
            const var& d = deps->getReference(i);

            String uid (d ["id"].toString());
            String version (d ["version"].toString());

            //xxx to do - also need to find version conflicts
            jassertfalse

        }
    }
}
