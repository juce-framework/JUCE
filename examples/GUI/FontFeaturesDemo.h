/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             FontFeaturesDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays different font features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        FontFeaturesDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

static const std::map<FontFeatureTag, std::pair<const char*, const char*>> featureDescriptionMap
{
    { "abvs", { "Above-base Substitutions",             "\xe0\xa4\x95\xe0\xa4\xbf" } },
    { "abvf", { "Above-base Forms",                     "\xe0\xa4\x95\xe0\xa4\x82" } },
    { "akhn", { "Akhand Ligatures",                     "\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xb7" } },
    { "blwf", { "Below-base Forms",                     "\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xa4" } },
    { "blws", { "Below-base Substitutions",             "\xe0\xa4\x9f\xe0\xa5\x81" } },
    { "abvm", { "Above-Base Mark Positioning",          "\xe0\xa4\x95\xe0\xa4\x82" } },
    { "blwm", { "Below-Base Mark Positioning",          "\xe0\xa4\x95\xe0\xa5\x83\xe0\xa4\xb7\xe0\xa5\x8d\xe0\xa4\xa3" } },
    { "cjct", { "Conjunct Forms",                       "\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xa4" } },
    { "nukt", { "Nukta Forms",                          "\xe0\xa4\x95\xe0\xa4\xbc" } },
    { "pres", { "Pre-base Substitutions",               "\xe0\xa4\xb0\xe0\xa5\x8d\xe0\xa4\x95" } },
    { "psts", { "Post-base Substitutions",              "\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xaf" } },
    { "rkrf", { "Rakar Forms",                          "\xe0\xa4\x9f\xe0\xa5\x8d\xe0\xa4\xb0" } },
    { "rphf", { "Reph Forms",                           "\xe0\xa4\xb0\xe0\xa5\x8d\xe0\xa4\x95" } },
    { "vatu", { "Vattu Variants",                       "\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xb0" } },
    { "mark", { "Mark Positioning",                     "\x72\xc3\xa9\x73\x75\x6d\xc3\xa9" } },
    { "mkmk", { "Mark to Mark Positioning",             "\xe1\xba\xa5" } },
    { "locl", { "Localized Forms",                      "This is fancy" } },
    { "curs", { "Cursive Positioning",                  "\xd8\xb9\xd8\xb1\xd8\xa8\xd9\x8a" } },
    { "dist", { "Distances (for complex scripts)",      "\xe0\xb9\x80\xe0\xb8\x9b\xe0\xb9\x87\xe0\xb8\x99\xe0\xb9\x84\xe0\xb8\x97\xe0\xb8\xa2" } },
    { "pref", { "Pre-base Forms",                       "\xe0\xa4\xb0\xe0\xa5\x8d\xe0\xa4\x95" } },
    { "pstf", { "Post-base Forms",                      "\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xaf" } },
    { "half", { "Half Forms",                           "\xe0\xa4\x95\xe0\xa5\x8d" } },
    { "haln", { "Halant Forms",                         "\xe0\xa4\x95\xe0\xa5\x8d" } },
    { "fina", { "Terminal Forms",                       "\xd8\xb9" } },
    { "init", { "Initial Forms",                        "\xd8\xb9" } },
    { "isol", { "Isolated Forms",                       "\xd8\xb9" } },
    { "medi", { "Medical Forms",                        "\xd8\xb9" } },
    { "rclt", { "Required Contextual Alternates",       "\x66\x69" } },
    { "rvrn", { "Required Variation Alternates",        "Aaa" } },
    { "liga", { "Standard Ligatures",                   "official flight" } },
    { "dlig", { "Discretionary Ligatures",              "ct sp st" } },
    { "calt", { "Contextual Alternates",                "The Last Bloom" } },
    { "clig", { "Contextual Ligatures",                 "swift" } },
    { "cswh", { "Contextual Swash",                     "Feeling Good" } },
    { "hlig", { "Historical Ligatures",                 "historical finger" } },
    { "rlig", { "Required Ligatures",                   "\xd9\x84\xd8\xa7" } },
    { "ccmp", { "Glyph Composition/Decomposition",      "\xc3\xb1" } },
    { "kern", { "Kerning",                              "AWAY" } },
    { "fwid", { "Full Width",                           "AMA" } },
    { "hwid", { "Half Width",                           "AMA" } },
    { "pwid", { "Proportional Width",                   "AMA" } },
    { "twid", { "Third Width",                          "AMA" } },
    { "qwid", { "Quarter Widths",                       "AMA" } },
    { "smcp", { "Small Capitals",                       "Small" } },
    { "c2sc", { "Caps to Small Caps",                   "CAPS" } },
    { "pcap", { "Petite Capitals",                      "Petite" } },
    { "c2pc", { "Caps to Petite Caps",                  "CAPS" } },
    { "unic", { "Unicase",                              "Mixed case" } },
    { "case", { "Case-Sensitive Forms",                 "\x7b\xc2\xbf\x48\x4f\x4c\x41\x21\x7d" } },
    { "cpsp", { "Capital Spacing",                      "ALL CAPS" } },
    { "salt", { "Stylistic Alternates",                 "Hidden Garden" } },
    { "aalt", { "Access All Alternates",                "a" } },
    { "swsh", { "Swash",                                "The Juiciest JUCE" } },
    { "titl", { "Titling Alternates",                   "Headline" } },
    { "hist", { "Historical Forms",                     "looong s" } },
    { "rand", { "Randomize Alternates",                 "Random!" } },
    { "frac", { "Fractions",                            "1/2" } },
    { "afrc", { "Alternative Fractions",                "1/2" } },
    { "numr", { "Numerators",                           "32" } },
    { "dnom", { "Denominators",                         "45" } },
    { "sups", { "Superscript",                          "x2" } },
    { "subs", { "Subscript",                            "H2O" } },
    { "sinf", { "Scientific Inferiors",                 "H2O SOx YCbCr NO2" } },
    { "mgrk", { "Mathematical Greek",                   "\xce\x91\xce\xb1\x20\xce\x95\xce\xb5\x20\xce\x94\xce\xb4" } },
    { "ordn", { "Ordinals",                             "1st, 2nd, 3rd" } },
    { "zero", { "Slashed Zero",                         "0x0001" } },
    { "pnum", { "Proportional Figures",                 "0123456789" } },
    { "tnum", { "Tabular Figures",                      "0123456789" } },
    { "lnum", { "Lining Figures",                       "0123456789" } },
    { "onum", { "Oldstyle Figures",                     "0123456789" } },
    { "jp78", { "Japanese 1978 Forms",                  "\xe8\xbe\xbb" } },
    { "jp83", { "Japanese 1983 Forms",                  "\xe5\x86\x86" } },
    { "jp90", { "Japanese 1990 Forms",                  "\xe8\x91\x89" } },
    { "jp04", { "Japanese 2004 Forms",                  "\xe9\xaa\xa8" } },
    { "trad", { "Traditional Forms",                    "\xe5\x8f\xb0" } },
    { "vert", { "Vertical Writing",                     "A" } },
    { "vrt2", { "Vertical Alternates and Rotation",     "\xe2\x80\x94" } },
    { "size", { "Optical Size",                         "Text at 12pts" } },
    { "ornm", { "Ornaments",                            "zwzwzwzwzwzy" } },
    { "nalt", { "Alternate Annotation Forms",           "\xe3\x81\x82" } },
    { "expt", { "Export Forms",                         "apple" } },
    { "halt", { "Halant Forms",                         "\xe0\xa4\x95\xe0\xa5\x8d" } },
    { "hkna", { "Horizontal Kana Alternates",           "\xe3\x81\x8b" } },
    { "hojo", { "Hojo Kanji Forms",                     "\xe4\xbe\xae" } },
    { "ital", { "Italics",                              "Italics" } },
    { "nlck", { "NLC Kanji Forms",                      "\xe5\x9c\x8b" } },
    { "palt", { "Proportional Alternate Widths",        "\xe5\x9b\xbd" } },
    { "ruby", { "Ruby Notation Forms",                  "\xe6\xbc\xa2" } },
    { "vkna", { "Vertical Kana Alternates",             "\xe3\x81\x8b" } },
    { "vkrn", { "Vertical Kerning",                     "AV" } },
    { "vpal", { "Vertical Alternates and Positioning",  "\xe3\x83\xbb" } },
    { "vhal", { "Vertical Alternates for Hangul",       "\xed\x95\x9c" } },
    { "pkna", { "Proportional Kana",                    "\xe3\x81\x8b" } },
    { "requ", { "Required Ligatures",                   "\x66\x69" } },
    { "smpl", { "Simplified Forms",                     "\xe8\xaf\xb4" } },
    { "reqd", { "Required Contextual Alternates",       "\x66\x69" } },
    { "dpng", { "Diphthongs",                           "\xc3\xa6" } },
    { "hope", { "Historical OpenType Processing",       "\xc5\xbf" } },
    { "cpct", { "Centered CJK Punctuation",             "\xe3\x80\x82" } },
    { "rtla", { "Right-to-Left Alternates",             "\xd9\xa1" } },
    { "lfbd", { "Left Bounds",                          "Left" } },
    { "rtbd", { "Right Bounds",                         "Right" } },
    { "dtls", { "Dotless Forms",                        "\xc4\xb1" } },
    { "flac", { "Flattened accent components",          "\xc3\xa9" } }
};


