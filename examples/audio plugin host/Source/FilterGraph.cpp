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
#include "MainHostWindow.h"
#include "FilterGraph.h"
#include "InternalFilters.h"
#include "GraphEditorPanel.h"


//==============================================================================
const int FilterGraph::midiChannelNumber = 0x1000;

FilterGraph::FilterGraph (AudioPluginFormatManager& fm)
    : FileBasedDocument (filenameSuffix,
                         filenameWildcard,
                         "Load a filter graph",
                         "Save a filter graph"),
      formatManager (fm)
{
    InternalPluginFormat internalFormat;

    addFilter (internalFormat.audioInDesc,  { 0.5,  0.1 });
    addFilter (internalFormat.midiInDesc,   { 0.25, 0.1 });
    addFilter (internalFormat.audioOutDesc, { 0.5,  0.9 });

    graph.addListener (this);

    setChangedFlag (false);
}

FilterGraph::~FilterGraph()
{
    graph.addListener (this);
    graph.clear();
}

uint32 FilterGraph::getNextUID() noexcept
{
    return ++lastUID;
}

//==============================================================================
int FilterGraph::getNumFilters() const noexcept
{
    return graph.getNumNodes();
}

AudioProcessorGraph::Node::Ptr FilterGraph::getNode (int index) const noexcept
{
    return graph.getNode (index);
}

AudioProcessorGraph::Node::Ptr FilterGraph::getNodeForId (uint32 uid) const
{
    return graph.getNodeForId (uid);
}

AudioProcessorGraph::Node::Ptr FilterGraph::getNodeForName (const String& name) const
{
    for (int i = 0; i < graph.getNumNodes(); i++)
        if (auto node = graph.getNode (i))
            if (auto p = node->getProcessor())
                if (p->getName().equalsIgnoreCase (name))
                    return node;

    return nullptr;
}

void FilterGraph::addFilter (const PluginDescription& desc, Point<double> p)
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

    formatManager.createPluginInstanceAsync (desc, graph.getSampleRate(), graph.getBlockSize(),
                                             new AsyncCallback (*this, p));
}

void FilterGraph::addFilterCallback (AudioPluginInstance* instance, const String& error, Point<double> pos)
{
    if (instance == nullptr)
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     TRANS("Couldn't create filter"),
                                     error);
    }
    else
    {
        instance->enableAllBuses();

        if (auto* node = graph.addNode (instance))
        {
            node->properties.set ("x", pos.x);
            node->properties.set ("y", pos.y);
            changed();
        }
    }
}

void FilterGraph::removeFilter (const uint32 id)
{
    PluginWindow::closeCurrentlyOpenWindowsFor (id);

    if (graph.removeNode (id))
        changed();
}

void FilterGraph::disconnectFilter (const uint32 id)
{
    if (graph.disconnectNode (id))
        changed();
}

void FilterGraph::removeIllegalConnections()
{
    if (graph.removeIllegalConnections())
        changed();
}

void FilterGraph::setNodePosition (const uint32 nodeId, double x, double y)
{
    if (AudioProcessorGraph::Node::Ptr n = graph.getNodeForId (nodeId))
    {
        n->properties.set ("x", jlimit (0.0, 1.0, x));
        n->properties.set ("y", jlimit (0.0, 1.0, y));
    }
}

Point<double> FilterGraph::getNodePosition (const uint32 nodeId) const
{
    if (auto n = graph.getNodeForId (nodeId))
        return { static_cast<double> (n->properties ["x"]),
                 static_cast<double> (n->properties ["y"]) };

    return {};
}

//==============================================================================
int FilterGraph::getNumConnections() const noexcept
{
    return graph.getNumConnections();
}

const AudioProcessorGraph::Connection* FilterGraph::getConnection (const int index) const noexcept
{
    return graph.getConnection (index);
}

const AudioProcessorGraph::Connection* FilterGraph::getConnectionBetween (uint32 sourceFilterUID, int sourceFilterChannel,
                                                                          uint32 destFilterUID, int destFilterChannel) const noexcept
{
    return graph.getConnectionBetween (sourceFilterUID, sourceFilterChannel,
                                       destFilterUID, destFilterChannel);
}

bool FilterGraph::canConnect (uint32 sourceFilterUID, int sourceFilterChannel,
                              uint32 destFilterUID, int destFilterChannel) const noexcept
{
    return graph.canConnect (sourceFilterUID, sourceFilterChannel,
                             destFilterUID, destFilterChannel);
}

bool FilterGraph::addConnection (uint32 sourceFilterUID, int sourceFilterChannel,
                                 uint32 destFilterUID, int destFilterChannel)
{
    const bool result = graph.addConnection (sourceFilterUID, sourceFilterChannel,
                                             destFilterUID, destFilterChannel);

    if (result)
        changed();

    return result;
}

