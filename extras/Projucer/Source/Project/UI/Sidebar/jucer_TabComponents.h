/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class ConcertinaHeader    : public Component,
                            public ChangeBroadcaster
{
public:
    ConcertinaHeader (String n, Path p)
        : Component (n), name (n), iconPath (p)
    {
        setTitle (getName());

        panelIcon = Icon (iconPath, Colours::white);

        nameLabel.setText (name, dontSendNotification);
        nameLabel.setJustificationType (Justification::centredLeft);
        nameLabel.setInterceptsMouseClicks (false, false);
        nameLabel.setAccessible (false);
        nameLabel.setColour (Label::textColourId, Colours::white);

        addAndMakeVisible (nameLabel);
    }

    void resized() override
    {
        auto b = getLocalBounds().toFloat();

        iconBounds = b.removeFromLeft (b.getHeight()).reduced (7, 7);
        arrowBounds = b.removeFromRight (b.getHeight());
        nameLabel.setBounds (b.toNearestInt());
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (defaultButtonBackgroundColourId));
        g.fillRoundedRectangle (getLocalBounds().reduced (2, 3).toFloat(), 2.0f);

        g.setColour (Colours::white);
        g.fillPath (arrowPath = ProjucerLookAndFeel::getArrowPath (arrowBounds,
                                                                   getParentComponent()->getBoundsInParent().getY() == yPosition ? 2 : 0,
                                                                   true, Justification::centred));

        panelIcon.draw (g, iconBounds.toFloat(), false);
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (! e.mouseWasDraggedSinceMouseDown())
            sendChangeMessage();
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this,
                                                       AccessibilityRole::button,
                                                       AccessibilityActions().addAction (AccessibilityActionType::press,
                                                                                         [this] { sendChangeMessage(); }));
    }

    int direction = 0;
    int yPosition = 0;

private:
    String name;
    Label nameLabel;

    Path iconPath;
    Icon panelIcon;

    Rectangle<float> arrowBounds, iconBounds;
    Path arrowPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaHeader)
};

//==============================================================================
class FindPanel    : public Component,
                     private Timer,
                     private FocusChangeListener
{
public:
    FindPanel (std::function<void (const String&)> cb)
        : callback (cb)
    {
        addAndMakeVisible (editor);
        editor.onTextChange = [this] { startTimer (250); };
        editor.onFocusLost  = [this]
        {
            isFocused = false;
            repaint();
        };

        Desktop::getInstance().addFocusChangeListener (this);

        lookAndFeelChanged();
    }

    ~FindPanel() override
    {
        Desktop::getInstance().removeFocusChangeListener (this);
    }

    void paintOverChildren (Graphics& g) override
    {
        if (! isFocused)
            return;

        g.setColour (findColour (defaultHighlightColourId));

        Path p;
        p.addRoundedRectangle (getLocalBounds().reduced (2), 3.0f);
        g.strokePath (p, PathStrokeType (2.0f));
    }


    void resized() override
    {
        editor.setBounds (getLocalBounds().reduced (2));
    }

private:
    TextEditor editor;
    bool isFocused = false;
    std::function<void (const String&)> callback;

    //==============================================================================
    void lookAndFeelChanged() override
    {
        editor.setTextToShowWhenEmpty ("Filter...", findColour (widgetTextColourId).withAlpha (0.3f));
    }

    void globalFocusChanged (Component* focusedComponent) override
    {
        if (focusedComponent == &editor)
        {
            isFocused = true;
            repaint();
        }
    }

    void timerCallback() override
    {
        stopTimer();
        callback (editor.getText());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FindPanel)
};

//==============================================================================
class ConcertinaTreeComponent    : public Component
{
public:
    class AdditionalComponents
    {
    public:
        enum Type
        {
            addButton      = (1 << 0),
            settingsButton = (1 << 1),
            findPanel      = (1 << 2)
        };

        AdditionalComponents with (Type t)
        {
            auto copy = *this;
            copy.componentTypes |= t;

            return copy;
        }

        bool has (Type t) const noexcept
        {
            return (componentTypes & t) != 0;
        }

    private:
        int componentTypes = 0;
    };

    ConcertinaTreeComponent (const String& name,
                             TreePanelBase* tree,
                             AdditionalComponents additionalComponents)
         : Component (name),
           treeToDisplay (tree)
    {
        setTitle (getName());
        setFocusContainerType (FocusContainerType::focusContainer);

        if (additionalComponents.has (AdditionalComponents::addButton))
        {
            addButton = std::make_unique<IconButton> ("Add", getIcons().plus);
            addAndMakeVisible (addButton.get());
            addButton->onClick = [this] { showAddMenu(); };
        }

        if (additionalComponents.has (AdditionalComponents::settingsButton))
        {
            settingsButton = std::make_unique<IconButton> ("Settings", getIcons().settings);
            addAndMakeVisible (settingsButton.get());
            settingsButton->onClick = [this] { showSettings(); };
        }

        if (additionalComponents.has (AdditionalComponents::findPanel))
        {
            findPanel = std::make_unique<FindPanel> ([this] (const String& filter) { treeToDisplay->rootItem->setSearchFilter (filter); });
            addAndMakeVisible (findPanel.get());
        }

        addAndMakeVisible (treeToDisplay.get());
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        if (addButton != nullptr || settingsButton != nullptr || findPanel != nullptr)
        {
            auto bottomSlice = bounds.removeFromBottom (25);
            bottomSlice.removeFromRight (3);

            if (addButton != nullptr)
                addButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (settingsButton != nullptr)
                settingsButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (findPanel != nullptr)
                findPanel->setBounds (bottomSlice.reduced (2));
        }

        treeToDisplay->setBounds (bounds);
    }

    TreePanelBase* getTree() const noexcept    { return treeToDisplay.get(); }

private:
    std::unique_ptr<TreePanelBase> treeToDisplay;
    std::unique_ptr<IconButton> addButton, settingsButton;
    std::unique_ptr<FindPanel> findPanel;

    void showAddMenu()
    {
        auto numSelected = treeToDisplay->tree.getNumSelectedItems();

        if (numSelected > 1)
            return;

        if (numSelected == 0)
        {
            if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
                root->showPopupMenu (addButton->getScreenBounds().getCentre());
        }
        else
        {
            if (auto* item = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getSelectedItem (0)))
                item->showAddMenu (addButton->getScreenBounds().getCentre());
        }
    }

    void showSettings()
    {
        if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
        {
            treeToDisplay->tree.clearSelectedItems();
            root->showDocument();
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaTreeComponent)
};