class FontsListModel : public ListBoxModel
{
public:
    FontsListModel()
    {
        Font::findFonts (fonts);

        fonts.removeIf ([] (const Font& f)
        {
            return f.getTypefacePtr()->getSupportedFeatures().empty();
        });
    }

    std::function<void()> onFontSelected;

    int getNumRows() override
    {
        return fonts.size();
    }

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width,
                           int height,
                           bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);

        const Font options { FontOptions { getFaceForRow (rowNumber) } };

        AttributedString s;
        s.setWordWrap (AttributedString::none);
        s.setJustification (Justification::centredLeft);
        s.append (getNameForRow (rowNumber),
                  options.withPointHeight ((float) height * 0.7f),
                  Colours::black);

        s.append ("   " + getNameForRow (rowNumber),
                  FontOptions{}.withPointHeight ((float) height * 0.5f).withStyle ("Italic"),
                  Colours::grey);

        s.draw (g, Rectangle (width, height).expanded (-4, 50).toFloat());
    }

    void selectedRowsChanged (int) override
    {
        NullCheckedInvocation::invoke (onFontSelected);
    }

    Typeface::Ptr getFaceForRow (int rowNumber) const
    {
        return fonts.getReference (rowNumber).getTypefacePtr();
    }

    String getNameForRow (int rowNumber) override
    {
        return fonts.getReference (rowNumber).getTypefaceName();
    }

