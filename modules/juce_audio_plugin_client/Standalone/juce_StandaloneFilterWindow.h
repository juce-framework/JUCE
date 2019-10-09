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

#if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
extern juce::AudioProcessor* JUCE_API JUCE_CALLTYPE createPluginFilterOfType (juce::AudioProcessor::WrapperType type);
#endif

namespace juce
{

//==============================================================================
/**
    An object that creates and plays a standalone instance of an AudioProcessor.

    The object will create your processor using the same createPluginFilter()
    function that the other plugin wrappers use, and will run it through the
    computer's audio/MIDI devices using AudioDeviceManager and AudioProcessorPlayer.

    @tags{Audio}
*/
class StandalonePluginHolder    : private AudioIODeviceCallback,
                                  private Timer,
                                  private Value::Listener
{
public:
    //==============================================================================
    /** Structure used for the number of inputs and outputs. */
    struct PluginInOuts   { short numIns, numOuts; };

    //==============================================================================
    /** Creates an instance of the default plugin.

        The settings object can be a PropertySet that the class should use to store its
        settings - the takeOwnershipOfSettings indicates whether this object will delete
        the settings automatically when no longer needed. The settings can also be nullptr.

        A default device name can be passed in.

        Preferably a complete setup options object can be used, which takes precedence over
        the preferredDefaultDeviceName and allows you to select the input & output device names,
        sample rate, buffer size etc.

        In all instances, the settingsToUse will take precedence over the "preferred" options if not null.
    */
    StandalonePluginHolder (PropertySet* settingsToUse,
                            bool takeOwnershipOfSettings = true,
                            const String& preferredDefaultDeviceName = String(),
                            const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions = nullptr,
                            const Array<PluginInOuts>& channels = Array<PluginInOuts>(),
                           #if JUCE_ANDROID || JUCE_IOS
                            bool shouldAutoOpenMidiDevices = true
                           #else
                            bool shouldAutoOpenMidiDevices = false
                           #endif
                            )

        : settings (settingsToUse, takeOwnershipOfSettings),
          channelConfiguration (channels),
          autoOpenMidiDevices (shouldAutoOpenMidiDevices)
    {
        shouldMuteInput.addListener (this);
        shouldMuteInput = ! isInterAppAudioConnected();

        createPlugin();

        auto inChannels = (channelConfiguration.size() > 0 ? channelConfiguration[0].numIns
                                                           : processor->getMainBusNumInputChannels());

        if (preferredSetupOptions != nullptr)
            options.reset (new AudioDeviceManager::AudioDeviceSetup (*preferredSetupOptions));

        auto audioInputRequired = (inChannels > 0);

        if (audioInputRequired && RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
            && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
            RuntimePermissions::request (RuntimePermissions::recordAudio,
                                         [this, preferredDefaultDeviceName] (bool granted) { init (granted, preferredDefaultDeviceName); });
        else
            init (audioInputRequired, preferredDefaultDeviceName);
    }

    void init (bool enableAudioInput, const String& preferredDefaultDeviceName)
    {
        setupAudioDevices (enableAudioInput, preferredDefaultDeviceName, options.get());
        reloadPluginState();
        startPlaying();

       if (autoOpenMidiDevices)
           startTimer (500);
    }

    virtual ~StandalonePluginHolder() override
    {
        stopTimer();

        deletePlugin();
        shutDownAudioDevices();
    }

    //==============================================================================
    virtual void createPlugin()
    {
      #if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
        processor.reset (::createPluginFilterOfType (AudioProcessor::wrapperType_Standalone));
      #else
        AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Standalone);
        processor.reset (createPluginFilter());
        AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Undefined);
      #endif
        jassert (processor != nullptr); // Your createPluginFilter() function must return a valid object!

        processor->disableNonMainBuses();
        processor->setRateAndBufferSizeDetails (44100, 512);

        int inChannels = (channelConfiguration.size() > 0 ? channelConfiguration[0].numIns
                                                          : processor->getMainBusNumInputChannels());

        int outChannels = (channelConfiguration.size() > 0 ? channelConfiguration[0].numOuts
                                                           : processor->getMainBusNumOutputChannels());

        processorHasPotentialFeedbackLoop = (inChannels > 0 && outChannels > 0);
    }

    virtual void deletePlugin()
    {
        stopPlaying();
        processor = nullptr;
    }

