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
#include "GraphEditorPanel.h"
#include "InternalFilters.h"
#include "MainHostWindow.h"
#include "FilterIOConfiguration.h"


//==============================================================================
class NumberedBoxes   : public TableListBox,
                        private TableListBoxModel,
                        private Button::Listener
{
public:
    struct Listener
    {
        virtual ~Listener() {}

        virtual void addColumn()    = 0;
        virtual void removeColumn() = 0;
        virtual void columnSelected (int columnId) = 0;
    };

    enum
    {
        plusButtonColumnId  = 128,
        minusButtonColumnId = 129
    };

    //==============================================================================
    NumberedBoxes (Listener& listenerToUse, bool canCurrentlyAddColumn, bool canCurrentlyRemoveColumn)
        : TableListBox ("NumberedBoxes", this),
          listener (listenerToUse),
          canAddColumn (canCurrentlyAddColumn),
          canRemoveColumn (canCurrentlyRemoveColumn)
    {
        auto& tableHeader = getHeader();

        for (int i = 0; i < 16; ++i)
            tableHeader.addColumn (String (i + 1), i + 1, 40);

        setHeaderHeight (0);
        setRowHeight (40);
        getHorizontalScrollBar().setAutoHide (false);
    }

    void setSelected (int columnId)
    {
        if (auto* button = dynamic_cast<TextButton*> (getCellComponent (columnId, 0)))
            button->setToggleState (true, NotificationType::dontSendNotification);
    }

    void setCanAddColumn (bool canCurrentlyAdd)
    {
        if (canCurrentlyAdd != canAddColumn)
        {
            canAddColumn = canCurrentlyAdd;

            if (auto* button = dynamic_cast<TextButton*> (getCellComponent (plusButtonColumnId, 0)))
                button->setEnabled (true);
        }
    }

    void setCanRemoveColumn (bool canCurrentlyRemove)
    {
        if (canCurrentlyRemove != canRemoveColumn)
        {
            canRemoveColumn = canCurrentlyRemove;

            if (auto* button = dynamic_cast<TextButton*> (getCellComponent (minusButtonColumnId, 0)))
                button->setEnabled (true);
        }
    }

private:
    //==============================================================================
    int getNumRows() override                                             { return 1; }
    void paintCell (Graphics&, int, int, int, int, bool) override         {}
    void paintRowBackground (Graphics& g, int, int, int, bool) override   { g.fillAll (Colours::grey); }

    Component* refreshComponentForCell (int, int columnId, bool,
                                        Component* existingComponentToUpdate) override
    {
        auto* textButton = dynamic_cast<TextButton*> (existingComponentToUpdate);

        if (textButton == nullptr)
            textButton = new TextButton();

        textButton->setButtonText (getButtonName (columnId));
        textButton->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight |
                                       Button::ConnectedOnTop  | Button::ConnectedOnBottom);

        const bool isPlusMinusButton = (columnId == plusButtonColumnId || columnId == minusButtonColumnId);

        if (isPlusMinusButton)
        {
            textButton->setEnabled (columnId == plusButtonColumnId ? canAddColumn : canRemoveColumn);
        }
        else
        {
            textButton->setRadioGroupId (1, NotificationType::dontSendNotification);
            textButton->setClickingTogglesState (true);

            auto busColour = Colours::green.withRotatedHue (static_cast<float> (columnId) / 5.0f);
            textButton->setColour (TextButton::buttonColourId, busColour);
            textButton->setColour (TextButton::buttonOnColourId, busColour.withMultipliedBrightness (2.0f));
        }

        textButton->addListener (this);

        return textButton;
    }

    //==============================================================================
    String getButtonName (int columnId)
    {
        if (columnId == plusButtonColumnId)  return "+";
        if (columnId == minusButtonColumnId) return "-";

        return String (columnId);
    }

    void buttonClicked (Button* btn) override
    {
        auto text = btn->getButtonText();

        if (text == "+") listener.addColumn();
        if (text == "-") listener.removeColumn();
    }

    void buttonStateChanged (Button* btn) override
    {
        auto text = btn->getButtonText();

        if (text == "+" || text == "-")
            return;

        if (btn->getToggleState())
            listener.columnSelected (text.getIntValue());
    }

    //==============================================================================
    Listener& listener;
    bool canAddColumn, canRemoveColumn;
};