private:
    Array<Font> fonts;
};

class FeatureListModel : public ListBoxModel
{
    struct Feature
    {
        FontFeatureTag tag;
        String description;
        String exampleText;
    };

public:
    FeatureListModel() = default;

    int getNumRows() override
    {
        return (int) features.size();
    }

    void setFont (Typeface::Ptr face)
    {
        if (currentFace == face)
            return;

        features.clear();
        currentFace = face;

        if (currentFace == nullptr)
            return;

        for (auto feature : currentFace->getSupportedFeatures())
        {
            String description;
            String exampleText;

            const auto iter = featureDescriptionMap.find (feature);

            if (iter == featureDescriptionMap.end())
            {
                const auto string = feature.toString();

                // A malformed feature tag can result in a string with less than 4 characters.
                if (string.length() != 4)
                    continue;

                const auto isIndexed = std::isalnum ((int) string[2])
                                    && std::isalnum ((int) string[3]);
                const auto isStylisticSet = string.startsWith ("ss")
                                         && isIndexed;
                const auto isCharacterVariant = string.startsWith ("cv")
                                             && isIndexed;

                if (isStylisticSet)
                {
                    description << "Stylistic Set " << string.substring (2)
                                                             .getIntValue();
                    exampleText << "Some Example Text";
                }
                else if (isCharacterVariant)
                {
                    description << "Character Variant " << string.substring (2)
                                                                 .getIntValue();
                    exampleText << "aBcDeF123";
                }
                else
                {
                    description << "Unknown Feature";
                }
            }
            else
            {
                description = String::fromUTF8 (iter->second.first);
                exampleText = String::fromUTF8 (iter->second.second);
            }

            features.push_back ({ feature, description, exampleText });
        }
    }

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width,
                           int height,
                           bool /*rowIsSelected*/) override
    {
        auto feature = features[(size_t) rowNumber];
        const Font baseLineFont { FontOptions { currentFace }.withFeatureDisabled (feature.tag) };
        const Font exampleFont { FontOptions { currentFace }.withFeatureEnabled (feature.tag) };

        auto bounds = Rectangle { width, height }.reduced (10, 3).toFloat();

        Path boundsPath;
        boundsPath.addRoundedRectangle (bounds, 4);

        g.reduceClipRegion (boundsPath);
        g.fillAll (Colours::white);

        bounds.reduce (7, 2);

        auto getGlyphArrangementBoundingBox = [] (const GlyphArrangement& ga)
        {
            return ga.getBoundingBox (0, ga.getNumGlyphs(), true);
        };

        const FontStringPair description[] =
        {
            FontStringPair { FontOptions{}.withPointHeight (15).withStyle ("bold"),
                             feature.tag.toString() },
            FontStringPair { FontOptions{}.withPointHeight (15).withStyle ("italic"),
                             " - " + feature.description }
        };

        const FontStringPair example[] =
        {
            FontStringPair { baseLineFont.withPointHeight (16),
                             feature.exampleText },
            FontStringPair { baseLineFont.withPointHeight (16),
                             " " + String::fromUTF8 ("\xe2\x86\x92") },
            FontStringPair { exampleFont.withPointHeight (16),
                             feature.exampleText }
        };

        const auto descriptionWidth = getGlyphArrangementBoundingBox (buildMultiFontText (bounds,
                                                                                          Justification::topLeft,
                                                                                          description)).getWidth();
        const auto exampleWidth = getGlyphArrangementBoundingBox (buildMultiFontText (bounds,
                                                                                      Justification::topLeft,
                                                                                      example)).getWidth();
        const auto exampleBounds = bounds.removeFromRight (exampleWidth);
        const auto descriptionBounds = bounds.removeFromLeft (descriptionWidth);

        auto descriptionGa = buildMultiFontText (descriptionBounds,
                                                 Justification::centredLeft,
                                                 description);

        g.setGradientFill (ColourGradient (Colours::black,
                                           exampleBounds.getX() - 30.0f,
                                           0,
                                           Colours::transparentBlack,
                                           exampleBounds.getX() - 10.0f,
                                           0,
                                           false));

        descriptionGa.draw (g);

        auto exampleGa = buildMultiFontText (exampleBounds,
                                             Justification::centredLeft,
                                             example);
        exampleGa.justifyGlyphs (0,
                                 exampleGa.getNumGlyphs(),
                                 exampleBounds.getX(),
                                 exampleBounds.getY(),
                                 exampleBounds.getWidth(),
                                 exampleBounds.getHeight(),
                                 Justification::centredRight);

        g.setColour (Colours::black);
        exampleGa.draw (g);

        const FontStringPair strings[] =
        {
            FontStringPair { baseLineFont.withPointHeight (16), feature.exampleText },
            FontStringPair { exampleFont.withPointHeight (16), feature.exampleText }
        };

        const auto pre = buildMultiFontText (Rectangle<float> { 1000, 50 },
                                             Justification::centredLeft,
                                             Span { strings, 1 });

        const auto post = buildMultiFontText (Rectangle<float> { 1000, 50 },
                                              Justification::centredLeft,
                                              Span { strings + 1, 1 });

        if (compareArrangements (pre, post))
        {
            g.setColour (Colours::grey.withAlpha (0.6f));
            g.fillRoundedRectangle (Rectangle { width, height }.reduced (10, 3)
                                                               .toFloat(), 4);
        }
    }

    struct FontStringPair
    {
        Font font;
        String string;
    };

    static GlyphArrangement buildMultiFontText (Rectangle<float> bounds,
                                                Justification justification,
                                                Span<const FontStringPair> strings)
    {
        GlyphArrangement ga;
        float offset = 0;

        for (const auto& pair : strings)
        {
            ga.addFittedText (pair.font,
                              pair.string,
                              bounds.getX() + offset,
                              bounds.getY(),
                              bounds.getWidth(),
                              bounds.getHeight(),
                              justification,
                              1,
                              1);

            const auto whitespaceWidth = GlyphArrangement::getStringWidth (pair.font, " ");
            offset = whitespaceWidth + ga.getBoundingBox (0, ga.getNumGlyphs(), true).getWidth();
        }

        return ga;
    }

    static bool compareArrangements (const GlyphArrangement& a, const GlyphArrangement& b)
    {
        static auto compare = [] (const PositionedGlyph& pgA, const PositionedGlyph& pgB)
        {
            const auto tie = [] (const auto& x) { return std::tuple (x.getGlyphIndex(),
                                                                     x.getBounds()); };
            return tie (pgA) == tie (pgB);
        };

        return std::equal (a.begin(), a.end(), b.begin(), b.end(), compare);
    }

    Typeface::Ptr currentFace;
    std::vector<Feature> features;
};

