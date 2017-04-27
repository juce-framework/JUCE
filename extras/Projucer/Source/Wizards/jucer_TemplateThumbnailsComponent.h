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

#pragma once

//==============================================================================
/**
    Template option tile button.
    The drawable button object class for the tile icons and buttons in the TemplateTileBrowser
*/
class TemplateOptionButton   : public DrawableButton
{
public:
    TemplateOptionButton (const String& buttonName, ButtonStyle buttonStyle, const char* thumbSvg)
       : DrawableButton (buttonName, buttonStyle)
    {
        // svg for thumbnail icon
        ScopedPointer<XmlElement> svg (XmlDocument::parse (thumbSvg));
        jassert (svg != nullptr);

        thumb = Drawable::createFromSVG (*svg);

        // svg for thumbnail background highlight
        ScopedPointer<XmlElement> backSvg (XmlDocument::parse (BinaryData::wizard_Highlight_svg));
        jassert (backSvg != nullptr);

        hoverBackground = Drawable::createFromSVG (*backSvg);

        name = buttonName;

        description = "<insert description>";
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/) override
    {
        const Rectangle<float> r (getLocalBounds().toFloat());
        const Colour buttonColour (0xffA35E93);

        if (isMouseOverButton)
        {
            if (getStyle() == ImageFitted)
            {
                hoverBackground->drawWithin (g, r, RectanglePlacement::centred, 1.0);
                thumb->drawWithin (g, r, RectanglePlacement::centred, 1.0);
            }
            else
            {
                g.setColour (buttonColour.withAlpha (0.3f));
                g.fillRoundedRectangle (r.reduced (2.0f, 2.0f), 10.0f);
                g.setColour (buttonColour);
                g.drawRoundedRectangle (r.reduced (2.0f, 2.0f), 10.0f, 2.0f);
            }
        }
        else
        {
            if (getStyle() == ImageFitted)
            {
                thumb->drawWithin (g, r, RectanglePlacement::centred, 1.0);
            }
            else
            {
                g.setColour (buttonColour);
                g.drawRoundedRectangle (r.reduced (2.0f, 2.0f), 10.0f, 2.0f);
            }
        }

        Rectangle<float> textTarget;

        // center the text for the text buttons or position the text in the image buttons
        if (getStyle() != ImageFitted)
        {
            textTarget = getLocalBounds().toFloat();
        }
        else
        {
            textTarget = RectanglePlacement (RectanglePlacement::centred).appliedTo (thumb->getDrawableBounds(), r);
            textTarget = textTarget.removeFromBottom (textTarget.getHeight() * 0.3f);
        }

        g.setColour (findColour (defaultTextColourId));
        g.drawText (name, textTarget, Justification::centred, true);
    }

    void resized() override
    {
        thumb->setBoundsToFit (0, 0, getWidth(), getHeight(), Justification::centred, false);
    }

    void setDescription (String descript) noexcept
    {
        description = descript;
    }

    String getDescription() const noexcept
    {
        return description;
    }

private:
    ScopedPointer<Drawable> thumb, hoverBackground;
    String name, description;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateOptionButton)
};



