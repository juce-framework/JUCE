/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             FontVariablesDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases OpenType variable font support.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, vs2026, linux_make, androidstudio,
                   xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        FontVariablesDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/FontListComponent.h"

class VariableListComponent : public Component
{
    struct FontVariablesEditorComponent : public Component
    {
        using Pair = std::pair<std::unique_ptr<Label>, std::unique_ptr<Slider>>;

        explicit FontVariablesEditorComponent (std::function<void()> callback)
            : onVariablesChanged (std::move (callback))
        {
        }

        void clear()
        {
            sliders.clear();
        }

        int getNumSliders() const { return (int) sliders.size(); }

        [[nodiscard]] std::vector<FontVariableSetting> getVariables() const
        {
            std::vector<FontVariableSetting> settings;

            for (const auto& [label, slider] : sliders)
            {
                settings.push_back ({ FontFeatureTag::fromString (label->getText()),
                                      (float) slider->getValue() });
            }

            return settings;
        }

        void setNamedInstance (Span<const FontVariableSetting> instanceSettings)
        {
            jassert (instanceSettings.size() == sliders.size());

            for (const auto& setting : instanceSettings)
            {
                auto iter = std::find_if (sliders.begin(),
                                          sliders.end(),
                                          [&setting] (const auto& pair)
                {
                    return pair.first->getText() == setting.tag.toString();
                });

                if (iter != sliders.end())
                    iter->second->setValue (setting.value, dontSendNotification);
            }

            triggerVariablesChangedEvent();
        }

        void randomise()
        {
            for (auto& [label, slider] : sliders)
            {
                ignoreUnused (label);

                auto range = slider->getRange();
                auto val = jmap (Random::getSystemRandom().nextFloat(),
                                 (float) range.getStart(),
                                 (float) range.getEnd());

                slider->setValue (val, dontSendNotification);
            }

            triggerVariablesChangedEvent();
        }

        void add (FontVariableSetting setting, Range<float> range)
        {
            auto label = std::make_unique<Label>();
            auto slider = std::make_unique<Slider>();

            slider->onValueChange = [this] { triggerVariablesChangedEvent(); };

            label->setText (setting.tag.toString(), dontSendNotification);
            slider->setRange (range.getStart(), range.getEnd(), 0.1);
            slider->setValue (setting.value, dontSendNotification);

            addAndMakeVisible (*label);
            addAndMakeVisible (*slider);

            sliders.push_back ({ std::move (label), std::move (slider) });
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            for (auto& [label, slider] : sliders)
            {
                auto b = bounds.removeFromTop (33);

                label->setBounds (b.removeFromLeft (b.proportionOfWidth (0.1f)));
                slider->setBounds (b);
            }
        }

    private:
        void triggerVariablesChangedEvent()
        {
            NullCheckedInvocation::invoke (onVariablesChanged);
        }

        std::function<void()> onVariablesChanged;
        std::vector<Pair> sliders;
    };

public:
    VariableListComponent()
    {
        addAndMakeVisible (viewport);
        addAndMakeVisible (instanceComboBox);
        addAndMakeVisible (randomiseButton);

        viewport.setScrollBarsShown (true, false, false, false);
        viewport.setViewedComponent (&container, false);

        instanceComboBox.setEditableText (false);
        instanceComboBox.setTextWhenNothingSelected ("Click to select a named instance");
        instanceComboBox.onChange = [this]
        {
            if (auto id = instanceComboBox.getSelectedItemIndex(); id >= 0)
                container.setNamedInstance (namedInstances[(size_t) id].second);
        };

        randomiseButton.setButtonText ("Randomise");
        randomiseButton.onClick = [this]
        {
            container.randomise();
        };
    }

    std::function<void()> onVariablesChanged;

    [[nodiscard]] std::vector<FontVariableSetting> getVariables() const
    {
        return container.getVariables();
    }