//==============================================================================
class FilterIOConfigurationWindow::InputOutputConfig  : public Component,
                                                        private ComboBox::Listener,
                                                        private Button::Listener,
                                                        private NumberedBoxes::Listener
{
public:
    InputOutputConfig (FilterIOConfigurationWindow& parent, bool direction)
        : owner (parent),
          ioTitle ("ioLabel", direction ? "Input Configuration" : "Output Configuration"),
          nameLabel ("nameLabel", "Bus Name:"),
          layoutLabel ("layoutLabel", "Channel Layout:"),
          enabledToggle ("Enabled"),
          ioBuses (*this, false, false),
          isInput (direction),
          currentBus (0)
    {
        ioTitle.setFont (ioTitle.getFont().withStyle (Font::bold));
        nameLabel.setFont (nameLabel.getFont().withStyle (Font::bold));
        layoutLabel.setFont (layoutLabel.getFont().withStyle (Font::bold));
        enabledToggle.setClickingTogglesState (true);

        layouts.addListener (this);
        enabledToggle.addListener (this);

        addAndMakeVisible (layoutLabel);
        addAndMakeVisible (layouts);
        addAndMakeVisible (enabledToggle);
        addAndMakeVisible (ioTitle);
        addAndMakeVisible (nameLabel);
        addAndMakeVisible (name);
        addAndMakeVisible (ioBuses);

        updateBusButtons();
        updateBusLayout();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);

        ioTitle.setBounds (r.removeFromTop (14));
        r.reduce (10, 0);
        r.removeFromTop (16);

        ioBuses.setBounds (r.removeFromTop (60));

        {
            auto label = r.removeFromTop (24);

            nameLabel.setBounds (label.removeFromLeft (100));
            enabledToggle.setBounds (label.removeFromRight (80));
            name.setBounds (label);
        }

        {
            auto label = r.removeFromTop (24);

            layoutLabel.setBounds (label.removeFromLeft (100));
            layouts.setBounds (label);
        }
    }

private:
    void updateBusButtons()
    {
        if (auto* filter = owner.getAudioProcessor())
        {
            auto& header = ioBuses.getHeader();
            header.removeAllColumns();

            const int n = filter->getBusCount (isInput);

            for (int i = 0; i < n; ++i)
                header.addColumn ("", i + 1, 40);

            header.addColumn ("+", NumberedBoxes::plusButtonColumnId,  20);
            header.addColumn ("-", NumberedBoxes::minusButtonColumnId, 20);

            ioBuses.setCanAddColumn    (filter->canAddBus    (isInput));
            ioBuses.setCanRemoveColumn (filter->canRemoveBus (isInput));
        }

        ioBuses.setSelected (currentBus + 1);
    }

    void updateBusLayout()
    {
        if (auto* filter = owner.getAudioProcessor())
        {
            if (auto* bus = filter->getBus (isInput, currentBus))
            {
                name.setText (bus->getName(), NotificationType::dontSendNotification);

                int i;
                for (i = 1; i < AudioChannelSet::maxChannelsOfNamedLayout; ++i)
                    if ((layouts.indexOfItemId(i) == -1) != bus->supportedLayoutWithChannels (i).isDisabled())
                        break;

                // supported layouts have changed
                if (i < AudioChannelSet::maxChannelsOfNamedLayout)
                {
                    layouts.clear();

                    for (i = 1; i < AudioChannelSet::maxChannelsOfNamedLayout; ++i)
                    {
                        auto set = bus->supportedLayoutWithChannels (i);

                        if (! set.isDisabled())
                            layouts.addItem (set.getDescription(), i);
                    }
                }

                layouts.setSelectedId (bus->getLastEnabledLayout().size());

                const bool canBeDisabled = bus->isNumberOfChannelsSupported (0);

                if (canBeDisabled != enabledToggle.isEnabled())
                    enabledToggle.setEnabled (canBeDisabled);

                enabledToggle.setToggleState (bus->isEnabled(), NotificationType::dontSendNotification);
            }
        }
    }

    //==============================================================================
    void comboBoxChanged (ComboBox* combo) override
    {
        if (combo == &layouts)
        {
            if (auto* audioProcessor = owner.getAudioProcessor())
            {
                if (auto* bus = audioProcessor->getBus (isInput, currentBus))
                {
                    auto selectedNumChannels = layouts.getSelectedId();

                    if (selectedNumChannels != bus->getLastEnabledLayout().size())
                    {
                        if (isPositiveAndBelow (selectedNumChannels, AudioChannelSet::maxChannelsOfNamedLayout)
                             && bus->setCurrentLayoutWithoutEnabling (bus->supportedLayoutWithChannels (selectedNumChannels)))
                        {
                            if (auto* config = owner.getConfig (! isInput))
                                config->updateBusLayout();

                            owner.update();
                        }
                    }
                }
            }
        }
    }

    void buttonClicked (Button*) override {}

    void buttonStateChanged (Button* btn) override
    {
        if (btn == &enabledToggle && enabledToggle.isEnabled())
        {
            if (auto* audioProcessor = owner.getAudioProcessor())
            {
                if (auto* bus = audioProcessor->getBus (isInput, currentBus))
                {
                    if (bus->isEnabled() != enabledToggle.getToggleState())
                    {
                        bool success;

                        if (enabledToggle.getToggleState())
                            success = bus->enable();
                        else
                            success = bus->setCurrentLayout (AudioChannelSet::disabled());

                        if (success)
                        {
                            updateBusLayout();

                            if (InputOutputConfig* config = owner.getConfig (! isInput))
                                config->updateBusLayout();

                            owner.update();
                        }
                        else
                        {
                            enabledToggle.setToggleState (! enabledToggle.getToggleState(),
                                                          NotificationType::dontSendNotification);
                        }
                    }
                }
            }
        }
    }

    //==============================================================================
    void addColumn() override
    {
        if (auto* audioProcessor = owner.getAudioProcessor())
        {
            if (audioProcessor->canAddBus (isInput))
            {
                if (audioProcessor->addBus (isInput))
                {
                    updateBusButtons();
                    updateBusLayout();

                    if (auto* config = owner.getConfig (! isInput))
                    {
                        config->updateBusButtons();
                        config->updateBusLayout();
                    }

                    owner.update();
                }
            }
        }
    }

    void removeColumn() override
    {
        if (auto* audioProcessor = owner.getAudioProcessor())
        {
            if (audioProcessor->getBusCount (isInput) > 1 && audioProcessor->canRemoveBus (isInput))
            {
                if (audioProcessor->removeBus (isInput))
                {
                    currentBus = jmin (audioProcessor->getBusCount (isInput) - 1, currentBus);

                    updateBusButtons();
                    updateBusLayout();

                    if (auto* config = owner.getConfig (! isInput))
                    {
                        config->updateBusButtons();
                        config->updateBusLayout();
                    }

                    owner.update();
                }
            }
        }
    }

    void columnSelected (int columnId) override
    {
        const int newBus = columnId - 1;

        if (currentBus != newBus)
        {
            currentBus = newBus;
            ioBuses.setSelected (currentBus + 1);
            updateBusLayout();
        }
    }

    //==============================================================================
    FilterIOConfigurationWindow& owner;
    Label ioTitle, nameLabel, name, layoutLabel;
    ToggleButton enabledToggle;
    ComboBox layouts;
    NumberedBoxes ioBuses;
    bool isInput;
    int currentBus;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputOutputConfig)
};


