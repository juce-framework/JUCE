/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../jucer_JucerDocument.h"

//==============================================================================
class ResourceEditorPanel  : public Component,
                             private TableListBoxModel,
                             private ChangeListener
{
public:
    ResourceEditorPanel (JucerDocument& document);
    ~ResourceEditorPanel() override;

    void resized() override;
    void paint (Graphics& g) override;
    void visibilityChanged() override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    int getNumRows() override;
    void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
    int getColumnAutoSizeWidth (int columnId) override;
    void sortOrderChanged (int newSortColumnId, bool isForwards) override;
    void selectedRowsChanged (int lastRowSelected) override;

private:
    void lookAndFeelChanged() override;
    void reloadAll();

    JucerDocument& document;
    std::unique_ptr<TableListBox> listBox;
    TextButton addButton, reloadAllButton, delButton;
};
