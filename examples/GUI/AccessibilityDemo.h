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

 name:             AccessibilityDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple UI demonstrating accessibility features on supported
                   platforms.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AccessibilityDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/**
    A simple holder component with some content, a title and an info tooltip
    containing a brief description.

    This component sets its accessibility title and help text properties and
    also acts as a focus container for its children.
*/
class ContentComponent final : public Component
{
public:
    ContentComponent (const String& title, const String& info, Component& contentToDisplay)
         : titleLabel ({}, title),
           content (contentToDisplay)
    {
        addAndMakeVisible (titleLabel);
        addAndMakeVisible (infoIcon);

        setTitle (title);
        setDescription (info);
        setFocusContainerType (FocusContainerType::focusContainer);

        infoIcon.setTooltip (info);
        infoIcon.setHelpText (info);

        addAndMakeVisible (content);
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colours::black);
        g.drawRoundedRectangle (getLocalBounds().reduced (2).toFloat(), 5.0f, 3.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (5);

        auto topArea = bounds.removeFromTop (30);
        infoIcon.setBounds (topArea.removeFromLeft (30).reduced (5));
        titleLabel.setBounds (topArea.reduced (5));

        content.setBounds (bounds);
    }

private:
    //==============================================================================
    struct InfoIcon final : public Component,
                            public SettableTooltipClient
    {
        InfoIcon()
        {
            constexpr uint8 infoPathData[] =
            {
              110,109,0,0,122,67,0,0,0,0,98,79,35,224,66,0,0,0,0,0,0,0,0,79,35,224,66,0,0,0,0,0,0,122,67,98,0,0,
              0,0,44,247,193,67,79,35,224,66,0,0,250,67,0,0,122,67,0,0,250,67,98,44,247,193,67,0,0,250,67,0,0,
              250,67,44,247,193,67,0,0,250,67,0,0,122,67,98,0,0,250,67,79,35,224,66,44,247,193,67,0,0,0,0,0,0,122,
              67,0,0,0,0,99,109,114,79,101,67,79,35,224,66,108,71,88,135,67,79,35,224,66,108,71,88,135,67,132,229,
              28,67,108,116,79,101,67,132,229,28,67,108,116,79,101,67,79,35,224,66,99,109,79,35,149,67,106,132,
              190,67,108,98,185,123,67,106,132,190,67,98,150,123,106,67,106,132,190,67,176,220,97,67,168,17,187,
              67,176,220,97,67,18,150,177,67,108,176,220,97,67,248,52,108,67,98,176,220,97,67,212,8,103,67,238,
              105,94,67,18,150,99,67,204,61,89,67,18,150,99,67,108,98,185,73,67,18,150,99,67,108,98,185,73,67,88,
              238,59,67,108,160,70,120,67,88,238,59,67,98,54,194,132,67,88,238,59,67,169,17,137,67,60,141,68,67,
              169,17,137,67,8,203,85,67,108,169,17,137,67,26,97,166,67,98,169,17,137,67,43,247,168,67,10,203,138,
              67,141,176,170,67,27,97,141,67,141,176,170,67,108,80,35,149,67,141,176,170,67,108,80,35,149,67,106,
              132,190,67,99,101,0,0
            };

            infoPath.loadPathFromData (infoPathData, numElementsInArray (infoPathData));

            setTitle ("Info");
        }

        void paint (Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced (2);

            g.setColour (Colours::white);
            g.fillPath (infoPath, RectanglePlacement (RectanglePlacement::centred)
                                    .getTransformToFit (infoPath.getBounds(), bounds));
        }

        Path infoPath;
    };

    //==============================================================================
    Label titleLabel;
    InfoIcon infoIcon;
    Component& content;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentComponent)
};

//==============================================================================
/**
    The top-level component containing the accessible JUCE widget examples.

    Most JUCE UI elements have built-in accessibility support and will be
    visible and controllable by accessibility clients. There are a few examples
    of some widgets in this demo such as Sliders, Buttons and a TreeView.
*/
class JUCEWidgetsComponent final : public Component
{
public:
    JUCEWidgetsComponent()
    {
        setTitle ("JUCE Widgets");
        setDescription ("A demo of a few of the accessible built-in JUCE widgets.");
        setFocusContainerType (FocusContainerType::focusContainer);

        addAndMakeVisible (descriptionLabel);

        addAndMakeVisible (buttons);
        addAndMakeVisible (sliders);
        addAndMakeVisible (treeView);
    }

    void resized() override
    {
        Grid grid;

        grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (2)) };
        grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)) };

        grid.items = { GridItem (descriptionLabel).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }),
                       GridItem (buttons).withMargin ({ 2 }),
                       GridItem (sliders).withMargin ({ 2 }),
                       GridItem (treeView).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }) };

        grid.performLayout (getLocalBounds());
    }