    static String getFilePatterns (const String& fileSuffix)
    {
        if (fileSuffix.isEmpty())
            return {};

        return (fileSuffix.startsWithChar ('.') ? "*" : "*.") + fileSuffix;
    }

    //==============================================================================
    Value& getMuteInputValue()                           { return shouldMuteInput; }
    bool getProcessorHasPotentialFeedbackLoop() const    { return processorHasPotentialFeedbackLoop; }
    void valueChanged (Value& value) override            { muteInput = (bool) value.getValue(); }

    //==============================================================================
    File getLastFile() const
    {
        File f;

        if (settings != nullptr)
            f = File (settings->getValue ("lastStateFile"));

        if (f == File())
            f = File::getSpecialLocation (File::userDocumentsDirectory);

        return f;
    }

    void setLastFile (const FileChooser& fc)
    {
        if (settings != nullptr)
            settings->setValue ("lastStateFile", fc.getResult().getFullPathName());
    }

    /** Pops up a dialog letting the user save the processor's state to a file. */
    void askUserToSaveState (const String& fileSuffix = String())
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        FileChooser fc (TRANS("Save current state"), getLastFile(), getFilePatterns (fileSuffix));

        if (fc.browseForFileToSave (true))
        {
            setLastFile (fc);

            MemoryBlock data;
            processor->getStateInformation (data);

            if (! fc.getResult().replaceWithData (data.getData(), data.getSize()))
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  TRANS("Error whilst saving"),
                                                  TRANS("Couldn't write to the specified file!"));
        }
       #else
        ignoreUnused (fileSuffix);
       #endif
    }

    /** Pops up a dialog letting the user re-load the processor's state from a file. */
    void askUserToLoadState (const String& fileSuffix = String())
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        FileChooser fc (TRANS("Load a saved state"), getLastFile(), getFilePatterns (fileSuffix));

        if (fc.browseForFileToOpen())
        {
            setLastFile (fc);

            MemoryBlock data;

            if (fc.getResult().loadFileAsData (data))
                processor->setStateInformation (data.getData(), (int) data.getSize());
            else
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  TRANS("Error whilst loading"),
                                                  TRANS("Couldn't read from the specified file!"));
        }
       #else
        ignoreUnused (fileSuffix);
       #endif
    }

    //==============================================================================
    void startPlaying()
    {
        player.setProcessor (processor.get());

       #if JucePlugin_Enable_IAA && JUCE_IOS
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
        {
            processor->setPlayHead (device->getAudioPlayHead());
            device->setMidiMessageCollector (&player.getMidiMessageCollector());
        }
       #endif
    }

    void stopPlaying()
    {
        player.setProcessor (nullptr);
    }

    //==============================================================================
    /** Shows an audio properties dialog box modally. */
    void showAudioSettingsDialog()
    {
        DialogWindow::LaunchOptions o;

        int minNumInputs  = std::numeric_limits<int>::max(), maxNumInputs  = 0,
            minNumOutputs = std::numeric_limits<int>::max(), maxNumOutputs = 0;

        auto updateMinAndMax = [] (int newValue, int& minValue, int& maxValue)
        {
            minValue = jmin (minValue, newValue);
            maxValue = jmax (maxValue, newValue);
        };

        if (channelConfiguration.size() > 0)
        {
            auto defaultConfig = channelConfiguration.getReference (0);
            updateMinAndMax ((int) defaultConfig.numIns,  minNumInputs,  maxNumInputs);
            updateMinAndMax ((int) defaultConfig.numOuts, minNumOutputs, maxNumOutputs);
        }

        if (auto* bus = processor->getBus (true, 0))
            updateMinAndMax (bus->getDefaultLayout().size(), minNumInputs, maxNumInputs);

        if (auto* bus = processor->getBus (false, 0))
            updateMinAndMax (bus->getDefaultLayout().size(), minNumOutputs, maxNumOutputs);

        minNumInputs  = jmin (minNumInputs,  maxNumInputs);
        minNumOutputs = jmin (minNumOutputs, maxNumOutputs);

        o.content.setOwned (new SettingsComponent (*this, deviceManager,
                                                          minNumInputs,
                                                          maxNumInputs,
                                                          minNumOutputs,
                                                          maxNumOutputs));
        o.content->setSize (500, 550);

        o.dialogTitle                   = TRANS("Audio/MIDI Settings");
        o.dialogBackgroundColour        = o.content->getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
        o.escapeKeyTriggersCloseButton  = true;
        o.useNativeTitleBar             = true;
        o.resizable                     = false;

        o.launchAsync();
    }

    void saveAudioDeviceState()
    {
        if (settings != nullptr)
        {
            auto xml = deviceManager.createStateXml();

            settings->setValue ("audioSetup", xml.get());

           #if ! (JUCE_IOS || JUCE_ANDROID)
            settings->setValue ("shouldMuteInput", (bool) shouldMuteInput.getValue());
           #endif
        }
    }

    void reloadAudioDeviceState (bool enableAudioInput,
                                 const String& preferredDefaultDeviceName,
                                 const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions)
    {
        std::unique_ptr<XmlElement> savedState;

        if (settings != nullptr)
        {
            savedState = settings->getXmlValue ("audioSetup");

           #if ! (JUCE_IOS || JUCE_ANDROID)
            shouldMuteInput.setValue (settings->getBoolValue ("shouldMuteInput", true));
           #endif
        }

        auto totalInChannels  = processor->getMainBusNumInputChannels();
        auto totalOutChannels = processor->getMainBusNumOutputChannels();

        if (channelConfiguration.size() > 0)
        {
            auto defaultConfig = channelConfiguration.getReference (0);
            totalInChannels  = defaultConfig.numIns;
            totalOutChannels = defaultConfig.numOuts;
        }

        deviceManager.initialise (enableAudioInput ? totalInChannels : 0,
                                  totalOutChannels,
                                  savedState.get(),
                                  true,
                                  preferredDefaultDeviceName,
                                  preferredSetupOptions);
    }

    //==============================================================================
    void savePluginState()
    {
        if (settings != nullptr && processor != nullptr)
        {
            MemoryBlock data;
            processor->getStateInformation (data);

            settings->setValue ("filterState", data.toBase64Encoding());
        }
    }

    void reloadPluginState()
    {
        if (settings != nullptr)
        {
            MemoryBlock data;

            if (data.fromBase64Encoding (settings->getValue ("filterState")) && data.getSize() > 0)
                processor->setStateInformation (data.getData(), (int) data.getSize());
        }
    }

    //==============================================================================
    void switchToHostApplication()
    {
       #if JUCE_IOS
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
            device->switchApplication();
       #endif
    }

    bool isInterAppAudioConnected()
    {
       #if JUCE_IOS
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
            return device->isInterAppAudioConnected();
       #endif

        return false;
    }

   #if JUCE_MODULE_AVAILABLE_juce_gui_basics
    Image getIAAHostIcon (int size)
    {
       #if JUCE_IOS && JucePlugin_Enable_IAA
        if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
            return device->getIcon (size);
       #else
        ignoreUnused (size);
       #endif

        return {};
    }
   #endif

    static StandalonePluginHolder* getInstance();

    //==============================================================================
    OptionalScopedPointer<PropertySet> settings;
    std::unique_ptr<AudioProcessor> processor;
    AudioDeviceManager deviceManager;
    AudioProcessorPlayer player;
    Array<PluginInOuts> channelConfiguration;

    // avoid feedback loop by default
    bool processorHasPotentialFeedbackLoop = true;
    std::atomic<bool> muteInput { false };
    Value shouldMuteInput;
    AudioBuffer<float> emptyBuffer;
    bool autoOpenMidiDevices;

    std::unique_ptr<AudioDeviceManager::AudioDeviceSetup> options;
    Array<MidiDeviceInfo> lastMidiDevices;

