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

#include "../JuceDemoHeader.h"

//==============================================================================
class DemoThumbnailComp  : public Component,
                           public ChangeListener,
                           public FileDragAndDropTarget,
                           public ChangeBroadcaster,
                           private ScrollBar::Listener,
                           private Timer
{
public:
    DemoThumbnailComp (AudioFormatManager& formatManager,
                       AudioTransportSource& transportSource_,
                       Slider& slider)
        : transportSource (transportSource_),
          zoomSlider (slider),
          scrollbar (false),
          thumbnailCache (5),
          thumbnail (512, formatManager, thumbnailCache),
          isFollowingTransport (false)
    {
        thumbnail.addChangeListener (this);

        addAndMakeVisible (scrollbar);
        scrollbar.setRangeLimits (visibleRange);
        scrollbar.setAutoHide (false);
        scrollbar.addListener (this);

        currentPositionMarker.setFill (Colours::white.withAlpha (0.85f));
        addAndMakeVisible (currentPositionMarker);
    }

    ~DemoThumbnailComp()
    {
        scrollbar.removeListener (this);
        thumbnail.removeChangeListener (this);
    }

    void setFile (const File& file)
    {
        if (! file.isDirectory())
        {
            thumbnail.setSource (new FileInputSource (file));
            const Range<double> newRange (0.0, thumbnail.getTotalLength());
            scrollbar.setRangeLimits (newRange);
            setRange (newRange);

            startTimerHz (40);
        }
    }

    File getLastDroppedFile() const noexcept                    { return lastFileDropped; }

    void setZoomFactor (double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            const double newScale = jmax (0.001, thumbnail.getTotalLength() * (1.0 - jlimit (0.0, 0.99, amount)));
            const double timeAtCentre = xToTime (getWidth() / 2.0f);
            setRange (Range<double> (timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5));
        }
    }

    void setRange (Range<double> newRange)
    {
        visibleRange = newRange;
        scrollbar.setCurrentRange (visibleRange);
        updateCursorPosition();
        repaint();
    }

    void setFollowsTransport (bool shouldFollow)
    {
        isFollowingTransport = shouldFollow;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::darkgrey);
        g.setColour (Colours::lightblue);

        if (thumbnail.getTotalLength() > 0.0)
        {
            Rectangle<int> thumbArea (getLocalBounds());
            thumbArea.removeFromBottom (scrollbar.getHeight() + 4);
            thumbnail.drawChannels (g, thumbArea.reduced (2),
                                    visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No audio file selected)", getLocalBounds(), Justification::centred, 2);
        }
    }

    void resized() override
    {
        scrollbar.setBounds (getLocalBounds().removeFromBottom (14).reduced (2));
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    bool isInterestedInFileDrag (const StringArray& /*files*/) override
    {
        return true;
    }

    void filesDropped (const StringArray& files, int /*x*/, int /*y*/) override
    {
        lastFileDropped = File (files[0]);
        sendChangeMessage();
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (canMoveTransport())
            transportSource.setPosition (jmax (0.0, xToTime ((float) e.x)));
    }

    void mouseUp (const MouseEvent&) override
    {
        transportSource.start();
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            double newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
            newStart = jlimit (0.0, jmax (0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

            if (canMoveTransport())
                setRange (Range<double> (newStart, newStart + visibleRange.getLength()));

            if (wheel.deltaY != 0.0f)
                zoomSlider.setValue (zoomSlider.getValue() - wheel.deltaY);

            repaint();
        }
    }


private:
    AudioTransportSource& transportSource;
    Slider& zoomSlider;
    ScrollBar scrollbar;

    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    Range<double> visibleRange;
    bool isFollowingTransport;
    File lastFileDropped;

    DrawableRectangle currentPositionMarker;

    float timeToX (const double time) const
    {
        return getWidth() * (float) ((time - visibleRange.getStart()) / (visibleRange.getLength()));
    }

    double xToTime (const float x) const
    {
        return (x / getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    bool canMoveTransport() const noexcept
    {
        return ! (isFollowingTransport && transportSource.isPlaying());
    }

    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        if (scrollBarThatHasMoved == &scrollbar)
            if (! (isFollowingTransport && transportSource.isPlaying()))
                setRange (visibleRange.movedToStartAt (newRangeStart));
    }

    void timerCallback() override
    {
        if (canMoveTransport())
            updateCursorPosition();
        else
            setRange (visibleRange.movedToStartAt (transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }

    void updateCursorPosition()
    {
        currentPositionMarker.setVisible (transportSource.isPlaying() || isMouseButtonDown());

        currentPositionMarker.setRectangle (Rectangle<float> (timeToX (transportSource.getCurrentPosition()) - 0.75f, 0,
                                                              1.5f, (float) (getHeight() - scrollbar.getHeight())));
    }
};

//==============================================================================
class AudioPlaybackDemo  : public Component,
                           private FileBrowserListener,
                           private Button::Listener,
                           private Slider::Listener,
                           private ChangeListener
{
public:
    AudioPlaybackDemo()
        : deviceManager (MainAppWindow::getSharedAudioDeviceManager()),
          thread ("audio file preview"),
          directoryList (nullptr, thread),
          fileTreeComp (directoryList)
    {
        addAndMakeVisible (zoomLabel);
        zoomLabel.setText ("zoom:", dontSendNotification);
        zoomLabel.setFont (Font (15.00f, Font::plain));
        zoomLabel.setJustificationType (Justification::centredRight);
        zoomLabel.setEditable (false, false, false);
        zoomLabel.setColour (TextEditor::textColourId, Colours::black);
        zoomLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (followTransportButton);
        followTransportButton.setButtonText ("Follow Transport");
        followTransportButton.addListener (this);

        addAndMakeVisible (explanation);
        explanation.setText ("Select an audio file in the treeview above, and this page will display its waveform, and let you play it..", dontSendNotification);
        explanation.setFont (Font (14.00f, Font::plain));
        explanation.setJustificationType (Justification::bottomRight);
        explanation.setEditable (false, false, false);
        explanation.setColour (TextEditor::textColourId, Colours::black);
        explanation.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.setSliderStyle (Slider::LinearHorizontal);
        zoomSlider.setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
        zoomSlider.addListener (this);
        zoomSlider.setSkewFactor (2);

        addAndMakeVisible (thumbnail = new DemoThumbnailComp (formatManager, transportSource, zoomSlider));
        thumbnail->addChangeListener (this);

        addAndMakeVisible (startStopButton);
        startStopButton.setButtonText ("Play/Stop");
        startStopButton.addListener (this);
        startStopButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));

        addAndMakeVisible (fileTreeComp);

        // audio setup
        formatManager.registerBasicFormats();

        directoryList.setDirectory (File::getSpecialLocation (File::userHomeDirectory), true, true);
        thread.startThread (3);

        fileTreeComp.setColour (FileTreeComponent::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
        fileTreeComp.addListener (this);

        deviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);

        setOpaque (true);
    }

    ~AudioPlaybackDemo()
    {
        transportSource.setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);

        deviceManager.removeAudioCallback (&audioSourcePlayer);
        fileTreeComp.removeListener (this);
        thumbnail->removeChangeListener (this);
        followTransportButton.removeListener (this);
        zoomSlider.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (4));

        Rectangle<int> controls (r.removeFromBottom (90));

        explanation.setBounds (controls.removeFromRight (controls.getWidth() / 3));
        Rectangle<int> zoom (controls.removeFromTop (25));
        zoomLabel.setBounds (zoom.removeFromLeft (50));
        zoomSlider.setBounds (zoom);
        followTransportButton.setBounds (controls.removeFromTop (25));
        startStopButton.setBounds (controls);

        r.removeFromBottom (6);
        thumbnail->setBounds (r.removeFromBottom (140));
        r.removeFromBottom (6);
        fileTreeComp.setBounds (r);
    }

private:
    AudioDeviceManager& deviceManager;
    AudioFormatManager formatManager;
    TimeSliceThread thread;
    DirectoryContentsList directoryList;

    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;

    ScopedPointer<DemoThumbnailComp> thumbnail;
    Label zoomLabel, explanation;
    Slider zoomSlider;
    ToggleButton followTransportButton;
    TextButton startStopButton;
    FileTreeComponent fileTreeComp;

    //==============================================================================
    void showFile (const File& file)
    {
        loadFileIntoTransport (file);

        zoomSlider.setValue (0, dontSendNotification);
        thumbnail->setFile (file);
    }

    void loadFileIntoTransport (const File& audioFile)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource = nullptr;

        AudioFormatReader* reader = formatManager.createReaderFor (audioFile);

        if (reader != nullptr)
        {
            currentAudioFileSource = new AudioFormatReaderSource (reader, true);

            // ..and plug it into our transport source
            transportSource.setSource (currentAudioFileSource,
                                       32768,                   // tells it to buffer this many samples ahead
                                       &thread,                 // this is the background thread to use for reading-ahead
                                       reader->sampleRate);     // allows for sample rate correction
        }
    }

    void selectionChanged() override
    {
        showFile (fileTreeComp.getSelectedFile());
    }

    void fileClicked (const File&, const MouseEvent&) override          {}
    void fileDoubleClicked (const File&) override                       {}
    void browserRootChanged (const File&) override                      {}

    void sliderValueChanged (Slider* sliderThatWasMoved) override
    {
        if (sliderThatWasMoved == &zoomSlider)
            thumbnail->setZoomFactor (zoomSlider.getValue());
    }

    void buttonClicked (Button* buttonThatWasClicked) override
    {
        if (buttonThatWasClicked == &startStopButton)
        {
            if (transportSource.isPlaying())
            {
                transportSource.stop();
            }
            else
            {
                transportSource.setPosition (0);
                transportSource.start();
            }
        }
        else if (buttonThatWasClicked == &followTransportButton)
        {
            thumbnail->setFollowsTransport (followTransportButton.getToggleState());
        }
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == thumbnail)
            showFile (thumbnail->getLastDroppedFile());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<AudioPlaybackDemo> demo ("31 Audio: File Playback");