private:
    //==============================================================================
    class ButtonsComponent final : public Component
    {
    public:
        ButtonsComponent()
        {
            addAndMakeVisible (radioButtons);

            textButton.setHasFocusOutline (true);
            addAndMakeVisible (textButton);

            shapeButton.setShape (getJUCELogoPath(), false, true, false);
            shapeButton.onClick = [this]
            {
                auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                                 "Alert",
                                                                 "This is an AlertWindow");
                messageBox = AlertWindow::showScopedAsync (options, nullptr);
            };
            shapeButton.setHasFocusOutline (true);
            addAndMakeVisible (shapeButton);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            radioButtons.setBounds (bounds.removeFromLeft (bounds.getWidth() / 2).reduced (5));
            textButton.setBounds (bounds.removeFromTop (bounds.getHeight() / 2).reduced (5));
            shapeButton.setBounds (bounds.reduced (5));
        }

    private:
        //==============================================================================
        struct RadioButtonsGroupComponent final : public Component
        {
            RadioButtonsGroupComponent()
            {
                for (const auto [n, b] : enumerate (radioButtons, 1))
                {
                    b.setRadioGroupId (1);
                    b.setButtonText ("Button " + String (n));
                    b.setHasFocusOutline (true);
                    addAndMakeVisible (b);
                }

                radioButtons[(size_t) Random::getSystemRandom().nextInt ((int) radioButtons.size())].setToggleState (true, dontSendNotification);

                setTitle ("Radio Buttons Group");
                setFocusContainerType (FocusContainerType::focusContainer);
            }

            void resized() override
            {
                auto bounds = getLocalBounds();
                const auto height = bounds.getHeight() / (int) radioButtons.size();

                for (auto& b : radioButtons)
                    b.setBounds (bounds.removeFromTop (height).reduced (2));
            }

            std::array<ToggleButton, 3> radioButtons;
        };

        //==============================================================================
        RadioButtonsGroupComponent radioButtons;
        TextButton textButton { "Press me!" };
        ShapeButton shapeButton { "Pressable JUCE Logo",
                                  Colours::darkorange,
                                  Colours::darkorange.brighter (0.5f),
                                  Colours::darkorange.brighter (0.75f) };
        ScopedMessageBox messageBox;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonsComponent)
    };

    //==============================================================================
    class SlidersComponent final : public Component
    {
    public:
        SlidersComponent()
        {
            auto setUpSlider = [this] (Slider& slider, Slider::SliderStyle style,
                                       double start, double end, double interval)
            {
                slider.setSliderStyle (style);
                slider.setRange (start, end, interval);

                if (style != Slider::IncDecButtons)
                    slider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);

                slider.setValue (start + (end - start) * Random::getSystemRandom().nextDouble());

                addAndMakeVisible (slider);
            };

            setUpSlider (horizontalSlider, Slider::LinearHorizontal, 1.0, 100.0, 1.0);
            setUpSlider (incDecSlider, Slider::IncDecButtons, 1.0, 10.0, 1.0);

            for (auto& rotary : rotarySliders)
                setUpSlider (rotary, Slider::Rotary, 1.0, 10.0, 1.0);
        }

        void resized() override
        {
            Grid grid;

            grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (2)) };

            grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)),
                                     Grid::TrackInfo (Grid::Fr (1)),
                                     Grid::TrackInfo (Grid::Fr (1)) };

            grid.items = { GridItem (horizontalSlider).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }),
                           GridItem (incDecSlider).withMargin ({ 2 }) };

            for (auto& rotary : rotarySliders)
                grid.items.add (GridItem (rotary).withMargin ({ 2 }));

            grid.performLayout (getLocalBounds());
        }

    private:
        Slider horizontalSlider, incDecSlider;
        std::array<Slider, 3> rotarySliders;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidersComponent)
    };

    //==============================================================================
    class TreeViewComponent final : public Component
    {
    public:
        TreeViewComponent()
        {
            tree.setRootItem (&root);
            tree.setRootItemVisible (false);

            addAndMakeVisible (tree);
        }

        void resized() override
        {
            tree.setBounds (getLocalBounds());
        }

    private:
        //==============================================================================
        struct RootItem final : public TreeViewItem
        {
            RootItem()
            {
                struct Item final : public TreeViewItem
                {
                    Item (int index, int depth, int numSubItems)
                        : textToDisplay ("Item " + String (index)
                                         + ". Depth: " + String (depth)
                                         + ". Num sub-items: " + String (numSubItems))
                    {
                        for (int i = 0; i < numSubItems; ++i)
                            addSubItem (new Item (i,
                                                  depth + 1,
                                                  Random::getSystemRandom().nextInt (jmax (0, 5 - depth))));
                    }

                    bool mightContainSubItems() override
                    {
                        return getNumSubItems() > 0;
                    }

                    void paintItem (Graphics& g, int width, int height) override
                    {
                        if (isSelected())
                        {
                            g.setColour (Colours::yellow.withAlpha (0.3f));
                            g.fillRect (0, 0, width, height);
                        }

                        g.setColour (Colours::black);
                        g.drawRect (0, height - 1, width, 1);

                        g.setColour (Colours::white);
                        g.drawText (textToDisplay, 0, 0, width, height, Justification::centredLeft);
                    }

                    String getAccessibilityName() override  { return textToDisplay; }

                    const String textToDisplay;
                };

                for (int i = 0; i < 10; ++i)
                    addSubItem (new Item (i, 0, Random::getSystemRandom().nextInt (10)));
            }

            bool mightContainSubItems() override
            {
                return true;
            }
        };

        //==============================================================================
        TreeView tree;
        RootItem root;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewComponent)
    };

    //==============================================================================
    Label descriptionLabel { {}, "This is a demo of a few of the accessible built-in JUCE widgets.\n\n"
                                 "To navigate this demo with a screen reader, either enable VoiceOver on macOS and iOS, "
                                 "TalkBack on Android, or Narrator on Windows and follow the navigational prompts." };

    ButtonsComponent buttonsComponent;
    SlidersComponent slidersComponent;
    TreeViewComponent treeViewComponent;

    ContentComponent buttons { "Buttons",
                               "Examples of some JUCE buttons.",
                               buttonsComponent };
    ContentComponent sliders { "Sliders",
                               "Examples of some JUCE sliders.",
                               slidersComponent };
    ContentComponent treeView { "TreeView",
                                "A JUCE TreeView.",
                                treeViewComponent };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JUCEWidgetsComponent)
};