private:
    //==============================================================================
    class SettingsComponent : public Component
    {
    public:
        SettingsComponent (StandalonePluginHolder& pluginHolder,
                           AudioDeviceManager& deviceManagerToUse,
                           int minAudioInputChannels,
                           int maxAudioInputChannels,
                           int minAudioOutputChannels,
                           int maxAudioOutputChannels)
            : owner (pluginHolder),
              deviceSelector (deviceManagerToUse,
                              minAudioInputChannels, maxAudioInputChannels,
                              minAudioOutputChannels, maxAudioOutputChannels,
                              true,
                              (pluginHolder.processor.get() != nullptr && pluginHolder.processor->producesMidi()),
                              true, false),
              shouldMuteLabel  ("Feedback Loop:", "Feedback Loop:"),
              shouldMuteButton ("Mute audio input")
        {
            setOpaque (true);

            shouldMuteButton.setClickingTogglesState (true);
            shouldMuteButton.getToggleStateValue().referTo (owner.shouldMuteInput);

            addAndMakeVisible (deviceSelector);

            if (owner.getProcessorHasPotentialFeedbackLoop())
            {
                addAndMakeVisible (shouldMuteButton);
                addAndMakeVisible (shouldMuteLabel);

                shouldMuteLabel.attachToComponent (&shouldMuteButton, true);
            }
        }

        void paint (Graphics& g) override
        {
            g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        }

        void resized() override
        {
            auto r = getLocalBounds();

            if (owner.getProcessorHasPotentialFeedbackLoop())
            {
                auto itemHeight = deviceSelector.getItemHeight();
                auto extra = r.removeFromTop (itemHeight);

                auto seperatorHeight = (itemHeight >> 1);
                shouldMuteButton.setBounds (Rectangle<int> (extra.proportionOfWidth (0.35f), seperatorHeight,
                                                            extra.proportionOfWidth (0.60f), deviceSelector.getItemHeight()));

                r.removeFromTop (seperatorHeight);
            }

            deviceSelector.setBounds (r);
        }

    private:
        //==============================================================================
        StandalonePluginHolder& owner;
        AudioDeviceSelectorComponent deviceSelector;
        Label shouldMuteLabel;
        ToggleButton shouldMuteButton;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
    };

    //==============================================================================
    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels,
                                float** outputChannelData,
                                int numOutputChannels,
                                int numSamples) override
    {
        if (muteInput)
        {
            emptyBuffer.clear();
            inputChannelData = emptyBuffer.getArrayOfReadPointers();
        }

        player.audioDeviceIOCallback (inputChannelData, numInputChannels,
                                      outputChannelData, numOutputChannels, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        emptyBuffer.setSize (device->getActiveInputChannels().countNumberOfSetBits(), device->getCurrentBufferSizeSamples());
        emptyBuffer.clear();

        player.audioDeviceAboutToStart (device);
        player.setMidiOutput (deviceManager.getDefaultMidiOutput());
    }

    void audioDeviceStopped() override
    {
        player.setMidiOutput (nullptr);
        player.audioDeviceStopped();
        emptyBuffer.setSize (0, 0);
    }

    //==============================================================================
    void setupAudioDevices (bool enableAudioInput,
                            const String& preferredDefaultDeviceName,
                            const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions)
    {
        deviceManager.addAudioCallback (this);
        deviceManager.addMidiInputDeviceCallback ({}, &player);

        reloadAudioDeviceState (enableAudioInput, preferredDefaultDeviceName, preferredSetupOptions);
    }

    void shutDownAudioDevices()
    {
        saveAudioDeviceState();

        deviceManager.removeMidiInputDeviceCallback ({}, &player);
        deviceManager.removeAudioCallback (this);
    }

    void timerCallback() override
    {
        auto newMidiDevices = MidiInput::getAvailableDevices();

        if (newMidiDevices != lastMidiDevices)
        {
            for (auto& oldDevice : lastMidiDevices)
                if (! newMidiDevices.contains (oldDevice))
                    deviceManager.setMidiInputDeviceEnabled (oldDevice.identifier, false);

            for (auto& newDevice : newMidiDevices)
                if (! lastMidiDevices.contains (newDevice))
                    deviceManager.setMidiInputDeviceEnabled (newDevice.identifier, true);

            lastMidiDevices = newMidiDevices;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandalonePluginHolder)
};

//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your JUCEApplicationBase::initialise() method, and
    let it do its work. It will create your filter object using the same createPluginFilter() function
    that the other plugin wrappers use.

    @tags{Audio}
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                  public Button::Listener
{
public:
    //==============================================================================
    typedef StandalonePluginHolder::PluginInOuts PluginInOuts;

    //==============================================================================
    /** Creates a window with a given title and colour.
        The settings object can be a PropertySet that the class should use to
        store its settings (it can also be null). If takeOwnershipOfSettings is
        true, then the settings object will be owned and deleted by this object.
    */
    StandaloneFilterWindow (const String& title,
                            Colour backgroundColour,
                            PropertySet* settingsToUse,
                            bool takeOwnershipOfSettings,
                            const String& preferredDefaultDeviceName = String(),
                            const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions = nullptr,
                            const Array<PluginInOuts>& constrainToConfiguration = {},
                           #if JUCE_ANDROID || JUCE_IOS
                            bool autoOpenMidiDevices = true
                           #else
                            bool autoOpenMidiDevices = false
                           #endif
                            )
        : DocumentWindow (title, backgroundColour, DocumentWindow::minimiseButton | DocumentWindow::closeButton),
          optionsButton ("Options")
    {
       #if JUCE_IOS || JUCE_ANDROID
        setTitleBarHeight (0);
       #else
        setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, false);

        Component::addAndMakeVisible (optionsButton);
        optionsButton.addListener (this);
        optionsButton.setTriggeredOnMouseDown (true);
       #endif

        pluginHolder.reset (new StandalonePluginHolder (settingsToUse, takeOwnershipOfSettings,
                                                        preferredDefaultDeviceName, preferredSetupOptions,
                                                        constrainToConfiguration, autoOpenMidiDevices));

       #if JUCE_IOS || JUCE_ANDROID
        setFullScreen (true);
        setContentOwned (new MainContentComponent (*this), false);
       #else
        setContentOwned (new MainContentComponent (*this), true);

        if (auto* props = pluginHolder->settings.get())
        {
            const int x = props->getIntValue ("windowX", -100);
            const int y = props->getIntValue ("windowY", -100);

            if (x != -100 && y != -100)
                setBoundsConstrained ({ x, y, getWidth(), getHeight() });
            else
                centreWithSize (getWidth(), getHeight());
        }
        else
        {
            centreWithSize (getWidth(), getHeight());
        }
       #endif
    }

    ~StandaloneFilterWindow() override
    {
       #if (! JUCE_IOS) && (! JUCE_ANDROID)
        if (auto* props = pluginHolder->settings.get())
        {
            props->setValue ("windowX", getX());
            props->setValue ("windowY", getY());
        }
       #endif

        pluginHolder->stopPlaying();
        clearContentComponent();
        pluginHolder = nullptr;
    }

    //==============================================================================
    AudioProcessor* getAudioProcessor() const noexcept      { return pluginHolder->processor.get(); }
    AudioDeviceManager& getDeviceManager() const noexcept   { return pluginHolder->deviceManager; }

    /** Deletes and re-creates the plugin, resetting it to its default state. */
    void resetToDefaultState()
    {
        pluginHolder->stopPlaying();
        clearContentComponent();
        pluginHolder->deletePlugin();

        if (auto* props = pluginHolder->settings.get())
            props->removeValue ("filterState");

        pluginHolder->createPlugin();
        setContentOwned (new MainContentComponent (*this), true);
        pluginHolder->startPlaying();
    }

    //==============================================================================
    void closeButtonPressed() override
    {
        pluginHolder->savePluginState();

        JUCEApplicationBase::quit();
    }

    void buttonClicked (Button*) override
    {
        PopupMenu m;
        m.addItem (1, TRANS("Audio/MIDI Settings..."));
        m.addSeparator();
        m.addItem (2, TRANS("Save current state..."));
        m.addItem (3, TRANS("Load a saved state..."));
        m.addSeparator();
        m.addItem (4, TRANS("Reset to default state"));

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (menuCallback, this));
    }

    void handleMenuResult (int result)
    {
        switch (result)
        {
            case 1:  pluginHolder->showAudioSettingsDialog(); break;
            case 2:  pluginHolder->askUserToSaveState(); break;
            case 3:  pluginHolder->askUserToLoadState(); break;
            case 4:  resetToDefaultState(); break;
            default: break;
        }
    }

    static void menuCallback (int result, StandaloneFilterWindow* button)
    {
        if (button != nullptr && result != 0)
            button->handleMenuResult (result);
    }

    void resized() override
    {
        DocumentWindow::resized();
        optionsButton.setBounds (8, 6, 60, getTitleBarHeight() - 8);
    }

    virtual StandalonePluginHolder* getPluginHolder()    { return pluginHolder.get(); }

    std::unique_ptr<StandalonePluginHolder> pluginHolder;