class FeaturesListComponent : public Component
{
public:
    FeaturesListComponent()
    {
        featureList.setTitle ("Fonts");
        featureList.setRowHeight (40);

        addAndMakeVisible (featureList);
    }

    void setFont (Typeface::Ptr face)
    {
        listModel.setFont (face);
        featureList.updateContent();
    }

    void resized() override
    {
        featureList.setBounds (getLocalBounds());
    }

    FeatureListModel listModel;
    ListBox featureList { {}, &listModel };
};

//==============================================================================
class FontFeaturesDemo : public Component
{
public:
    FontFeaturesDemo()
    {
        fontsListBox.setTitle ("Fonts");
        fontsListBox.setRowHeight (20);
        fontsListBox.setColour (ListBox::textColourId, Colours::black);
        fontsListBox.setColour (ListBox::backgroundColourId, Colours::white);

        fontsListModel.onFontSelected = [this]
        {
            featureListBox.setFont (fontsListModel.getFaceForRow (fontsListBox.getSelectedRow()));
        };

        fontsListBox.selectRow (0);

        infoLabel.setFont (FontOptions{}.withPointHeight (16));
        infoLabel.setText ("Supported Features - "
                           "(Greyed out items are supported but not affected by the example)",
                           dontSendNotification);

        addAndMakeVisible (fontsListBox);
        addAndMakeVisible (infoLabel);
        addAndMakeVisible (featureListBox);

        setSize (750, 750);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (5);

        fontsListBox.setBounds (bounds.removeFromLeft (bounds.proportionOfWidth (0.3f)));
        infoLabel.setBounds (bounds.removeFromTop (30).reduced (5));
        featureListBox.setBounds (bounds);
    }

private:
    FontsListModel fontsListModel;
    ListBox fontsListBox { {}, &fontsListModel };
    Label infoLabel;
    FeaturesListComponent featureListBox;

    JUCE_DECLARE_NON_COPYABLE (FontFeaturesDemo)
};