struct NameAndRole
{
    const char* name;
    AccessibilityRole role;
};

constexpr NameAndRole accessibilityRoles[]
{
    { "Ignored",       AccessibilityRole::ignored },
    { "Unspecified",   AccessibilityRole::unspecified },
    { "Button",        AccessibilityRole::button },
    { "Toggle",        AccessibilityRole::toggleButton },
    { "ComboBox",      AccessibilityRole::comboBox },
    { "Slider",        AccessibilityRole::slider },
    { "Static Text",   AccessibilityRole::staticText },
    { "Editable Text", AccessibilityRole::editableText },
    { "Image",         AccessibilityRole::image },
    { "Group",         AccessibilityRole::group },
    { "Window",        AccessibilityRole::window }
};

//==============================================================================
/**
    The top-level component containing a customisable accessible widget.

    The AccessibleComponent class just draws a JUCE logo and overrides the
    Component::createAccessibilityHandler() method to return a custom AccessibilityHandler.
    The properties of this handler are set by the various controls in the demo.
*/
class CustomWidgetComponent final : public Component
{
public:
    CustomWidgetComponent()
    {
        setTitle ("Custom Widget");
        setDescription ("A demo of a customisable accessible widget.");
        setFocusContainerType (FocusContainerType::focusContainer);

        addAndMakeVisible (descriptionLabel);

        addAndMakeVisible (infoComponent);
        addAndMakeVisible (actionsComponent);
        addAndMakeVisible (valueInterfaceComponent);
        addAndMakeVisible (stateComponent);
        addAndMakeVisible (accessibleComponent);
    }

    void resized() override
    {
        Grid grid;

        grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)),
                              Grid::TrackInfo (Grid::Fr (2)),
                              Grid::TrackInfo (Grid::Fr (2)),
                              Grid::TrackInfo (Grid::Fr (2)) };

        grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)) };

        grid.items = { GridItem (descriptionLabel).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }),
                       GridItem (infoComponent).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }),
                       GridItem (actionsComponent).withMargin ({ 2 }),
                       GridItem (valueInterfaceComponent).withMargin ({ 2 }),
                       GridItem (stateComponent).withMargin ({ 2 }),
                       GridItem (accessibleComponent).withMargin ({ 10 }) };

        grid.performLayout (getLocalBounds());
    }

