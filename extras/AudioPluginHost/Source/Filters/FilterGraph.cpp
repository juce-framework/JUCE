/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "../UI/MainHostWindow.h"
#include "FilterGraph.h"
#include "InternalFilters.h"
#include "../UI/GraphEditorPanel.h"


//==============================================================================
FilterGraph::FilterGraph (AudioPluginFormatManager& fm, KnownPluginList& kpl)
    : FileBasedDocument (getFilenameSuffix(),
                         getFilenameWildcard(),
                         "Load a filter graph",
                         "Save a filter graph"),
      formatManager (fm), knownPluginList(kpl)
{
    newDocument();

    graph.addListener (this);
    graph.addChangeListener (this);

    setChangedFlag (false);
}

FilterGraph::~FilterGraph()
{
    graph.removeListener (this);
    graph.removeChangeListener (this);
    graph.clear();
}

FilterGraph::NodeID FilterGraph::getNextUID() noexcept
{
    return ++lastUID;
}

//==============================================================================
void FilterGraph::changeListenerCallback (ChangeBroadcaster*)
{
    changed();

    for (int i = activePluginWindows.size(); --i >= 0;)
        if (! graph.getNodes().contains (activePluginWindows.getUnchecked(i)->node))
            activePluginWindows.remove (i);
}

void FilterGraph::addPlugin (const PluginDescription& desc, Point<double> p)
{
    struct AsyncCallback : public AudioPluginFormat::InstantiationCompletionCallback
    {
        AsyncCallback (FilterGraph& g, Point<double> pos)  : owner (g), position (pos)
        {}

        void completionCallback (AudioPluginInstance* instance, const String& error) override
        {
            owner.addFilterCallback (instance, error, position);
        }

        FilterGraph& owner;
        Point<double> position;
    };

    formatManager.createPluginInstanceAsync (desc,
                                             graph.getSampleRate(),
                                             graph.getBlockSize(),
                                             new AsyncCallback (*this, p));
}

void FilterGraph::addFilterCallback (AudioPluginInstance* instance, const String& error, Point<double> pos)
{
    if (instance == nullptr)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          TRANS("Couldn't create filter"),
                                          error);
    }
    else
    {
        instance->enableAllBuses();

        if (auto node = graph.addNode (instance))
        {
            node->properties.set ("x", pos.x);
            node->properties.set ("y", pos.y);
            changed();
        }
    }
}


//==============================================================================
void FilterGraph::clear()
{
    closeAnyOpenPluginWindows();
    graph.clear();
    changed();
}

PluginWindow* FilterGraph::getOrCreateWindowFor (AudioProcessorGraph::Node* node, PluginWindow::Type type)
{
    jassert (node != nullptr);

   #if JUCE_IOS || JUCE_ANDROID
    closeAnyOpenPluginWindows();
   #else
    for (auto* w : activePluginWindows)
        if (w->node == node && w->type == type)
            return w;
   #endif

    if (auto* processor = node->getProcessor())
    {
        if (auto* plugin = dynamic_cast<AudioPluginInstance*> (processor))
        {
            auto description = plugin->getPluginDescription();

            if (description.pluginFormatName == "Internal")
            {
                getCommandManager().invokeDirectly (CommandIDs::showAudioSettings, false);
                return nullptr;
            }
        }

        return activePluginWindows.add (new PluginWindow (node, type, activePluginWindows));
    }

    return nullptr;
}

bool FilterGraph::closeAnyOpenPluginWindows()
{
    bool wasEmpty = activePluginWindows.isEmpty();
    activePluginWindows.clear();
    return ! wasEmpty;
}

//==============================================================================
String FilterGraph::getDocumentTitle()
{
    if (! getFile().exists())
        return "Unnamed";

    return getFile().getFileNameWithoutExtension();
}