    void setFont (Typeface::Ptr face)
    {
        container.clear();
        instanceComboBox.clear (dontSendNotification);
        instanceComboBox.setSelectedId (0);
        namedInstances.clear();

        for (auto tag : face->getSupportedVariables())
        {
            FontVariableSetting setting { tag, *face->getDefaultValueForVariable (tag) };
            container.add (setting, *face->getRangeForVariable (tag));
        }

        for (const auto& name : face->getInstanceNames())
        {
            auto settings = face->getNamedInstanceConfiguration (name);
            namedInstances.push_back ({ name, { settings.begin(), settings.end() } });
        }

        if (! namedInstances.empty())
        {
            int index = 1;

            for (const auto& instance : namedInstances)
                instanceComboBox.addItem (instance.first, index++);
        }

        resized();
        container.resized();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        randomiseButton.setBounds (bounds.removeFromBottom (33));
        instanceComboBox.setBounds (bounds.removeFromBottom (33));

        viewport.setBounds (bounds);

        container.setSize (viewport.getWidth() - viewport.getScrollBarThickness(),
                           container.getNumSliders() * 33);
    }

    Viewport viewport;
    FontVariablesEditorComponent container { [this] { onVariablesChanged(); } };
    ComboBox instanceComboBox;
    TextButton randomiseButton;
    std::vector<std::pair<String, std::vector<FontVariableSetting>>> namedInstances;
};

//==============================================================================
class FontVariablesDemo : public Component
{
public:
    FontVariablesDemo()
    {
        fontsListBox.onFontSelected = [this]
        {
            auto typeface = fontsListBox.getFaceForRow (fontsListBox.getSelectedRow());
            variableListBox.setFont (typeface);
            setFont (typeface, {});
        };

        variableListBox.onVariablesChanged = [this]
        {
            setFont (fontsListBox.getFaceForRow (fontsListBox.getSelectedRow()),
                     variableListBox.getVariables());
        };

        fontsListBox.rescanAndRemoveIf ([] (const Font& font)
        {
            return font.getTypefacePtr()->getSupportedVariables().empty();
        });

        fontsListBox.selectRow (0);

        textLabel.setColour (Label::ColourIds::backgroundColourId, Colours::white);
        textLabel.setColour (Label::ColourIds::backgroundWhenEditingColourId, Colours::white);
        textLabel.setColour (Label::ColourIds::textColourId, Colours::black);
        textLabel.setColour (Label::ColourIds::textWhenEditingColourId, Colours::black);
        textLabel.setMinimumHorizontalScale (1.0f);

        textLabel.onEditorShow = [this]
        {
            textLabel.getCurrentTextEditor()->setJustification (textLabel.getJustificationType());
        };

        textLabel.setJustificationType (Justification::centred);
        textLabel.setText ("The quick brown fox jumps over the lazy dog", dontSendNotification);
        textLabel.setEditable (false, true);

        addAndMakeVisible (fontsListBox);
        addAndMakeVisible (textLabel);
        addAndMakeVisible (variableListBox);

        setSize (750, 750);
    }

    void resized() override
    {
        static constexpr auto padding = 5;

        auto bounds = getLocalBounds().reduced (padding);

        fontsListBox.setBounds (bounds.removeFromLeft (bounds.proportionOfWidth (0.3f))
                                      .reduced (padding));

        textLabel.setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.4f))
                                   .reduced (padding));
        variableListBox.setBounds (bounds.reduced (padding));
    }

    void setFont (Typeface::Ptr typeface, const std::vector<FontVariableSetting>& variables)
    {
        FontOptions options;

        options = options.withPointHeight (60);
        options = options.withStyle ({});
        options = options.withName ({});
        options = options.withTypeface (typeface->cloneWithVariableSettings (variables));

        textLabel.setFont (options);
    }

private:
    FontListComponent fontsListBox;
    Label textLabel;
    VariableListComponent variableListBox;

    JUCE_DECLARE_NON_COPYABLE (FontVariablesDemo)
};