private:
    //==============================================================================
    class AccessibleComponent final : public Component
    {
    public:
        explicit AccessibleComponent (CustomWidgetComponent& owner)
            : customWidgetComponent (owner)
        {
        }

        void paint (Graphics& g) override
        {
            g.setColour (Colours::darkorange);

            g.fillPath (juceLogoPath,
                        juceLogoPath.getTransformToScaleToFit (getLocalBounds().toFloat(), true));
        }

        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            /**
                The AccessibilityHandler class is the interface between JUCE components
                and accessibility clients. This derived class represents the properties
                set via the demo UI.
            */
            struct CustomAccessibilityHandler final : public AccessibilityHandler
            {
                explicit CustomAccessibilityHandler (CustomWidgetComponent& comp)
                    : AccessibilityHandler (comp.accessibleComponent,
                                            comp.infoComponent.getRole(),
                                            comp.actionsComponent.getActions(),
                                            { comp.valueInterfaceComponent.getValueInterface() }),
                      customWidgetComponent (comp)
                {
                }

                String getTitle() const override                  { return customWidgetComponent.infoComponent.getTitle(); }
                String getDescription() const override            { return customWidgetComponent.infoComponent.getDescription(); }
                String getHelp() const override                   { return customWidgetComponent.infoComponent.getHelp(); }

                AccessibleState getCurrentState() const override  { return customWidgetComponent.stateComponent.getAccessibleState(); }

                CustomWidgetComponent& customWidgetComponent;
            };

            return std::make_unique<CustomAccessibilityHandler> (customWidgetComponent);
        }

    private:
        CustomWidgetComponent& customWidgetComponent;
        Path juceLogoPath { getJUCELogoPath() };

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibleComponent)
    };

    //==============================================================================
    class InfoComponent final : public Component
    {
    public:
        explicit InfoComponent (CustomWidgetComponent& owner)
        {
            titleEditor.setText ("Custom");
            descriptionEditor.setText ("A short description of the custom component.");
            helpEditor.setText ("Some help text for the custom component.");

            titleEditor.onTextChange = [&owner]
            {
                if (auto* handler = owner.accessibleComponent.getAccessibilityHandler())
                    handler->notifyAccessibilityEvent (AccessibilityEvent::titleChanged);
            };

            for (auto* editor : { &descriptionEditor, &helpEditor })
            {
                editor->setMultiLine (true);
                editor->setReturnKeyStartsNewLine (true);
                editor->setJustification (Justification::centredLeft);
            }

            addAndMakeVisible (titleLabel);
            addAndMakeVisible (titleEditor);

            addAndMakeVisible (descriptionLabel);
            addAndMakeVisible (descriptionEditor);

            addAndMakeVisible (helpLabel);
            addAndMakeVisible (helpEditor);

            setUpAccessibilityRoleSelector (owner);
            addAndMakeVisible (roleBox);
            addAndMakeVisible (roleLabel);
        }

        void resized() override
        {
            Grid grid;

            grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (3)) };

            grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)),
                                     Grid::TrackInfo (Grid::Fr (1)),
                                     Grid::TrackInfo (Grid::Fr (1)),
                                     Grid::TrackInfo (Grid::Fr (1)),
                                     Grid::TrackInfo (Grid::Fr (1)),
                                     Grid::TrackInfo (Grid::Fr (1)) };

            grid.items = { GridItem (titleLabel).withMargin ({ 2 }),
                           GridItem (titleEditor).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }),

                           GridItem (roleLabel).withMargin ({ 2 }),
                           GridItem (roleBox).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }),

                           GridItem (descriptionLabel).withMargin ({ 2 }),
                           GridItem (descriptionEditor).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }),

                           GridItem (helpLabel).withMargin ({ 2 }),
                           GridItem (helpEditor).withMargin ({ 2 }).withColumn ({ GridItem::Span (2), {} }) };

            grid.performLayout (getLocalBounds());
        }

        AccessibilityRole getRole() const noexcept  { return accessibilityRoles[(size_t) roleBox.getSelectedItemIndex()].role; }

        String getTitle() const noexcept            { return titleEditor.getText(); }
        String getDescription() const noexcept      { return descriptionEditor.getText(); }
        String getHelp() const noexcept             { return helpEditor.getText(); }

    private:
        void setUpAccessibilityRoleSelector (CustomWidgetComponent& owner)
        {
            int itemId = 1;
            for (const auto& nameAndRole : accessibilityRoles)
                roleBox.addItem (nameAndRole.name, itemId++);

            roleBox.setSelectedItemIndex (1);
            roleBox.onChange = [&owner] { owner.accessibleComponent.invalidateAccessibilityHandler(); };
        }

        //==============================================================================
        Label titleLabel { {}, "Title" }, descriptionLabel { {}, "Description" }, helpLabel { {}, "Help" };
        TextEditor titleEditor, descriptionEditor, helpEditor;

        Label roleLabel { {}, "Role" };
        ComboBox roleBox;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoComponent)
    };

    //==============================================================================
    class ActionsComponent final : public Component
    {
    public:
        explicit ActionsComponent (CustomWidgetComponent& owner)
            : customWidgetComponent (owner)
        {
            addAndMakeVisible (press);
            addAndMakeVisible (toggle);
            addAndMakeVisible (focus);
            addAndMakeVisible (showMenu);
        }

        void resized() override
        {
            Grid grid;

            grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)) };
            grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)) };

            grid.items = { GridItem (press).withMargin (2), GridItem (toggle).withMargin (2),
                           GridItem (focus).withMargin (2), GridItem (showMenu).withMargin (2), };

            grid.performLayout (getLocalBounds());
        }

        AccessibilityActions getActions() noexcept
        {
            AccessibilityActions result;

            if (press.isActionEnabled())     result.addAction (AccessibilityActionType::press,    [this] { press.onAction(); });
            if (toggle.isActionEnabled())    result.addAction (AccessibilityActionType::toggle,   [this] { toggle.onAction(); });
            if (focus.isActionEnabled())     result.addAction (AccessibilityActionType::focus,    [this] { focus.onAction(); });
            if (showMenu.isActionEnabled())  result.addAction (AccessibilityActionType::showMenu, [this] { showMenu.onAction(); });

            return result;
        }

    private:
        //==============================================================================
        class AccessibilityActionComponent final : public Component,
                                                   private Timer
        {
        public:
            AccessibilityActionComponent (CustomWidgetComponent& owner,
                                          const String& actionName,
                                          bool initialState)
            {
                enabledToggle.setButtonText (actionName);
                enabledToggle.onClick = [&owner] { owner.accessibleComponent.invalidateAccessibilityHandler(); };
                enabledToggle.setToggleState (initialState, dontSendNotification);

                addAndMakeVisible (enabledToggle);
            }

            void resized() override
            {
                auto bounds = getLocalBounds();

                flashArea = bounds.removeFromRight (bounds.getHeight()).reduced (5);
                bounds.removeFromRight (5);
                enabledToggle.setBounds (bounds);
            }

            void paint (Graphics& g) override
            {
                g.setColour (flashColour);
                g.fillRoundedRectangle (flashArea.toFloat(), 5.0f);
            }

            void onAction()
            {
                if (isTimerRunning())
                    reset();

                startTime = Time::getMillisecondCounter();
                startTimer (5);
            }

            bool isActionEnabled() const noexcept  { return enabledToggle.getToggleState(); }

        private:
            void timerCallback() override
            {
                const auto alpha = [this]
                {
                    const auto progress =  static_cast<float> (Time::getMillisecondCounter() - startTime) / (flashTimeMs / 2);

                    return progress > 1.0f ? 2.0f - progress
                                           : progress;
                }();

                if (alpha < 0.0f)
                {
                    reset();
                    return;
                }

                flashColour = defaultColour.overlaidWith (Colours::yellow.withAlpha (alpha));
                repaint();
            }

            void reset()
            {
                stopTimer();
                flashColour = defaultColour;
                repaint();
            }

            //==============================================================================
            static constexpr int flashTimeMs = 500;

            ToggleButton enabledToggle;
            Rectangle<int> flashArea;
            uint32 startTime = 0;
            Colour defaultColour = Colours::lightgrey, flashColour = defaultColour;

            //==============================================================================
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityActionComponent)
        };

        //==============================================================================
        CustomWidgetComponent& customWidgetComponent;

        AccessibilityActionComponent press    { customWidgetComponent, "Press",     true },
                                     toggle   { customWidgetComponent, "Toggle",    false },
                                     focus    { customWidgetComponent, "Focus",     true },
                                     showMenu { customWidgetComponent, "Show menu", false };

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActionsComponent)
    };

    //==============================================================================
    class ValueInterfaceComponent final : public Component
    {
    public:
        explicit ValueInterfaceComponent (CustomWidgetComponent& owner)
            : customWidgetComponent (owner)
        {
            valueTypeBox.addItemList ({ "Numeric", "Ranged", "Text" }, 1);
            valueTypeBox.setSelectedId (1);
            addAndMakeVisible (valueTypeBox);

            valueTypeBox.onChange = [this]
            {
                updateValueUI();
                customWidgetComponent.accessibleComponent.invalidateAccessibilityHandler();
            };

            addAndMakeVisible (readOnlyToggle);

            numericValueEditor.setInputRestrictions (10, "0123456789.");
            addChildComponent (numericValueEditor);

            addChildComponent (rangedValueComponent);
            addChildComponent (textValueEditor);

            updateValueUI();
        }

        void resized() override
        {
            Grid grid;

            grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (2)) };
            grid.templateColumns = { Grid::TrackInfo (Grid::Fr (2)), Grid::TrackInfo (Grid::Fr (1)) };

            auto& valueEditComponent = [this]() -> Component&
            {
                if (numericValueEditor.isVisible())    return numericValueEditor;
                if (rangedValueComponent.isVisible())  return rangedValueComponent;
                if (textValueEditor.isVisible())       return textValueEditor;

                jassertfalse;
                return numericValueEditor;
            }();

            grid.items = { GridItem (valueTypeBox).withMargin (2), GridItem (readOnlyToggle).withMargin (2),
                           GridItem (valueEditComponent).withMargin (2).withColumn ({ GridItem::Span (2), {} }), };

            grid.performLayout (getLocalBounds());
        }

        std::unique_ptr<AccessibilityValueInterface> getValueInterface()
        {
            struct Numeric final : public AccessibilityNumericValueInterface
            {
                explicit Numeric (ValueInterfaceComponent& o)
                    : owner (o)
                {
                }

                bool isReadOnly() const override          { return owner.readOnlyToggle.getToggleState(); }
                double getCurrentValue() const override   { return owner.numericValueEditor.getText().getDoubleValue(); }
                void setValue (double newValue) override  { owner.numericValueEditor.setText (String (newValue)); }

                ValueInterfaceComponent& owner;
            };

            struct Ranged final : public AccessibilityRangedNumericValueInterface
            {
                explicit Ranged (ValueInterfaceComponent& o)
                    : owner (o)
                {
                }

                bool isReadOnly() const override                { return owner.readOnlyToggle.getToggleState(); }
                double getCurrentValue() const override         { return owner.rangedValueComponent.valueSlider.getValue(); }
                void setValue (double newValue) override        { owner.rangedValueComponent.valueSlider.setValue (newValue); }

                AccessibleValueRange getRange() const override
                {
                    const auto& slider = owner.rangedValueComponent.valueSlider;

                    return { { slider.getMinimum(), slider.getMaximum() },
                             slider.getInterval() };
                }

                ValueInterfaceComponent& owner;
            };

            struct Text final : public AccessibilityTextValueInterface
            {
                explicit Text (ValueInterfaceComponent& o)
                    : owner (o)
                {
                }

                bool isReadOnly() const override                         { return owner.readOnlyToggle.getToggleState(); }
                String getCurrentValueAsString() const override          { return owner.textValueEditor.getText(); }
                void setValueAsString (const String& newValue) override  { owner.textValueEditor.setText (newValue); }

                ValueInterfaceComponent& owner;
            };

            const auto valueType = indexToValueType (valueTypeBox.getSelectedId());

            if (valueType == ValueType::numeric)  return std::make_unique<Numeric> (*this);
            if (valueType == ValueType::ranged)   return std::make_unique<Ranged>  (*this);
            if (valueType == ValueType::text)     return std::make_unique<Text>    (*this);

            jassertfalse;
            return nullptr;
        }

    private:
        //==============================================================================
        struct RangedValueComponent final : public Component
        {
            RangedValueComponent()
            {
                const auto setUpNumericTextEditor = [this] (TextEditor& ed, double initialValue)
                {
                    ed.setInputRestrictions (10, "0123456789.");
                    ed.setText (String (initialValue));
                    ed.onReturnKey = [this] { updateSliderRange(); };

                    addAndMakeVisible (ed);
                };

                setUpNumericTextEditor (minValueEditor,      0.0);
                setUpNumericTextEditor (maxValueEditor,      10.0);
                setUpNumericTextEditor (intervalValueEditor, 0.1);

                addAndMakeVisible (minLabel);
                addAndMakeVisible (maxLabel);
                addAndMakeVisible (intervalLabel);

                valueSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
                addAndMakeVisible (valueSlider);
                updateSliderRange();
            }

            void resized() override
            {
                Grid grid;

                grid.templateRows = { Grid::TrackInfo (Grid::Fr (2)), Grid::TrackInfo (Grid::Fr (3)), Grid::TrackInfo (Grid::Fr (3)) };
                grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)) };

                grid.items = { GridItem (minLabel).withMargin (2),       GridItem (maxLabel).withMargin (2),       GridItem (intervalLabel).withMargin (2),
                               GridItem (minValueEditor).withMargin (2), GridItem (maxValueEditor).withMargin (2), GridItem (intervalValueEditor).withMargin (2),
                               GridItem (valueSlider).withMargin (2).withColumn ({ GridItem::Span (3), {} }) };

                grid.performLayout (getLocalBounds());
            }

            void updateSliderRange()
            {
                auto minValue = minValueEditor.getText().getDoubleValue();
                auto maxValue = maxValueEditor.getText().getDoubleValue();
                const auto intervalValue = jmax (intervalValueEditor.getText().getDoubleValue(), 0.0001);

                if (maxValue <= minValue)
                {
                    maxValue = minValue + intervalValue;
                    maxValueEditor.setText (String (maxValue));
                }
                else if (minValue >= maxValue)
                {
                    minValue = maxValue - intervalValue;
                    minValueEditor.setText (String (minValue));
                }

                valueSlider.setRange (minValue, maxValue, intervalValue);
            }

            Label minLabel { {}, "Min" }, maxLabel { {}, "Max" }, intervalLabel { {}, "Interval" };
            TextEditor minValueEditor, maxValueEditor, intervalValueEditor;
            Slider valueSlider;
        };

        //==============================================================================
        enum class ValueType { numeric, ranged, text };

        static ValueType indexToValueType (int index) noexcept
        {
            if (index == 1) return ValueType::numeric;
            if (index == 2) return ValueType::ranged;
            if (index == 3) return ValueType::text;

            jassertfalse;
            return ValueType::numeric;
        }

        void updateValueUI()
        {
            const auto valueType = indexToValueType (valueTypeBox.getSelectedId());

            numericValueEditor.setVisible (valueType == ValueType::numeric);
            textValueEditor.setVisible (valueType == ValueType::text);
            rangedValueComponent.setVisible (valueType == ValueType::ranged);

            resized();
        }

        //==============================================================================
        CustomWidgetComponent& customWidgetComponent;

        ComboBox valueTypeBox;
        ToggleButton readOnlyToggle { "Read-Only" };

        TextEditor numericValueEditor, textValueEditor;
        RangedValueComponent rangedValueComponent;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueInterfaceComponent)
    };

    //==============================================================================
    class StateComponent final : public Component
    {
    public:
        StateComponent()
        {
            for (auto& property : properties)
                addAndMakeVisible (property.button);
        }

        void resized() override
        {
            Grid grid;

            grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)) };

            grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)) };

            for (auto& property : properties)
                grid.items.add (GridItem (property.button));

            grid.performLayout (getLocalBounds());
        }

        AccessibleState getAccessibleState() const
        {
            AccessibleState result;

            for (auto& property : properties)
                if (property.button.getToggleState())
                    result = property.setState (std::move (result));

            return result;
        }

    private:
        struct Property
        {
            Property (const String& name,
                      bool initialState,
                      AccessibleState (AccessibleState::* fn)() const)
                : button (name),
                  setStateFn (fn)
            {
                button.setToggleState (initialState, dontSendNotification);
            }

            AccessibleState setState (AccessibleState s) const noexcept { return (s.*setStateFn)(); }

            ToggleButton button;
            AccessibleState (AccessibleState::* setStateFn)() const;
        };

        std::array<Property, 12> properties
        { {
            { "Checkable",            false, &AccessibleState::withCheckable },
            { "Checked",              false, &AccessibleState::withChecked },
            { "Collapsed",            false, &AccessibleState::withCollapsed },
            { "Expandable",           false, &AccessibleState::withExpandable },
            { "Expanded",             false, &AccessibleState::withExpanded },
            { "Focusable",            true,  &AccessibleState::withFocusable },
            { "Focused",              false, &AccessibleState::withFocused },
            { "Ignored",              false, &AccessibleState::withIgnored },
            { "Selectable",           false, &AccessibleState::withSelectable },
            { "Multi-Selectable",     false, &AccessibleState::withMultiSelectable },
            { "Selected",             false, &AccessibleState::withSelected },
            { "Accessible Offscreen", false, &AccessibleState::withAccessibleOffscreen }
        } };

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StateComponent)
    };

    //==============================================================================
    Label descriptionLabel { {}, "This is a demo of a custom accessible widget.\n\n"
                                 "The JUCE logo component at the bottom of the page will use the settable properties when queried by "
                                 "an accessibility client." };

    InfoComponent infoComponent                     { *this };
    ActionsComponent actionsComponent               { *this };
    ValueInterfaceComponent valueInterfaceComponent { *this };
    StateComponent stateComponent;

    ContentComponent info                    { "Info",
                                               "Set the title, role, description and help text properties of the component.",
                                               infoComponent };
    ContentComponent actions                 { "Actions",
                                               "Specify the accessibility actions that the component can perform. When invoked the indicator will flash.",
                                               actionsComponent };
    ContentComponent valueInterface          { "Value",
                                               "Sets the value that this component represents. This can be numeric, ranged or textual and can optionally be read-only.",
                                                valueInterfaceComponent };
    ContentComponent state                   { "State",
                                               "Modify the AccessibleState properties of the component.",
                                               stateComponent };

    AccessibleComponent accessibleComponent { *this };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomWidgetComponent)
};

