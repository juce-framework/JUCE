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

#ifndef __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
#define __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__

#include "../keyboard/juce_KeyPressMappingSet.h"
#include "../controls/juce_TreeView.h"
#include "../buttons/juce_TextButton.h"


//==============================================================================
/**
    A component to allow editing of the keymaps stored by a KeyPressMappingSet
    object.

    @see KeyPressMappingSet
*/
class JUCE_API  KeyMappingEditorComponent  : public Component,
                                             public TreeViewItem,
                                             public ChangeListener,
                                             private ButtonListener
{
public:
    //==============================================================================
    /** Creates a KeyMappingEditorComponent.

        @param mappingSet   this is the set of mappings to display and
                            edit. Make sure the mappings object is not
                            deleted before this component!
        @param showResetToDefaultButton     if true, then at the bottom of the
                            list, the component will include a 'reset to
                            defaults' button.
    */
    KeyMappingEditorComponent (KeyPressMappingSet* const mappingSet,
                               const bool showResetToDefaultButton);

    /** Destructor. */
    virtual ~KeyMappingEditorComponent();

    //==============================================================================
    /** Sets up the colours to use for parts of the component.

        @param mainBackground       colour to use for most of the background
        @param textColour           colour to use for the text
    */
    void setColours (const Colour& mainBackground,
                     const Colour& textColour);

    /** Returns the KeyPressMappingSet that this component is acting upon.
    */
    KeyPressMappingSet* getMappings() const throw()                 { return mappings; }


    //==============================================================================
    /** Can be overridden if some commands need to be excluded from the list.

        By default this will use the KeyPressMappingSet's shouldCommandBeVisibleInEditor()
        method to decide what to return, but you can override it to handle special cases.
    */
    virtual bool shouldCommandBeIncluded (const CommandID commandID);

    /** Can be overridden to indicate that some commands are shown as read-only.

        By default this will use the KeyPressMappingSet's shouldCommandBeReadOnlyInEditor()
        method to decide what to return, but you can override it to handle special cases.
    */
    virtual bool isCommandReadOnly (const CommandID commandID);

    /** This can be overridden to let you change the format of the string used
        to describe a keypress.

        This is handy if you're using non-standard KeyPress objects, e.g. for custom
        keys that are triggered by something else externally. If you override the
        method, be sure to let the base class's method handle keys you're not
        interested in.
    */
    virtual const String getDescriptionForKeyPress (const KeyPress& key);


    //==============================================================================
    /** @internal */
    void parentHierarchyChanged();
    /** @internal */
    void resized();
    /** @internal */
    void changeListenerCallback (void*);
    /** @internal */
    bool mightContainSubItems();
    /** @internal */
    const String getUniqueName() const;
    /** @internal */
    void buttonClicked (Button* button);

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    KeyPressMappingSet* mappings;
    TreeView* tree;
    friend class KeyMappingTreeViewItem;
    friend class KeyCategoryTreeViewItem;
    friend class KeyMappingItemComponent;
    friend class KeyMappingChangeButton;
    Colour backgroundColour, textColour;
    TextButton* resetButton;

    void assignNewKey (const CommandID commandID, int index);

    KeyMappingEditorComponent (const KeyMappingEditorComponent&);
    const KeyMappingEditorComponent& operator= (const KeyMappingEditorComponent&);
};


#endif   // __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
