/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainHostWindow.h"
#include "FilterGraph.h"
#include "InternalFilters.h"
#include "GraphEditorPanel.h"


//==============================================================================
const int FilterGraph::midiChannelNumber = 0x1000;

FilterGraph::FilterGraph (AudioPluginFormatManager& formatManager_)
    : FileBasedDocument (filenameSuffix,
                         filenameWildcard,
                         "Load a filter graph",
                         "Save a filter graph"),
      formatManager (formatManager_), lastUID (0)
{
    InternalPluginFormat internalFormat;

    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::audioInputFilter),  0.5f,  0.1f);
    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::midiInputFilter),   0.25f, 0.1f);
    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::audioOutputFilter), 0.5f,  0.9f);

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

const AudioProcessorGraph::Node::Ptr FilterGraph::getNode (const int index) const noexcept
{
    return graph.getNode (index);
}

const AudioProcessorGraph::Node::Ptr FilterGraph::getNodeForId (const uint32 uid) const noexcept
{
    return graph.getNodeForId (uid);
}

void FilterGraph::addFilter (const PluginDescription* desc, double x, double y)
{
    if (desc != nullptr)
    {
        struct AsyncCallback : public AudioPluginFormat::InstantiationCompletionCallback
        {
            AsyncCallback (FilterGraph* myself, double inX, double inY)
                : owner (myself), posX (inX), posY (inY)
            {}

            void completionCallback (AudioPluginInstance* instance, const String& error) override
            {
                owner->addFilterCallback (instance, error, posX, posY);
            }

            FilterGraph* owner;
            double posX, posY;
        };

        formatManager.createPluginInstanceAsync (*desc, graph.getSampleRate(), graph.getBlockSize(),
                                                 new AsyncCallback (this, x, y));
    }
}

void FilterGraph::addFilterCallback (AudioPluginInstance* instance, const String& error, double x, double y)
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
        AudioProcessorGraph::Node* node = graph.addNode (instance);

        if (node != nullptr)
        {
            node->properties.set ("x", x);
            node->properties.set ("y", y);
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
    if (AudioProcessorGraph::Node::Ptr n = graph.getNodeForId (nodeId))
        return Point<double> (static_cast<double> (n->properties ["x"]),
                              static_cast<double> (n->properties ["y"]));

    return Point<double>();
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

    setFile (File());

    InternalPluginFormat internalFormat;

    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::audioInputFilter),  0.5f,  0.1f);
    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::midiInputFilter),   0.25f, 0.1f);
    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::audioOutputFilter), 0.5f,  0.9f);

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
    {
        // xxx handle ins + outs
    }

    if (instance == nullptr)
        return;

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

                if (PluginWindow* const w = PluginWindow::getWindowFor (node, type, graph))
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