//==============================================================================
/**
    The top-level component containing an example of custom child component navigation.
*/
class CustomNavigationComponent final : public Component
{
public:
    CustomNavigationComponent()
    {
        setTitle ("Custom Navigation");
        setDescription ("A demo of custom component navigation.");
        setFocusContainerType (FocusContainerType::focusContainer);

        addAndMakeVisible (descriptionLabel);
        addAndMakeVisible (navigableComponents);
    }

    void resized() override
    {
        Grid grid;

        grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)),
                              Grid::TrackInfo (Grid::Fr (2)) };

        grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)) };

        grid.items = { GridItem (descriptionLabel).withMargin (2),
                       GridItem (navigableComponents).withMargin (5) };

        grid.performLayout (getLocalBounds());
    }

private:
    //==============================================================================
    class NavigableComponentsHolder final : public Component
    {
    public:
        NavigableComponentsHolder()
        {
            setTitle ("Navigable Components");
            setDescription ("A container of some navigable components.");
            setFocusContainerType (FocusContainerType::focusContainer);

            constexpr int numChildren = 12;

            for (int i = 1; i <= numChildren; ++i)
            {
                children.push_back (std::make_unique<NavigableComponent> (i, numChildren, *this));
                addAndMakeVisible (*children.back());
            }
        }

        void resized() override
        {
            Grid grid;

            grid.templateRows = { Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)),
                                  Grid::TrackInfo (Grid::Fr (1)) };

            grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)), Grid::TrackInfo (Grid::Fr (1)) };

            for (auto& child : children)
                grid.items.add (GridItem (*child).withMargin (5));

            grid.performLayout (getLocalBounds());
        }

        std::unique_ptr<ComponentTraverser> createFocusTraverser() override
        {
            struct CustomTraverser final : public FocusTraverser
            {
                explicit CustomTraverser (NavigableComponentsHolder& owner)
                    : navigableComponentsHolder (owner)  {}

                Component* getDefaultComponent (Component*) override
                {
                    for (auto& child : navigableComponentsHolder.children)
                        if (child->defaultToggle.getToggleState() && child->focusableToggle.getToggleState())
                            return child.get();

                    return nullptr;
                }

                Component* getNextComponent (Component* current) override
                {
                    const auto& comps = navigableComponentsHolder.children;

                    const auto iter = std::find_if (comps.cbegin(), comps.cend(),
                                                    [current] (const std::unique_ptr<NavigableComponent>& c) { return c.get() == current; });

                    if (iter != comps.cend() && iter != std::prev (comps.cend()))
                        return std::next (iter)->get();

                    return nullptr;
                }

                Component* getPreviousComponent (Component* current) override
                {
                    const auto& comps = navigableComponentsHolder.children;

                    const auto iter = std::find_if (comps.cbegin(), comps.cend(),
                                                    [current] (const std::unique_ptr<NavigableComponent>& c) { return c.get() == current; });

                    if (iter != comps.cend() && iter != comps.cbegin())
                        return std::prev (iter)->get();

                    return nullptr;
                }

                std::vector<Component*> getAllComponents (Component*) override
                {
                    std::vector<Component*> comps;

                    for (auto& child : navigableComponentsHolder.children)
                        if (child->focusableToggle.getToggleState())
                            comps.push_back (child.get());

                    return comps;
                }

                NavigableComponentsHolder& navigableComponentsHolder;
            };

            return std::make_unique<CustomTraverser> (*this);
        }

    private:
        struct NavigableComponent final : public Component
        {
            NavigableComponent (int index, int total, NavigableComponentsHolder& owner)
            {
                const auto textColour = Colours::black.withAlpha (0.8f);

                titleLabel.setColour (Label::textColourId, textColour);
                orderLabel.setColour (Label::textColourId, textColour);

                const auto setToggleButtonColours = [textColour] (ToggleButton& b)
                {
                    b.setColour (ToggleButton::textColourId, textColour);
                    b.setColour (ToggleButton::tickDisabledColourId, textColour);
                    b.setColour (ToggleButton::tickColourId, textColour);
                };

                setToggleButtonColours (focusableToggle);
                setToggleButtonColours (defaultToggle);

                const auto title = "Component " + String (index);
                setTitle (title);
                titleLabel.setText (title, dontSendNotification);
                focusableToggle.setToggleState (true, dontSendNotification);
                defaultToggle.setToggleState (index == 1, dontSendNotification);

                for (int i = 1; i <= total; ++i)
                    orderBox.addItem (String (i), i);

                orderBox.setSelectedId (index);

                orderBox.onChange = [this, &owner] { owner.orderChanged (*this); };

                focusableToggle.onClick = [this] { repaint(); };

                defaultToggle.onClick = [this, &owner]
                {
                    if (! defaultToggle.getToggleState())
                        defaultToggle.setToggleState (true, dontSendNotification);
                    else
                        owner.defaultChanged (*this);
                };

                addAndMakeVisible (titleLabel);

                addAndMakeVisible (focusableToggle);
                addAndMakeVisible (defaultToggle);
                addAndMakeVisible (orderLabel);
                addAndMakeVisible (orderBox);

                setFocusContainerType (FocusContainerType::focusContainer);
            }

            void paint (Graphics& g) override
            {
                g.fillAll (backgroundColour.withAlpha (focusableToggle.getToggleState() ? 1.0f : 0.5f));
            }

            void resized() override
            {
                Grid grid;

                grid.templateRows = { Grid::TrackInfo (Grid::Fr (2)),
                                      Grid::TrackInfo (Grid::Fr (3)),
                                      Grid::TrackInfo (Grid::Fr (3)) };

                grid.templateColumns = { Grid::TrackInfo (Grid::Fr (1)),
                                         Grid::TrackInfo (Grid::Fr (1)),
                                         Grid::TrackInfo (Grid::Fr (1)),
                                         Grid::TrackInfo (Grid::Fr (1)) };

                grid.items = { GridItem (titleLabel).withMargin (2).withColumn ({ GridItem::Span (4), {} }),
                               GridItem (focusableToggle).withMargin (2).withColumn ({ GridItem::Span (2), {} }),
                               GridItem (defaultToggle).withMargin (2).withColumn ({ GridItem::Span (2), {} }),
                               GridItem (orderLabel).withMargin (2),
                               GridItem (orderBox).withMargin (2).withColumn ({ GridItem::Span (3), {} }) };

                grid.performLayout (getLocalBounds());
            }

            Colour backgroundColour = getRandomBrightColour();
            Label titleLabel;
            ToggleButton focusableToggle { "Focusable" }, defaultToggle { "Default" };
            Label orderLabel { {}, "Order" };
            ComboBox orderBox;
        };

        void orderChanged (const NavigableComponent& changedChild)
        {
            const auto addressMatches = [&] (const std::unique_ptr<NavigableComponent>& child)
            {
                return child.get() == &changedChild;
            };

            const auto iter = std::find_if (children.begin(), children.end(), addressMatches);

            if (iter != children.end())
                std::swap (*iter, *std::next (children.begin(), changedChild.orderBox.getSelectedItemIndex()));

            int order = 1;

            for (auto& child : children)
                child->orderBox.setSelectedId (order++);

            if (auto* handler = getAccessibilityHandler())
                handler->notifyAccessibilityEvent (AccessibilityEvent::structureChanged);
        }

        void defaultChanged (const NavigableComponent& newDefault)
        {
            for (auto& child : children)
                if (child->defaultToggle.getToggleState() && child.get() != &newDefault)
                    child->defaultToggle.setToggleState (false, dontSendNotification);
        }

        std::vector<std::unique_ptr<NavigableComponent>> children;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigableComponentsHolder)
    };

    //==============================================================================
    Label descriptionLabel { {}, "This is a demo of how to control the navigation order of components when navigating with an accessibility client.\n\n"
                                 "You can set the order of navigation, whether components are focusable and set a default component which will "
                                 "receive the focus first." };

    NavigableComponentsHolder navigableComponents;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomNavigationComponent)
};

