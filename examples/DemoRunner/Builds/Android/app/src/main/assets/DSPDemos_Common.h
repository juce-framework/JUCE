/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

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

#pragma once

using namespace dsp;

//==============================================================================
struct DSPDemoParameterBase    : public ChangeBroadcaster
{
    DSPDemoParameterBase (const String& labelName) : name (labelName) {}
    virtual ~DSPDemoParameterBase() {}

    virtual Component* getComponent() = 0;

    virtual int getPreferredHeight()  = 0;
    virtual int getPreferredWidth()   = 0;

    String name;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DSPDemoParameterBase)
};

//==============================================================================
struct SliderParameter   : public DSPDemoParameterBase
{
    SliderParameter (Range<double> range, double skew, double initialValue,
                     const String& labelName, const String& suffix = {})
        : DSPDemoParameterBase (labelName)
    {
        slider.setRange (range.getStart(), range.getEnd(), 0.01);
        slider.setSkewFactor (skew);
        slider.setValue (initialValue);

        if (suffix.isNotEmpty())
            slider.setTextValueSuffix (suffix);

        slider.onValueChange = [this] { sendChangeMessage(); };
    }

    Component* getComponent() override    { return &slider; }

    int getPreferredHeight() override     { return 40; }
    int getPreferredWidth()  override     { return 500; }

    double getCurrentValue() const        { return slider.getValue(); }

private:
    Slider slider;
};

//==============================================================================
struct ChoiceParameter    : public DSPDemoParameterBase
{
    ChoiceParameter (const StringArray& options, int initialId, const String& labelName)
        : DSPDemoParameterBase (labelName)
    {
        parameterBox.addItemList (options, 1);
        parameterBox.onChange = [this] { sendChangeMessage(); };

        parameterBox.setSelectedId (initialId);
    }

    Component* getComponent() override    { return &parameterBox; }

    int getPreferredHeight() override     { return 25; }
    int getPreferredWidth()  override     { return 250; }

    int getCurrentSelectedID() const      { return parameterBox.getSelectedId(); }

private:
    ComboBox parameterBox;
};

//==============================================================================
class AudioThumbnailComponent    : public Component,
                                   public FileDragAndDropTarget,
                                   public ChangeBroadcaster,
                                   private ChangeListener,
                                   private Timer
{
public:
    AudioThumbnailComponent (AudioDeviceManager& adm, AudioFormatManager& afm)
        : audioDeviceManager (adm),
          thumbnailCache (5),
          thumbnail (128, afm, thumbnailCache)
    {
        thumbnail.addChangeListener (this);
    }

    ~AudioThumbnailComponent() override
    {
        thumbnail.removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colour (0xff495358));

        g.setColour (Colours::white);

        if (thumbnail.getTotalLength() > 0.0)
        {
            thumbnail.drawChannels (g, getLocalBounds().reduced (2),
                                    0.0, thumbnail.getTotalLength(), 1.0f);

            g.setColour (Colours::black);
            g.fillRect (static_cast<float> (currentPosition * getWidth()), 0.0f,
                        1.0f, static_cast<float> (getHeight()));
        }
        else
        {
            g.drawFittedText ("No audio file loaded.\nDrop a file here or click the \"Load File...\" button.", getLocalBounds(),
                              Justification::centred, 2);
        }
    }

    bool isInterestedInFileDrag (const StringArray&) override          { return true; }
    void filesDropped (const StringArray& files, int, int) override    { loadURL (URL (File (files[0])), true); }

    void setCurrentURL (const URL& u)
    {
        if (currentURL == u)
            return;

        loadURL (u);
    }

    URL getCurrentURL()    { return currentURL; }

    void setTransportSource (AudioTransportSource* newSource)
    {
        transportSource = newSource;

        struct ResetCallback  : public CallbackMessage
        {
            ResetCallback (AudioThumbnailComponent& o) : owner (o) {}
            void messageCallback() override    { owner.reset(); }

            AudioThumbnailComponent& owner;
        };

        (new ResetCallback (*this))->post();
    }

