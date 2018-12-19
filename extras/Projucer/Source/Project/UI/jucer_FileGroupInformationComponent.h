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
class FileGroupInformationComponent  : public Component,
                                       private ListBoxModel,
                                       private ValueTree::Listener
{
public:
    FileGroupInformationComponent (const Project::Item& group)
        : item (group),
          header (item.getName(), { getIcons().openFolder, Colours::transparentBlack })
    {
        list.setHeaderComponent (new ListBoxHeader ( { "File", "Binary Resource", "Xcode Resource", "Compile", "Compiler Flags Setting" },
                                                     { 0.4f, 0.2f, 0.2f, 0.2f, 0.4f } ));
        list.setModel (this);
        list.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (list);
        list.updateContent();
        list.setRowHeight (30);
        item.state.addListener (this);
        lookAndFeelChanged();

        addAndMakeVisible (header);
    }

    ~FileGroupInformationComponent()
    {
        item.state.removeListener (this);
    }
    
    void lookAndFeelChanged() override
    {
        LookAndFeel& lookAndFeel = getLookAndFeel();
        const auto C = findColour (widgetTextColourId);
        lookAndFeel.setColour(ComboBox::textColourId, C);
        lookAndFeel.setColour(ComboBox::arrowColourId, C);
        lookAndFeel.setColour(ComboBox::outlineColourId, C);
        lookAndFeel.setColour(ToggleButton::textColourId, C);
        lookAndFeel.setColour(ToggleButton::tickColourId, C);
        setLookAndFeel(&lookAndFeel);
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
        const auto C = findColour (widgetBackgroundColourId);
        g.setColour( rowNumber % 2 == 0 ? C : C.darker(0.1f) );
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
    class FileOptionComponent  : public Component,
                                 private ValueTree::Listener,
                                 private Value::Listener,
                                 private ComboBox::Listener
    {
    public:
        FileOptionComponent (const Project::Item& fileItem, ListBoxHeader* listBoxHeader)
            : item (fileItem),
              header (listBoxHeader)
        {
            if (item.isFile())
            {
                addAndMakeVisible (compileButton);
                compileButton.getToggleStateValue().referTo (item.getShouldCompileValue());

                addAndMakeVisible (binaryResourceButton);
                binaryResourceButton.getToggleStateValue().referTo (item.getShouldAddToBinaryResourcesValue());

                addAndMakeVisible (xcodeResourceButton);
                xcodeResourceButton.getToggleStateValue().referTo (item.getShouldAddToXcodeResourcesValue());

                addChildComponent(settingsNameEditor);
                settingsNameEditor.setEditable(true);
                settingsNameEditor.setEnabled(false);
                
                addAndMakeVisible (compilerFlagsComboBox);
                const auto setComboBoxEnabled = [this]{
                    const auto compileOn = compileButton.getToggleState();
                    compilerFlagsComboBox.setVisible(compileOn);
                    compilerFlagsComboBox.setEnabled(compileOn);
                };
                setComboBoxEnabled();
                compileButton.onStateChange = setComboBoxEnabled;
                
                updateCompilerFlagsSettings();
                
                compilerFlagsComboBox.addListener(this);
                
                item.state.addListener(this);
                
                compilerFlagsSettingsValue = item.project.getProjectValue(Ids::compilerFlagsSettings);
                compilerFlagsSettingsValue.addListener(this);
            }
        }
        
        ~FileOptionComponent()
        {
            compilerFlagsSettingsValue.removeListener(this);
            item.state.removeListener(this);
        }

        void paint (Graphics& g) override
        {
            if (header != nullptr)
            {
                auto textBounds = getLocalBounds().removeFromLeft (roundToInt (header->getProportionAtIndex (0) * getWidth()));

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
                auto width = getWidth();

                bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (0) * width));

                binaryResourceButton.setBounds  (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (1) * width)));
                xcodeResourceButton.setBounds   (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (2) * width)));
                compileButton.setBounds         (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (3) * width)));
                compilerFlagsComboBox.setBounds ((bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (4) * width))).reduced(3));
                settingsNameEditor.setBounds    (compilerFlagsComboBox.getBounds());
            }
        }
        
        void valueTreePropertyChanged (ValueTree&, const Identifier&) override
        {
            updateCompilerFlagsSettings();
        }

        void valueTreeChildAdded (ValueTree&, ValueTree&) override
        {
            updateCompilerFlagsSettings();
        }
        
        void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override
        {
            updateCompilerFlagsSettings();
        }
        
        void valueTreeChildOrderChanged (ValueTree&, int, int) override {}
        void valueTreeParentChanged (ValueTree&) override               {}

        void valueChanged (Value&) override
        {
            updateCompilerFlagsSettings();
        }
        
        void comboBoxChanged( ComboBox* cb ) override
        {
            if ( cb != &compilerFlagsComboBox ) return;
            auto selectedID = cb->getSelectedId();
            if ( selectedID == 0 ) {
                selectedID = 1;
                cb->setSelectedId(selectedID);
            }
            if ( selectedID > 0 ) {
                cb->setItemEnabled(-2, selectedID>1 );
                const auto activeSetting = cb->getItemText(cb->indexOfItemId(selectedID));
                item.setCompilerFlagsSetting(activeSetting);
            } else if ( selectedID == -1 ) {
                cb->setVisible(false);
                cb->setEnabled(false);
                settingsNameEditor.setVisible(true);
                settingsNameEditor.setEnabled(true);
                settingsNameEditor.setText("New setting", NotificationType::dontSendNotification );
                settingsNameEditor.onEditorHide = [this] {
                    settingsNameEditor.onEditorHide = nullptr;
                    const auto newSettingName = sanitizeSymbol(settingsNameEditor.getText().trim());
                    if( newSettingName.length() > 0 ) {
                        item.project.addCompilerFlagsSetting(newSettingName);
                        item.setCompilerFlagsSetting(newSettingName);
                    }
                    settingsNameEditor.setVisible(false);
                    settingsNameEditor.setEnabled(false);
                    compilerFlagsComboBox.setEnabled(true);
                    compilerFlagsComboBox.setVisible(true);
                };
                settingsNameEditor.showEditor();
            } else if ( selectedID == -2 ) {
                const auto currentSetting = item.getCompilerFlagsSetting();
                if ( currentSetting != "default" ) {
                    item.setCompilerFlagsSetting(cb->getItemText(cb->indexOfItemId(1)));
                    item.project.removeCompilerFlagsSetting(currentSetting);
                }
            }
        }
        

        Project::Item item;

    private:
        
        static String sanitizeSymbol( const String& s )
        {
            const auto s2 = s.replaceCharacters(" ,.;:-","______").toStdString();
            String result;
            for( const auto c : s2 ) {
                if ( isalnum(c) || c == '_' ) result += c;
            }
            return result;
        }
        
        void updateCompilerFlagsSettings()
        {
            auto& cb = compilerFlagsComboBox;
            cb.clear();
            cb.addItemList (item.project.getCompilerFlagsSettings(), 1);
            cb.addSeparator();
            cb.addItem ("Add new setting ...", -1 );
            cb.addItem ("Remove selected setting ...", -2 );
            cb.setTextWhenNothingSelected("setting unavailable");
            const auto activeItemText = item.getCompilerFlagsSetting();
            for ( int i = 0; i<cb.getNumItems(); ++i ) {
                if ( cb.getItemText(i) == activeItemText ) {
                    cb.setSelectedId(cb.getItemId(i));
                    break;
                }
            }
        }
        
        ListBoxHeader* header;

        ToggleButton compileButton, binaryResourceButton, xcodeResourceButton;
        ComboBox compilerFlagsComboBox;
        Label settingsNameEditor;
        
        Value compilerFlagsSettingsValue;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileGroupInformationComponent)
};
