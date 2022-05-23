/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:                  HostPluginDemo
 version:               1.0.0
 vendor:                JUCE
 website:               http://juce.com
 description:           Plugin that can host other plugins

 dependencies:          juce_audio_basics, juce_audio_devices, juce_audio_formats,
                        juce_audio_plugin_client, juce_audio_processors,
                        juce_audio_utils, juce_core, juce_data_structures,
                        juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:             xcode_mac, vs2022, linux_make

 moduleFlags:           JUCE_STRICT_REFCOUNTEDPOINTER=1
                        JUCE_PLUGINHOST_LV2=1
                        JUCE_PLUGINHOST_VST3=1
                        JUCE_PLUGINHOST_VST=0
                        JUCE_PLUGINHOST_AU=1

 type:                  AudioProcessor
 mainClass:             HostAudioProcessor

 useLocalCopy:          1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn, pluginProducesMidiOut,
                        pluginEditorRequiresKeys

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

//==============================================================================
enum class EditorStyle { thisWindow, newWindow };

class HostAudioProcessorImpl : public AudioProcessor,
                               private ChangeListener
{
public:
    HostAudioProcessorImpl()
        : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                           .withOutput ("Output", AudioChannelSet::stereo(), true))
    {
        appProperties.setStorageParameters ([&]
        {
            PropertiesFile::Options opt;
            opt.applicationName = getName();
            opt.commonToAllUsers = false;
            opt.doNotSave = false;
            opt.filenameSuffix = ".props";
            opt.ignoreCaseOfKeyNames = false;
            opt.storageFormat = PropertiesFile::StorageFormat::storeAsXML;
            opt.osxLibrarySubFolder = "Application Support";
            return opt;
        }());

        pluginFormatManager.addDefaultFormats();

        if (auto savedPluginList = appProperties.getUserSettings()->getXmlValue ("pluginList"))
            pluginList.recreateFromXml (*savedPluginList);

        MessageManagerLock lock;
        pluginList.addChangeListener (this);
    }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const auto& mainOutput = layouts.getMainOutputChannelSet();
        const auto& mainInput  = layouts.getMainInputChannelSet();

        if (! mainInput.isDisabled() && mainInput != mainOutput)
            return false;

        if (mainOutput.size() > 2)
            return false;

        return true;
    }

    void prepareToPlay (double sr, int bs) override
    {
        const ScopedLock sl (innerMutex);

        active = true;

        if (inner != nullptr)
        {
            inner->setRateAndBufferSizeDetails (sr, bs);
            inner->prepareToPlay (sr, bs);
        }
    }

    void releaseResources() override
    {
        const ScopedLock sl (innerMutex);

        active = false;

        if (inner != nullptr)
            inner->releaseResources();
    }

    void reset() override
    {
        const ScopedLock sl (innerMutex);

        if (inner != nullptr)
            inner->reset();
    }

    // In this example, we don't actually pass any audio through the inner processor.
    // In a 'real' plugin, we'd need to add some synchronisation to ensure that the inner
    // plugin instance was never modified (deleted, replaced etc.) during a call to processBlock.
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override
    {
        jassert (! isUsingDoublePrecision());
    }

    void processBlock (AudioBuffer<double>&, MidiBuffer&) override
    {
        jassert (isUsingDoublePrecision());
    }

    bool hasEditor() const override                                   { return false; }
    AudioProcessorEditor* createEditor() override                     { return nullptr; }

    const String getName() const override                             { return "HostPluginDemo"; }
    bool acceptsMidi() const override                                 { return true; }
    bool producesMidi() const override                                { return true; }
    double getTailLengthSeconds() const override                      { return 0.0; }

    int getNumPrograms() override                                     { return 0; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const String getProgramName (int) override                        { return "None"; }
    void changeProgramName (int, const String&) override              {}

    void getStateInformation (MemoryBlock& destData) override
    {
        const ScopedLock sl (innerMutex);

        XmlElement xml ("state");

        if (inner != nullptr)
        {
            xml.setAttribute (editorStyleTag, (int) editorStyle);
            xml.addChildElement (inner->getPluginDescription().createXml().release());
            xml.addChildElement ([this]
            {
                MemoryBlock innerState;
                inner->getStateInformation (innerState);

                auto stateNode = std::make_unique<XmlElement> (innerStateTag);
                stateNode->addTextElement (innerState.toBase64Encoding());
                return stateNode.release();
            }());
        }

        const auto text = xml.toString();
        destData.replaceAll (text.toRawUTF8(), text.getNumBytesAsUTF8());
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        const ScopedLock sl (innerMutex);

        auto xml = XmlDocument::parse (String (CharPointer_UTF8 (static_cast<const char*> (data)), (size_t) sizeInBytes));

        if (auto* pluginNode = xml->getChildByName ("PLUGIN"))
        {
            PluginDescription pd;
            pd.loadFromXml (*pluginNode);

            MemoryBlock innerState;
            innerState.fromBase64Encoding (xml->getChildElementAllSubText (innerStateTag, {}));

            setNewPlugin (pd,
                          (EditorStyle) xml->getIntAttribute (editorStyleTag, 0),
                          innerState);
        }
    }

    void setNewPlugin (const PluginDescription& pd, EditorStyle where, const MemoryBlock& mb = {})
    {
        const ScopedLock sl (innerMutex);

        const auto callback = [this, where, mb] (std::unique_ptr<AudioPluginInstance> instance, const String& error)
        {
            if (error.isNotEmpty())
            {
                NativeMessageBox::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                                       "Plugin Load Failed",
                                                       error,
                                                       nullptr,
                                                       nullptr);
                return;
            }

            inner = std::move (instance);
            editorStyle = where;

            if (inner != nullptr && ! mb.isEmpty())
                inner->setStateInformation (mb.getData(), (int) mb.getSize());

            // In a 'real' plugin, we'd also need to set the bus configuration of the inner plugin.
            // One possibility would be to match the bus configuration of the wrapper plugin, but
            // the inner plugin isn't guaranteed to support the same layout. Alternatively, we
            // could try to apply a reasonably similar layout, and maintain a mapping between the
            // inner/outer channel layouts.
            //
            // In any case, it is essential that the inner plugin is told about the bus
            // configuration that will be used. The AudioBuffer passed to the inner plugin must also
            // exactly match this layout.

            if (active)
            {
                inner->setRateAndBufferSizeDetails (getSampleRate(), getBlockSize());
                inner->prepareToPlay (getSampleRate(), getBlockSize());
            }

            NullCheckedInvocation::invoke (pluginChanged);
        };

        pluginFormatManager.createPluginInstanceAsync (pd, getSampleRate(), getBlockSize(), callback);
    }

    void clearPlugin()
    {
        const ScopedLock sl (innerMutex);

        inner = nullptr;
        NullCheckedInvocation::invoke (pluginChanged);
    }

    bool isPluginLoaded() const
    {
        const ScopedLock sl (innerMutex);
        return inner != nullptr;
    }

    std::unique_ptr<AudioProcessorEditor> createInnerEditor() const
    {
        const ScopedLock sl (innerMutex);
        return rawToUniquePtr (inner->hasEditor() ? inner->createEditorIfNeeded() : nullptr);
    }

    EditorStyle getEditorStyle() const noexcept { return editorStyle; }

    ApplicationProperties appProperties;
    AudioPluginFormatManager pluginFormatManager;
    KnownPluginList pluginList;
    std::function<void()> pluginChanged;

