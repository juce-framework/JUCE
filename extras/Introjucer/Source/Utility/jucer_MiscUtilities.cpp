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

#include "../jucer_Headers.h"


//==============================================================================
String createAlphaNumericUID()
{
    String uid;
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Random r;

    uid << chars [r.nextInt (52)]; // make sure the first character is always a letter

    for (int i = 5; --i >= 0;)
    {
        r.setSeedRandomly();
        uid << chars [r.nextInt (62)];
    }

    return uid;
}

String hexString8Digits (int value)
{
    return String::toHexString (value).paddedLeft ('0', 8);
}

String createGUID (const String& seed)
{
    const String hex (MD5 ((seed + "_guidsalt").toUTF8()).toHexString().toUpperCase());

    return "{" + hex.substring (0, 8)
         + "-" + hex.substring (8, 12)
         + "-" + hex.substring (12, 16)
         + "-" + hex.substring (16, 20)
         + "-" + hex.substring (20, 32)
         + "}";
}

String escapeSpaces (const String& s)
{
    return s.replace (" ", "\\ ");
}

String addQuotesIfContainsSpaces (const String& text)
{
    return (text.containsChar (' ') && ! text.isQuotedString()) ? text.quoted() : text;
}

void setValueIfVoid (Value value, const var& defaultValue)
{
    if (value.getValue().isVoid())
        value = defaultValue;
}

//==============================================================================
StringPairArray parsePreprocessorDefs (const String& text)
{
    StringPairArray result;
    String::CharPointerType s (text.getCharPointer());

    while (! s.isEmpty())
    {
        String token, value;
        s = s.findEndOfWhitespace();

        while ((! s.isEmpty()) && *s != '=' && ! s.isWhitespace())
            token << s.getAndAdvance();

        s = s.findEndOfWhitespace();

        if (*s == '=')
        {
            ++s;

            s = s.findEndOfWhitespace();

            while ((! s.isEmpty()) && ! s.isWhitespace())
            {
                if (*s == ',')
                {
                    ++s;
                    break;
                }

                if (*s == '\\' && (s[1] == ' ' || s[1] == ','))
                    ++s;

                value << s.getAndAdvance();
            }
        }

        if (token.isNotEmpty())
            result.set (token, value);
    }

    return result;
}

StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs)
{
    for (int i = 0; i < overridingDefs.size(); ++i)
        inheritedDefs.set (overridingDefs.getAllKeys()[i], overridingDefs.getAllValues()[i]);

    return inheritedDefs;
}

String createGCCPreprocessorFlags (const StringPairArray& defs)
{
    String s;

    for (int i = 0; i < defs.size(); ++i)
    {
        String def (defs.getAllKeys()[i]);
        const String value (defs.getAllValues()[i]);
        if (value.isNotEmpty())
            def << "=" << value;

        if (! def.endsWithChar ('"'))
            def = def.quoted();

        s += " -D " + def;
    }

    return s;
}

String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString)
{
    for (int i = 0; i < definitions.size(); ++i)
    {
        const String key (definitions.getAllKeys()[i]);
        const String value (definitions.getAllValues()[i]);

        sourceString = sourceString.replace ("${" + key + "}", value);
    }

    return sourceString;
}

StringArray getSearchPathsFromString (const String& searchPath)
{
    StringArray s;
    s.addTokens (searchPath, ";\r\n", String::empty);
    s.trim();
    s.removeEmptyStrings();
    s.removeDuplicates (false);
    return s;
}

//==============================================================================
void autoScrollForMouseEvent (const MouseEvent& e, bool scrollX, bool scrollY)
{
    Viewport* const viewport = e.eventComponent->findParentComponentOfClass<Viewport>();

    if (viewport != nullptr)
    {
        const MouseEvent e2 (e.getEventRelativeTo (viewport));
        viewport->autoScroll (scrollX ? e2.x : 20, scrollY ? e2.y : 20, 8, 16);
    }
}

