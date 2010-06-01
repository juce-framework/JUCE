/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__
#define __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__


//==============================================================================
/**
*/
class DrawableTreeViewItem  : public JucerTreeViewBase,
                              public ValueTree::Listener,
                              public ChangeListener
{
public:
    DrawableTreeViewItem (DrawableEditor& editor_, const ValueTree& drawableRoot)
        : editor (editor_), node (drawableRoot), typeName (drawableRoot.getType().toString())
    {
        node.getState().addListener (this);
        editor.getSelection().addChangeListener (this);
    }

    ~DrawableTreeViewItem()
    {
        editor.getSelection().removeChangeListener (this);
        node.getState().removeListener (this);
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
    {
    }

    void valueTreeChildrenChanged (ValueTree& tree)
    {
        if (tree == node.getState())
            refreshSubItems();
    }

    void valueTreeParentChanged (ValueTree& tree)
    {
    }

    //==============================================================================
    // TreeViewItem stuff..
    bool mightContainSubItems()
    {
        return node.getState().getType() == DrawableComposite::valueTreeType;
    }

    const String getUniqueName() const
    {
        jassert (node.getID().isNotEmpty());
        return node.getID();
    }

    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
            refreshSubItems();
    }

    void refreshSubItems()
    {
        if (node.getState().getType() == DrawableComposite::valueTreeType)
        {
            ScopedPointer <XmlElement> oldOpenness (getOpennessState());

            clearSubItems();

            DrawableComposite::ValueTreeWrapper composite (node.getState());

            for (int i = 0; i < composite.getNumDrawables(); ++i)
            {
                ValueTree subNode (composite.getDrawableState (i));
                DrawableTreeViewItem* const item = new DrawableTreeViewItem (editor, subNode);
                addSubItem (item);
            }

            if (oldOpenness != 0)
                restoreOpennessState (*oldOpenness);

            editor.getSelection().changed();
        }
    }

    const String getDisplayName() const
    {
        const String name (getRenamingName());
        return typeName + (name.isEmpty() ? String::empty
                                          : (" \"" + name + "\""));
    }

    const String getRenamingName() const
    {
        return node.getID();
    }

    void setName (const String& newName)
    {
    }

    bool isMissing()            { return false; }

    const Image getIcon() const
    {
        return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage();
    }

    void itemClicked (const MouseEvent& e)
    {
    }

    void itemDoubleClicked (const MouseEvent& e)
    {
    }

    void itemSelectionChanged (bool isNowSelected)
    {
        const String objectId (node.getID());

        if (isNowSelected)
            editor.getSelection().addToSelection (objectId);
        else
            editor.getSelection().deselect (objectId);
    }

    void changeListenerCallback (void*)
    {
        setSelected (editor.getSelection().isSelected (node.getID()), false);
    }

    const String getTooltip()
    {
        return String::empty;
    }

    const String getDragSourceDescription()
    {
        return drawableItemDragType;
    }

    //==============================================================================
    // Drag-and-drop stuff..
    bool isInterestedInFileDrag (const StringArray& files)
    {
        return false;
    }

    void filesDropped (const StringArray& files, int insertIndex)
    {
    }

    bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent)
    {
        return false;
    }

    void itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex)
    {
    }

    //==============================================================================
    void showRenameBox()
    {
    }

    // Text editor listener for renaming..
    void textEditorTextChanged (TextEditor& textEditor)         {}
    void textEditorReturnKeyPressed (TextEditor& textEditor)    { textEditor.exitModalState (1); }
    void textEditorEscapeKeyPressed (TextEditor& textEditor)    { textEditor.exitModalState (0); }
    void textEditorFocusLost (TextEditor& textEditor)           { textEditor.exitModalState (0); }

    //==============================================================================
    DrawableEditor& editor;
    Drawable::ValueTreeWrapperBase node;

private:
    String typeName;

    DrawableEditor* getEditor() const
    {
        return getOwnerView()->findParentComponentOfClass ((DrawableEditor*) 0);
    }
};


#endif   // __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__
