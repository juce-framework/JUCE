/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "FilterGraph.h"
#include "InternalFilters.h"


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
class PluginWindow  : public DocumentWindow
{
public:
    PluginWindow (Component* const uiComp,
                  FilterInGraph& owner_)
        : DocumentWindow (uiComp->getName(), Colours::lightblue,
                          DocumentWindow::minimiseButton | DocumentWindow::closeButton),
          owner (owner_)
    {
        setSize (400, 300);

        setContentComponent (uiComp, true, true);

        setTopLeftPosition (owner.lastX, owner.lastY);
        setVisible (true);
    }

    ~PluginWindow()
    {
        setContentComponent (0);
    }

    void moved()
    {
        owner.lastX = getX();
        owner.lastY = getY();
    }

    void closeButtonPressed()
    {
        owner.activeUI = 0;
        delete this;
    }

private:
    FilterInGraph& owner;
};


//==============================================================================
FilterInGraph::FilterInGraph (FilterGraph& owner_, AudioPluginInstance* const filter_)
    : owner (owner_),
      filter (filter_),
      uid (0),
      processedAudio (1, 1),
      activeUI (0)
{
    lastX = 100 + Random::getSystemRandom().nextInt (400);
    lastY = 100 + Random::getSystemRandom().nextInt (400);
}

FilterInGraph::~FilterInGraph()
{
    delete activeUI;
    delete filter;
}

void FilterInGraph::setPosition (double newX, double newY) throw()
{
    x = jlimit (0.0, 1.0, newX);
    y = jlimit (0.0, 1.0, newY);
}

void FilterInGraph::showUI()
{
    if (activeUI == 0)
    {
        Component* ui = filter->createEditorIfNeeded();

        if (ui != 0)
        {
            ui->setName (filter->getName());
            activeUI = new PluginWindow (ui, *this);
        }
    }

    if (activeUI != 0)
        activeUI->toFront (true);
}

void FilterInGraph::prepareBuffers (int blockSize)
{
    processedAudio.setSize (jmax (1, filter->getNumInputChannels(), filter->getNumOutputChannels()), blockSize);
    processedAudio.clear();

    processedMidi.clear();
}

void FilterInGraph::renderBlock (int numSamples,
                                 const ReferenceCountedArray <FilterInGraph>& filters,
                                 const OwnedArray <FilterConnection>& connections)
{
    processedAudio.setSize (jmax (1, filter->getNumInputChannels(), filter->getNumOutputChannels()), numSamples);

    // this isn't particularly efficient - could do with some optimising here
    processedAudio.clear();
    processedMidi.clear();

    for (int i = connections.size(); --i >= 0;)
    {
        const FilterConnection* const fc = connections.getUnchecked(i);

        if (fc->destFilterID == uid)
        {
            for (int j = filters.size(); --j >= 0;)
            {
                const FilterInGraph* const input = filters.getUnchecked(j);

                if (filters.getUnchecked(j)->uid == fc->sourceFilterID)
                {
                    if (fc->sourceChannel == FilterGraph::midiChannelNumber)
                    {
                        processedMidi.addEvents (input->processedMidi, 0, numSamples, 0);
                    }
                    else
                    {
                        if (fc->destChannel < filter->getNumInputChannels()
                            && fc->sourceChannel < input->filter->getNumOutputChannels())
                        {
                            processedAudio.addFrom (fc->destChannel, 0, input->processedAudio,
                                                    fc->sourceChannel, 0, numSamples);
                        }
                    }

                    break;
                }
            }
        }
    }

    filter->processBlock (processedAudio, processedMidi);
}

XmlElement* FilterInGraph::createXml() const
{
    XmlElement* e = new XmlElement ("FILTER");
    e->setAttribute (T("uid"), (int) uid);
    e->setAttribute (T("x"), x);
    e->setAttribute (T("y"), y);
    e->setAttribute (T("uiLastX"), lastX);
    e->setAttribute (T("uiLastY"), lastY);

    PluginDescription pd;
    pd.fillInFromInstance (*filter);
    e->addChildElement (pd.createXml());

    XmlElement* state = new XmlElement ("STATE");

    juce::MemoryBlock m;
    filter->getStateInformation (m);
    state->addTextElement (m.toBase64Encoding());
    e->addChildElement (state);

    return e;
}

FilterInGraph* FilterInGraph::createForDescription (FilterGraph& owner, const PluginDescription& desc)
{
    AudioPluginInstance* instance = desc.createInstance();

    if (instance != 0)
        return new FilterInGraph (owner, instance);

    return 0;
}

