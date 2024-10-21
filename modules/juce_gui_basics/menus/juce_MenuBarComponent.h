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
    A menu bar component.

    @see MenuBarModel

    @tags{GUI}
*/
class JUCE_API  MenuBarComponent  : public Component,
                                    private MenuBarModel::Listener,
                                    private Timer
{
public:
    //==============================================================================
    /** Creates a menu bar.

        @param model    the model object to use to control this bar. You can
                        pass omit the parameter or pass nullptr into this if you like,
                        and set the model later using the setModel() method.
    */
    MenuBarComponent (MenuBarModel* model = nullptr);

    /** Destructor. */
    ~MenuBarComponent() override;

    //==============================================================================
    /** Changes the model object to use to control the bar.

        This can be a null pointer, in which case the bar will be empty. Don't delete the object
        that is passed-in while it's still being used by this MenuBar.
    */
    void setModel (MenuBarModel* newModel);

    /** Returns the current menu bar model being used. */
    MenuBarModel* getModel() const noexcept;

    //==============================================================================
    /** Pops up one of the menu items.

        This lets you manually open one of the menus - it could be triggered by a
        key shortcut, for example.
    */
    void showMenu (int menuIndex);

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void mouseEnter (const MouseEvent&) override;
    /** @internal */
    void mouseExit (const MouseEvent&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    void mouseMove (const MouseEvent&) override;
    /** @internal */
    void handleCommandMessage (int commandId) override;
    /** @internal */
    bool keyPressed (const KeyPress&) override;
    /** @internal */
    void menuBarItemsChanged (MenuBarModel*) override;
    /** @internal */
    void menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo&) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    class AccessibleItemComponent;

    //==============================================================================
    void timerCallback() override;

    int getItemAt (Point<int>);
    void setItemUnderMouse (int);
    void setOpenItem (int);
    void updateItemUnderMouse (Point<int>);
    void repaintMenuItem (int);
    void menuDismissed (int, int);

    void updateItemComponents (const StringArray&);
    int indexOfItemComponent (AccessibleItemComponent*) const;

    //==============================================================================
    MenuBarModel* model = nullptr;
    std::vector<std::unique_ptr<AccessibleItemComponent>> itemComponents;

    Point<int> lastMousePos;
    int itemUnderMouse = -1, currentPopupIndex = -1, topLevelIndexDismissed = 0;
    int numActiveMenus = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuBarComponent)
};

} // namespace juce