private:
    CriticalSection innerMutex;
    std::unique_ptr<AudioPluginInstance> inner;
    EditorStyle editorStyle = EditorStyle{};
    bool active = false;

    static constexpr const char* innerStateTag = "inner_state";
    static constexpr const char* editorStyleTag = "editor_style";

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source != &pluginList)
            return;

        if (auto savedPluginList = pluginList.createXml())
        {
            appProperties.getUserSettings()->setValue ("pluginList", savedPluginList.get());
            appProperties.saveIfNeeded();
        }
    }
};


constexpr const char* HostAudioProcessorImpl::innerStateTag;
constexpr const char* HostAudioProcessorImpl::editorStyleTag;

//==============================================================================
constexpr auto margin = 10;

static void doLayout (Component* main, Component& bottom, int bottomHeight, Rectangle<int> bounds)
{
    Grid grid;
    grid.setGap (Grid::Px { margin });
    grid.templateColumns = { Grid::TrackInfo { Grid::Fr { 1 } } };
    grid.templateRows = { Grid::TrackInfo { Grid::Fr { 1 } },
                          Grid::TrackInfo { Grid::Px { bottomHeight }} };
    grid.items = { GridItem { main }, GridItem { bottom }.withMargin ({ 0, margin, margin, margin }) };
    grid.performLayout (bounds);
}