FilterIOConfigurationWindow::FilterIOConfigurationWindow (AudioProcessor* const p)
    : AudioProcessorEditor (p),
      title ("title", p->getName())
{
    jassert (p != nullptr);
    setOpaque (true);

    title.setFont (title.getFont().withStyle (Font::bold));
    addAndMakeVisible (title);

    {
        ScopedLock renderLock (p->getCallbackLock());
        p->suspendProcessing (true);
        p->releaseResources();
    }

    if (p->getBusCount (true)  > 0 || p->canAddBus (true))
        addAndMakeVisible (inConfig = new InputOutputConfig (*this, true));

    if (p->getBusCount (false) > 0 || p->canAddBus (false))
        addAndMakeVisible (outConfig = new InputOutputConfig (*this, false));

    currentLayout = p->getBusesLayout();
    setSize (400, (inConfig != nullptr && outConfig != nullptr ? 160 : 0) + 200);
}

FilterIOConfigurationWindow::~FilterIOConfigurationWindow()
{
    if (auto* graph = getGraph())
    {
        if (auto* p = getAudioProcessor())
        {
            ScopedLock renderLock (graph->getCallbackLock());

            graph->suspendProcessing (true);
            graph->releaseResources();

            p->prepareToPlay (graph->getSampleRate(), graph->getBlockSize());
            p->suspendProcessing (false);

            graph->prepareToPlay (graph->getSampleRate(), graph->getBlockSize());
            graph->suspendProcessing (false);
        }
    }
}

void FilterIOConfigurationWindow::paint (Graphics& g)
{
     g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void FilterIOConfigurationWindow::resized()
{
    auto r = getLocalBounds().reduced (10);

    title.setBounds (r.removeFromTop (14));
    r.reduce (10, 0);

    if (inConfig != nullptr)
        inConfig->setBounds (r.removeFromTop (160));

    if (outConfig != nullptr)
        outConfig->setBounds (r.removeFromTop (160));
}

void FilterIOConfigurationWindow::update()
{
    auto nodeId = getNodeId();

    if (auto* graph = getGraph())
        if (nodeId != -1)
            graph->disconnectNode (static_cast<uint32> (nodeId));

    if (auto* graphEditor = getGraphEditor())
        if (auto* panel = graphEditor->graphPanel)
            panel->updateComponents();
}

int32 FilterIOConfigurationWindow::getNodeId() const
{
    if (auto* graph = getGraph())
    {
        const int n = graph->getNumNodes();

        for (int i = 0; i < n; ++i)
            if (auto* node = graph->getNode (i))
                if (node->getProcessor() == getAudioProcessor())
                    return static_cast<int32> (node->nodeId);
    }

    return -1;
}

MainHostWindow* FilterIOConfigurationWindow::getMainWindow() const
{
    Component* comp;

    for (int idx = 0; (comp = Desktop::getInstance().getComponent(idx)) != nullptr; ++idx)
        if (auto* mainWindow = dynamic_cast<MainHostWindow*> (comp))
            return mainWindow;

    return nullptr;
}

GraphDocumentComponent* FilterIOConfigurationWindow::getGraphEditor() const
{
    if (auto* mainWindow = getMainWindow())
        if (auto* graphEditor = mainWindow->getGraphEditor())
            return graphEditor;

    return nullptr;
}

AudioProcessorGraph* FilterIOConfigurationWindow::getGraph() const
{
    if (auto* graphEditor = getGraphEditor())
        if (auto* graph = graphEditor->graph.get())
            return &graph->getGraph();

    return nullptr;
}