private:
    AudioDeviceManager& audioDeviceManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    AudioTransportSource* transportSource = nullptr;

    URL currentURL;
    double currentPosition = 0.0;

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster*) override    { repaint(); }

    void reset()
    {
        currentPosition = 0.0;
        repaint();

        if (transportSource == nullptr)
            stopTimer();
        else
            startTimerHz (25);
    }

    void loadURL (const URL& u, bool notify = false)
    {
        if (currentURL == u)
            return;

        currentURL = u;

        InputSource* inputSource = nullptr;

       #if ! JUCE_IOS
        if (u.isLocalFile())
        {
            inputSource = new FileInputSource (u.getLocalFile());
        }
        else
       #endif
        {
            if (inputSource == nullptr)
                inputSource = new URLInputSource (u);
        }

        thumbnail.setSource (inputSource);

        if (notify)
            sendChangeMessage();
    }

    void timerCallback() override
    {
        if (transportSource != nullptr)
        {
            currentPosition = transportSource->getCurrentPosition() / thumbnail.getTotalLength();
            repaint();
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (transportSource != nullptr)
        {
            const ScopedLock sl (audioDeviceManager.getAudioCallbackLock());

            transportSource->setPosition ((jmax (static_cast<double> (e.x), 0.0) / getWidth())
                                            * thumbnail.getTotalLength());
        }
    }
};

//==============================================================================
class DemoParametersComponent    : public Component
{
public:
    DemoParametersComponent (const std::vector<DSPDemoParameterBase*>& demoParams)
    {
        parameters = demoParams;

        for (auto demoParameter : parameters)
        {
            addAndMakeVisible (demoParameter->getComponent());

            auto* paramLabel = new Label ({}, demoParameter->name);

            paramLabel->attachToComponent (demoParameter->getComponent(), true);
            paramLabel->setJustificationType (Justification::centredLeft);
            addAndMakeVisible (paramLabel);
            labels.add (paramLabel);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft (100);

        for (auto* p : parameters)
        {
            auto* comp = p->getComponent();

            comp->setSize (jmin (bounds.getWidth(), p->getPreferredWidth()), p->getPreferredHeight());

            auto compBounds = bounds.removeFromTop (p->getPreferredHeight());
            comp->setCentrePosition (compBounds.getCentre());
        }
    }

    int getHeightNeeded()
    {
        auto height = 0;

        for (auto* p : parameters)
            height += p->getPreferredHeight();

        return height + 10;
    }

private:
    std::vector<DSPDemoParameterBase*> parameters;
    OwnedArray<Label> labels;
};

//==============================================================================
template <class DemoType>
struct DSPDemo  : public AudioSource,
                  public ProcessorWrapper<DemoType>,
                  private ChangeListener
{
    DSPDemo (AudioSource& input)
        : inputSource (&input)
    {
        for (auto* p : getParameters())
            p->addChangeListener (this);
    }

    void prepareToPlay (int blockSize, double sampleRate) override
    {
        inputSource->prepareToPlay (blockSize, sampleRate);
        this->prepare ({ sampleRate, (uint32) blockSize, 2 });
    }

    void releaseResources() override
    {
        inputSource->releaseResources();
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (bufferToFill.buffer == nullptr)
        {
            jassertfalse;
            return;
        }

        inputSource->getNextAudioBlock (bufferToFill);

        AudioBlock<float> block (*bufferToFill.buffer,
                                 (size_t) bufferToFill.startSample);

        ScopedLock audioLock (audioCallbackLock);
        this->process (ProcessContextReplacing<float> (block));
    }

    const std::vector<DSPDemoParameterBase*>& getParameters()
    {
        return this->processor.parameters;
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        ScopedLock audioLock (audioCallbackLock);
        static_cast<DemoType&> (this->processor).updateParameters();
    }

    CriticalSection audioCallbackLock;

    AudioSource* inputSource;
};

//==============================================================================
template <class DemoType>
class AudioFileReaderComponent  : public Component,
                                  private TimeSliceThread,
                                  private Value::Listener,
                                  private ChangeListener
{
public:
    //==============================================================================
    AudioFileReaderComponent()
        : TimeSliceThread ("Audio File Reader Thread"),
          header (audioDeviceManager, formatManager, *this)
    {
        loopState.addListener (this);

        formatManager.registerBasicFormats();
        audioDeviceManager.addAudioCallback (&audioSourcePlayer);

       #ifndef JUCE_DEMO_RUNNER
        audioDeviceManager.initialiseWithDefaultDevices (0, 2);
       #endif

        init();
        startThread();

        setOpaque (true);

        addAndMakeVisible (header);

        setSize (800, 250);
    }

    ~AudioFileReaderComponent() override
    {
        signalThreadShouldExit();
        stop();
        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
        waitForThreadToExit (10000);
    }

    void paint (Graphics& g) override
    {
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.fillRect (getLocalBounds());
    }

    void resized() override
    {
        auto r = getLocalBounds();

        header.setBounds (r.removeFromTop (120));

        r.removeFromTop (20);

        if (parametersComponent.get() != nullptr)
            parametersComponent->setBounds (r.removeFromTop (parametersComponent->getHeightNeeded()).reduced (20, 0));
    }

    //==============================================================================
    bool loadURL (const URL& fileToPlay)
    {
        stop();

        audioSourcePlayer.setSource (nullptr);
        getThumbnailComponent().setTransportSource (nullptr);
        transportSource.reset();
        readerSource.reset();

        AudioFormatReader* newReader = nullptr;

       #if ! JUCE_IOS
        if (fileToPlay.isLocalFile())
        {
            newReader = formatManager.createReaderFor (fileToPlay.getLocalFile());
        }
        else
       #endif
        {
            if (newReader == nullptr)
                newReader = formatManager.createReaderFor (fileToPlay.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)));
        }

