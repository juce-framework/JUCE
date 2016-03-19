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
};

class SurroundEditor : public AudioProcessorEditor,
                       public ButtonListener,
                       private Timer
{
public:
    SurroundEditor (AudioProcessor& parent)
        : AudioProcessorEditor (parent),
          currentChannelLayout (AudioChannelSet::disabled()),
          noChannelsLabel ("noChannelsLabel", "Input disabled")
    {
        addAndMakeVisible (noChannelsLabel);
        setSize (640, 64);

        timerCallback();
        startTimer (500);
    }

    ~SurroundEditor()
    {
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();

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

private:
    void timerCallback() override
    {
        const AudioChannelSet& channelSet = getAudioProcessor()->busArrangement.outputBuses.getReference (0).channels;

        if (channelSet != currentChannelLayout)
        {
            currentChannelLayout = channelSet;
            channelButtons.clear();

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
        }
    }

    AudioChannelSet currentChannelLayout;
    Label noChannelsLabel;
    OwnedArray<TextButton> channelButtons;
};
