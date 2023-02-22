/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../UI/PluginWindow.h"

//==============================================================================
/** A type that encapsulates a PluginDescription and some preferences regarding
    how plugins of that description should be instantiated.
*/
struct PluginDescriptionAndPreference
{
    enum class UseARA { no, yes };

    PluginDescriptionAndPreference() = default;

    explicit PluginDescriptionAndPreference (PluginDescription pd)
        : pluginDescription (std::move (pd)),
          useARA (pluginDescription.hasARAExtension ? PluginDescriptionAndPreference::UseARA::yes
                                                    : PluginDescriptionAndPreference::UseARA::no)
    {}

    PluginDescriptionAndPreference (PluginDescription pd, UseARA ara)
        : pluginDescription (std::move (pd)), useARA (ara)
    {}

    PluginDescription pluginDescription;
    UseARA useARA = UseARA::no;
};

//==============================================================================
/**
    A collection of plugins and some connections between them.
*/
class PluginGraph   : public FileBasedDocument,
                      public AudioProcessorListener,
                      private ChangeListener
{
public:
    //==============================================================================
    PluginGraph (AudioPluginFormatManager&, KnownPluginList&);
    ~PluginGraph() override;

    //==============================================================================
    using NodeID = AudioProcessorGraph::NodeID;

    void addPlugin (const PluginDescriptionAndPreference&, Point<double>);

    AudioProcessorGraph::Node::Ptr getNodeForName (const String& name) const;

    void setNodePosition (NodeID, Point<double>);
    Point<double> getNodePosition (NodeID) const;

    //==============================================================================
    void clear();

    PluginWindow* getOrCreateWindowFor (AudioProcessorGraph::Node*, PluginWindow::Type);
    void closeCurrentlyOpenWindowsFor (AudioProcessorGraph::NodeID);
    bool closeAnyOpenPluginWindows();

    //==============================================================================
    void audioProcessorParameterChanged (AudioProcessor*, int, float) override {}
    void audioProcessorChanged (AudioProcessor*, const ChangeDetails&) override { changed(); }

    //==============================================================================
    std::unique_ptr<XmlElement> createXml() const;
    void restoreFromXml (const XmlElement&);

    static const char* getFilenameSuffix()      { return ".filtergraph"; }
    static const char* getFilenameWildcard()    { return "*.filtergraph"; }

    //==============================================================================
    void newDocument();
    String getDocumentTitle() override;
    Result loadDocument (const File& file) override;
    Result saveDocument (const File& file) override;
    File getLastDocumentOpened() override;
    void setLastDocumentOpened (const File& file) override;

    static File getDefaultGraphDocumentOnMobile();

    //==============================================================================
    AudioProcessorGraph graph;

private:
    //==============================================================================
    AudioPluginFormatManager& formatManager;
    KnownPluginList& knownPlugins;
    OwnedArray<PluginWindow> activePluginWindows;
    ScopedMessageBox messageBox;

    NodeID lastUID;
    NodeID getNextUID() noexcept;

    void createNodeFromXml (const XmlElement&);
    void addPluginCallback (std::unique_ptr<AudioPluginInstance>,
                            const String& error,
                            Point<double>,
                            PluginDescriptionAndPreference::UseARA useARA);
    void changeListenerCallback (ChangeBroadcaster*) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginGraph)
};