class PluginLoaderComponent : public Component
{
public:
    template <typename Callback>
    PluginLoaderComponent (AudioPluginFormatManager& manager,
                           KnownPluginList& list,
                           Callback&& callback)
        : pluginListComponent (manager, list, {}, {})
    {
        pluginListComponent.getTableListBox().setMultipleSelectionEnabled (false);

        addAndMakeVisible (pluginListComponent);
        addAndMakeVisible (buttons);

        const auto getCallback = [this, &list, callback = std::forward<Callback> (callback)] (EditorStyle style)
        {
            return [this, &list, callback, style]
            {
                const auto index = pluginListComponent.getTableListBox().getSelectedRow();
                const auto& types = list.getTypes();

                if (isPositiveAndBelow (index, types.size()))
                    NullCheckedInvocation::invoke (callback, types.getReference (index), style);
            };
        };

        buttons.thisWindowButton.onClick = getCallback (EditorStyle::thisWindow);
        buttons.newWindowButton .onClick = getCallback (EditorStyle::newWindow);
    }

    void resized() override
    {
        doLayout (&pluginListComponent, buttons, 80, getLocalBounds());
    }

private:
    struct Buttons : public Component
    {
        Buttons()
        {
            label.setJustificationType (Justification::centred);

            addAndMakeVisible (label);
            addAndMakeVisible (thisWindowButton);
            addAndMakeVisible (newWindowButton);
        }

        void resized() override
        {
            Grid vertical;
            vertical.autoFlow = Grid::AutoFlow::row;
            vertical.setGap (Grid::Px { margin });
            vertical.autoRows = vertical.autoColumns = Grid::TrackInfo { Grid::Fr { 1 } };
            vertical.items.insertMultiple (0, GridItem{}, 2);
            vertical.performLayout (getLocalBounds());

            label.setBounds (vertical.items[0].currentBounds.toNearestInt());

            Grid grid;
            grid.autoFlow = Grid::AutoFlow::column;
            grid.setGap (Grid::Px { margin });
            grid.autoRows = grid.autoColumns = Grid::TrackInfo { Grid::Fr { 1 } };
            grid.items = { GridItem { thisWindowButton },
                           GridItem { newWindowButton } };

            grid.performLayout (vertical.items[1].currentBounds.toNearestInt());
        }

        Label label { "", "Select a plugin from the list, then display it using the buttons below." };
        TextButton thisWindowButton { "Open In This Window" };
        TextButton newWindowButton { "Open In New Window" };
    };

    PluginListComponent pluginListComponent;
    Buttons buttons;
};

//==============================================================================
class PluginEditorComponent : public Component
{
public:
    template <typename Callback>
    PluginEditorComponent (std::unique_ptr<AudioProcessorEditor> editorIn, Callback&& onClose)
        : editor (std::move (editorIn))
    {
        addAndMakeVisible (editor.get());
        addAndMakeVisible (closeButton);

        childBoundsChanged (editor.get());

        closeButton.onClick = std::forward<Callback> (onClose);
    }

    void setScaleFactor (float scale)
    {
        if (editor != nullptr)
            editor->setScaleFactor (scale);
    }

    void resized() override
    {
        doLayout (editor.get(), closeButton, buttonHeight, getLocalBounds());
    }

    void childBoundsChanged (Component* child) override
    {
        if (child != editor.get())
            return;

        const auto size = editor != nullptr ? editor->getLocalBounds()
                                            : Rectangle<int>();

        setSize (size.getWidth(), margin + buttonHeight + size.getHeight());
    }

private:
    static constexpr auto buttonHeight = 40;

    std::unique_ptr<AudioProcessorEditor> editor;
    TextButton closeButton { "Close Plugin" };
};

