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

#ifndef __JUCER_FONTPROPERTYCOMPONENT_H_930624E__
#define __JUCER_FONTPROPERTYCOMPONENT_H_930624E__


//==============================================================================
class FontNameValueSource   : public Value::ValueSource,
                              public Value::Listener
{
public:
    FontNameValueSource (const Value& source)
       : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    ~FontNameValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }

    const var getValue() const
    {
        return Font::fromString (sourceValue.toString()).getTypefaceName();
    }

    void setValue (const var& newValue)
    {
        Font font (Font::fromString (sourceValue.toString()));
        font.setTypefaceName (newValue.toString());
        sourceValue = font.toString();
    }

    static ChoicePropertyComponent* createProperty (const String& title, const Value& value)
    {
        StringArray fontNames;
        fontNames.add (Font::getDefaultSansSerifFontName());
        fontNames.add (Font::getDefaultSerifFontName());
        fontNames.add (Font::getDefaultMonospacedFontName());
        fontNames.add (String::empty);
        fontNames.addArray (StoredSettings::getInstance()->getFontNames());

        Array<var> values;
        for (int i = 0; i < fontNames.size(); ++i)
            values.add (fontNames[i]);

        return new ChoicePropertyComponent (Value (new FontNameValueSource (value)), title, fontNames, values);
    }

private:
    Value sourceValue;
};

//==============================================================================
class FontSizeValueSource   : public Value::ValueSource,
                              public Value::Listener
{
public:
    FontSizeValueSource (const Value& source)
       : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    ~FontSizeValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }

    const var getValue() const
    {
        return Font::fromString (sourceValue.toString()).getHeight();
    }

    void setValue (const var& newValue)
    {
        Font font (Font::fromString (sourceValue.toString()));
        font.setHeight (newValue);
        sourceValue = font.toString();
    }

    static PropertyComponent* createProperty (const String& title, const Value& value)
    {
        return new SliderPropertyComponent (Value (new FontSizeValueSource (value)), title, 1.0, 150.0, 0.1, 0.5);
    }

private:
    Value sourceValue;
};

//==============================================================================
class FontStyleValueSource   : public Value::ValueSource,
                               public Value::Listener
{
public:
    FontStyleValueSource (const Value& source)
       : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    ~FontStyleValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }

    const var getValue() const
    {
        const Font f (Font::fromString (sourceValue.toString()));

        if (f.isBold() && f.isItalic()) return getStyles() [3];
        if (f.isBold())                 return getStyles() [1];
        if (f.isItalic())               return getStyles() [2];

        return getStyles() [0];
    }

    void setValue (const var& newValue)
    {
        Font font (Font::fromString (sourceValue.toString()));
        font.setBold (newValue.toString().containsIgnoreCase ("Bold"));
        font.setItalic (newValue.toString().containsIgnoreCase ("Italic"));
        sourceValue = font.toString();
    }

    static PropertyComponent* createProperty (const String& title, const Value& value)
    {
        return new ChoicePropertyComponent (Value (new FontStyleValueSource (value)), title, StringArray (getStyles()), Array<var> (getStyles()));
    }

    static const char* const* getStyles()
    {
        static const char* const fontStyles[] = { "Normal", "Bold", "Italic", "Bold + Italic", 0 };
        return fontStyles;
    }

private:
    Value sourceValue;
};


#endif  // __JUCER_FONTPROPERTYCOMPONENT_H_930624E__
