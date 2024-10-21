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
class FileGroupInformationComponent final : public Component,
                                            private ListBoxModel,
                                            private ValueTree::Listener
{
public:
    FileGroupInformationComponent (const Project::Item& group)
        : item (group),
          header (item.getName(), { getIcons().openFolder, Colours::transparentBlack })
    {
        list.setHeaderComponent (std::make_unique<ListBoxHeader> (Array<String> { "File", "Binary Resource", "Xcode Resource", "Compile", "Skip PCH", "Compiler Flag Scheme" },
                                                                  Array<float>  {  0.25f,  0.125f,            0.125f,           0.125f,    0.125f,     0.25f }));
        list.setModel (this);
        list.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (list);
        list.updateContent();
        list.setRowHeight (30);
        item.state.addListener (this);
        lookAndFeelChanged();

        addAndMakeVisible (header);
    }

    ~FileGroupInformationComponent() override
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.setColour (findColour (secondaryBackgroundColourId));
        g.fillRect (getLocalBounds().reduced (12, 0));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (12, 0);

        header.setBounds (bounds.removeFromTop (40));
        list.setBounds (bounds.reduced (10, 4));
    }

    void parentSizeChanged() override
    {
        setSize (jmax (550, getParentWidth()), getParentHeight());
    }

    int getNumRows() override
    {
        return item.getNumChildren();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (findColour (rowNumber % 2 == 0 ? widgetBackgroundColourId
                                                    : secondaryWidgetBackgroundColourId));
        g.fillRect (0, 0, width, height - 1);
    }

    Component* refreshComponentForRow (int rowNumber, bool /*isRowSelected*/, Component* existingComponentToUpdate) override
    {
        std::unique_ptr<Component> existing (existingComponentToUpdate);

        if (rowNumber < getNumRows())
        {
            auto child = item.getChild (rowNumber);

            if (existingComponentToUpdate == nullptr
                 || dynamic_cast<FileOptionComponent*> (existing.get())->item != child)
            {
                existing.reset();
                existing.reset (new FileOptionComponent (child, dynamic_cast<ListBoxHeader*> (list.getHeaderComponent())));
            }
        }

        return existing.release();
    }

    String getGroupPath() const    { return item.getFile().getFullPathName(); }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override         { itemChanged(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override           { itemChanged(); }
    void valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

private:
    Project::Item item;
    ListBox list;
    ContentViewHeader header;

    void itemChanged()
    {
        list.updateContent();
        repaint();
    }

    //==============================================================================
    class FileOptionComponent final : public Component
    {
    public:
        FileOptionComponent (const Project::Item& fileItem, ListBoxHeader* listBoxHeader)
            : item (fileItem),
              header (listBoxHeader),
              compilerFlagSchemeSelector (item)
        {
            if (item.isFile())
            {
                auto isSourceFile = item.isSourceFile();

                if (isSourceFile)
                {
                    addAndMakeVisible (compileButton);
                    compileButton.getToggleStateValue().referTo (item.getShouldCompileValue());
                    compileButton.onStateChange = [this] { compileEnablementChanged(); };
                }

                addAndMakeVisible (binaryResourceButton);
                binaryResourceButton.getToggleStateValue().referTo (item.getShouldAddToBinaryResourcesValue());

                addAndMakeVisible (xcodeResourceButton);
                xcodeResourceButton.getToggleStateValue().referTo (item.getShouldAddToXcodeResourcesValue());

                if (isSourceFile)
                {
                    addChildComponent (skipPCHButton);
                    skipPCHButton.getToggleStateValue().referTo (item.getShouldSkipPCHValue());

                    addChildComponent (compilerFlagSchemeSelector);

                    compileEnablementChanged();
                }
            }
        }

        void paint (Graphics& g) override
        {
            if (header != nullptr)
            {
                auto textBounds = getLocalBounds().removeFromLeft (roundToInt (header->getProportionAtIndex (0) * (float) getWidth()));

                auto iconBounds = textBounds.removeFromLeft (25);

                if (item.isImageFile())
                    iconBounds.reduce (5, 5);

                item.getIcon().withColour (findColour (treeIconColourId)).draw (g, iconBounds.toFloat(), item.isIconCrossedOut());

                g.setColour (findColour (widgetTextColourId));

                g.drawText (item.getName(), textBounds, Justification::centredLeft);
            }
        }

        void resized() override
        {
            if (header != nullptr)
            {
                auto bounds = getLocalBounds();
                auto width = (float) getWidth();

                bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (0) * width));

                binaryResourceButton.setBounds       (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (1) * width)));
                xcodeResourceButton.setBounds        (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (2) * width)));
                compileButton.setBounds              (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (3) * width)));
                skipPCHButton.setBounds              (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (4) * width)));
                compilerFlagSchemeSelector.setBounds (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (5) * width)));
            }
        }

        Project::Item item;

    private:
        //==============================================================================
        class CompilerFlagSchemeSelector final : public Component,
                                                 private Value::Listener
        {
        public:
            CompilerFlagSchemeSelector (Project::Item& it)
                : item (it)
            {
                schemeBox.setTextWhenNothingSelected ("None");
                updateCompilerFlagSchemeComboBox();
                schemeBox.onChange = [this] { handleComboBoxSelection(); };

                addAndMakeVisible (schemeBox);
                addChildComponent (newSchemeLabel);

                newSchemeLabel.setEditable (true);
                newSchemeLabel.setJustificationType (Justification::centredLeft);
                newSchemeLabel.onEditorHide = [this]
                {
                    newSchemeLabel.setVisible (false);
                    schemeBox.setVisible (true);

                    auto newScheme = newSchemeLabel.getText();

                    item.project.addCompilerFlagScheme (newScheme);

                    if (item.getCompilerFlagSchemeString().isEmpty())
                        item.setCompilerFlagScheme (newScheme);

                    updateCompilerFlagSchemeComboBox();
                };

                selectScheme (item.getCompilerFlagSchemeString());

                projectCompilerFlagSchemesValue = item.project.getProjectValue (Ids::compilerFlagSchemes);
                projectCompilerFlagSchemesValue.addListener (this);

                lookAndFeelChanged();
            }

            void resized() override
            {
                auto b =  getLocalBounds();

                schemeBox.setBounds (b);
                newSchemeLabel.setBounds (b);
            }

        private:
            void valueChanged (Value&) override   { updateCompilerFlagSchemeComboBox(); }

            void lookAndFeelChanged() override
            {
                schemeBox.setColour (ComboBox::outlineColourId, Colours::transparentBlack);
                schemeBox.setColour (ComboBox::textColourId,    findColour (defaultTextColourId));
            }

            void updateCompilerFlagSchemeComboBox()
            {
                auto itemScheme = item.getCompilerFlagSchemeString();
                auto allSchemes = item.project.getCompilerFlagSchemes();

                if (itemScheme.isNotEmpty() && ! allSchemes.contains (itemScheme))
                {
                    item.clearCurrentCompilerFlagScheme();
                    itemScheme = {};
                }

                schemeBox.clear();

                schemeBox.addItemList (allSchemes, 1);
                schemeBox.addSeparator();
                schemeBox.addItem ("Add a new scheme...", -1);
                schemeBox.addItem ("Delete selected scheme", -2);
                schemeBox.addItem ("Clear", -3);

                selectScheme (itemScheme);
            }

            void handleComboBoxSelection()
            {
                auto selectedID = schemeBox.getSelectedId();

                if (selectedID > 0)
                {
                    item.setCompilerFlagScheme (schemeBox.getItemText (selectedID - 1));
                }
                else if (selectedID == -1)
                {
                    newSchemeLabel.setText ("NewScheme", dontSendNotification);

                    schemeBox.setVisible (false);
                    newSchemeLabel.setVisible (true);

                    newSchemeLabel.showEditor();

                    if (auto* ed = newSchemeLabel.getCurrentTextEditor())
                        ed->setInputRestrictions (64, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_");
                }
                else if (selectedID == -2)
                {
                    auto currentScheme = item.getCompilerFlagSchemeString();

                    if (currentScheme.isNotEmpty())
                    {
                        item.project.removeCompilerFlagScheme (currentScheme);
                        item.clearCurrentCompilerFlagScheme();
                    }

                    updateCompilerFlagSchemeComboBox();
                }
                else if (selectedID == -3)
                {
                    schemeBox.setSelectedId (0);
                    item.clearCurrentCompilerFlagScheme();
                }
            }

            void selectScheme (const String& schemeToSelect)
            {
                if (schemeToSelect.isNotEmpty())
                {
                    for (int i = 0; i < schemeBox.getNumItems(); ++i)
                    {
                        if (schemeBox.getItemText (i) == schemeToSelect)
                        {
                            schemeBox.setSelectedItemIndex (i);
                            return;
                        }
                    }
                }

                schemeBox.setSelectedId (0);
            }

            Project::Item& item;
            Value projectCompilerFlagSchemesValue;

            ComboBox schemeBox;
            Label newSchemeLabel;
        };

        void compileEnablementChanged()
        {
            auto shouldBeCompiled = compileButton.getToggleState();

            skipPCHButton.setVisible (shouldBeCompiled);
            compilerFlagSchemeSelector.setVisible (shouldBeCompiled);
        }

        //==============================================================================
        ListBoxHeader* header;

        ToggleButton compileButton, binaryResourceButton, xcodeResourceButton, skipPCHButton;
        CompilerFlagSchemeSelector compilerFlagSchemeSelector;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileGroupInformationComponent)
};