private:
    //==============================================================================
    class MainContentComponent  : public Component,
                                  private Value::Listener,
                                  private Button::Listener,
                                  private ComponentListener
    {
    public:
        MainContentComponent (StandaloneFilterWindow& filterWindow)
            : owner (filterWindow), notification (this),
              editor (owner.getAudioProcessor()->hasEditor() ? owner.getAudioProcessor()->createEditorIfNeeded()
                                                             : new GenericAudioProcessorEditor (*owner.getAudioProcessor()))
        {
            Value& inputMutedValue = owner.pluginHolder->getMuteInputValue();

            if (editor != nullptr)
            {
                editor->addComponentListener (this);
                componentMovedOrResized (*editor, false, true);

                addAndMakeVisible (editor.get());
            }

            addChildComponent (notification);

            if (owner.pluginHolder->getProcessorHasPotentialFeedbackLoop())
            {
                inputMutedValue.addListener (this);
                shouldShowNotification = inputMutedValue.getValue();
            }

            inputMutedChanged (shouldShowNotification);
        }

        ~MainContentComponent() override
        {
            if (editor != nullptr)
            {
                editor->removeComponentListener (this);
                owner.pluginHolder->processor->editorBeingDeleted (editor.get());
                editor = nullptr;
            }
        }

        void resized() override
        {
            auto r = getLocalBounds();

            if (shouldShowNotification)
                notification.setBounds (r.removeFromTop (NotificationArea::height));

            if (editor != nullptr)
                editor->setBounds (editor->getLocalArea (this, r)
                                          .withPosition (r.getTopLeft().transformedBy (editor->getTransform().inverted())));
        }

    private:
        //==============================================================================
        class NotificationArea : public Component
        {
        public:
            enum { height = 30 };

            NotificationArea (Button::Listener* settingsButtonListener)
                : notification ("notification", "Audio input is muted to avoid feedback loop"),
                 #if JUCE_IOS || JUCE_ANDROID
                  settingsButton ("Unmute Input")
                 #else
                  settingsButton ("Settings...")
                 #endif
            {
                setOpaque (true);

                notification.setColour (Label::textColourId, Colours::black);

                settingsButton.addListener (settingsButtonListener);

                addAndMakeVisible (notification);
                addAndMakeVisible (settingsButton);
            }

            void paint (Graphics& g) override
            {
                auto r = getLocalBounds();

                g.setColour (Colours::darkgoldenrod);
                g.fillRect (r.removeFromBottom (1));

                g.setColour (Colours::lightgoldenrodyellow);
                g.fillRect (r);
            }

            void resized() override
            {
                auto r = getLocalBounds().reduced (5);

                settingsButton.setBounds (r.removeFromRight (70));
                notification.setBounds (r);
            }
        private:
            Label notification;
            TextButton settingsButton;
        };

        //==============================================================================
        void inputMutedChanged (bool newInputMutedValue)
        {
            shouldShowNotification = newInputMutedValue;
            notification.setVisible (shouldShowNotification);

           #if JUCE_IOS || JUCE_ANDROID
            resized();
           #else
            if (editor != nullptr)
            {
                auto rect = getSizeToContainEditor();

                setSize (rect.getWidth(),
                         rect.getHeight() + (shouldShowNotification ? NotificationArea::height : 0));
            }
           #endif
        }

        void valueChanged (Value& value) override     { inputMutedChanged (value.getValue()); }
        void buttonClicked (Button*) override
        {
           #if JUCE_IOS || JUCE_ANDROID
            owner.pluginHolder->getMuteInputValue().setValue (false);
           #else
            owner.pluginHolder->showAudioSettingsDialog();
           #endif
        }

        //==============================================================================
        void componentMovedOrResized (Component&, bool, bool) override
        {
            if (editor != nullptr)
            {
                auto rect = getSizeToContainEditor();

                setSize (rect.getWidth(),
                         rect.getHeight() + (shouldShowNotification ? NotificationArea::height : 0));
            }
        }

        Rectangle<int> getSizeToContainEditor() const
        {
            if (editor != nullptr)
                return getLocalArea (editor.get(), editor->getLocalBounds());

            return {};
        }

        //==============================================================================
        StandaloneFilterWindow& owner;
        NotificationArea notification;
        std::unique_ptr<AudioProcessorEditor> editor;
        bool shouldShowNotification = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
    };

    //==============================================================================
    TextButton optionsButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandaloneFilterWindow)
};

inline StandalonePluginHolder* StandalonePluginHolder::getInstance()
{
   #if JucePlugin_Enable_IAA || JucePlugin_Build_Standalone
    if (PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_Standalone)
    {
        auto& desktop = Desktop::getInstance();
        const int numTopLevelWindows = desktop.getNumComponents();

        for (int i = 0; i < numTopLevelWindows; ++i)
            if (auto window = dynamic_cast<StandaloneFilterWindow*> (desktop.getComponent (i)))
                return window->getPluginHolder();
    }
   #endif

    return nullptr;
}

} // namespace juce
