/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

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

 name:             InAppPurchasesDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases in-app purchases features. To run this demo you must enable the
                   "In-App Purchases Capability" option in the Projucer exporter.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_cryptography, juce_data_structures, juce_events,
                   juce_graphics, juce_gui_basics, juce_gui_extra,
                   juce_product_unlocking
 exporters:        xcode_mac, xcode_iphone, androidstudio

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        InAppPurchasesDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

/*
    To finish the setup of this demo, do the following in the Projucer project:

    1. In the project settings, set the "Bundle Identifier" to com.roli.juceInAppPurchaseSample
    2. In the Android exporter settings, change the following settings:
         - "In-App Billing" - Enabled
         - "Key Signing: key.store" - path to InAppPurchase.keystore file in examples/Assets/Signing
         - "Key Signing: key.store.password" - amazingvoices
         - "Key Signing: key-alias" - InAppPurchase
         - "Key Signing: key.alias.password" - amazingvoices
    3. Re-save the project
*/

//==============================================================================
class VoicePurchases      : private InAppPurchases::Listener
{
public:
    //==============================================================================
    struct VoiceProduct
    {
        const char* identifier;
        const char* humanReadable;
        bool isPurchased, priceIsKnown, purchaseInProgress;
        String purchasePrice;
    };

    //==============================================================================
    VoicePurchases (AsyncUpdater& asyncUpdater)
         : guiUpdater (asyncUpdater)
    {
        voiceProducts = Array<VoiceProduct>(
                        { VoiceProduct {"robot",  "Robot",  true,   true,  false, "Free" },
                          VoiceProduct {"jules",  "Jules",  false,  false, false, "Retrieving price..." },
                          VoiceProduct {"fabian", "Fabian", false,  false, false, "Retrieving price..." },
                          VoiceProduct {"ed",     "Ed",     false,  false, false, "Retrieving price..." },
                          VoiceProduct {"lukasz", "Lukasz", false,  false, false, "Retrieving price..." },
                          VoiceProduct {"jb",     "JB",     false,  false, false, "Retrieving price..." } });
    }

    ~VoicePurchases()
    {
        InAppPurchases::getInstance()->removeListener (this);
    }

    //==============================================================================
    VoiceProduct getPurchase (int voiceIndex)
    {
        if (! havePurchasesBeenRestored)
        {
            havePurchasesBeenRestored = true;
            InAppPurchases::getInstance()->addListener (this);

            InAppPurchases::getInstance()->restoreProductsBoughtList (true);
        }

        return voiceProducts[voiceIndex];
    }

    void purchaseVoice (int voiceIndex)
    {
        if (havePricesBeenFetched && isPositiveAndBelow (voiceIndex, voiceProducts.size()))
        {
            auto& product = voiceProducts.getReference (voiceIndex);

            if (! product.isPurchased)
            {
                purchaseInProgress = true;

                product.purchaseInProgress = true;
                InAppPurchases::getInstance()->purchaseProduct (product.identifier);

                guiUpdater.triggerAsyncUpdate();
            }
        }
    }

    StringArray getVoiceNames() const
    {
        StringArray names;

        for (auto& voiceProduct : voiceProducts)
            names.add (voiceProduct.humanReadable);

        return names;
    }

