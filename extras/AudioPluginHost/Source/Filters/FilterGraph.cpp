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
FilterGraph::FilterGraph (AudioPluginFormatManager& fm)
    : FileBasedDocument (getFilenameSuffix(),
                         getFilenameWildcard(),
                         "Load a filter graph",
                         "Save a filter graph"),
      formatManager (fm)
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

AudioProcessorGraph::Node::Ptr FilterGraph::getNodeForName (const String& name) const
{
    for (auto* node : graph.getNodes())
        if (auto p = node->getProcessor())
            if (p->getName().equalsIgnoreCase (name))
                return node;

    return nullptr;
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

void FilterGraph::setNodePosition (NodeID nodeID, Point<double> pos)
{
    if (auto* n = graph.getNodeForId (nodeID))
    {
        n->properties.set ("x", jlimit (0.0, 1.0, pos.x));
        n->properties.set ("y", jlimit (0.0, 1.0, pos.y));
    }
}

Point<double> FilterGraph::getNodePosition (NodeID nodeID) const
{
    if (auto* n = graph.getNodeForId (nodeID))
        return { static_cast<double> (n->properties ["x"]),
                 static_cast<double> (n->properties ["y"]) };

    return {};
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

    addPlugin (internalFormat.audioInDesc,  { 0.5,  0.1 });
    addPlugin (internalFormat.midiInDesc,   { 0.25, 0.1 });
    addPlugin (internalFormat.audioOutDesc, { 0.5,  0.9 });

    setChangedFlag (false);
}

Result FilterGraph::loadDocument (const File& file)
{
    clear();

    InternalPluginFormat internalFormat;
    addPlugin(internalFormat.midiInDesc, { 0,0 });
    addPlugin(internalFormat.audioOutDesc, { 0,0 });

    XmlArchive::Load(file.getFullPathName().getCharPointer(), m_performer);
    for (int i = 0; i < 1/*m_performer.Root.Racks.Rack.size()*/; ++i)
    {
        auto &rack = m_performer.Root.Racks.Rack[i];

        PluginDescription pd;
        pd.name = rack.PluginName;
        pd.pluginFormatName = "VST";
        pd.isInstrument = true;
        pd.fileOrIdentifier = rack.PluginFile;

        String errorMessage;

        if (auto* instance = formatManager.createPluginInstance(pd, graph.getSampleRate(), graph.getBlockSize(), errorMessage))
        {
            // not sure about this stuff, dont need the addBus for VST
            //auto layout = instance->getBusesLayout();
            //layout.inputBuses.add(instance->getChannelLayoutOfBus(true, 0));
            //layout.outputBuses.add(instance->getChannelLayoutOfBus(false, 0));
            //layout.outputBuses.add(instance->getChannelLayoutOfBus(false, 1));
            //instance->setBusesLayout(layout);

            auto node = graph.addNode(instance, (NodeID)rack.ID);

            // State stuff for later
            //MemoryBlock m;
            //m.fromBase64Encoding(char*);
            //node->getProcessor()->setStateInformation(m.getData(), (int)m.getSize());

            //node->properties.set("x", 0);
            //node->properties.set("y", 0);

            if (auto w = getOrCreateWindowFor(node, PluginWindow::Type::normal))
                w->toFront(true);
        }
    }



    changed();

    auto test = graph.addConnection({ { (NodeID)m_performer.Root.Racks.Rack[0].ID, 0 }, { 0, 0 } });


    //graph.addConnection({ { (NodeID)e->getIntAttribute("srcFilter"), e->getIntAttribute("srcChannel") }, { (NodeID)e->getIntAttribute("dstFilter"), e->getIntAttribute("dstChannel") } });

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


File FilterGraph::getDefaultGraphDocumentOnMobile()
{
    auto persistantStorageLocation = File::getSpecialLocation (File::userApplicationDataDirectory);
    return persistantStorageLocation.getChildFile ("state.filtergraph");
}