FilterInGraph* FilterInGraph::createFromXml (FilterGraph& owner, const XmlElement& xml)
{
    PluginDescription pd;

    forEachXmlChildElement (xml, e)
    {
        if (pd.loadFromXml (*e))
            break;
    }

    FilterInGraph* const c = createForDescription (owner, pd);

    if (c == 0)
        return 0;

    const XmlElement* const state = xml.getChildByName (T("STATE"));

    if (state != 0)
    {
        juce::MemoryBlock m;
        m.fromBase64Encoding (state->getAllSubText());

        c->filter->setStateInformation (m.getData(), m.getSize());
    }

    c->uid = xml.getIntAttribute (T("uid"));
    c->x = xml.getDoubleAttribute (T("x"));
    c->y = xml.getDoubleAttribute (T("y"));
    c->lastX = xml.getIntAttribute (T("uiLastX"), c->lastX);
    c->lastY = xml.getIntAttribute (T("uiLastY"), c->lastY);

    return c;
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
    clear();
}

uint32 FilterGraph::getNextUID() throw()
{
    return ++lastUID;
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
FilterInGraph* FilterGraph::getFilterForUID (const uint32 uid) const throw()
{
    for (int i = filters.size(); --i >= 0;)
        if (filters.getUnchecked(i)->uid == uid)
            return filters.getUnchecked(i);

    return 0;
}

void FilterGraph::addFilter (const FilterInGraph::Ptr& newFilter)
{
    if (newFilter->uid == 0)
        newFilter->uid = getNextUID();
    else if (newFilter->uid > lastUID)
        lastUID = newFilter->uid;

    filters.add (newFilter);

    changed();
}

void FilterGraph::addFilter (const PluginDescription* desc, double x, double y)
{
    if (desc != 0)
    {
        FilterInGraph* cf = FilterInGraph::createForDescription (*this, *desc);

        if (cf != 0)
        {
            cf->setPosition (x, y);
            addFilter (cf);
        }
    }
}

void FilterGraph::removeFilter (const uint32 uid)
{
    FilterInGraph* const filter = getFilterForUID (uid);

    if (filter != 0)
    {
        disconnectFilter (uid);

        filters.removeObject (filter);
        changed();
    }
}

void FilterGraph::disconnectFilter (const uint32 uid)
{
    for (int i = connections.size(); --i >= 0;)
    {
        const FilterConnection* const fc = connections.getUnchecked(i);

        if (fc->sourceFilterID == uid
             || fc->destFilterID == uid)
        {
            removeConnection (i);
        }
    }
}

bool FilterGraph::isAnInputTo (const uint32 possibleInput, const uint32 possibleDestination, int recursionCheck) const throw()
{
    if (recursionCheck > 0)
    {
        for (int i = 0; i < connections.size(); ++i)
        {
            if (connections.getUnchecked(i)->destFilterID == possibleDestination
                 && (connections.getUnchecked(i)->sourceFilterID == possibleInput
                      || isAnInputTo (possibleInput, connections.getUnchecked(i)->sourceFilterID, recursionCheck - 1)))
                return true;
        }
    }

    return false;
}

bool FilterGraph::isAnInputTo (const uint32 possibleInput, const uint32 possibleDestination) const throw()
{
    return isAnInputTo (possibleInput, possibleDestination, filters.size() + 1);
}

FilterConnection* FilterGraph::getConnectionBetween (uint32 sourceFilterUID, int sourceFilterChannel,
                                                     uint32 destFilterUID, int destFilterChannel) const throw()
{
    for (int i = connections.size(); --i >= 0;)
    {
        FilterConnection* const fc = connections.getUnchecked(i);

        if (fc->sourceFilterID == sourceFilterUID
             && fc->sourceChannel == sourceFilterChannel
             && fc->destFilterID == destFilterUID
             && fc->destChannel == destFilterChannel)
        {
            return fc;
        }
    }

    return 0;
}

bool FilterGraph::canConnect (uint32 sourceFilterUID, int sourceFilterChannel,
                              uint32 destFilterUID, int destFilterChannel) const throw()
{
    if (sourceFilterChannel < 0
         || destFilterChannel < 0
         || sourceFilterUID == destFilterUID
         || (destFilterChannel == midiChannelNumber) != (sourceFilterChannel == midiChannelNumber))
        return false;

    const FilterInGraph* const source = getFilterForUID (sourceFilterUID);

    if (source == 0
         || (sourceFilterChannel != midiChannelNumber && (sourceFilterChannel < 0 || sourceFilterChannel >= source->filter->getNumOutputChannels()))
         || (sourceFilterChannel == midiChannelNumber && ! source->filter->producesMidi()))
        return false;

    const FilterInGraph* const dest = getFilterForUID (destFilterUID);

    if (dest == 0
         || (destFilterChannel != midiChannelNumber && (destFilterChannel < 0 || destFilterChannel >= dest->filter->getNumInputChannels()))
         || (destFilterChannel == midiChannelNumber && ! dest->filter->acceptsMidi()))
        return false;

    if (getConnectionBetween (sourceFilterUID, sourceFilterChannel, destFilterUID, destFilterChannel) != 0)
        return false;

    return true;
}

bool FilterGraph::addConnection (uint32 sourceFilterUID, int sourceChannel,
                                 uint32 destFilterUID, int destChannel)
{
    if (canConnect (sourceFilterUID, sourceChannel, destFilterUID, destChannel))
    {
        FilterConnection* const conn = new FilterConnection (*this);

        conn->sourceFilterID = sourceFilterUID;
        conn->sourceChannel = sourceChannel;
        conn->destFilterID = destFilterUID;
        conn->destChannel = destChannel;

        connections.add (conn);
        changed();

        return true;
    }

    return false;
}

void FilterGraph::removeConnection (const int index)
{
    if (connections [index] != 0)
    {
        connections.remove (index);
        changed();
    }
}

void FilterGraph::removeIllegalConnections()
{
    for (int i = connections.size(); --i >= 0;)
    {
        const FilterConnection* const fc = connections.getUnchecked(i);

        bool ok = true;
        const FilterInGraph* const source = getFilterForUID (fc->sourceFilterID);

        if (source == 0
             || (fc->sourceChannel != midiChannelNumber && (fc->sourceChannel < 0 || fc->sourceChannel >= source->filter->getNumOutputChannels()))
             || (fc->sourceChannel == midiChannelNumber && ! source->filter->producesMidi()))
            ok = false;

        const FilterInGraph* const dest = getFilterForUID (fc->destFilterID);

        if (dest == 0
             || (fc->destChannel != midiChannelNumber && (fc->destChannel < 0 || fc->destChannel >= dest->filter->getNumInputChannels()))
             || (fc->destChannel == midiChannelNumber && ! dest->filter->acceptsMidi()))
            ok = false;


        if (! ok)
            removeConnection (i);
    }
}

void FilterGraph::removeConnection (uint32 sourceFilterUID, int sourceFilterChannel,
                                    uint32 destFilterUID, int destFilterChannel)
{
    for (int i = connections.size(); --i >= 0;)
    {
        const FilterConnection* const fc = connections.getUnchecked(i);

        if (fc->sourceFilterID == sourceFilterUID
             && fc->sourceChannel == sourceFilterChannel
             && fc->destFilterID == destFilterUID
             && fc->destChannel == destFilterChannel)
        {
            removeConnection (i);
        }
    }
}

void FilterGraph::clear()
{
    connections.clear();
    filters.clear();
    changed();
}

XmlElement* FilterGraph::createXml() const
{
    XmlElement* xml = new XmlElement ("FILTERGRAPH");

    int i;
    for (i = 0; i < filters.size(); ++i)
        xml->addChildElement (filters.getUnchecked(i)->createXml());

    for (i = 0; i < connections.size(); ++i)
    {
        const FilterConnection* const fc = connections.getUnchecked(i);

        XmlElement* e = new XmlElement ("CONNECTION");

        e->setAttribute (T("srcFilter"), (int) fc->sourceFilterID);
        e->setAttribute (T("srcChannel"), fc->sourceChannel);
        e->setAttribute (T("dstFilter"), (int) fc->destFilterID);
        e->setAttribute (T("dstChannel"), fc->destChannel);

        xml->addChildElement (e);
    }

    return xml;
}

void FilterGraph::restoreFromXml (const XmlElement& xml)
{
    clear();

    forEachXmlChildElementWithTagName (xml, e, T("FILTER"))
    {
        FilterInGraph* f = FilterInGraph::createFromXml (*this, *e);

        if (f != 0)
            addFilter (f);
    }

    forEachXmlChildElementWithTagName (xml, e, T("CONNECTION"))
    {
        addConnection ((uint32) e->getIntAttribute (T("srcFilter")),
                       e->getIntAttribute (T("srcChannel")),
                       (uint32) e->getIntAttribute (T("dstFilter")),
                       e->getIntAttribute (T("dstChannel")));
    }

    removeIllegalConnections();
}

//==============================================================================
FilterGraphPlayer::FilterGraphPlayer (FilterGraph& graph_)
    : graph (graph_),
      sampleRate (44100.0),
      blockSize (512),
      deviceManager (0),
      inputChannelData (0),
      totalNumInputChannels (0),
      outputChannelData (0),
      totalNumOutputChannels (0)
{
    setAudioDeviceManager (0);
    keyState.addListener (&messageCollector);
    graph.addChangeListener (this);
}

FilterGraphPlayer::~FilterGraphPlayer()
{
    graph.removeChangeListener (this);
    keyState.removeListener (&messageCollector);
}

void FilterGraphPlayer::setAudioDeviceManager (AudioDeviceManager* dm)
{
    if (deviceManager != 0)
    {
        deviceManager->removeMidiInputCallback (this);
        deviceManager->setAudioCallback (0);
    }

    deviceManager = dm;

    if (dm != 0)
    {
        dm->addMidiInputCallback (String::empty, this);
        dm->setAudioCallback (this);
    }
}

int FilterGraphPlayer::compareElements (FilterInGraph* const first, FilterInGraph* const second) throw()
{
    const bool firstIsInputToSecond = first->owner.isAnInputTo (first->uid, second->uid);
    const bool secondInInputToFirst = first->owner.isAnInputTo (second->uid, first->uid);

    if (firstIsInputToSecond == secondInInputToFirst)
        return 0;

    return firstIsInputToSecond ? -1 : 1;
}

void FilterGraphPlayer::update()
{
    ReferenceCountedArray <FilterInGraph> filtersBeingRemoved (filters);

    ReferenceCountedArray <FilterInGraph> newFilters (graph.filters);
    int i;
    for (i = newFilters.size(); --i >= 0;)
        if (filters.contains (newFilters.getUnchecked(i)))
            newFilters.remove (i);

    for (i = filtersBeingRemoved.size(); --i >= 0;)
        if (graph.filters.contains (filtersBeingRemoved.getUnchecked(i)))
            filtersBeingRemoved.remove (i);

    // prepare any new filters for use..
    for (i = 0; i < newFilters.size(); ++i)
        newFilters.getUnchecked(i)->filter->prepareToPlay (sampleRate, blockSize);

    ReferenceCountedArray <FilterInGraph> sortedFilters (graph.filters);
    sortedFilters.sort (*this, true);

    for (i = sortedFilters.size(); --i >= 0;)
    {
        PlayerAwareFilter* const specialFilter = dynamic_cast <PlayerAwareFilter*> (sortedFilters.getUnchecked(i)->filter);

        if (specialFilter != 0)
            specialFilter->setPlayer (this);
    }

    {
        const ScopedLock sl (processLock);

        filters = sortedFilters;
        connections.clear();

        for (int i = 0; i < graph.connections.size(); ++i)
            connections.add (new FilterConnection (*graph.connections.getUnchecked(i)));
    }

    // release any old ones..
    for (int i = 0; i < filtersBeingRemoved.size(); ++i)
    {
        filtersBeingRemoved.getUnchecked(i)->filter->releaseResources();

        PlayerAwareFilter* const specialFilter = dynamic_cast <PlayerAwareFilter*> (filtersBeingRemoved.getUnchecked(i)->filter);

        if (specialFilter != 0)
            specialFilter->setPlayer (0);
    }
}

void FilterGraphPlayer::changeListenerCallback (void*)
{
    update();
}

void FilterGraphPlayer::audioDeviceIOCallback (const float** inputChannelData_,
                                               int totalNumInputChannels_,
                                               float** outputChannelData_,
                                               int totalNumOutputChannels_,
                                               int numSamples)
{
    incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);

    for (int i = 0; i < totalNumOutputChannels_; ++i)
        if (outputChannelData_[i] != 0)
            zeromem (outputChannelData_[i], sizeof (float) * numSamples);

    const ScopedLock sl (processLock);

    inputChannelData = inputChannelData_;
    totalNumInputChannels = totalNumInputChannels_;
    outputChannelData = outputChannelData_;
    totalNumOutputChannels = totalNumOutputChannels_;

    for (int i = 0; i < filters.size(); ++i)
    {
        FilterInGraph* const cf = filters.getUnchecked(i);

        cf->renderBlock (numSamples, filters, connections);
    }
}

void FilterGraphPlayer::audioDeviceAboutToStart (double sampleRate_, int numSamplesPerBlock)
{
    const ScopedLock sl (processLock);

    sampleRate = sampleRate_;
    blockSize = numSamplesPerBlock;

    messageCollector.reset (sampleRate_);

    for (int i = 0; i < filters.size(); ++i)
    {
        filters.getUnchecked(i)->filter->prepareToPlay (sampleRate, blockSize);
        filters.getUnchecked(i)->prepareBuffers (blockSize);
    }

    graph.sendChangeMessage (&graph);
}

void FilterGraphPlayer::audioDeviceStopped()
{
    const ScopedLock sl (processLock);

    for (int i = 0; i < filters.size(); ++i)
        filters.getUnchecked(i)->filter->releaseResources();
}

void FilterGraphPlayer::handleIncomingMidiMessage (MidiInput*, const MidiMessage& message)
{
//    keyState.processNextMidiEvent (message);
    messageCollector.addMessageToQueue (message);
}
