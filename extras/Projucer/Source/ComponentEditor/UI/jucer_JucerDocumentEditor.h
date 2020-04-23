/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
                              public ChangeListener
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
    void changeListenerCallback (ChangeBroadcaster*) override;
    bool keyPressed (const KeyPress&) override;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    static JucerDocumentEditor* getActiveDocumentHolder();

private:
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