    bool isPurchaseInProgress() const noexcept { return purchaseInProgress; }

private:
    //==============================================================================
    void productsInfoReturned (const Array<InAppPurchases::Product>& products) override
    {
        if (! InAppPurchases::getInstance()->isInAppPurchasesSupported())
        {
            for (auto idx = 1; idx < voiceProducts.size(); ++idx)
            {
                auto& voiceProduct = voiceProducts.getReference (idx);

                voiceProduct.isPurchased  = false;
                voiceProduct.priceIsKnown = false;
                voiceProduct.purchasePrice = "In-App purchases unavailable";
            }

            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "In-app purchase is unavailable!",
                                              "In-App purchases are not available. This either means you are trying "
                                              "to use IAP on a platform that does not support IAP or you haven't setup "
                                              "your app correctly to work with IAP.",
                                              "OK");
        }
        else
        {
            for (auto product : products)
            {
                auto idx = findVoiceIndexFromIdentifier (product.identifier);

                if (isPositiveAndBelow (idx, voiceProducts.size()))
                {
                    auto& voiceProduct = voiceProducts.getReference (idx);

                    voiceProduct.priceIsKnown = true;
                    voiceProduct.purchasePrice = product.price;
                }
            }

            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Your credit card will be charged!",
                                              "You are running the sample code for JUCE In-App purchases. "
                                              "Although this is only sample code, it will still CHARGE YOUR CREDIT CARD!",
                                              "Understood!");
        }

        guiUpdater.triggerAsyncUpdate();
    }

    void productPurchaseFinished (const PurchaseInfo& info, bool success, const String&) override
    {
        purchaseInProgress = false;

        auto idx = findVoiceIndexFromIdentifier (info.purchase.productId);

        if (isPositiveAndBelow (idx, voiceProducts.size()))
        {
            auto& voiceProduct = voiceProducts.getReference (idx);

            voiceProduct.isPurchased = success;
            voiceProduct.purchaseInProgress = false;
        }
        else
        {
            // On failure Play Store will not tell us which purchase failed
            for (auto& voiceProduct : voiceProducts)
                voiceProduct.purchaseInProgress = false;
        }

        guiUpdater.triggerAsyncUpdate();
    }

    void purchasesListRestored (const Array<PurchaseInfo>& infos, bool success, const String&) override
    {
        if (success)
        {
            for (auto& info : infos)
            {
                auto idx = findVoiceIndexFromIdentifier (info.purchase.productId);

                if (isPositiveAndBelow (idx, voiceProducts.size()))
                {
                    auto& voiceProduct = voiceProducts.getReference (idx);

                    voiceProduct.isPurchased = true;
                }
            }

            guiUpdater.triggerAsyncUpdate();
        }

        if (! havePricesBeenFetched)
        {
            havePricesBeenFetched = true;
            StringArray identifiers;

            for (auto& voiceProduct : voiceProducts)
                identifiers.add (voiceProduct.identifier);

            InAppPurchases::getInstance()->getProductsInformation (identifiers);
        }
    }

    //==============================================================================
    int findVoiceIndexFromIdentifier (String identifier) const
    {
        identifier = identifier.toLowerCase();

        for (auto i = 0; i < voiceProducts.size(); ++i)
            if (String (voiceProducts.getReference (i).identifier) == identifier)
                return i;

        return -1;
    }

    //==============================================================================
    AsyncUpdater& guiUpdater;
    bool havePurchasesBeenRestored = false, havePricesBeenFetched = false, purchaseInProgress = false;
    Array<VoiceProduct> voiceProducts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoicePurchases)
};

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
    StringArray phrases {"I love JUCE!", "The five dimensions of touch", "Make it fast!"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhraseModel)
};

