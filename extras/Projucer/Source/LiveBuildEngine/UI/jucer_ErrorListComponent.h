/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class ErrorListComp     : public TreePanelBase,
                          private ChangeListener
{
public:
    ErrorListComp (ErrorList& el)
        : TreePanelBase (nullptr, String()),
          errorList (el)
    {
        setName ("Errors and Warnings");
        setEmptyTreeMessage ("(No Messages)");

        tree.setMultiSelectEnabled (false);
        tree.setRootItemVisible (false);
        setRoot (new ErrorRootTreeItem (errorList));

        errorList.addChangeListener (this);
        errorListChanged();
    }

    ~ErrorListComp() override
    {
        errorList.removeChangeListener (this);
    }

    void errorListChanged()
    {
        static_cast<ErrorRootTreeItem*> (rootItem.get())->refreshSubItems();
    }

    void moveBy (const int delta)
    {
        if (delta < 0)
            if (TreeViewItem* selected = tree.getSelectedItem (0))
                if (selected->getRowNumberInTree() <= 1)
                    return;

        tree.moveSelectedRow (delta);

        if (dynamic_cast<ErrorMessageTreeItem*> (tree.getSelectedItem (0)) == nullptr)
            tree.moveSelectedRow (delta);
    }

    void showNext()         { moveBy (1); }
    void showPrevious()     { moveBy (-1); }

private:
    TreeView list;
    ErrorList& errorList;
    struct ErrorMessageTreeItem;

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        errorListChanged();
    }

    static void limitNumberOfSubItems (TreeViewItem& item, const int maxSubItems)
    {
        while (item.getNumSubItems() > maxSubItems)
            item.removeSubItem (item.getNumSubItems() - 1);
    }

    //==============================================================================
    class ErrorRootTreeItem  : public JucerTreeViewBase
    {
    public:
        ErrorRootTreeItem (ErrorList& el)  : errorList (el) {}

        String getRenamingName() const override          { return getDisplayName(); }
        String getDisplayName() const override           { return "Errors and Warnings"; }
        void setName (const String&) override            {}
        bool isMissing() const override                  { return false; }
        Icon getIcon() const override                    { return Icon (getIcons().bug, getContentColour (true)); }
        bool canBeSelected() const override              { return true; }
        bool mightContainSubItems() override             { return true; }
        String getUniqueName() const override            { return "errors"; }

        void refreshSubItems()
        {
            Array<DiagnosticMessage> errors;
            errorList.takeCopy (errors);

            StringArray files;

            for (const auto& m : errors)
            {
                files.addIfNotAlreadyThere (m.mainFile);

                if (m.associatedDiagnostic != nullptr)
                    files.addIfNotAlreadyThere (m.associatedDiagnostic->mainFile);
            }

            limitNumberOfSubItems (*this, files.size());

            int i = 0;

            for (const auto& f : files)
            {
                if (i >= getNumSubItems() || static_cast<CompileUnitTreeItem*> (getSubItem (i))->compileUnit != f)
                {
                    limitNumberOfSubItems (*this, i);
                    addSubItem (new CompileUnitTreeItem (f));
                }

                static_cast<CompileUnitTreeItem*> (getSubItem (i))->refresh (errors);
                ++i;
            }
        }

    private:
        ErrorList& errorList;
    };

    //==============================================================================
    struct CompileUnitTreeItem  : public JucerTreeViewBase
    {
        CompileUnitTreeItem (const String& filename)   : compileUnit (filename) {}

        void setName (const String&) override    {}
        void addSubItems() override              {}
        bool isMissing() const override          { return false; }
        Icon getIcon() const override            { return Icon (getIcons().bug, getContentColour (true)); }
        bool canBeSelected() const override      { return true; }
        bool mightContainSubItems() override     { return true; }
        String getUniqueName() const override    { return String::toHexString (compileUnit.hashCode64()); }

        String getRenamingName() const override  { return getDisplayName(); }
        String getDisplayName() const override
        {
            if (File::isAbsolutePath (compileUnit))
            {
                File f (compileUnit);
                return f.exists() ? f.getFileName() : compileUnit;
            }

            if (! compileUnit.isEmpty())
                return compileUnit;

            return String ("Global");
        }

        void showOverlays()
        {
            for (int i = 0; i < getNumSubItems(); ++i)
                if (auto* e = dynamic_cast<ErrorMessageTreeItem*> (getSubItem (i)))
                    e->showOverlays();
        }

        ErrorMessageTreeItem* getItemForError (const DiagnosticMessage& m) const
        {
            for (int i = 0; i < getNumSubItems(); ++i)
                if (auto* item = dynamic_cast<ErrorMessageTreeItem*> (getSubItem(i)))
                    if (item->message == m)
                        return item;

            return nullptr;
        }

        void refresh (const Array<DiagnosticMessage>& allErrors)
        {
            clearSubItems();

            for (const auto& error : allErrors)
                if (error.mainFile == compileUnit && error.associatedDiagnostic == nullptr)
                    addSubItem (new ErrorMessageTreeItem (error));

            for (const auto& error : allErrors)
                if (error.mainFile == compileUnit && error.associatedDiagnostic != nullptr)
                    if (ErrorMessageTreeItem* parent = getItemForError (*error.associatedDiagnostic))
                        parent->addSubItem (new ErrorMessageTreeItem (error));
        }

        void showDocument() override
        {
            if (ProjectContentComponent* pcc = getProjectContentComponent())
                if (File::isAbsolutePath (compileUnit) && File (compileUnit).exists())
                    pcc->showEditorForFile (File (compileUnit), true);
        }

        String compileUnit;
    };

    //==============================================================================
    struct ErrorMessageTreeItem  : public JucerTreeViewBase
    {
        ErrorMessageTreeItem (const DiagnosticMessage& m)
            : message (m), itemHeight (25)
        {
            setOpenness (Openness::opennessClosed);
            uniqueID << message.message << ':' << message.range.toString();
        }

        ~ErrorMessageTreeItem() override
        {
            overlay.deleteAndZero();
        }

        String getRenamingName() const override          { return getDisplayName(); }
        String getDisplayName() const override           { return message.message; }
        void setName (const String&) override            {}
        bool isMissing() const override                  { return false; }
        Icon getIcon() const override                    { return Icon (message.isNote() ? getIcons().info
                                                                                         : getIcons().warning, getContentColour (true)); }
        bool canBeSelected() const override              { return true; }
        bool mightContainSubItems() override             { return getNumSubItems() != 0; }
        String getUniqueName() const override            { return uniqueID; }

        void paintContent (Graphics& g, const Rectangle<int>& area) override
        {
            jassert (area.getWidth() >= 0);

            AttributedString s (message.message);
            s.setFont (Font (12.0f));
            s.setColour (getContentColour (false));
            s.setJustification (Justification::centredLeft);

            text.createLayout (s, (float) area.getWidth());

            const auto newHeight = 2 + jmax (25, (int) text.getHeight());
            if (itemHeight != newHeight)
            {
                itemHeight = newHeight;
                treeHasChanged();
            }

            text.draw (g, area.toFloat());
        }

        Colour getContentColour (bool isIcon) const override
        {
            if (isIcon)
            {
                if (isSelected())
                    return getOwnerView()->findColour (defaultHighlightedTextColourId);

                if (message.isError())
                    return Colours::red;

                if (message.isWarning())
                    return Colours::yellow;

                return getOwnerView()->findColour (treeIconColourId);
            }

            return getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId
                                                            : defaultTextColourId);
        }

        void showPopupMenu() override
        {
            PopupMenu menu;
            menu.addItem (1, "Copy");
            launchPopupMenu (menu);
        }

        void handlePopupMenuResult (int resultCode) override
        {
            if (resultCode == 1)
                SystemClipboard::copyTextToClipboard (message.toString());
        }

        int getItemHeight() const override
        {
            return itemHeight;
        }

        SourceCodeEditor* getEditor()
        {
            if (ProjectContentComponent* pcc = getProjectContentComponent())
            {
                const File file (File::createFileWithoutCheckingPath (message.range.file));

                if (message.range.isValid() && file.exists() && pcc->showEditorForFile (file, false))
                {
                    if (SourceCodeEditor* ed = dynamic_cast<SourceCodeEditor*> (pcc->getEditorComponent()))
                    {
                        return ed;
                    }
                }
            }

            return nullptr;
        }

        void showDocument() override
        {
            if (SourceCodeEditor* ed = getEditor())
            {
                ed->grabKeyboardFocus();
                ed->highlight (message.range.range, false);

                if (auto cu = findCompileUnitParent())
                    cu->showOverlays();
            }
        }

        CompileUnitTreeItem* findCompileUnitParent()
        {
            for (TreeViewItem* p = getParentItem(); p != nullptr; p = p->getParentItem())
                if (auto cu = dynamic_cast<CompileUnitTreeItem*> (p))
                    return cu;

            return nullptr;
        }

        void showOverlays()
        {
            overlay.deleteAndZero();

            if (ProjectContentComponent* pcc = getProjectContentComponent())
            {
                if (SourceCodeEditor* ed = dynamic_cast<SourceCodeEditor*> (pcc->getEditorComponent()))
                {
                    auto start = CodeDocument::Position (ed->editor->getDocument(), message.range.range.getStart());
                    auto end   = CodeDocument::Position (ed->editor->getDocument(), message.range.range.getEnd());

                    if (auto* ce = dynamic_cast<LiveBuildCodeEditor*> (ed->editor.get()))
                        overlay = ce->addDiagnosticOverlay (start, end, message.type);
                }
            }

            for (int i = 0; i < getNumSubItems(); ++i)
                if (auto* e = dynamic_cast<ErrorMessageTreeItem*> (getSubItem (i)))
                    e->showOverlays();
        }

        DiagnosticMessage message;

    private:
        String uniqueID;
        TextLayout text;
        int itemHeight;
        Component::SafePointer<Component> overlay;
    };
};
