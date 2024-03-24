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
    void handleResize();

    void handleChange();
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
