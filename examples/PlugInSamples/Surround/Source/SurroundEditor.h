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

class ChannelClickListener
{
public:
    virtual ~ChannelClickListener() {}
    virtual void channelButtonClicked (int channelIndex) = 0;
    virtual bool isChannelActive (int channelIndex) = 0;
};

class SurroundEditor : public AudioProcessorEditor,
                       public Button::Listener,
                       private Timer
{
public:
    SurroundEditor (AudioProcessor& parent)
        : AudioProcessorEditor (parent),
          currentChannelLayout (AudioChannelSet::disabled()),
          noChannelsLabel ("noChannelsLabel", "Input disabled"),
          layoutTitle ("LayoutTitleLabel", getLayoutName())
    {
        layoutTitle.setJustificationType (Justification::centred);
        addAndMakeVisible (layoutTitle);
        addAndMakeVisible (noChannelsLabel);

        setSize (640, 64);

        lastSuspended = ! getAudioProcessor()->isSuspended();
        timerCallback();
        startTimer (500);
    }

    ~SurroundEditor()
    {
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();

        layoutTitle.setBounds (r.removeFromBottom (16));

        noChannelsLabel.setBounds (r);

        if (channelButtons.size() > 0)
        {
            const int buttonWidth = r.getWidth() / channelButtons.size();
            for (int i = 0; i < channelButtons.size(); ++i)
                channelButtons[i]->setBounds (r.removeFromLeft (buttonWidth));
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void buttonClicked (Button* btn) override
    {
        if (TextButton* textButton = dynamic_cast<TextButton*> (btn))
        {
            const int channelIndex = channelButtons.indexOf (textButton);


            if (ChannelClickListener* listener = dynamic_cast<ChannelClickListener*> (getAudioProcessor()))
                listener->channelButtonClicked (channelIndex);
        }
    }

    void updateGUI()
    {
        const AudioChannelSet& channelSet = getAudioProcessor()->getChannelLayoutOfBus (false, 0);

        if (channelSet != currentChannelLayout)
        {
            currentChannelLayout = channelSet;

            layoutTitle.setText (currentChannelLayout.getDescription(), NotificationType::dontSendNotification);
            channelButtons.clear();
            activeChannels.resize (currentChannelLayout.size());

            if (currentChannelLayout == AudioChannelSet::disabled())
            {
                noChannelsLabel.setVisible (true);
            }
            else
            {
                const int numChannels = currentChannelLayout.size();

                for (int i = 0; i < numChannels; ++i)
                {
                    const String channelName =
                    AudioChannelSet::getAbbreviatedChannelTypeName (currentChannelLayout.getTypeOfChannel (i));

                    TextButton* newButton;
                    channelButtons.add (newButton = new TextButton (channelName, channelName));

                    newButton->addListener (this);
                    addAndMakeVisible (newButton);
                }

                noChannelsLabel.setVisible (false);
                resized();
            }

            if (ChannelClickListener* listener = dynamic_cast<ChannelClickListener*> (getAudioProcessor()))
            {
                const auto   activeColour = getLookAndFeel().findColour (Slider::thumbColourId);
                const auto inactiveColour = getLookAndFeel().findColour (Slider::trackColourId);

                for (int i = 0; i < activeChannels.size(); ++i)
                {
                    bool isActive = listener->isChannelActive (i);
                    activeChannels.getReference (i) = isActive;
                    channelButtons [i]->setColour (TextButton::buttonColourId, isActive ? activeColour : inactiveColour);
                    channelButtons [i]->repaint();
                }
            }
        }
    }

private:
    String getLayoutName() const
    {
        if (AudioProcessor* p = getAudioProcessor())
            return p->getChannelLayoutOfBus (false, 0).getDescription();

        return "Unknown";
    }

    void timerCallback() override
    {
        if (getAudioProcessor()->isSuspended() != lastSuspended)
        {
            lastSuspended = getAudioProcessor()->isSuspended();
            updateGUI();
        }

        if (! lastSuspended)
        {
            if (ChannelClickListener* listener = dynamic_cast<ChannelClickListener*> (getAudioProcessor()))
            {
                const auto   activeColour = getLookAndFeel().findColour (Slider::thumbColourId);
                const auto inactiveColour = getLookAndFeel().findColour (Slider::trackColourId);

                for (int i = 0; i < activeChannels.size(); ++i)
                {
                    bool isActive = listener->isChannelActive (i);
                    if (activeChannels.getReference (i) != isActive)
                    {
                        activeChannels.getReference (i) = isActive;
                        channelButtons [i]->setColour (TextButton::buttonColourId, isActive ? activeColour : inactiveColour);
                        channelButtons [i]->repaint();
                    }
                }
            }
        }
    }

    AudioChannelSet currentChannelLayout;
    Label noChannelsLabel, layoutTitle;
    OwnedArray<TextButton> channelButtons;
    Array<bool> activeChannels;

    bool lastSuspended;
};
