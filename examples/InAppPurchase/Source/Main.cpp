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
#include "../JuceLibraryCode/JuceHeader.h"
#include "VoicePurchases.h"

//==============================================================================
class InAppPurchaseApplication  : public JUCEApplication,
                                  private AsyncUpdater
{
public:
    //==============================================================================
    InAppPurchaseApplication()   : voicePurchases (*this) {}

    //==============================================================================
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return false; }

    //==============================================================================
    void initialise (const String&) override
    {
        Desktop::getInstance().getDefaultLookAndFeel().setUsingNativeAlertWindows (true);

        dm.addAudioCallback (&player);
        dm.initialiseWithDefaultDevices (0, 2);

        mainWindow = new MainWindow;
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        dm.closeAudioDevice();
        dm.removeAudioCallback (&player);
    }

    static InAppPurchaseApplication* getInstance()
    {
        return static_cast<InAppPurchaseApplication*> (JUCEApplication::getInstance());
    }

    //==============================================================================
    SoundPlayer& getPlayer()
    {
        return player;
    }

    VoicePurchases& getPurchases()
    {
        return voicePurchases;
    }

    //==============================================================================
    class MainContentComponent : public Component, private Button::Listener
    {
    public:
        MainContentComponent()
        {
            setOpaque (true);

            phraseListBox.setRowHeight (33);
            phraseListBox.selectRow (0);
            phraseListBox.updateContent();

            voiceListBox.setRowHeight (66);
            voiceListBox.selectRow (0);
            voiceListBox.updateContent();
            voiceListBox.getViewport()->setScrollOnDragEnabled (true);

            addAndMakeVisible (phraseLabel);
            addAndMakeVisible (phraseListBox);
            addAndMakeVisible (playStopButton);
            addAndMakeVisible (voiceLabel);
            addAndMakeVisible (voiceListBox);

            playStopButton.addListener (this);

           #if JUCE_ANDROID || JUCE_IOS
            auto screenBounds = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
            setSize (screenBounds.getWidth(), screenBounds.getHeight());
           #else
            setSize (800, 600);
           #endif
        }

        void updateDisplay()
        {
            voiceListBox.updateContent();
            voiceListBox.repaint();
        }

    private:
        //==============================================================================
        class PhraseModel : public ListBoxModel
        {
        public:
            PhraseModel() {}

            int getNumRows() override    { return phrases.size(); }

            void paintListBoxItem (int row, Graphics& g, int w, int h, bool isSelected) override
            {
                Rectangle<int> r (0, 0, w, h);

                auto& lf = Desktop::getInstance().getDefaultLookAndFeel();
                g.setColour (lf.findColour (isSelected ? TextEditor::highlightColourId : ListBox::backgroundColourId));
                g.fillRect (r);

                g.setColour (lf.findColour (ListBox::textColourId));

                g.setFont (18);

                String phrase = (isPositiveAndBelow (row, phrases.size()) ? phrases[row] : String{});
                g.drawText (phrase, 10, 0, w, h, Justification::centredLeft);
            }

        private:
            static StringArray phrases;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhraseModel)
        };

        //==============================================================================
        class VoiceModel : public ListBoxModel
        {
        public:
            //==============================================================================
            class VoiceRow : public Component, private Button::Listener, private Timer
            {
            public:
                VoiceRow ()    : voices (getInstance()->getPurchases().getVoiceNames())
                {
                    addAndMakeVisible (nameLabel);
                    addAndMakeVisible (purchaseButton);
                    addAndMakeVisible (priceLabel);

                    purchaseButton.addListener (this);

                    setSize (600, 33);
                }

                void paint (Graphics& g) override
                {
                    auto r = getLocalBounds().reduced (4);
                    {
                        auto voiceIconBounds = r.removeFromLeft (r.getHeight());
                        g.setColour (Colours::black);
                        g.drawRect (voiceIconBounds);

                        voiceIconBounds.reduce (1, 1);
                        g.setColour (hasBeenPurchased ? Colours::white : Colours::grey);
                        g.fillRect (voiceIconBounds);

                        g.drawImage (avatar, voiceIconBounds.toFloat());

                        if (! hasBeenPurchased)
                        {
                            g.setColour (Colours::white.withAlpha (0.8f));
                            g.fillRect (voiceIconBounds);

                            if (purchaseInProgress)
                                getLookAndFeel().drawSpinningWaitAnimation (g, Colours::darkgrey,
                                                                            voiceIconBounds.getX(), voiceIconBounds.getY(),
                                                                            voiceIconBounds.getWidth(), voiceIconBounds.getHeight());
                        }
                    }
                }

                void resized() override
                {
                    auto r = getLocalBounds().reduced (4 + 8, 4);
                    auto h = r.getHeight();
                    auto w = static_cast<int> (h * 1.5);

                    r.removeFromLeft (h);
                    purchaseButton.setBounds (r.removeFromRight (w).withSizeKeepingCentre (w, h / 2));

                    nameLabel.setBounds (r.removeFromTop (18));
                    priceLabel.setBounds (r.removeFromTop (18));
                }

                void update (int rowNumber, bool rowIsSelected)
                {
                    isSelected  = rowIsSelected;
                    rowSelected = rowNumber;

                    if (isPositiveAndBelow (rowNumber, voices.size()))
                    {
                        auto imageResourceName = voices[rowNumber] + "_png";

                        nameLabel.setText (voices[rowNumber], NotificationType::dontSendNotification);

                        auto purchase = getInstance()->getPurchases().getPurchase (rowNumber);
                        hasBeenPurchased = purchase.isPurchased;
                        purchaseInProgress = purchase.purchaseInProgress;

                        if (purchaseInProgress)
                            startTimer (1000 / 50);
                        else
                            stopTimer();

                        nameLabel.setFont (Font (16).withStyle (Font::bold | (hasBeenPurchased ? 0 : Font::italic)));
                        nameLabel.setColour (Label::textColourId, hasBeenPurchased ? Colours::white : Colours::grey);

                        priceLabel.setFont (Font (10).withStyle (purchase.priceIsKnown ? 0 : Font::italic));
                        priceLabel.setColour (Label::textColourId, hasBeenPurchased ? Colours::white : Colours::grey);
                        priceLabel.setText (purchase.purchasePrice, NotificationType::dontSendNotification);

                        if (rowNumber == 0)
                        {
                            purchaseButton.setButtonText ("Internal");
                            purchaseButton.setEnabled (false);
                        }
                        else
                        {
                            purchaseButton.setButtonText (hasBeenPurchased ? "Purchased" : "Purchase");
                            purchaseButton.setEnabled (! hasBeenPurchased && purchase.priceIsKnown);
                        }

                        setInterceptsMouseClicks (! hasBeenPurchased, ! hasBeenPurchased);

                        int rawSize;
                        if (auto* rawData = BinaryData::getNamedResource (imageResourceName.toRawUTF8(), rawSize))
                        {
                            MemoryInputStream imageData (rawData, static_cast<size_t> (rawSize), false);
                            avatar = PNGImageFormat().decodeImage (imageData);
                        }
                    }
                }
            private:
                //==============================================================================
                void buttonClicked (Button*) override
                {
                    if (rowSelected >= 0)
                    {
                        if (! hasBeenPurchased)
                        {
                            getInstance()->getPurchases().purchaseVoice (rowSelected);
                            purchaseInProgress = true;
                            startTimer (1000 / 50);
                        }
                    }
                }

                void timerCallback() override   { repaint(); }

                //==============================================================================
                bool isSelected = false, hasBeenPurchased = false, purchaseInProgress = false;
                int rowSelected = -1;
                Image avatar;

                StringArray voices;

                Label nameLabel, priceLabel;
                TextButton purchaseButton {"Purchase"};

                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceRow)
            };

            //==============================================================================
            VoiceModel() : voiceProducts (getInstance()->getPurchases().getVoiceNames()) {}

            int getNumRows() override    { return voiceProducts.size(); }

            Component* refreshComponentForRow (int row, bool selected, Component* existing) override
            {
                if (isPositiveAndBelow (row, voiceProducts.size()))
                {
                    if (existing == nullptr)
                        existing = new VoiceRow;

                    if (auto* voiceRow = dynamic_cast<VoiceRow*> (existing))
                        voiceRow->update (row, selected);

                    return existing;
                }

                return nullptr;
            }

            void paintListBoxItem (int, Graphics& g, int w, int h, bool isSelected) override
            {
                auto r = Rectangle<int> (0, 0, w, h).reduced (4);

                auto& lf = Desktop::getInstance().getDefaultLookAndFeel();
                g.setColour (lf.findColour (isSelected ? TextEditor::highlightColourId : ListBox::backgroundColourId));
                g.fillRect (r);
            }

        private:
            StringArray voiceProducts;
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceModel)
        };

        //==============================================================================
        void resized() override
        {
            auto r = getLocalBounds().reduced (20);

            {
                auto phraseArea = r.removeFromTop (r.getHeight() / 2);

                phraseLabel.setBounds (phraseArea.removeFromTop (36).reduced (0, 10));
                playStopButton.setBounds (phraseArea.removeFromBottom (50).reduced (0, 10));
                phraseListBox.setBounds (phraseArea);
            }

            {
                auto voiceArea = r;

                voiceLabel.setBounds (voiceArea.removeFromTop (36).reduced (0, 10));
                voiceListBox.setBounds (voiceArea);
            }
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Desktop::getInstance().getDefaultLookAndFeel()
                          .findColour (ResizableWindow::backgroundColourId));
        }

        //==============================================================================
        void buttonClicked (Button*) override
        {
            MemoryOutputStream resourceName;

            auto idx = voiceListBox.getSelectedRow();
            if (isPositiveAndBelow (idx, soundNames.size()))
            {
                resourceName << soundNames[idx] << phraseListBox.getSelectedRow() << "_ogg";
                int numBytes;

                if (auto* data = BinaryData::getNamedResource (resourceName.toString().toRawUTF8(), numBytes))
                    getInstance()->getPlayer().play (data, static_cast<size_t> (numBytes));
            }
        }

        //==============================================================================
        StringArray soundNames = getInstance()->getPurchases().getVoiceNames();

        PhraseModel phraseModel;
        Label phraseLabel {"phraseLabel", NEEDS_TRANS ("Phrases:")};
        ListBox phraseListBox { "phraseListBox", &phraseModel };
        TextButton playStopButton {"Play"};

        VoiceModel voiceModel;
        Label voiceLabel {"voiceLabel", NEEDS_TRANS ("Voices:")};
        ListBox voiceListBox { "voiceListBox", &voiceModel };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
    };

    //==============================================================================
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow ()  : DocumentWindow (ProjectInfo::projectName,
                                         Desktop::getInstance().getDefaultLookAndFeel()
                                                               .findColour (ResizableWindow::backgroundColourId),
                                         DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainContentComponent(), true);

           #if JUCE_ANDROID || JUCE_IOS
            setFullScreen (true);
           #else
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    //==============================================================================
    void handleAsyncUpdate() override
    {
        if (auto* contentComponent = dynamic_cast<MainContentComponent*> (mainWindow->getContentComponent()))
            contentComponent->updateDisplay();
    }

    //==============================================================================
    VoicePurchases voicePurchases;
    AudioDeviceManager dm;
    SoundPlayer player;
    ScopedPointer<MainWindow> mainWindow;
    ScopedPointer<AlertWindow> alertWindow;
};

StringArray InAppPurchaseApplication::MainContentComponent::PhraseModel::phrases {{"I love JUCE!", "The five dimensions of touch", "Make it fast!"}};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (InAppPurchaseApplication)