void FilterGraph::removeConnection (const int index)
{
    graph.removeConnection (index);
    changed();
}

void FilterGraph::removeConnection (uint32 sourceFilterUID, int sourceFilterChannel,
                                    uint32 destFilterUID, int destFilterChannel)
{
    if (graph.removeConnection (sourceFilterUID, sourceFilterChannel,
                                destFilterUID, destFilterChannel))
        changed();
}

void FilterGraph::clear()
{
    PluginWindow::closeAllCurrentlyOpenWindows();

    graph.clear();
    changed();
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

    addFilter (internalFormat.audioInDesc,  { 0.5,  0.1 });
    addFilter (internalFormat.midiInDesc,   { 0.25, 0.1 });
    addFilter (internalFormat.audioOutDesc, { 0.5,  0.9 });

    setChangedFlag (false);
}

Result FilterGraph::loadDocument (const File& file)
{
    XmlDocument doc (file);
    ScopedPointer<XmlElement> xml (doc.getDocumentElement());

    if (xml == nullptr || ! xml->hasTagName ("FILTERGRAPH"))
        return Result::fail ("Not a valid filter graph file");

    restoreFromXml (*xml);
    return Result::ok();
}

Result FilterGraph::saveDocument (const File& file)
{
    ScopedPointer<XmlElement> xml (createXml());

    if (! xml->writeToFile (file, String()))
        return Result::fail ("Couldn't write to the file");

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

//==============================================================================
static void readBusLayoutFromXml (AudioProcessor::BusesLayout& busesLayout, AudioProcessor* plugin, const XmlElement& xml, const bool isInput)
{
    Array<AudioChannelSet>& targetBuses = (isInput ? busesLayout.inputBuses : busesLayout.outputBuses);
    int maxNumBuses = 0;

    if (auto* buses = xml.getChildByName (isInput ? "INPUTS" : "OUTPUTS"))
    {
        forEachXmlChildElementWithTagName (*buses, e, "BUS")
        {
            const int busIdx = e->getIntAttribute ("index");
            maxNumBuses = jmax (maxNumBuses, busIdx + 1);

            // the number of buses on busesLayout may not be in sync with the plugin after adding buses
            // because adding an input bus could also add an output bus
            for (int actualIdx = plugin->getBusCount (isInput) - 1; actualIdx < busIdx; ++actualIdx)
                if (! plugin->addBus (isInput)) return;

            for (int actualIdx = targetBuses.size() - 1; actualIdx < busIdx; ++actualIdx)
                targetBuses.add (plugin->getChannelLayoutOfBus (isInput, busIdx));

            const String& layout = e->getStringAttribute("layout");

            if (layout.isNotEmpty())
                targetBuses.getReference (busIdx) = AudioChannelSet::fromAbbreviatedString (layout);
        }
    }

    // if the plugin has more buses than specified in the xml, then try to remove them!
    while (maxNumBuses < targetBuses.size())
    {
        if (! plugin->removeBus (isInput))
            return;

        targetBuses.removeLast();
    }
}

//==============================================================================
static XmlElement* createBusLayoutXml (const AudioProcessor::BusesLayout& layout, const bool isInput)
{
    const Array<AudioChannelSet>& buses = (isInput ? layout.inputBuses : layout.outputBuses);

    XmlElement* xml = new XmlElement (isInput ? "INPUTS" : "OUTPUTS");

    const int n = buses.size();
    for (int busIdx = 0; busIdx < n; ++busIdx)
    {
        XmlElement* bus = new XmlElement ("BUS");
        bus->setAttribute ("index", busIdx);

        const AudioChannelSet& set = buses.getReference (busIdx);
        const String layoutName = set.isDisabled() ? "disabled" : set.getSpeakerArrangementAsString();

        bus->setAttribute ("layout", layoutName);

        xml->addChildElement (bus);
    }

    return xml;
}

static XmlElement* createNodeXml (AudioProcessorGraph::Node* const node) noexcept
{
    AudioPluginInstance* plugin = dynamic_cast<AudioPluginInstance*> (node->getProcessor());

    if (plugin == nullptr)
    {
        jassertfalse;
        return nullptr;
    }

    XmlElement* e = new XmlElement ("FILTER");
    e->setAttribute ("uid", (int) node->nodeId);
    e->setAttribute ("x", node->properties ["x"].toString());
    e->setAttribute ("y", node->properties ["y"].toString());

    for (int i = 0; i < PluginWindow::NumTypes; ++i)
    {
        PluginWindow::WindowFormatType type = (PluginWindow::WindowFormatType) i;

        if (node->properties.contains (getOpenProp (type)))
        {
            e->setAttribute (getLastXProp (type), node->properties[getLastXProp (type)].toString());
            e->setAttribute (getLastYProp (type), node->properties[getLastYProp (type)].toString());
            e->setAttribute (getOpenProp (type),  node->properties[getOpenProp (type)].toString());
        }
    }

    PluginDescription pd;
    plugin->fillInPluginDescription (pd);

    e->addChildElement (pd.createXml());

    XmlElement* state = new XmlElement ("STATE");

    MemoryBlock m;
    node->getProcessor()->getStateInformation (m);
    state->addTextElement (m.toBase64Encoding());
    e->addChildElement (state);

    XmlElement* layouts = new XmlElement ("LAYOUT");
    const AudioProcessor::BusesLayout layout = plugin->getBusesLayout();

    const bool isInputChoices[] = { true, false };
    for (bool isInput : isInputChoices)
        layouts->addChildElement (createBusLayoutXml (layout, isInput));

    e->addChildElement (layouts);

    return e;
}

void FilterGraph::createNodeFromXml (const XmlElement& xml)
{
    PluginDescription pd;

    forEachXmlChildElement (xml, e)
    {
        if (pd.loadFromXml (*e))
            break;
    }

    String errorMessage;

    AudioPluginInstance* instance = formatManager.createPluginInstance (pd, graph.getSampleRate(), graph.getBlockSize(), errorMessage);

    if (instance == nullptr)
        return;

    if (const XmlElement* const layoutEntity = xml.getChildByName ("LAYOUT"))
    {
        AudioProcessor::BusesLayout layout = instance->getBusesLayout();

        const bool isInputChoices[] = { true, false };
        for (bool isInput : isInputChoices)
            readBusLayoutFromXml (layout, instance, *layoutEntity, isInput);

        instance->setBusesLayout (layout);
    }

    AudioProcessorGraph::Node::Ptr node (graph.addNode (instance, (uint32) xml.getIntAttribute ("uid")));

    if (const XmlElement* const state = xml.getChildByName ("STATE"))
    {
        MemoryBlock m;
        m.fromBase64Encoding (state->getAllSubText());

        node->getProcessor()->setStateInformation (m.getData(), (int) m.getSize());
    }

    node->properties.set ("x", xml.getDoubleAttribute ("x"));
    node->properties.set ("y", xml.getDoubleAttribute ("y"));

    for (int i = 0; i < PluginWindow::NumTypes; ++i)
    {
        PluginWindow::WindowFormatType type = (PluginWindow::WindowFormatType) i;

        if (xml.hasAttribute (getOpenProp (type)))
        {
            node->properties.set (getLastXProp (type), xml.getIntAttribute (getLastXProp (type)));
            node->properties.set (getLastYProp (type), xml.getIntAttribute (getLastYProp (type)));
            node->properties.set (getOpenProp (type), xml.getIntAttribute (getOpenProp (type)));

            if (node->properties[getOpenProp (type)])
            {
                jassert (node->getProcessor() != nullptr);

                if (PluginWindow* const w = PluginWindow::getWindowFor (node, type))
                    w->toFront (true);
            }
        }
    }
}

XmlElement* FilterGraph::createXml() const
{
    XmlElement* xml = new XmlElement ("FILTERGRAPH");

    for (int i = 0; i < graph.getNumNodes(); ++i)
        xml->addChildElement (createNodeXml (graph.getNode (i)));

    for (int i = 0; i < graph.getNumConnections(); ++i)
    {
        const AudioProcessorGraph::Connection* const fc = graph.getConnection(i);

        XmlElement* e = new XmlElement ("CONNECTION");

        e->setAttribute ("srcFilter", (int) fc->sourceNodeId);
        e->setAttribute ("srcChannel", fc->sourceChannelIndex);
        e->setAttribute ("dstFilter", (int) fc->destNodeId);
        e->setAttribute ("dstChannel", fc->destChannelIndex);

        xml->addChildElement (e);
    }

    return xml;
}

void FilterGraph::restoreFromXml (const XmlElement& xml)
{
    clear();

    forEachXmlChildElementWithTagName (xml, e, "FILTER")
    {
        createNodeFromXml (*e);
        changed();
    }

    forEachXmlChildElementWithTagName (xml, e, "CONNECTION")
    {
        addConnection ((uint32) e->getIntAttribute ("srcFilter"),
                       e->getIntAttribute ("srcChannel"),
                       (uint32) e->getIntAttribute ("dstFilter"),
                       e->getIntAttribute ("dstChannel"));
    }

    graph.removeIllegalConnections();
}
