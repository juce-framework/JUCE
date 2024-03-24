/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class ContentViewComponent final : public Component
{
public:
    ContentViewComponent()
    {
        setTitle ("Content");
        setFocusContainerType (Component::FocusContainerType::focusContainer);

        addAndMakeVisible (logoComponent);

        addAndMakeVisible (fileNameLabel);
        fileNameLabel.setJustificationType (Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        fileNameLabel.setBounds (bounds.removeFromTop (15));

        if (content != nullptr)
            content->setBounds (bounds);
        else
            logoComponent.setBounds (bounds);
    }

    Component* getCurrentComponent() noexcept
    {
        return content.get();
    }

    void setContent (std::unique_ptr<Component> newContent,
                     const String& labelText)
    {
        content = std::move (newContent);
        addAndMakeVisible (content.get());

        fileNameLabel.setVisible (labelText.isNotEmpty());
        fileNameLabel.setText (labelText, dontSendNotification);

        resized();
    }

private:
    class LogoComponent final : public Component
    {
    public:
        void paint (Graphics& g) override
        {
            g.setColour (findColour (defaultTextColourId));

            auto bounds = getLocalBounds();
            bounds.reduce (bounds.getWidth() / 6, bounds.getHeight() / 6);

            g.setFont (15.0f);
            g.drawFittedText (versionInfo, bounds.removeFromBottom (50), Justification::centredBottom, 3);

            if (logo != nullptr)
                logo->drawWithin (g, bounds.withTrimmedBottom (bounds.getHeight() / 4).toFloat(),
                                  RectanglePlacement (RectanglePlacement::centred), 1.0f);
        }

    private:
        std::unique_ptr<Drawable> logo = []() -> std::unique_ptr<Drawable>
        {
            if (auto svg = parseXML (BinaryData::background_logo_svg))
                return Drawable::createFromSVG (*svg);

            jassertfalse;
            return {};
        }();

        String versionInfo = SystemStats::getJUCEVersion()
                             + newLine
                             + ProjucerApplication::getApp().getVersionDescription();
    };

    std::unique_ptr<Component> content;
    LogoComponent logoComponent;
    Label fileNameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentViewComponent)
};
