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

#ifndef __JUCER_RESOURCEEDITORPANEL_JUCEHEADER__
#define __JUCER_RESOURCEEDITORPANEL_JUCEHEADER__

#include "../model/jucer_JucerDocument.h"



//==============================================================================
/**
*/
class ResourceEditorPanel  : public Component,
                             private TableListBoxModel,
                             private ChangeListener,
                             private ButtonListener
{
public:
    //==============================================================================
    ResourceEditorPanel (JucerDocument& document);
    ~ResourceEditorPanel();

    void resized();
    void visibilityChanged();
    void changeListenerCallback (void*);
    void buttonClicked (Button*);

    int getNumRows();
    void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected);
    void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);
    Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate);
    int getColumnAutoSizeWidth (int columnId);
    void sortOrderChanged (int newSortColumnId, const bool isForwards);
    void selectedRowsChanged (int lastRowSelected);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    JucerDocument& document;
    TableListBox* listBox;
    TextButton* addButton;
    TextButton* reloadAllButton;
    TextButton* delButton;
};




#endif   // __JUCER_RESOURCEEDITORPANEL_JUCEHEADER__
