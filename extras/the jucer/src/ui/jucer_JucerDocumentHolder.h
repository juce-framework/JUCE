/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCER_JUCERDOCUMENTHOLDER_JUCEHEADER__
#define __JUCER_JUCERDOCUMENTHOLDER_JUCEHEADER__

#include "../model/jucer_JucerDocument.h"
#include "jucer_ComponentLayoutEditor.h"
#include "jucer_PaintRoutineEditor.h"
#include "jucer_ComponentLayoutPanel.h"


//==============================================================================
/**
*/
class JucerDocumentHolder   : public Component,
                              public ApplicationCommandTarget,
                              public ChangeListener
{
public:
    //==============================================================================
    JucerDocumentHolder (JucerDocument* const document);
    ~JucerDocumentHolder();

    JucerDocument* getDocument() const throw()                  { return document; }

    bool close();

    void refreshPropertiesPanel() const;
    void updateTabs();

    void showLayout();
    void showGraphics (PaintRoutine* routine);

    void setViewportToLastPos (Viewport* vp);
    void storeLastViewportPos (Viewport* vp);

    Image* createComponentLayerSnapshot() const;

    //==============================================================================
    void paint (Graphics& g);
    void resized();
    void changeListenerCallback (void*);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

    static JucerDocumentHolder* getActiveDocumentHolder();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    JucerDocument* const document;
    TabbedComponent* tabbedComponent;
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


#endif   // __JUCER_JUCERDOCUMENTHOLDER_JUCEHEADER__
