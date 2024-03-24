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

namespace juce
{

//==============================================================================
/**
    A component which lists all menu items and groups them into categories
    by their respective parent menus. This kind of component is often used
    for so-called "burger" menus in mobile apps.

    @see MenuBarModel

    @tags{GUI}
*/
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
    ~BurgerMenuComponent() override;

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

    /** @internal */
    void lookAndFeelChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

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