        reader.reset (newReader);

        if (reader.get() != nullptr)
        {
            readerSource.reset (new AudioFormatReaderSource (reader.get(), false));
            readerSource->setLooping (loopState.getValue());

            init();

            return true;
        }

        return false;
    }

    void togglePlay()
    {
        if (playState.getValue())
            stop();
        else
            play();
    }

    void stop()
    {
        playState = false;

        if (transportSource.get() != nullptr)
        {
            transportSource->stop();
            transportSource->setPosition (0);
        }
    }

    void init()
    {
        if (transportSource.get() == nullptr)
        {
            transportSource.reset (new AudioTransportSource());
            transportSource->addChangeListener (this);

            if (readerSource.get() != nullptr)
            {
                if (auto* device = audioDeviceManager.getCurrentAudioDevice())
                {
                    transportSource->setSource (readerSource.get(), roundToInt (device->getCurrentSampleRate()), this, reader->sampleRate);

                    getThumbnailComponent().setTransportSource (transportSource.get());
                }
            }
        }

        audioSourcePlayer.setSource (nullptr);
        currentDemo.reset();

        if (currentDemo.get() == nullptr)
            currentDemo.reset (new DSPDemo<DemoType> (*transportSource));

        audioSourcePlayer.setSource (currentDemo.get());

        initParameters();
    }

    void play()
    {
        if (readerSource.get() == nullptr)
            return;

        if (transportSource->getCurrentPosition() >= transportSource->getLengthInSeconds()
             || transportSource->getCurrentPosition() < 0)
            transportSource->setPosition (0);

        transportSource->start();
        playState = true;
    }

    void setLooping (bool shouldLoop)
    {
        if (readerSource.get() != nullptr)
            readerSource->setLooping (shouldLoop);
    }

    AudioThumbnailComponent& getThumbnailComponent()    { return header.thumbnailComp; }

    void initParameters()
    {
        auto& parameters = currentDemo->getParameters();

        parametersComponent.reset();

        if (parameters.size() > 0)
        {
            parametersComponent.reset (new DemoParametersComponent (parameters));
            addAndMakeVisible (parametersComponent.get());
        }

        resized();
    }