//==============================================================================
/**
    The top-level component containing an example of how to post system announcements.

    The AccessibilityHandler::postAnnouncement() method will post some text to the native
    screen reader application to be read out along with a priority determining how
    it should be read out (whether it should interrupt other announcements, etc.).
*/
class AnnouncementsComponent final : public Component
{
public:
    AnnouncementsComponent()
    {
        addAndMakeVisible (descriptionLabel);

        textEntryBox.setMultiLine (true);
        textEntryBox.setReturnKeyStartsNewLine (true);
        textEntryBox.setText ("Announcement text.");
        addAndMakeVisible (textEntryBox);

        priorityComboBox.addItemList ({ "Priority - Low", "Priority - Medium", "Priority - High" }, 1);
        priorityComboBox.setSelectedId (2);
        addAndMakeVisible (priorityComboBox);

        announceButton.onClick = [this]
        {
            auto priority = [this]
            {
                switch (priorityComboBox.getSelectedId())
                {
                    case 1:   return AccessibilityHandler::AnnouncementPriority::low;
                    case 2:   return AccessibilityHandler::AnnouncementPriority::medium;
                    case 3:   return AccessibilityHandler::AnnouncementPriority::high;
                }

                jassertfalse;
                return AccessibilityHandler::AnnouncementPriority::medium;
            }();

            AccessibilityHandler::postAnnouncement (textEntryBox.getText(), priority);
        };

        addAndMakeVisible (announceButton);

        setTitle ("Announcements");
        setHelpText ("Type some text into the box and click the announce button to have it read out.");
        setFocusContainerType (FocusContainerType::focusContainer);
    }

