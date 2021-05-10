/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

/** Basic accessible interface for a TextEditor.

    @tags{Accessibility}
*/
class JUCE_API  TextEditorAccessibilityHandler  : public AccessibilityHandler
{
public:
    explicit TextEditorAccessibilityHandler (TextEditor& textEditorToWrap)
        : AccessibilityHandler (textEditorToWrap,
                                textEditorToWrap.isReadOnly() ? AccessibilityRole::staticText : AccessibilityRole::editableText,
                                {},
                                { textEditorToWrap.isReadOnly() ? nullptr : std::make_unique<TextEditorTextInterface> (textEditorToWrap) }),
          textEditor (textEditorToWrap)
    {
    }

    String getTitle() const override
    {
        return textEditor.isReadOnly() ? textEditor.getText() : textEditor.getTitle();
    }

private:
    class TextEditorTextInterface  : public AccessibilityTextInterface
    {
    public:
        explicit TextEditorTextInterface (TextEditor& editor)
            : textEditor (editor)
        {
        }

        bool isDisplayingProtectedText() const override      { return textEditor.getPasswordCharacter() != 0; }

        int getTotalNumCharacters() const override           { return textEditor.getText().length(); }
        Range<int> getSelection() const override             { return textEditor.getHighlightedRegion(); }
        void setSelection (Range<int> r) override            { textEditor.setHighlightedRegion (r); }

        String getText (Range<int> r) const override
        {
            if (isDisplayingProtectedText())
                return String::repeatedString (String::charToString (textEditor.getPasswordCharacter()),
                                               getTotalNumCharacters());

            return textEditor.getTextInRange (r);
        }

        void setText (const String& newText) override
        {
            textEditor.setText (newText);
        }

        int getTextInsertionOffset() const override          { return textEditor.getCaretPosition(); }

        RectangleList<int> getTextBounds (Range<int> textRange) const override
        {
            auto localRects = textEditor.getTextBounds (textRange);
            RectangleList<int> globalRects;

            std::for_each (localRects.begin(), localRects.end(),
                           [&] (const Rectangle<int>& r) { globalRects.add (textEditor.localAreaToGlobal (r)); });

            return globalRects;
        }

        int getOffsetAtPoint (Point<int> point) const override
        {
            auto localPoint = textEditor.getLocalPoint (nullptr, point);
            return textEditor.getTextIndexAt (localPoint.x, localPoint.y);
        }

    private:
        TextEditor& textEditor;
    };

    TextEditor& textEditor;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextEditorAccessibilityHandler)
};

} // namespace juce