private:
    //==============================================================================
    class AudioPlayerHeader     : public Component,
                                  private ChangeListener,
                                  private Value::Listener
    {
    public:
        AudioPlayerHeader (AudioDeviceManager& adm,
                           AudioFormatManager& afm,
                           AudioFileReaderComponent& afr)
            : thumbnailComp (adm, afm),
              audioFileReader (afr)
        {
            setOpaque (true);

            addAndMakeVisible (loadButton);
            addAndMakeVisible (playButton);
            addAndMakeVisible (loopButton);

            playButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
            playButton.setColour (TextButton::textColourOffId, Colours::black);

            loadButton.setColour (TextButton::buttonColourId, Colour (0xff797fed));
            loadButton.setColour (TextButton::textColourOffId, Colours::black);

            loadButton.onClick = [this] { openFile(); };
            playButton.onClick = [this] { audioFileReader.togglePlay(); };

            addAndMakeVisible (thumbnailComp);
            thumbnailComp.addChangeListener (this);

            audioFileReader.playState.addListener (this);
            loopButton.getToggleStateValue().referTo (audioFileReader.loopState);
        }

        ~AudioPlayerHeader() override
        {
            audioFileReader.playState.removeListener (this);
        }

        void paint (Graphics& g) override
        {
            g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
            g.fillRect (getLocalBounds());
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            auto buttonBounds = bounds.removeFromLeft (jmin (250, bounds.getWidth() / 4));
            auto loopBounds = buttonBounds.removeFromBottom (30);

            loadButton.setBounds (buttonBounds.removeFromTop (buttonBounds.getHeight() / 2));
            playButton.setBounds (buttonBounds);

            loopButton.setSize (0, 25);
            loopButton.changeWidthToFitText();
            loopButton.setCentrePosition (loopBounds.getCentre());

            thumbnailComp.setBounds (bounds);
        }

        AudioThumbnailComponent thumbnailComp;

    private:
        //==============================================================================
        void openFile()
        {
            audioFileReader.stop();

            if (fileChooser != nullptr)
                return;

            if (! RuntimePermissions::isGranted (RuntimePermissions::readExternalStorage))
            {
                SafePointer<AudioPlayerHeader> safeThis (this);
                RuntimePermissions::request (RuntimePermissions::readExternalStorage,
                                             [safeThis] (bool granted) mutable
                                             {
                                                 if (safeThis != nullptr && granted)
                                                     safeThis->openFile();
                                             });
                return;
            }

            fileChooser.reset (new FileChooser ("Select an audio file...", File(), "*.wav;*.mp3;*.aif"));

            fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                      [this] (const FileChooser& fc) mutable
                                      {
                                          if (fc.getURLResults().size() > 0)
                                          {
                                              auto u = fc.getURLResult();

                                              if (! audioFileReader.loadURL (u))
                                                  NativeMessageBox::showAsync (MessageBoxOptions()
                                                                                 .withIconType (MessageBoxIconType::WarningIcon)
                                                                                 .withTitle ("Error loading file")
                                                                                 .withMessage ("Unable to load audio file"),
                                                                               nullptr);
                                              else
                                                  thumbnailComp.setCurrentURL (u);
                                          }

                                          fileChooser = nullptr;
                                      }, nullptr);
        }

        void changeListenerCallback (ChangeBroadcaster*) override
        {
            if (audioFileReader.playState.getValue())
                audioFileReader.stop();

            audioFileReader.loadURL (thumbnailComp.getCurrentURL());
        }

        void valueChanged (Value& v) override
        {
            playButton.setButtonText (v.getValue() ? "Stop" : "Play");
            playButton.setColour (TextButton::buttonColourId, v.getValue() ? Colour (0xffed797f) : Colour (0xff79ed7f));
        }

        //==============================================================================
        TextButton loadButton { "Load File..." }, playButton { "Play" };
        ToggleButton loopButton { "Loop File" };

        AudioFileReaderComponent& audioFileReader;
        std::unique_ptr<FileChooser> fileChooser;
    };

    //==============================================================================
    void valueChanged (Value& v) override
    {
        if (readerSource.get() != nullptr)
            readerSource->setLooping (v.getValue());
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        if (playState.getValue() && ! transportSource->isPlaying())
            stop();
    }

    //==============================================================================
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    AudioFormatManager formatManager;
    Value playState { var (false) };
    Value loopState { var (false) };

    double currentSampleRate = 44100.0;
    uint32 currentBlockSize = 512;
    uint32 currentNumChannels = 2;

    std::unique_ptr<AudioFormatReader> reader;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    std::unique_ptr<AudioTransportSource> transportSource;
    std::unique_ptr<DSPDemo<DemoType>> currentDemo;

    AudioSourcePlayer audioSourcePlayer;

    AudioPlayerHeader header;

    AudioBuffer<float> fileReadBuffer;

    std::unique_ptr<DemoParametersComponent> parametersComponent;
};
