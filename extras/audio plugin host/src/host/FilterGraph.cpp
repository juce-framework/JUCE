/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../includes.h"
#include "FilterGraph.h"
#include "InternalFilters.h"
#include "GraphEditorPanel.h"


//==============================================================================
FilterConnection::FilterConnection (FilterGraph& owner_)
    : owner (owner_)
{
}

FilterConnection::FilterConnection (const FilterConnection& other)
    : sourceFilterID (other.sourceFilterID),
      sourceChannel (other.sourceChannel),
      destFilterID (other.destFilterID),
      destChannel (other.destChannel),
      owner (other.owner)
{
}

FilterConnection::~FilterConnection()
{
}


//==============================================================================
const int FilterGraph::midiChannelNumber = 0x1000;

FilterGraph::FilterGraph()
    : FileBasedDocument (filenameSuffix,
                         filenameWildcard,
                         "Load a filter graph",
                         "Save a filter graph"),
      lastUID (0)
{
    InternalPluginFormat internalFormat;

    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::audioInputFilter),
               0.5f, 0.1f);

    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::midiInputFilter),
               0.25f, 0.1f);

    addFilter (internalFormat.getDescriptionFor (InternalPluginFormat::audioOutputFilter),
               0.5f, 0.9f);

    setChangedFlag (false);
}

FilterGraph::~FilterGraph()
{
    graph.clear();
}

uint32 FilterGraph::getNextUID() throw()
{
    return ++lastUID;
}

//==============================================================================
int FilterGraph::getNumFilters() const throw()
{
    return graph.getNumNodes();
}

const AudioProcessorGraph::Node::Ptr FilterGraph::getNode (const int index) const throw()
{
    return graph.getNode (index);
}

const AudioProcessorGraph::Node::Ptr FilterGraph::getNodeForId (const uint32 uid) const throw()
{
    return graph.getNodeForId (uid);
}

void FilterGraph::addFilter (const PluginDescription* desc, double x, double y)
{
    if (desc != 0)
    {
        String errorMessage;

        AudioPluginInstance* instance
            = AudioPluginFormatManager::getInstance()->createPluginInstance (*desc, errorMessage);

        AudioProcessorGraph::Node* node = 0;

        if (instance != 0)
            node = graph.addNode (instance);

        if (node != 0)
        {
            node->properties.setValue ("x", x);
            node->properties.setValue ("y", y);
            changed();
        }
        else
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS("Couldn't create filter"),
                                         errorMessage);
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

void FilterGraph::setNodePosition (const int nodeId, double x, double y)
{
    const AudioProcessorGraph::Node::Ptr n (graph.getNodeForId (nodeId));

    if (n != 0)
    {
        n->properties.setValue ("x", jlimit (0.0, 1.0, x));
        n->properties.setValue ("y", jlimit (0.0, 1.0, y));
    }
}

void FilterGraph::getNodePosition (const int nodeId, double& x, double& y) const
{
    x = y = 0;

    const AudioProcessorGraph::Node::Ptr n (graph.getNodeForId (nodeId));

    if (n != 0)
    {
        x = n->properties.getDoubleValue ("x");
        y = n->properties.getDoubleValue ("y");
    }
}

//==============================================================================
int FilterGraph::getNumConnections() const throw()
{
    return graph.getNumConnections();
}

const AudioProcessorGraph::Connection* FilterGraph::getConnection (const int index) const throw()
{
    return graph.getConnection (index);
}

const AudioProcessorGraph::Connection* FilterGraph::getConnectionBetween (uint32 sourceFilterUID, int sourceFilterChannel,
                                                                          uint32 destFilterUID, int destFilterChannel) const throw()
{
    return graph.getConnectionBetween (sourceFilterUID, sourceFilterChannel,
                                       destFilterUID, destFilterChannel);
}

bool FilterGraph::canConnect (uint32 sourceFilterUID, int sourceFilterChannel,
                              uint32 destFilterUID, int destFilterChannel) const throw()
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
const String FilterGraph::getDocumentTitle()
{
    if (! getFile().exists())
        return "Unnamed";

    return getFile().getFileNameWithoutExtension();
}

const String FilterGraph::loadDocument (const File& file)
{
    XmlDocument doc (file);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName (T("FILTERGRAPH")))
    {
        delete xml;
        return "Not a valid filter graph file";
    }

    restoreFromXml (*xml);
    delete xml;

    return String::empty;
}

