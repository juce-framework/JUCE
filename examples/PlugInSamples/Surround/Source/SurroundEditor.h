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

class ChannelClickListener
{
public:
    virtual ~ChannelClickListener() {}
    virtual void channelButtonClicked (int channelIndex) = 0;
    virtual bool isChannelActive (int channelIndex) = 0;
};

class SurroundEditor : public AudioProcessorEditor,
                       public ButtonListener,
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
        g.fillAll (Colours::white);
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
                for (int i = 0; i < activeChannels.size(); ++i)
                {
                    bool isActive = listener->isChannelActive (i);
                    activeChannels.getReference (i) = isActive;
                    channelButtons [i]->setColour (TextButton::buttonColourId, isActive ? Colours::lightsalmon : Colours::lightgrey);
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
                for (int i = 0; i < activeChannels.size(); ++i)
                {
                    bool isActive = listener->isChannelActive (i);
                    if (activeChannels.getReference (i) != isActive)
                    {
                        activeChannels.getReference (i) = isActive;
                        channelButtons [i]->setColour (TextButton::buttonColourId, isActive ? Colours::lightsalmon : Colours::lightgrey);
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