void drawComponentPlaceholder (Graphics& g, int w, int h, const String& text)
{
    g.fillAll (Colours::white.withAlpha (0.4f));
    g.setColour (Colours::grey);
    g.drawRect (0, 0, w, h);

    g.drawLine (0.5f, 0.5f, w - 0.5f, h - 0.5f);
    g.drawLine (0.5f, h - 0.5f, w - 0.5f, 0.5f);

    g.setColour (Colours::black);
    g.setFont (11.0f);
    g.drawFittedText (text, 2, 2, w - 4, h - 4, Justification::centredTop, 2);
}

static Image createTexturisedBackgroundTile()
{
    const Colour bkg (LookAndFeel::getDefaultLookAndFeel().findColour (mainBackgroundColourId));
    const int64 hash = bkg.getARGB() + 0x3474572a;

    Image tile (ImageCache::getFromHashCode (hash));

    if (tile.isNull())
    {
        const Image original (ImageCache::getFromMemory (BinaryData::brushed_aluminium_png,
                                                         BinaryData::brushed_aluminium_pngSize));

        tile = Image (Image::RGB, original.getWidth(), original.getHeight(), false);

        for (int y = 0; y < tile.getHeight(); ++y)
        {
            for (int x = 0; x < tile.getWidth(); ++x)
            {
                const float b = original.getPixelAt (x, y).getBrightness();
                tile.setPixelAt (x, y, bkg.withMultipliedBrightness (b + 0.4f));
            }
        }

        ImageCache::addImageToCache (tile, hash);
    }

    return tile;
}

void drawTexturedBackground (Graphics& g)
{
    g.setTiledImageFill (createTexturisedBackgroundTile(), 0, 0, 1.0f);
    g.fillAll();
}

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex)
{
    startIndex = jmax (0, startIndex);

    while (startIndex < lines.size())
    {
        if (lines[startIndex].trimStart().startsWithIgnoreCase (text))
            return startIndex;

        ++startIndex;
    }

    return -1;
}


//==============================================================================
RolloverHelpComp::RolloverHelpComp()
    : lastComp (nullptr)
{
    setInterceptsMouseClicks (false, false);
    startTimer (150);
}

void RolloverHelpComp::paint (Graphics& g)
{
    AttributedString s;
    s.setJustification (Justification::centredLeft);
    s.append (lastTip, Font (14.0f), findColour (mainBackgroundColourId).contrasting (0.7f));

    TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, getWidth() - 10.0f);
    if (tl.getNumLines() > 3)
        tl.createLayout (s, getWidth() - 10.0f);

    tl.draw (g, getLocalBounds().toFloat());
}

void RolloverHelpComp::timerCallback()
{
    Component* newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    if (newComp != nullptr
         && (newComp->getTopLevelComponent() != getTopLevelComponent()
              || newComp->isCurrentlyBlockedByAnotherModalComponent()))
        newComp = nullptr;

    if (newComp != lastComp)
    {
        lastComp = newComp;

        String newTip (findTip (newComp));

        if (newTip != lastTip)
        {
            lastTip = newTip;
            repaint();
        }
    }
}

String RolloverHelpComp::findTip (Component* c)
{
    while (c != nullptr)
    {
        TooltipClient* const tc = dynamic_cast <TooltipClient*> (c);
        if (tc != nullptr)
        {
            const String tip (tc->getTooltip());

            if (tip.isNotEmpty())
                return tip;
        }

        c = c->getParentComponent();
    }

    return String::empty;
}

//==============================================================================
FloatingLabelComponent::FloatingLabelComponent()
    : font (10.0f)
{
    setInterceptsMouseClicks (false, false);
}

void FloatingLabelComponent::remove()
{
    if (getParentComponent() != nullptr)
        getParentComponent()->removeChildComponent (this);
}