void FilterGraph::newDocument()
{
    clear();
    setFile ({});

    InternalPluginFormat internalFormat;
    String errorMessage;
    m_midiInNode = graph.addNode(formatManager.createPluginInstance(internalFormat.midiInDesc, graph.getSampleRate(), graph.getBlockSize(), errorMessage));
    m_audioOutNode = graph.addNode(formatManager.createPluginInstance(internalFormat.audioOutDesc, graph.getSampleRate(), graph.getBlockSize(), errorMessage));

    setChangedFlag (false);
}

Result FilterGraph::loadDocument (const File& file)
{
    clear();

    InternalPluginFormat internalFormat;
    String errorMessage;
    m_midiInNode = graph.addNode(formatManager.createPluginInstance(internalFormat.midiInDesc, graph.getSampleRate(), graph.getBlockSize(), errorMessage));
    m_audioOutNode = graph.addNode(formatManager.createPluginInstance(internalFormat.audioOutDesc, graph.getSampleRate(), graph.getBlockSize(), errorMessage));

    XmlArchive::Load(file.getFullPathName().getCharPointer(), m_performer);
    m_performer.ResolveIDs();

    if (m_performer.Root.Racks.Rack.size() == 0)
        return Result::fail("No racks");

    for (auto i = 0U; i < m_performer.Root.Racks.Rack.size(); ++i)
    {
        auto &rack = m_performer.Root.Racks.Rack[i];

        PluginDescription pd;
        pd.name = rack.PluginName;
        pd.pluginFormatName = "VST";
        pd.isInstrument = true;
        for (auto j = 0U; j < (unsigned)knownPluginList.getNumTypes(); ++j)
        {
            auto name = knownPluginList.getType(j)->name;
            if (name.compareIgnoreCase(pd.name)==0 || name.compareIgnoreCase(pd.name.removeCharacters(" "))==0 || name.compareIgnoreCase(pd.name + " VSTi") == 0)
            {
                pd.fileOrIdentifier = knownPluginList.getType(j)->fileOrIdentifier;
            }
        }

        if (auto* instance = formatManager.createPluginInstance(pd, graph.getSampleRate(), graph.getBlockSize(), errorMessage))
        {
            auto node = graph.addNode(instance, (NodeID)rack.ID);

            auto gain = graph.addNode(formatManager.createPluginInstance(internalFormat.gainDesc, graph.getSampleRate(), graph.getBlockSize(), errorMessage));

            // State stuff for later
            //MemoryBlock m;
            //m.fromBase64Encoding(char*);
            //node->getProcessor()->setStateInformation(m.getData(), (int)m.getSize()); 

            rack.m_node = (void*)node.get();
            rack.m_gainNode = (void*)gain.get();

            graph.addConnection({ { m_midiInNode->nodeID, 4096 },{ node->nodeID, 4096 } });

            graph.addConnection({ { node->nodeID, 0 },{ gain->nodeID, 0 } });
            graph.addConnection({ { node->nodeID, 1 },{ gain->nodeID, 1 } });

            graph.addConnection({ { gain->nodeID, 0 },{ m_audioOutNode->nodeID, 0 } });
            graph.addConnection({ { gain->nodeID, 1 },{ m_audioOutNode->nodeID, 1 } });
        }
    }

    changed();

    return Result::ok();
}

Result FilterGraph::saveDocument (const File& file)
{
    XmlArchive::Save(file.getFullPathName().getCharPointer(), m_performer);
    return Result::ok();
}

File FilterGraph::getLastDocumentOpened()
{
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                        ->getValue ("recentFilterGraphFiles"));

    return recentFiles.getFile (0);
}

void FilterGraph::setLastDocumentOpened (const File& file)
{
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                        ->getValue ("recentFilterGraphFiles"));

    recentFiles.addFile (file);

    getAppProperties().getUserSettings()
        ->setValue ("recentFilterGraphFiles", recentFiles.toString());
}

void FilterGraph::Import(const char *filename)
{
    m_performer.Import(filename);
}