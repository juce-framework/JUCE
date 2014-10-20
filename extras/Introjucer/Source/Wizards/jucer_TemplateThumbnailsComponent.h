/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCER_TEMPLATETHUMBNAILSCOMPONENT_H_INCLUDED
#define JUCER_TEMPLATETHUMBNAILSCOMPONENT_H_INCLUDED

//=====================================================================================================
/**
Template option tile button.
the drawable button object class for the tile icons and buttons in the TemplateTileBrowser
*/
class TemplateOptionButton   : public DrawableButton
{
public:
    explicit TemplateOptionButton (const String& buttonName, ButtonStyle buttonStyle, const char* thumbSvg)
    : DrawableButton (buttonName, buttonStyle)
    {
        // svg for thumbnail icon
        ScopedPointer<XmlElement> svg (XmlDocument::parse (thumbSvg));
        assert (svg != nullptr);

        thumb = Drawable::createFromSVG (*svg);

        // svg for thumbnail background highlight
        ScopedPointer<XmlElement> backSvg (XmlDocument::parse (BinaryData::iconHighlight_svg));
        assert (backSvg != nullptr);

        hoverBackground = Drawable::createFromSVG (*backSvg);

        name = buttonName;

        description = "<insert description>";
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/) override
    {
        if (isMouseOverButton)
        {
            if (getStyle() == ButtonStyle::ImageFitted)
            {
                hoverBackground->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::centred, 1.0);
                thumb->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::centred, 1.0);
            }
            else
            {
                g.setColour (Colour (243, 146, 0).withAlpha (0.3f));
                g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2, 2), 10.0f);
                g.setColour (Colour (243, 146, 0).withAlpha (1.0f));
                g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2, 2), 10.0f, 2.0f);
            }
        }
        else
        {
            if (getStyle() == ButtonStyle::ImageFitted)
            {
                thumb->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::centred, 1.0);
            }
            else
            {
                g.setColour (Colour (243, 146, 0).withAlpha (1.0f));
                g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2,2), 10.0f, 2.0f);
            }
        }


        Rectangle<float> textTarget;

        // center the text for the text buttons or position the text in the image buttons
        if (getStyle() != ButtonStyle::ImageFitted)
        {
            textTarget = getLocalBounds().toFloat();
        }
        else
        {
            textTarget = RectanglePlacement (RectanglePlacement::centred).appliedTo(thumb->getDrawableBounds(), getLocalBounds().toFloat());
        textTarget = textTarget.removeFromBottom(textTarget.getHeight() * 0.3);
        }

        g.setColour (Colours::white);
        g.drawText (name, textTarget, Justification::centred, true);

    }

    void resized()
    {
        thumb->setBoundsToFit (0, 0, getWidth(), getHeight(), Justification::centred, false);
    }

    void setDescription (String descript)
    {
        description = descript;
    }

    String getDescription()
    {
        return description;
    }

private:
    ScopedPointer<Drawable> thumb;
    ScopedPointer<Drawable> hoverBackground;

    String name;

    String description;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateOptionButton)
};



//=====================================================================================================
/**
Project Template Component for front page.
Features multiple icon buttons to select the type of project template
*/

