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

#include "../JuceDemoHeader.h"


class RSAComponent  : public Component,
                      private Button::Listener
{
public:
    RSAComponent()
    {
        addAndMakeVisible (rsaGroup);
        rsaGroup.setText ("RSA Encryption");

        bitSizeLabel.setText ("Num Bits to Use:", dontSendNotification);
        bitSizeLabel.attachToComponent (&bitSize, true);

        addAndMakeVisible (bitSize);
        bitSize.setText (String (256));

        addAndMakeVisible (generateRSAButton);
        generateRSAButton.setButtonText ("Generate RSA");
        generateRSAButton.addListener (this);

        addAndMakeVisible (rsaResultBox);
        rsaResultBox.setReadOnly (true);
        rsaResultBox.setMultiLine (true);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        rsaGroup.setBounds (area);
        area.removeFromTop (10);
        area.reduce (5, 5);

        Rectangle<int> topArea (area.removeFromTop (34));
        topArea.removeFromLeft (110);
        bitSize.setBounds (topArea.removeFromLeft (topArea.getWidth() / 2).reduced (5));
        generateRSAButton.setBounds (topArea.reduced (5));

        rsaResultBox.setBounds (area.reduced (5));
    }

private:
    void createRSAKey()
    {
        int bits = jlimit (32, 1024, bitSize.getText().getIntValue());
        bitSize.setText (String (bits), dontSendNotification);

        // Create a key-pair...
        RSAKey publicKey, privateKey;
        RSAKey::createKeyPair (publicKey, privateKey, bits);

        // Test the new key on a piece of data...
        BigInteger testValue;
        testValue.parseString ("1234567890abcdef", 16);

        BigInteger encodedValue (testValue);
        publicKey.applyToValue (encodedValue);

        BigInteger decodedValue (encodedValue);
        privateKey.applyToValue (decodedValue);

        // ..and show the results..
        String message;
        message << "Number of bits: " << bits << newLine
                << "Public Key: " << publicKey.toString() << newLine
                << "Private Key: " << privateKey.toString() << newLine
                << newLine
                << "Test input: " << testValue.toString (16) << newLine
                << "Encoded: " << encodedValue.toString (16) << newLine
                << "Decoded: " << decodedValue.toString (16) << newLine;

        rsaResultBox.setText (message, false);
    }

    GroupComponent rsaGroup;
    TextButton generateRSAButton;
    Label bitSizeLabel;
    TextEditor bitSize, rsaResultBox;

    void buttonClicked (Button* buttonThatWasClicked) override
    {
        if (buttonThatWasClicked == &generateRSAButton)
            createRSAKey();
    }

    void lookAndFeelChanged() override
    {
        rsaGroup.setColour (GroupComponent::outlineColourId,
                            getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::outline,
                                                    Colours::grey));
        rsaGroup.setColour (GroupComponent::textColourId,
                            getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::defaultText,
                                                    Colours::white));
        rsaResultBox.setColour (TextEditor::backgroundColourId,
                                getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::widgetBackground,
                                                        Colours::white.withAlpha (0.5f)));

        bitSize.applyFontToAllText (bitSize.getFont());
        rsaResultBox.applyFontToAllText (rsaResultBox.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSAComponent)
};

//==============================================================================
class HashesComponent  : public Component,
                         private TextEditor::Listener
{
public:
    HashesComponent()
    {
        addAndMakeVisible (hashGroup);
        hashGroup.setText ("Hashes");

        addAndMakeVisible (hashEntryBox);
        hashEntryBox.setMultiLine (true);

        hashEntryBox.setReturnKeyStartsNewLine (true);
        hashEntryBox.setText ("Type some text in this box and the resulting MD5, SHA and Whirlpool hashes will update below");
        hashEntryBox.addListener (this);

        hashLabel1.setText ("Text to Hash:", dontSendNotification);
        hashLabel2.setText ("MD5 Result:", dontSendNotification);
        hashLabel3.setText ("SHA Result:", dontSendNotification);
        hashLabel4.setText ("Whirlpool Result:", dontSendNotification);

        hashLabel1.attachToComponent (&hashEntryBox, true);
        hashLabel2.attachToComponent (&md5Result, true);
        hashLabel3.attachToComponent (&shaResult, true);
        hashLabel4.attachToComponent (&whirlpoolResult, true);

        addAndMakeVisible (md5Result);
        addAndMakeVisible (shaResult);
        addAndMakeVisible (whirlpoolResult);

        updateHashes();
    }

    void updateHashes()
    {
        String text = hashEntryBox.getText();
        updateMD5Result (text.toUTF8());
        updateSHA256Result (text.toUTF8());
        updateWhirlpoolResult (text.toUTF8());
    }

    void updateMD5Result (CharPointer_UTF8 text)
    {
        md5Result.setText (MD5 (text).toHexString(), dontSendNotification);
    }

    void updateSHA256Result (CharPointer_UTF8 text)
    {
        shaResult.setText (SHA256 (text).toHexString(), dontSendNotification);
    }

    void updateWhirlpoolResult (CharPointer_UTF8 text)
    {
        whirlpoolResult.setText (Whirlpool (text).toHexString(), dontSendNotification);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        hashGroup.setBounds (area);
        area.removeFromLeft (120);
        area.removeFromTop (10);
        area.reduce (5, 5);
        whirlpoolResult.setBounds (area.removeFromBottom (34));
        shaResult.setBounds (area.removeFromBottom (34));
        md5Result.setBounds (area.removeFromBottom (34));
        hashEntryBox.setBounds (area.reduced (5));
    }

private:
    GroupComponent hashGroup;
    TextEditor hashEntryBox;
    Label md5Result, shaResult, whirlpoolResult;
    Label hashLabel1, hashLabel2, hashLabel3, hashLabel4;

    void textEditorTextChanged (TextEditor&) override        { updateHashes(); }
    void textEditorReturnKeyPressed (TextEditor&) override   { updateHashes(); }
    void textEditorEscapeKeyPressed (TextEditor&) override   { updateHashes(); }
    void textEditorFocusLost (TextEditor&) override          { updateHashes(); }

    void lookAndFeelChanged() override
    {
        hashGroup.setColour (GroupComponent::outlineColourId,
                             getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::outline,
                                                     Colours::grey));
        hashGroup.setColour (GroupComponent::textColourId,
                             getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::defaultText,
                                                     Colours::white));
        hashEntryBox.setColour (TextEditor::backgroundColourId,
                                getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::widgetBackground,
                                                        Colours::white.withAlpha (0.5f)));

        hashEntryBox.applyFontToAllText (hashEntryBox.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HashesComponent)
};

//==============================================================================
class CryptographyDemo  : public Component
{
public:
    CryptographyDemo()
    {
        addAndMakeVisible (rsaDemo);
        addAndMakeVisible (hashDemo);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colour::greyLevel (0.4f)));
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        rsaDemo.setBounds (area.removeFromTop (getHeight() / 2).reduced (5));
        hashDemo.setBounds (area.reduced (5));
    }

private:
    RSAComponent rsaDemo;
    HashesComponent hashDemo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CryptographyDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<CryptographyDemo> demo ("40 Cryptography");
