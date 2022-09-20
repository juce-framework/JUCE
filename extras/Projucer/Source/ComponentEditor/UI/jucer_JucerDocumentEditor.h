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
#include "jucer_ComponentLayoutEditor.h"
#include "jucer_PaintRoutineEditor.h"
#include "jucer_ComponentLayoutPanel.h"

//==============================================================================
class JucerDocumentEditor   : public Component,
                              public ApplicationCommandTarget,
                              private ChangeListener
{
public:
    //==============================================================================
    JucerDocumentEditor (JucerDocument* const document);
    ~JucerDocumentEditor() override;

    JucerDocument* getDocument() const noexcept         { return document.get(); }

    void refreshPropertiesPanel() const;
    void updateTabs();

    void showLayout();
    void showGraphics (PaintRoutine* routine);

    void setViewportToLastPos (Viewport* vp, EditingPanelBase& editor);
    void storeLastViewportPos (Viewport* vp, EditingPanelBase& editor);

    Image createComponentLayerSnapshot() const;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    bool keyPressed (const KeyPress&) override;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    static JucerDocumentEditor* getActiveDocumentHolder();

private:
    void changeListenerCallback (ChangeBroadcaster*) override;

    std::unique_ptr<JucerDocument> document;
    ComponentLayoutPanel* compLayoutPanel = nullptr;

    struct JucerDocumentTabs  : public TabbedComponent
    {
        JucerDocumentTabs (JucerDocument* d)  : TabbedComponent (TabbedButtonBar::TabsAtTop), document (d) {}
        void currentTabChanged (int, const String&) override    { document->refreshCustomCodeFromDocument(); }

        JucerDocument* document;
    };

    JucerDocumentTabs tabbedComponent;

    int lastViewportX = 0, lastViewportY = 0;
    double currentZoomLevel = 1.0;

    void saveLastSelectedTab() const;
    void restoreLastSelectedTab();

    bool isSomethingSelected() const;
    bool areMultipleThingsSelected() const;

    // only non-zero if a layout tab is selected
    ComponentLayout* getCurrentLayout() const;
    // only non-zero if a graphics tab is selected
    PaintRoutine* getCurrentPaintRoutine() const;

    void setZoom (double scale);
    double getZoom() const;

    void addElement (int index);
    void addComponent (int index);
};
