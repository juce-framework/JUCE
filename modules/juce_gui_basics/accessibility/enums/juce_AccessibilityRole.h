/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** The list of available roles for an AccessibilityHandler object.

    When creating a custom AccessibilityHandler you should select the role that
    best describes the UI element being represented.

    @tags{Accessibility}
*/
enum class AccessibilityRole
{
    button,
    toggleButton,
    radioButton,
    comboBox,
    image,
    slider,
    label,
    staticText,
    editableText,
    menuItem,
    menuBar,
    popupMenu,
    table,
    tableHeader,
    column,
    row,
    cell,
    hyperlink,
    list,
    listItem,
    tree,
    treeItem,
    progressBar,
    group,
    dialogWindow,
    window,
    scrollBar,
    tooltip,
    splashScreen,
    ignored,
    unspecified
};

}
