/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#pragma once

#include "../jucer_JucerDocument.h"
#include "jucer_ComponentLayoutEditor.h"
#include "jucer_PaintRoutineEditor.h"
#include "jucer_ComponentLayoutPanel.h"


//==============================================================================
/**
*/
class JucerDocumentTabs : public TabbedComponent
{
public:
    JucerDocumentTabs(JucerDocument* const);
    
    void currentTabChanged (const int, const String&);
    
private:
    JucerDocument* const document;
};

class JucerDocumentEditor   : public Component,
                              public ApplicationCommandTarget,
                              public ChangeListener
{
public:
    //==============================================================================
    JucerDocumentEditor (JucerDocument* const document);
    ~JucerDocumentEditor();

    JucerDocument* getDocument() const noexcept         { return document; }

    void refreshPropertiesPanel() const;
    void updateTabs();

    void showLayout();
    void showGraphics (PaintRoutine* routine);

    void setViewportToLastPos (Viewport* vp, EditingPanelBase& editor);
    void storeLastViewportPos (Viewport* vp, EditingPanelBase& editor);

    Image createComponentLayerSnapshot() const;

    //==============================================================================
    void paint (Graphics& g);
    void resized();
    void changeListenerCallback (ChangeBroadcaster*);
    bool keyPressed (const KeyPress&);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>&);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo&);

    static JucerDocumentEditor* getActiveDocumentHolder();

private:
    ScopedPointer<JucerDocument> document;
    JucerDocumentTabs tabbedComponent;
    ComponentLayoutPanel* compLayoutPanel;

    bool isSomethingSelected() const;
    int lastViewportX, lastViewportY;

    double currentZoomLevel;

    // only non-zero if a layout tab is selected
    ComponentLayout* getCurrentLayout() const;
    // only non-zero if a graphics tab is selected
    PaintRoutine* getCurrentPaintRoutine() const;

    void setZoom (double scale);
    double getZoom() const;

    void addElement (const int index);
    void addComponent (const int index);
};
