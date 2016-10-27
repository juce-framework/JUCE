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

    ~ErrorListComp()
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
        bool isMissing() override                        { return false; }
        Icon getIcon() const override                    { return Icon (getIcons().bug, getContrastingColour (0.8f)); }
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

        String getRenamingName() const override          { return getDisplayName(); }
        String getDisplayName() const override           { return File (compileUnit).exists() ? File (compileUnit).getFileName() : compileUnit; }
        void setName (const String&) override            {}
        bool isMissing() override                        { return false; }
        Icon getIcon() const override                    { return Icon (getIcons().bug, getContrastingColour (0.8f)); }
        bool canBeSelected() const override              { return true; }
        bool mightContainSubItems() override             { return true; }
        String getUniqueName() const override            { return String::toHexString (compileUnit.hashCode64()); }
        void addSubItems() override {}

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
                if (File (compileUnit).exists())
                    pcc->showEditorForFile (File (compileUnit), true);
        }

        String compileUnit;
    };

    //==============================================================================
    struct ErrorMessageTreeItem  : public JucerTreeViewBase
    {
        ErrorMessageTreeItem (const DiagnosticMessage& m)
            : message (m), itemHeight (14)
        {
            setOpenness (Openness::opennessClosed);
            uniqueID << message.message << ':' << message.range.toString();
        }

        ~ErrorMessageTreeItem()
        {
            overlay.deleteAndZero();
        }

        String getRenamingName() const override          { return getDisplayName(); }
        String getDisplayName() const override           { return message.message; }
        void setName (const String&) override            {}
        bool isMissing() override                        { return false; }
        Icon getIcon() const override                    { return Icon (message.isNote() ? getIcons().info
                                                                                         : getIcons().warning, getTextColour()); }
        bool canBeSelected() const override              { return true; }
        bool mightContainSubItems() override             { return getNumSubItems() != 0; }
        String getUniqueName() const override            { return uniqueID; }
        Component* createItemComponent() override        { return new ErrorItemComponent (*this); }

        struct ErrorItemComponent   : public TreeItemComponent
        {
            ErrorItemComponent (ErrorMessageTreeItem& e)  : TreeItemComponent (e) {}

            void resized() override
            {
                TreeItemComponent::resized();

                const int width = getWidth();
                const int iconWidth = 25; // TODO: this shouldn't be a magic number

                if (width > iconWidth)
                    static_cast<ErrorMessageTreeItem&> (item).updateTextLayout (getWidth() - 30 /* accounting for icon */);
            }

            void lookAndFeelChanged() override
            {
                resized();
            }
        };

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

        void paintIcon (Graphics& g, Rectangle<int> area) override
        {
            getIcon().draw (g, area.toFloat(), isIconCrossedOut());
        }

        void paintContent (Graphics& g, const Rectangle<int>& area) override
        {
            text.draw (g, area.toFloat());
        }

        int getItemHeight() const override
        {
            return itemHeight;
        }

        Colour getTextColour() const
        {
            Colour bkg (getOwnerView()->findColour (mainBackgroundColourId));

            return bkg.contrasting (message.isError() ? Colours::darkred
                                                      : message.isWarning() ? Colours::yellow.darker()
                                                                            : Colours::grey, 0.4f);
        }

        void updateTextLayout (int width)
        {
            jassert (width >= 0);

            AttributedString s (message.message);
            s.setFont (Font (12.0f));
            s.setColour (getTextColour());

            text.createLayout (s, (float) width);

            const int newHeight = 2 + jmax (14, (int) text.getHeight());
            if (itemHeight != newHeight)
            {
                itemHeight = newHeight;
                treeHasChanged();
            }
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