//==============================================================================
class VoiceModel  : public ListBoxModel
{
public:
    //==============================================================================
    class VoiceRow  : public Component,
                      private Timer
    {
    public:
        VoiceRow (VoicePurchases& voicePurchases) : purchases (voicePurchases)
        {
            addAndMakeVisible (nameLabel);
            addAndMakeVisible (purchaseButton);
            addAndMakeVisible (priceLabel);

            purchaseButton.onClick = [this] { clickPurchase(); };

            voices = purchases.getVoiceNames();

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
                                                                    voiceIconBounds.getX(),
                                                                    voiceIconBounds.getY(),
                                                                    voiceIconBounds.getWidth(),
                                                                    voiceIconBounds.getHeight());
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
                auto imageResourceName = voices[rowNumber] + ".png";

                nameLabel.setText (voices[rowNumber], NotificationType::dontSendNotification);

                auto purchase = purchases.getPurchase (rowNumber);
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

                if (auto* assetStream = createAssetInputStream (String ("Purchases/" + String (imageResourceName)).toRawUTF8()))
                {
                    std::unique_ptr<InputStream> fileStream (assetStream);
                    avatar = PNGImageFormat().decodeImage (*fileStream);
                }
            }
        }
    private:
        //==============================================================================
        void clickPurchase()
        {
            if (rowSelected >= 0)
            {
                if (! hasBeenPurchased)
                {
                    purchases.purchaseVoice (rowSelected);
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

        VoicePurchases& purchases;

        Label nameLabel, priceLabel;
        TextButton purchaseButton {"Purchase"};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceRow)
    };

    //==============================================================================
    VoiceModel (VoicePurchases& voicePurchases) : purchases (voicePurchases)
    {
        voiceProducts = purchases.getVoiceNames();
    }

    int getNumRows() override    { return voiceProducts.size(); }

    Component* refreshComponentForRow (int row, bool selected, Component* existing) override
    {
        if (isPositiveAndBelow (row, voiceProducts.size()))
        {
            if (existing == nullptr)
                existing = new VoiceRow (purchases);

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

    VoicePurchases& purchases;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceModel)
};

//==============================================================================
class InAppPurchasesDemo : public Component,
                           private AsyncUpdater
{
public:
    InAppPurchasesDemo()
    {
        Desktop::getInstance().getDefaultLookAndFeel().setUsingNativeAlertWindows (true);

        dm.addAudioCallback (&player);
        dm.initialiseWithDefaultDevices (0, 2);

        setOpaque (true);

        phraseListBox.setModel (phraseModel.get());
        voiceListBox .setModel (voiceModel.get());

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

        playStopButton.onClick = [this] { playStopPhrase(); };

        soundNames = purchases.getVoiceNames();

       #if JUCE_ANDROID || JUCE_IOS
        auto screenBounds = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
        setSize (screenBounds.getWidth(), screenBounds.getHeight());
       #else
        setSize (800, 600);
       #endif
    }

    ~InAppPurchasesDemo()
    {
        dm.closeAudioDevice();
        dm.removeAudioCallback (&player);
    }

private:
    //==============================================================================
    void handleAsyncUpdate() override
    {
        voiceListBox.updateContent();
        voiceListBox.setEnabled (! purchases.isPurchaseInProgress());
        voiceListBox.repaint();
    }

    //==============================================================================
    void resized() override
    {
        auto r = getLocalBounds().reduced (20);

        {
            auto phraseArea = r.removeFromTop (r.getHeight() / 2);

            phraseLabel   .setBounds (phraseArea.removeFromTop (36).reduced (0, 10));
            playStopButton.setBounds (phraseArea.removeFromBottom (50).reduced (0, 10));
            phraseListBox .setBounds (phraseArea);
        }

        {
            auto voiceArea = r;

            voiceLabel  .setBounds (voiceArea.removeFromTop (36).reduced (0, 10));
            voiceListBox.setBounds (voiceArea);
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Desktop::getInstance().getDefaultLookAndFeel()
                      .findColour (ResizableWindow::backgroundColourId));
    }

    //==============================================================================
    void playStopPhrase()
    {
        auto idx = voiceListBox.getSelectedRow();
        if (isPositiveAndBelow (idx, soundNames.size()))
        {
            auto assetName = "Purchases/" + soundNames[idx] + String (phraseListBox.getSelectedRow()) + ".ogg";

            if (auto* assetStream = createAssetInputStream (assetName.toRawUTF8()))
            {
                std::unique_ptr<InputStream> fileStream (assetStream);

                currentPhraseData.reset();
                fileStream->readIntoMemoryBlock (currentPhraseData);

                player.play (currentPhraseData.getData(), currentPhraseData.getSize());
            }
        }
    }

    //==============================================================================
    StringArray soundNames;

    Label phraseLabel                          { "phraseLabel", NEEDS_TRANS ("Phrases:") };
    ListBox phraseListBox                      { "phraseListBox" };
    std::unique_ptr<ListBoxModel> phraseModel  { new PhraseModel() };
    TextButton playStopButton                  { "Play" };

    SoundPlayer player;
    VoicePurchases purchases                   { *this };
    AudioDeviceManager dm;

    Label voiceLabel                           { "voiceLabel", NEEDS_TRANS ("Voices:") };
    ListBox voiceListBox                       { "voiceListBox" };
    std::unique_ptr<VoiceModel> voiceModel     { new VoiceModel (purchases) };

    MemoryBlock currentPhraseData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InAppPurchasesDemo)
};
