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
        std::unique_ptr<XmlElement> svg (XmlDocument::parse (thumbSvg));
        jassert (svg != nullptr);

        thumb.reset (Drawable::createFromSVG (*svg));

        // svg for thumbnail background highlight
        std::unique_ptr<XmlElement> backSvg (XmlDocument::parse (BinaryData::wizard_Highlight_svg));
        jassert (backSvg != nullptr);

        hoverBackground.reset (Drawable::createFromSVG (*backSvg));

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
        thumb->setBoundsToFit (getLocalBounds(), Justification::centred, false);
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
    void clicked() override
    {
        StringPairArray data;
        data.set ("label", getName());

        Analytics::getInstance()->logEvent ("Start Page Button", data, ProjucerAnalyticsEvent::startPageEvent);
    }

    std::unique_ptr<Drawable> thumb, hoverBackground;
    String name, description;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateOptionButton)
};



//==============================================================================
/**
    Project Template Component for front page.
    Features multiple icon buttons to select the type of project template
*/
class TemplateTileBrowser   : public Component
{
public:
    TemplateTileBrowser (WizardComp* projectWizard)
    {
        const int numWizardButtons = getNumWizards() - 1; // ( - 1 because the last one is blank)

        for (int i = 0; i < numWizardButtons; ++i)
        {
            auto wizard = createWizardType (i);

            TemplateOptionButton* b = new TemplateOptionButton (wizard->getName(),
                                                                TemplateOptionButton::ImageFitted,
                                                                wizard->getIcon());
            optionButtons.add (b);
            addAndMakeVisible (b);
            b->setDescription (wizard->getDescription());
            b->onClick = [this, b] { showWizardButton (b); };
            b->onStateChange = [this] { repaint(); };
        }

        // Handle Open Project button functionality
        ApplicationCommandManager& commandManager = ProjucerApplication::getCommandManager();

        addAndMakeVisible (blankProjectButton);
        addAndMakeVisible (openProjectButton);
        addAndMakeVisible (browseDemosButton);
        addAndMakeVisible (viewTutorialsButton);

        blankProjectButton.onClick   = [this] { createBlankProject(); };
        openProjectButton.setCommandToTrigger (&commandManager, CommandIDs::open, true);
        browseDemosButton.setCommandToTrigger (&commandManager, CommandIDs::launchDemoRunner, true);
        viewTutorialsButton.setCommandToTrigger (&commandManager, CommandIDs::showTutorials, true);

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
                g.drawFittedText (optionButtons.getUnchecked(i)->getDescription(), descriptionBox, Justification::centredBottom, 5, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (40, 0);
        bounds.removeFromTop (60);

        {
            auto optionBounds = bounds.removeFromTop (roundToInt (bounds.getHeight() * 0.65f));

            auto topSlice = optionBounds.removeFromTop (optionBounds.getHeight() / 2).reduced (0, 10);
            auto bottomSlice = optionBounds.reduced (0, 10);

            const int numHorizontal = 4;

            for (int i = 0; i < optionButtons.size(); ++i)
            {
                auto& sliceToUse = (i < numHorizontal ? topSlice : bottomSlice);

                optionButtons.getUnchecked (i)->setBounds (sliceToUse.removeFromLeft (sliceToUse.getWidth() / (4 - (i % 4))).reduced (10, 0));
            }
        }

        bounds.removeFromTop (20);
        auto topButtonBounds = bounds.removeFromTop (50);

        openProjectButton.setBounds (topButtonBounds.reduced (80, 0));

        bounds.removeFromTop (10);
        auto bottomButtonBounds = bounds.removeFromTop (35);

        blankProjectButton.setBounds  (bottomButtonBounds.removeFromLeft (bottomButtonBounds.getWidth() / 3).reduced (10, 0));
        browseDemosButton.setBounds   (bottomButtonBounds.removeFromLeft (bottomButtonBounds.getWidth() / 2).reduced (10, 0));
        viewTutorialsButton.setBounds (bottomButtonBounds.removeFromLeft (bottomButtonBounds.getWidth()).reduced (10, 0));
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

private:
    OwnedArray<TemplateOptionButton> optionButtons;
    NewProjectWizardClasses::WizardComp* newProjectWizard;

    TemplateOptionButton blankProjectButton   { "Create Blank Project",  TemplateOptionButton::ImageOnButtonBackground, BinaryData::wizard_Openfile_svg },
                         openProjectButton    { "Open Existing Project", TemplateOptionButton::ImageOnButtonBackground, BinaryData::wizard_Openfile_svg },
                         browseDemosButton    { "Browse JUCE Demos",     TemplateOptionButton::ImageOnButtonBackground, BinaryData::wizard_Openfile_svg },
                         viewTutorialsButton  { "View JUCE Tutorials",   TemplateOptionButton::ImageOnButtonBackground, BinaryData::wizard_Openfile_svg };

    void showWizardButton (Button* b)
    {
        if (dynamic_cast<TemplateOptionButton*> (b) != nullptr)
            showWizard (b->getButtonText());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateTileBrowser)
};
