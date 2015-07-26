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


class RSAComponent  : public Component,
                      private Button::Listener
{
public:
    RSAComponent()
    {
        addAndMakeVisible (rsaGroup);
        rsaGroup.setText ("RSA Encryption");
        rsaGroup.setColour (GroupComponent::outlineColourId, Colours::darkgrey);
        rsaGroup.setColour (GroupComponent::textColourId, Colours::black);

        bitSizeLabel.setText ("Num Bits to Use:", dontSendNotification);
        bitSizeLabel.attachToComponent (&bitSize, true);

        addAndMakeVisible (bitSize);
        bitSize.setText (String (256));

        addAndMakeVisible (generateRSAButton);
        generateRSAButton.setButtonText ("Generate RSA");
        generateRSAButton.addListener (this);

        addAndMakeVisible (rsaResultBox);
        rsaResultBox.setColour (TextEditor::backgroundColourId, Colours::white.withAlpha (0.5f));
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
        int bits = jlimit (32, 512, bitSize.getText().getIntValue());
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
        hashGroup.setColour (GroupComponent::outlineColourId, Colours::darkgrey);
        hashGroup.setColour (GroupComponent::textColourId, Colours::black);

        addAndMakeVisible (hashEntryBox);
        hashEntryBox.setMultiLine (true);
        hashEntryBox.setColour (TextEditor::backgroundColourId, Colours::white.withAlpha (0.5f));

        hashEntryBox.setReturnKeyStartsNewLine (true);
        hashEntryBox.setText ("Type some text in this box and the resulting MD5 and SHA hashes will update below");
        hashEntryBox.addListener (this);

        hashLabel1.setText ("Text to Hash:", dontSendNotification);
        hashLabel2.setText ("MD5 Result:", dontSendNotification);
        hashLabel3.setText ("SHA Result:", dontSendNotification);

        hashLabel1.attachToComponent (&hashEntryBox, true);
        hashLabel2.attachToComponent (&md5Result, true);
        hashLabel3.attachToComponent (&shaResult, true);

        addAndMakeVisible (md5Result);
        addAndMakeVisible (shaResult);

        updateHashes();
    }

    void updateHashes()
    {
        updateMD5Result();
        updateSHA256Result();
    }

    void updateMD5Result()
    {
        const MD5 md5 (hashEntryBox.getText().toUTF8());

        md5Result.setText (md5.toHexString(), dontSendNotification);
    }

    void updateSHA256Result()
    {
        const SHA256 sha (hashEntryBox.getText().toUTF8());

        shaResult.setText (sha.toHexString(), dontSendNotification);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        hashGroup.setBounds (area);
        area.removeFromLeft (80);
        area.removeFromTop (10);
        area.reduce (5, 5);
        shaResult.setBounds (area.removeFromBottom (34).reduced (5));
        md5Result.setBounds (area.removeFromBottom (34).reduced (5));
        hashEntryBox.setBounds (area.reduced (5));
    }

private:
    GroupComponent hashGroup;
    TextEditor hashEntryBox;
    Label md5Result, shaResult;
    Label hashLabel1, hashLabel2, hashLabel3;

    void textEditorTextChanged (TextEditor& editor) override
    {
        if (&editor == &hashEntryBox)
            updateHashes();
    }

    void textEditorReturnKeyPressed (TextEditor& editor) override
    {
        if (&editor == &hashEntryBox)
            updateHashes();
    }

    void textEditorEscapeKeyPressed (TextEditor& editor) override
    {
        if (&editor == &hashEntryBox)
            updateHashes();
    }

    void textEditorFocusLost (TextEditor& editor) override
    {
        if (&editor == &hashEntryBox)
            updateHashes();
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
        g.fillAll (Colour::greyLevel (0.4f));
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