    void resized() override
    {
        Grid grid;

        grid.templateRows = { Grid::TrackInfo (Grid::Fr (3)),
                              Grid::TrackInfo (Grid::Fr (1)),
                              Grid::TrackInfo (Grid::Fr (1)),
                              Grid::TrackInfo (Grid::Fr (1)),
                              Grid::TrackInfo (Grid::Fr (1)),
                              Grid::TrackInfo (Grid::Fr (1)) };

        grid.templateColumns = { Grid::TrackInfo (Grid::Fr (3)),
                                 Grid::TrackInfo (Grid::Fr (2)) };

        grid.items = { GridItem (descriptionLabel).withMargin (2).withColumn ({ GridItem::Span (2), {} }),
                       GridItem (textEntryBox).withMargin (2).withArea ({ 2 }, { 1 }, { 5 }, { 2 }),
                       GridItem (priorityComboBox).withMargin (2).withArea ({ 5 }, { 1 }, { 6 }, { 2 }),
                       GridItem (announceButton).withMargin (2).withArea ({ 4 }, { 2 }, { 5 }, { 3 }) };

        grid.performLayout (getLocalBounds());
    }

private:
    Label descriptionLabel { {}, "This is a demo of posting system announcements that will be read out by an accessibility client.\n\n"
                                 "You can enter some text to be read out in the text box below, set a priority for the message and then "
                                 "post it using the \"Announce\" button." };