//==============================================================================
/**
    Project Template Component for front page.
    Features multiple icon buttons to select the type of project template
*/
class TemplateTileBrowser   : public Component,
                              private Button::Listener
{
public:
    TemplateTileBrowser (WizardComp* projectWizard)
    {
        const int numWizardButtons = getNumWizards() - 1; // ( - 1 because the last one is blank)

        for (int i = 0; i < numWizardButtons; ++i)
        {
            ScopedPointer<NewProjectWizard> wizard (createWizardType (i));

            TemplateOptionButton* b = new TemplateOptionButton (wizard->getName(),
                                                                TemplateOptionButton::ImageFitted,
                                                                wizard->getIcon());
            optionButtons.add (b);
            addAndMakeVisible (b);
            b->setDescription (wizard->getDescription());
            b->addListener (this);
        }

        // Handle Open Project button functionality
        ApplicationCommandManager& commandManager = ProjucerApplication::getCommandManager();

        addAndMakeVisible (blankProjectButton   = new TemplateOptionButton ("Create Blank Project",  TemplateOptionButton::ImageOnButtonBackground, BinaryData::wizard_Openfile_svg));
        addAndMakeVisible (exampleProjectButton = new TemplateOptionButton ("Open Example Project",  TemplateOptionButton::ImageOnButtonBackground, BinaryData::wizard_Openfile_svg));
        addAndMakeVisible (openProjectButton    = new TemplateOptionButton ("Open Existing Project", TemplateOptionButton::ImageOnButtonBackground, BinaryData::wizard_Openfile_svg));

        blankProjectButton->addListener (this);
        exampleProjectButton->addListener (this);
        openProjectButton->setCommandToTrigger (&commandManager, CommandIDs::open, true);

        newProjectWizard = projectWizard;
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (contentHeaderBackgroundColourId));
        g.fillRect (getLocalBounds().removeFromTop (60));

        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawText ("Create New Project", 0, 0, getWidth(), 60, Justification::centred, true);

        auto descriptionBox = (getLocalBounds().reduced (30).removeFromBottom (50));

        g.setColour (findColour (defaultTextColourId));
        g.setFont (15.0f);

        for (int i = 0; i < optionButtons.size(); ++i)
            if (optionButtons.getUnchecked(i)->isOver())
                g.drawFittedText (optionButtons.getUnchecked(i)->getDescription(), descriptionBox, Justification::centred, 5, 1.0f);
    }

    void resized() override
    {
        auto allOpts = getLocalBounds().reduced (40, 60);
        allOpts.removeFromBottom (allOpts.getHeight() / 4);

        const auto numHorizIcons = 4;
        const auto optStep = allOpts.getWidth() / numHorizIcons;

        for (int i = 0; i < optionButtons.size(); ++i)
        {
            const auto yShift = i < numHorizIcons ? 0 : 1;

            optionButtons.getUnchecked(i)->setBounds (Rectangle<int> (allOpts.getX() + (i % numHorizIcons) * optStep,
                                                                      allOpts.getY() + yShift * allOpts.getHeight() / 2,
                                                                      optStep, allOpts.getHeight() / 2)
                                                        .reduced (10, 10));
        }

        auto openButtonBounds = getLocalBounds();
        openButtonBounds.removeFromBottom (proportionOfHeight (0.12f));
        openButtonBounds = openButtonBounds.removeFromBottom (120);
        openButtonBounds.reduce (50, 40);

        blankProjectButton->setBounds (openButtonBounds.removeFromLeft (optStep - 20));
        exampleProjectButton->setBounds (openButtonBounds.removeFromRight (optStep - 20));
        openProjectButton->setBounds (openButtonBounds.reduced (18, 0));
    }

    void showWizard (const String& name)
    {
        newProjectWizard->projectType.setText (name);

        if (SlidingPanelComponent* parent = findParentComponentOfClass<SlidingPanelComponent>())
            parent->goToTab (1);
        else
            jassertfalse;
    }

    void createBlankProject()
    {
        showWizard (BlankAppWizard().getName());
    }

    void openExampleProject()
    {
        FileChooser fc ("Open File", findExamplesFolder());

        if (fc.browseForFileToOpen())
            ProjucerApplication::getApp().openFile (fc.getResult());
    }

    static File findExamplesFolder()
    {
        File appFolder (File::getSpecialLocation (File::currentApplicationFile));

        while (appFolder.exists()
                && appFolder.getParentDirectory() != appFolder)
        {
            File examples (appFolder.getSiblingFile ("examples"));

            if (examples.exists())
                return examples;

            appFolder = appFolder.getParentDirectory();
        }

        return {};
    }

private:
    OwnedArray<TemplateOptionButton> optionButtons;
    NewProjectWizardClasses::WizardComp* newProjectWizard;
    ScopedPointer<TemplateOptionButton> blankProjectButton, openProjectButton, exampleProjectButton;

    void buttonClicked (Button* b) override
    {
        if (b == blankProjectButton)
            createBlankProject();
        else if (b == exampleProjectButton)
            openExampleProject();
        else if (dynamic_cast<TemplateOptionButton*> (b) != nullptr)
            showWizard (b->getButtonText());
    }

    void buttonStateChanged (Button*) override
    {
        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateTileBrowser)
};