class TemplateTileBrowser   : public Component,
private Button::Listener
{
public:
    TemplateTileBrowser (NewProjectWizardComponents::WizardComp* projectWizard)
    {
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("GUI Application", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconGui_svg)));
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("Audio Application", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconAudio_svg)));
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("Audio Plug-in", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconPlugin_svg)));
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("Animated Application", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconAnimation_svg)));
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("Opengl Application", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconOpengl_svg)));
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("Console Application", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconConsole_svg)));
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("Static Library", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconStatic_svg)));
        addAndMakeVisible (optionButtons.add (new TemplateOptionButton ("Dynamic Library", TemplateOptionButton::ButtonStyle::ImageFitted, BinaryData::iconDynamic_svg)));

        // Add the descriptions for each button
        optionButtons.getUnchecked(0)->setDescription ("Creates a blank JUCE application with a single window component.");
        optionButtons.getUnchecked(1)->setDescription ("Creates a blank JUCE application with a single window component and Audio and MIDI in/out functions.");
        optionButtons.getUnchecked(2)->setDescription ("Creates a VST or AU audio plug-in for use within a host program. This template features a single window component and Audio/MIDI IO functions");
        optionButtons.getUnchecked(3)->setDescription ("Creates a blank JUCE application with a single window component that updates and draws at 60fps.");
        optionButtons.getUnchecked(4)->setDescription ("Creates a blank JUCE application with a single window component. This component supports all OPENGL drawing features including 3D model import and glsl shaders.");
        optionButtons.getUnchecked(5)->setDescription ("Creates a blank console application with support for all JUCE features.");
        optionButtons.getUnchecked(6)->setDescription ("Creates a Static Library template with support for all JUCE features");
        optionButtons.getUnchecked(7)->setDescription ("Creates a Dynamic Library template with support for all JUCE features");


        // Handle Open Project button functionality
        ApplicationCommandManager& commandManager = IntrojucerApp::getCommandManager();

        blankProjectButton = new TemplateOptionButton ("Create Blank Project", TemplateOptionButton::ButtonStyle::ImageOnButtonBackground, BinaryData::iconOpenfile_svg);

        openProjectButton = new TemplateOptionButton ("Open Existing Project", TemplateOptionButton::ButtonStyle::ImageOnButtonBackground, BinaryData::iconOpenfile_svg);
        openProjectButton->setCommandToTrigger (&commandManager, CommandIDs::open, true);

        exampleProjectButton = new TemplateOptionButton ("Open Example Project", TemplateOptionButton::ButtonStyle::ImageOnButtonBackground, BinaryData::iconOpenfile_svg);
        exampleProjectButton->setCommandToTrigger (&commandManager, CommandIDs::open, true);

        addAndMakeVisible (blankProjectButton);
        addAndMakeVisible (openProjectButton);
        addAndMakeVisible (exampleProjectButton);


        for (TemplateOptionButton* t : optionButtons)
        {
            t->addListener (this);
        }

        newProjectWizard = projectWizard;
    }


    void paint (Graphics& g) override
    {
        g.setColour (Colours::black.withAlpha (0.2f));
        g.fillRect (0, 0, getWidth(), 60);

        g.setColour (Colours::white);
        g.setFont (20);
        g.drawText ("Create New Project", 0, 0, getWidth(), 60, Justification::centred, true);

        // draw the descriptions of each template if hovered;
        // (repaint is called by the button listener on change state)
        Rectangle<int> descriptionBox = getBounds().reduced (30, 30);
        descriptionBox = descriptionBox.removeFromBottom (50);

        g.setColour (Colours::white.withAlpha (0.4f));
        g.setFont(15);

        for ( int i = 0; i < 8; i++ )
        {
            if (optionButtons.getUnchecked (i)->getState() == TemplateOptionButton::ButtonState::buttonOver)
            {
                g.drawFittedText (optionButtons.getUnchecked (i)->getDescription(), descriptionBox, Justification::centred, 5, 1.0f);
            }
        }
    }

    void resized()
    {
        Rectangle<int> allOpts = getBounds().reduced (40, 60);
        allOpts.removeFromBottom (allOpts.getHeight() * 0.25);

        int numHorizIcons = 4;

        int optStep = allOpts.getWidth()/numHorizIcons;

        for (int i = 0; i < 8; i++)
        {
            int yShift = i < numHorizIcons ? 0 : 1;

            Rectangle<int> bounds;

            bounds = Rectangle<int> (allOpts.getX() + i%numHorizIcons*optStep, allOpts.getY() + yShift * allOpts.getHeight() / 2, optStep, allOpts.getHeight() / 2);
            bounds.reduce (10, 10);

            optionButtons.getUnchecked (i)->setBounds (bounds);
        }

        Rectangle<int> openButtonBounds = getBounds();
        openButtonBounds.removeFromBottom (getHeight() * 0.12);
        openButtonBounds = openButtonBounds.removeFromBottom (120);
        openButtonBounds.reduce (50, 40);

        blankProjectButton->setBounds (openButtonBounds.removeFromLeft (optStep - 20));
        exampleProjectButton->setBounds (openButtonBounds.removeFromRight (optStep - 20));
        openProjectButton->setBounds (openButtonBounds.reduced (18, 0));
    }

    void buttonClicked (Button* b) override
    {
        if (b->getButtonText() == "GUI Application") newProjectWizard->projectType.setSelectedItemIndex (0);
        if (b->getButtonText() == "Console Application") newProjectWizard->projectType.setSelectedItemIndex (1);
        if (b->getButtonText() == "Audio Plug-in") newProjectWizard->projectType.setSelectedItemIndex (2);
        if (b->getButtonText() == "Static Library") newProjectWizard->projectType.setSelectedItemIndex (3);
        if (b->getButtonText() == "Dynamic Library") newProjectWizard->projectType.setSelectedItemIndex (4);

        //new templates without actual templates yet
        if (b->getButtonText() == "Animated Application") newProjectWizard->projectType.setSelectedItemIndex (0);
        if (b->getButtonText() == "Audio Application") newProjectWizard->projectType.setSelectedItemIndex (0);
        if (b->getButtonText() == "Opengl Application") newProjectWizard->projectType.setSelectedItemIndex (0);


        SlidingPanelComponent* parent = findParentComponentOfClass<SlidingPanelComponent>();
        jassert (parent != nullptr);

        if (parent->getNumTabs() > 0 && b->getButtonText() != "Open Existing Project") parent->goToTab (parent->getCurrentTabIndex() + 1);
    }

    void buttonStateChanged (Button*)
    {
        repaint();
    }

    private:

    OwnedArray<TemplateOptionButton> optionButtons;

    NewProjectWizardComponents::WizardComp* newProjectWizard;

    ScopedPointer<TemplateOptionButton> blankProjectButton;
    ScopedPointer<TemplateOptionButton> openProjectButton;
    ScopedPointer<TemplateOptionButton> exampleProjectButton;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateTileBrowser)

};

#endif  // JUCER_TEMPLATETHUMBNAILSCOMPONENT_H_INCLUDED