const String FilterGraph::saveDocument (const File& file)
{
    XmlElement* xml = createXml();

    String error;

    if (! xml->writeToFile (file, String::empty))
        error = "Couldn't write to the file";

    delete xml;
    return error;
}

const File FilterGraph::getLastDocumentOpened()
{
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (ApplicationProperties::getInstance()->getUserSettings()
                                        ->getValue ("recentFilterGraphFiles"));

    return recentFiles.getFile (0);
}

void FilterGraph::setLastDocumentOpened (const File& file)
{
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (ApplicationProperties::getInstance()->getUserSettings()
                                        ->getValue ("recentFilterGraphFiles"));

    recentFiles.addFile (file);

    ApplicationProperties::getInstance()->getUserSettings()
        ->setValue ("recentFilterGraphFiles", recentFiles.toString());
}

//==============================================================================
static XmlElement* createNodeXml (AudioProcessorGraph::Node* const node) throw()
{
    AudioPluginInstance* plugin = dynamic_cast <AudioPluginInstance*> (node->processor);

    if (plugin == 0)
    {
        jassertfalse
        return 0;
    }

    XmlElement* e = new XmlElement ("FILTER");
    e->setAttribute (T("uid"), (int) node->id);
    e->setAttribute (T("x"), node->properties.getDoubleValue("x"));
    e->setAttribute (T("y"), node->properties.getDoubleValue("y"));
    e->setAttribute (T("uiLastX"), node->properties.getIntValue("uiLastX"));
    e->setAttribute (T("uiLastY"), node->properties.getIntValue("uiLastY"));

    PluginDescription pd;

    plugin->fillInPluginDescription (pd);

    e->addChildElement (pd.createXml());

    XmlElement* state = new XmlElement ("STATE");

    MemoryBlock m;
    node->processor->getStateInformation (m);
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

    AudioPluginInstance* instance
        = AudioPluginFormatManager::getInstance()->createPluginInstance (pd, errorMessage);

    if (instance == 0)
    {
        // xxx handle ins + outs
    }

    if (instance == 0)
        return;

    AudioProcessorGraph::Node::Ptr node (graph.addNode (instance, xml.getIntAttribute (T("uid"))));

    const XmlElement* const state = xml.getChildByName (T("STATE"));

    if (state != 0)
    {
        MemoryBlock m;
        m.fromBase64Encoding (state->getAllSubText());

        node->processor->setStateInformation (m.getData(), m.getSize());
    }

    node->properties.setValue ("x", xml.getDoubleAttribute (T("x")));
    node->properties.setValue ("y", xml.getDoubleAttribute (T("y")));
    node->properties.setValue ("uiLastX", xml.getIntAttribute (T("uiLastX")));
    node->properties.setValue ("uiLastY", xml.getIntAttribute (T("uiLastY")));
}

XmlElement* FilterGraph::createXml() const
{
    XmlElement* xml = new XmlElement ("FILTERGRAPH");

    int i;
    for (i = 0; i < graph.getNumNodes(); ++i)
    {
        xml->addChildElement (createNodeXml (graph.getNode (i)));
    }

    for (i = 0; i < graph.getNumConnections(); ++i)
    {
        const AudioProcessorGraph::Connection* const fc = graph.getConnection(i);

        XmlElement* e = new XmlElement ("CONNECTION");

        e->setAttribute (T("srcFilter"), (int) fc->sourceNodeId);
        e->setAttribute (T("srcChannel"), fc->sourceChannelIndex);
        e->setAttribute (T("dstFilter"), (int) fc->destNodeId);
        e->setAttribute (T("dstChannel"), fc->destChannelIndex);

        xml->addChildElement (e);
    }

    return xml;
}

void FilterGraph::restoreFromXml (const XmlElement& xml)
{
    clear();

    forEachXmlChildElementWithTagName (xml, e, T("FILTER"))
    {
        createNodeFromXml (*e);
        changed();
    }

    forEachXmlChildElementWithTagName (xml, e, T("CONNECTION"))
    {
        addConnection ((uint32) e->getIntAttribute (T("srcFilter")),
                       e->getIntAttribute (T("srcChannel")),
                       (uint32) e->getIntAttribute (T("dstFilter")),
                       e->getIntAttribute (T("dstChannel")));
    }

    graph.removeIllegalConnections();
}