    TextEditor textEntryBox;
    ComboBox priorityComboBox;
    TextButton announceButton { "Announce" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnnouncementsComponent)
};

//==============================================================================
/**
    The main demo content component.

    This just contains a TabbedComponent with a tab for each of the top-level demos.
*/
class AccessibilityDemo final : public Component
{
public:
    AccessibilityDemo()
    {
        setTitle ("Accessibility Demo");
        setDescription ("A demo of JUCE's accessibility features.");
        setFocusContainerType (FocusContainerType::focusContainer);

        tabs.setTitle ("Demo tabs");

        const auto tabColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker (0.1f);

        tabs.addTab ("JUCE Widgets",      tabColour, &juceWidgetsComponent,      false);
        tabs.addTab ("Custom Widget",     tabColour, &customWidgetComponent,     false);
        tabs.addTab ("Custom Navigation", tabColour, &customNavigationComponent, false);
        tabs.addTab ("Announcements",     tabColour, &announcementsComponent,    false);
        addAndMakeVisible (tabs);

        setSize (800, 600);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        tabs.setBounds (getLocalBounds());
    }

private:
    TooltipWindow tooltipWindow { nullptr, 100 };

    TabbedComponent tabs { TabbedButtonBar::Orientation::TabsAtTop };

    JUCEWidgetsComponent juceWidgetsComponent;
    CustomWidgetComponent customWidgetComponent;
    CustomNavigationComponent customNavigationComponent;
    AnnouncementsComponent announcementsComponent;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityDemo)
};
