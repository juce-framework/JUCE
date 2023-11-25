/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

MessageBoxOptions MessageBoxOptions::makeOptionsOk (MessageBoxIconType iconType,
                                                    const String& title,
                                                    const String& message,
                                                    const String& buttonText,
                                                    Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (buttonText.isEmpty() ? TRANS ("OK") : buttonText)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType iconType,
                                                          const String& title,
                                                          const String& message,
                                                          const String& button1Text,
                                                          const String& button2Text,
                                                          Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("OK") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("Cancel") : button2Text)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsYesNo (MessageBoxIconType iconType,
                                                       const String& title,
                                                       const String& message,
                                                       const String& button1Text,
                                                       const String& button2Text,
                                                       Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("Yes") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("No") : button2Text)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsYesNoCancel (MessageBoxIconType iconType,
                                                             const String& title,
                                                             const String& message,
                                                             const String& button1Text,
                                                             const String& button2Text,
                                                             const String& button3Text,
                                                             Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("Yes") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("No") : button2Text)
        .withButton (button3Text.isEmpty() ? TRANS ("Cancel") : button3Text)
        .withAssociatedComponent (associatedComponent);
}

} // namespace juce