//==============================================================================
class ScaledDocumentWindow : public DocumentWindow
{
public:
    ScaledDocumentWindow (Colour bg, float scale)
        : DocumentWindow ("Editor", bg, 0), desktopScale (scale) {}

    float getDesktopScaleFactor() const override { return Desktop::getInstance().getGlobalScaleFactor() * desktopScale; }

private:
    float desktopScale = 1.0f;
};

//==============================================================================
class HostAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    explicit HostAudioProcessorEditor (HostAudioProcessorImpl& owner)
        : AudioProcessorEditor (owner),
          hostProcessor (owner),
          loader (owner.pluginFormatManager,
                  owner.pluginList,
                  [&owner] (const PluginDescription& pd,
                            EditorStyle editorStyle)
                  {
                      owner.setNewPlugin (pd, editorStyle);
                  }),
          scopedCallback (owner.pluginChanged, [this] { pluginChanged(); })
    {
        setSize (500, 500);
        setResizable (false, false);
        addAndMakeVisible (closeButton);
        addAndMakeVisible (loader);

        hostProcessor.pluginChanged();

        closeButton.onClick = [this] { clearPlugin(); };
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
    }

    void resized() override
    {
        closeButton.setBounds (getLocalBounds().withSizeKeepingCentre (200, buttonHeight));
        loader.setBounds (getLocalBounds());
    }

    void childBoundsChanged (Component* child) override
    {
        if (child != editor.get())
            return;

        const auto size = editor != nullptr ? editor->getLocalBounds()
                                            : Rectangle<int>();

        setSize (size.getWidth(), size.getHeight());
    }

    void setScaleFactor (float scale) override
    {
        currentScaleFactor = scale;
        AudioProcessorEditor::setScaleFactor (scale);

        const auto posted = MessageManager::callAsync ([ref = SafePointer<HostAudioProcessorEditor> (this), scale]
        {
            if (auto* r = ref.getComponent())
                if (auto* e = r->currentEditorComponent)
                    e->setScaleFactor (scale);
        });

        jassertquiet (posted);
    }

private:
    void pluginChanged()
    {
        loader.setVisible (! hostProcessor.isPluginLoaded());
        closeButton.setVisible (hostProcessor.isPluginLoaded());

        if (hostProcessor.isPluginLoaded())
        {
            auto editorComponent = std::make_unique<PluginEditorComponent> (hostProcessor.createInnerEditor(), [this]
            {
                const auto posted = MessageManager::callAsync ([this] { clearPlugin(); });
                jassertquiet (posted);
            });

            editorComponent->setScaleFactor (currentScaleFactor);
            currentEditorComponent = editorComponent.get();

            editor = [&]() -> std::unique_ptr<Component>
            {
                switch (hostProcessor.getEditorStyle())
                {
                    case EditorStyle::thisWindow:
                        addAndMakeVisible (editorComponent.get());
                        setSize (editorComponent->getWidth(), editorComponent->getHeight());
                        return std::move (editorComponent);

                    case EditorStyle::newWindow:
                        const auto bg = getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker();
                        auto window = std::make_unique<ScaledDocumentWindow> (bg, currentScaleFactor);
                        window->setAlwaysOnTop (true);
                        window->setContentOwned (editorComponent.release(), true);
                        window->centreAroundComponent (this, window->getWidth(), window->getHeight());
                        window->setVisible (true);
                        return window;
                }

                jassertfalse;
                return nullptr;
            }();
        }
        else
        {
            editor = nullptr;
            setSize (500, 500);
        }
    }

    void clearPlugin()
    {
        currentEditorComponent = nullptr;
        editor = nullptr;
        hostProcessor.clearPlugin();
    }

    static constexpr auto buttonHeight = 30;

    HostAudioProcessorImpl& hostProcessor;
    PluginLoaderComponent loader;
    std::unique_ptr<Component> editor;
    PluginEditorComponent* currentEditorComponent = nullptr;
    ScopedValueSetter<std::function<void()>> scopedCallback;
    TextButton closeButton { "Close Plugin" };
    float currentScaleFactor = 1.0f;
};

//==============================================================================
class HostAudioProcessor : public HostAudioProcessorImpl
{
public:
    bool hasEditor() const override { return true; }
    AudioProcessorEditor* createEditor() override { return new HostAudioProcessorEditor (*this); }
};
