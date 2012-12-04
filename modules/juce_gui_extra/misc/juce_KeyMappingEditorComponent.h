/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
#define __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__


//==============================================================================
/**
    A component to allow editing of the keymaps stored by a KeyPressMappingSet
    object.

    @see KeyPressMappingSet
*/
class JUCE_API  KeyMappingEditorComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a KeyMappingEditorComponent.

        @param mappingSet   this is the set of mappings to display and edit. Make sure the
                            mappings object is not deleted before this component!
        @param showResetToDefaultButton     if true, then at the bottom of the list, the
                                            component will include a 'reset to defaults' button.
    */
    KeyMappingEditorComponent (KeyPressMappingSet& mappingSet,
                               bool showResetToDefaultButton);

    /** Destructor. */
    virtual ~KeyMappingEditorComponent();

    //==============================================================================
    /** Sets up the colours to use for parts of the component.

        @param mainBackground       colour to use for most of the background
        @param textColour           colour to use for the text
    */
    void setColours (const Colour& mainBackground,
                     const Colour& textColour);

    /** Returns the KeyPressMappingSet that this component is acting upon. */
    KeyPressMappingSet& getMappings() const noexcept                { return mappings; }

    /** Returns the ApplicationCommandManager that this component is connected to. */
    ApplicationCommandManager& getCommandManager() const noexcept   { return mappings.getCommandManager(); }


    //==============================================================================
    /** Can be overridden if some commands need to be excluded from the list.

        By default this will use the KeyPressMappingSet's shouldCommandBeVisibleInEditor()
        method to decide what to return, but you can override it to handle special cases.
    */
    virtual bool shouldCommandBeIncluded (CommandID commandID);

    /** Can be overridden to indicate that some commands are shown as read-only.

        By default this will use the KeyPressMappingSet's shouldCommandBeReadOnlyInEditor()
        method to decide what to return, but you can override it to handle special cases.
    */
    virtual bool isCommandReadOnly (CommandID commandID);

    /** This can be overridden to let you change the format of the string used
        to describe a keypress.

        This is handy if you're using non-standard KeyPress objects, e.g. for custom
        keys that are triggered by something else externally. If you override the
        method, be sure to let the base class's method handle keys you're not
        interested in.
    */
    virtual String getDescriptionForKeyPress (const KeyPress& key);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        To change the colours of the menu that pops up

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId  = 0x100ad00,    /**< The background colour to fill the editor background. */
        textColourId        = 0x100ad01,    /**< The colour for the text. */
    };

    //==============================================================================
    /** @internal */
    void parentHierarchyChanged();
    /** @internal */
    void resized();

private:
    //==============================================================================
    KeyPressMappingSet& mappings;
    TreeView tree;
    TextButton resetButton;

    class TopLevelItem;
    class ChangeKeyButton;
    class MappingItem;
    class CategoryItem;
    class ItemComponent;
    friend class TopLevelItem;
    friend class OwnedArray <ChangeKeyButton>;
    friend class ScopedPointer<TopLevelItem>;
    ScopedPointer<TopLevelItem> treeItem;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappingEditorComponent)
};


#endif   // __JUCE_KEYMAPPINGEDITORCOMPONENT_JUCEHEADER__
