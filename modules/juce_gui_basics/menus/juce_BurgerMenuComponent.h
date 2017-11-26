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

namespace juce
{

//==============================================================================
/**
    A component which lists all menu items and groups them into categories
    by their respective parent menus. This kind of component is often used
    for so-called "burger" menus in mobile apps.

    @see MenuBarModel
*/
//==============================================================================
class BurgerMenuComponent     : public Component,
                                private ListBoxModel,
                                private MenuBarModel::Listener
{
public:
    //==============================================================================
    /** Creates a burger menu component.

        @param model    the model object to use to control this burger menu. You can
                        set the parameter or pass nullptr into this if you like,
                        and set the model later using the setModel() method.

        @see setModel
     */
    BurgerMenuComponent (MenuBarModel* model = nullptr);

    /** Destructor. */
    ~BurgerMenuComponent();

    //==============================================================================
    /** Changes the model object to use to control the burger menu.

        This can be a nullptr, in which case the bar will be empty. This object will not be
        owned by the BurgerMenuComponent so it is up to you to manage its lifetime.
        Don't delete the object that is passed-in while it's still being used by this MenuBar.
        Any submenus in your MenuBarModel will be recursively flattened and added to the
        top-level burger menu section.
     */
    void setModel (MenuBarModel* newModel);

    /** Returns the current burger menu model being used. */
    MenuBarModel* getModel() const noexcept;

private:
    //==============================================================================
    struct Row
    {
        bool isMenuHeader;
        int topLevelMenuIndex;
        PopupMenu::Item item;
    };

    void refresh();
    void paint (Graphics&) override;
    int getNumRows() override;
    void paintListBoxItem (int, Graphics&, int, int, bool) override;
    void listBoxItemClicked (int, const MouseEvent&) override;
    Component* refreshComponentForRow (int, bool, Component*) override;
    void resized() override;
    void menuBarItemsChanged (MenuBarModel*) override;
    void menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo&) override;
    void mouseUp (const MouseEvent&) override;
    void handleCommandMessage (int) override;
    void addMenuBarItemsForMenu (PopupMenu&, int);
    static bool hasSubMenu (const PopupMenu::Item&);

    //==============================================================================
    MenuBarModel* model = nullptr;
    ListBox listBox    {"BurgerMenuListBox", this};
    Array<Row> rows;

    int lastRowClicked = -1, inputSourceIndexOfLastClick = -1, topLevelIndexClicked = -1;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BurgerMenuComponent)
};

} // namespace juce