void FloatingLabelComponent::update (Component* parent, const String& text, const Colour& textColour, int x, int y, bool toRight, bool below)
{
    colour = textColour;

    Rectangle<int> r;

    if (text != getName())
    {
        setName (text);
        glyphs.clear();
        glyphs.addJustifiedText (font, text, 0, 0, 200.0f, Justification::left);
        glyphs.justifyGlyphs (0, std::numeric_limits<int>::max(), 0, 0, 1000, 1000, Justification::topLeft);

        r = glyphs.getBoundingBox (0, std::numeric_limits<int>::max(), false)
                  .getSmallestIntegerContainer().expanded (1, 1);
    }
    else
    {
        r = getLocalBounds();
    }

    r.setPosition (x + (toRight ? 3 : -(r.getWidth() + 3)), y + (below ? 2 : -(r.getHeight() + 2)));
    setBounds (r);
    parent->addAndMakeVisible (this);
}

void FloatingLabelComponent::paint (Graphics& g)
{
    g.setFont (font);
    g.setColour (Colours::white.withAlpha (0.5f));
    g.fillRoundedRectangle (0, 0, (float) getWidth(), (float) getHeight(), 3);

    g.setColour (colour);
    glyphs.draw (g, AffineTransform::translation (1.0f, 1.0f));
}


//==============================================================================
class UTF8Component  : public Component,
                       private TextEditorListener
{
public:
    UTF8Component()
        : desc (String::empty,
                "Type any string into the box, and it'll be shown below as a portable UTF-8 literal, ready to cut-and-paste into your source-code...")
    {
        setSize (400, 300);

        desc.setBounds ("8, 8, parent.width - 8, 55");
        desc.setJustificationType (Justification::centred);
        addAndMakeVisible (&desc);

        userText.setMultiLine (true, true);
        userText.setBounds ("8, 60, parent.width - 8, parent.height / 2 - 4");
        addAndMakeVisible (&userText);
        userText.addListener (this);

        resultText.setMultiLine (true, true);
        resultText.setReadOnly (true);
        resultText.setBounds ("8, parent.height / 2 + 4, parent.width - 8, parent.height - 8");
        addAndMakeVisible (&resultText);

        userText.setText (getLastText());
    }

    void textEditorTextChanged (TextEditor&)
    {
        update();
    }

    void textEditorEscapeKeyPressed (TextEditor&)
    {
        getTopLevelComponent()->exitModalState (0);
    }

    void update()
    {
        getLastText() = userText.getText();
        resultText.setText (CodeHelpers::stringLiteral (getLastText()), false);
    }

private:
    Label desc;
    TextEditor userText, resultText;

    String& getLastText()
    {
        static String t;
        return t;
    }
};

void showUTF8ToolWindow()
{
    UTF8Component comp;
    DialogWindow::showModalDialog ("UTF-8 String Literal Converter", &comp,
                                   nullptr, Colours::white, true, true);
}

bool cancelAnyModalComponents()
{
    const int numModal = ModalComponentManager::getInstance()->getNumModalComponents();

    for (int i = numModal; --i >= 0;)
        if (ModalComponentManager::getInstance()->getModalComponent(i) != nullptr)
            ModalComponentManager::getInstance()->getModalComponent(i)->exitModalState (0);

    return numModal > 0;
}

//==============================================================================
class AsyncCommandRetrier  : public Timer
{
public:
    AsyncCommandRetrier (const ApplicationCommandTarget::InvocationInfo& info_)
        : info (info_)
    {
        info.originatingComponent = nullptr;
        startTimer (500);
    }

    void timerCallback()
    {
        stopTimer();
        commandManager->invoke (info, true);
        delete this;
    }

    ApplicationCommandTarget::InvocationInfo info;

    JUCE_DECLARE_NON_COPYABLE (AsyncCommandRetrier);
};

bool reinvokeCommandAfterCancellingModalComps (const ApplicationCommandTarget::InvocationInfo& info)
{
    if (cancelAnyModalComponents())
    {
        new AsyncCommandRetrier (info);
        return true;
    }

    return false;
}
