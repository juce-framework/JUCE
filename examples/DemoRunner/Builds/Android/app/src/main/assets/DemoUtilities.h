/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#ifndef PIP_DEMO_UTILITIES_INCLUDED
 #define PIP_DEMO_UTILITIES_INCLUDED 1
#endif

//==============================================================================
/*
    This file contains a bunch of miscellaneous utilities that are
    used by the various demos.
*/

//==============================================================================
inline Colour getRandomColour (float brightness) noexcept
{
    return Colour::fromHSV (Random::getSystemRandom().nextFloat(), 0.5f, brightness, 1.0f);
}

inline Colour getRandomBrightColour() noexcept  { return getRandomColour (0.8f); }
inline Colour getRandomDarkColour() noexcept    { return getRandomColour (0.3f); }

inline Colour getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback = Colour (0xff4d4d4d)) noexcept
{
    if (auto* v4 = dynamic_cast<LookAndFeel_V4*> (&LookAndFeel::getDefaultLookAndFeel()))
        return v4->getCurrentColourScheme().getUIColour (uiColour);

    return fallback;
}

inline File getExamplesDirectory() noexcept
{
   #ifdef PIP_JUCE_EXAMPLES_DIRECTORY
    MemoryOutputStream mo;

    auto success = Base64::convertFromBase64 (mo, JUCE_STRINGIFY (PIP_JUCE_EXAMPLES_DIRECTORY));
    ignoreUnused (success);
    jassert (success);

    return mo.toString();
   #elif defined PIP_JUCE_EXAMPLES_DIRECTORY_STRING
    return File { PIP_JUCE_EXAMPLES_DIRECTORY_STRING };
   #else
    auto currentFile = File::getSpecialLocation (File::SpecialLocationType::currentApplicationFile);
    auto exampleDir = currentFile.getParentDirectory().getChildFile ("examples");

    if (exampleDir.exists())
        return exampleDir;

    // keep track of the number of parent directories so we don't go on endlessly
    for (int numTries = 0; numTries < 15; ++numTries)
    {
        if (currentFile.getFileName() == "examples")
            return currentFile;

        const auto sibling = currentFile.getSiblingFile ("examples");

        if (sibling.exists())
            return sibling;

        currentFile = currentFile.getParentDirectory();
    }

    return currentFile;
   #endif
}

inline std::unique_ptr<InputStream> createAssetInputStream (const char* resourcePath)
{
  #if JUCE_ANDROID
    ZipFile apkZip (File::getSpecialLocation (File::invokedExecutableFile));
    return std::unique_ptr<InputStream> (apkZip.createStreamForEntry (apkZip.getIndexOfFileName ("assets/" + String (resourcePath))));
  #else
   #if JUCE_IOS
    auto assetsDir = File::getSpecialLocation (File::currentExecutableFile)
                          .getParentDirectory().getChildFile ("Assets");
   #elif JUCE_MAC
    auto assetsDir = File::getSpecialLocation (File::currentExecutableFile)
                          .getParentDirectory().getParentDirectory().getChildFile ("Resources").getChildFile ("Assets");

    if (! assetsDir.exists())
        assetsDir = getExamplesDirectory().getChildFile ("Assets");
   #else
    auto assetsDir = getExamplesDirectory().getChildFile ("Assets");
   #endif

    auto resourceFile = assetsDir.getChildFile (resourcePath);
    jassert (resourceFile.existsAsFile());

    return resourceFile.createInputStream();
  #endif
}

inline Image getImageFromAssets (const char* assetName)
{
    auto hashCode = (String (assetName) + "@juce_demo_assets").hashCode64();
    auto img = ImageCache::getFromHashCode (hashCode);

    if (img.isNull())
    {
        std::unique_ptr<InputStream> juceIconStream (createAssetInputStream (assetName));

        if (juceIconStream == nullptr)
            return {};

        img = ImageFileFormat::loadFrom (*juceIconStream);

        ImageCache::addImageToCache (img, hashCode);
    }

    return img;
}

inline String loadEntireAssetIntoString (const char* assetName)
{
    std::unique_ptr<InputStream> input (createAssetInputStream (assetName));

    if (input == nullptr)
        return {};

    return input->readString();
}

//==============================================================================
inline Path getJUCELogoPath()
{
    return Drawable::parseSVGPath (
        "M250,301.3c-37.2,0-67.5-30.3-67.5-67.5s30.3-67.5,67.5-67.5s67.5,30.3,67.5,67.5S287.2,301.3,250,301.3zM250,170.8c-34.7,0-63,28.3-63,63s28.3,63,63,63s63-28.3,63-63S284.7,170.8,250,170.8z"
        "M247.8,180.4c0-2.3-1.8-4.1-4.1-4.1c-0.2,0-0.3,0-0.5,0c-10.6,1.2-20.6,5.4-29,12c-1,0.8-1.5,1.8-1.6,2.9c-0.1,1.2,0.4,2.3,1.3,3.2l32.5,32.5c0.5,0.5,1.4,0.1,1.4-0.6V180.4z"
        "M303.2,231.6c1.2,0,2.3-0.4,3.1-1.2c0.9-0.9,1.3-2.1,1.1-3.3c-1.2-10.6-5.4-20.6-12-29c-0.8-1-1.9-1.6-3.2-1.6c-1.1,0-2.1,0.5-3,1.3l-32.5,32.5c-0.5,0.5-0.1,1.4,0.6,1.4L303.2,231.6z"
        "M287.4,191.3c-0.1-1.1-0.6-2.2-1.6-2.9c-8.4-6.6-18.4-10.8-29-12c-0.2,0-0.3,0-0.5,0c-2.3,0-4.1,1.9-4.1,4.1v46c0,0.7,0.9,1.1,1.4,0.6l32.5-32.5C287,193.6,287.5,192.5,287.4,191.3z"
        "M252.2,287.2c0,2.3,1.8,4.1,4.1,4.1c0.2,0,0.3,0,0.5,0c10.6-1.2,20.6-5.4,29-12c1-0.8,1.5-1.8,1.6-2.9c0.1-1.2-0.4-2.3-1.3-3.2l-32.5-32.5c-0.5-0.5-1.4-0.1-1.4,0.6V287.2z"
        "M292.3,271.2L292.3,271.2c1.2,0,2.4-0.6,3.2-1.6c6.6-8.4,10.8-18.4,12-29c0.1-1.2-0.3-2.4-1.1-3.3c-0.8-0.8-1.9-1.2-3.1-1.2l-45.9,0c-0.7,0-1.1,0.9-0.6,1.4l32.5,32.5C290.2,270.8,291.2,271.2,292.3,271.2z"
        "M207.7,196.4c-1.2,0-2.4,0.6-3.2,1.6c-6.6,8.4-10.8,18.4-12,29c-0.1,1.2,0.3,2.4,1.1,3.3c0.8,0.8,1.9,1.2,3.1,1.2l45.9,0c0.7,0,1.1-0.9,0.6-1.4l-32.5-32.5C209.8,196.8,208.8,196.4,207.7,196.4z"
        "M242.6,236.1l-45.9,0c-1.2,0-2.3,0.4-3.1,1.2c-0.9,0.9-1.3,2.1-1.1,3.3c1.2,10.6,5.4,20.6,12,29c0.8,1,1.9,1.6,3.2,1.6c1.1,0,2.1-0.5,3-1.3c0,0,0,0,0,0l32.5-32.5C243.7,236.9,243.4,236.1,242.6,236.1z"
        "M213.8,273.1L213.8,273.1c-0.9,0.9-1.3,2-1.3,3.2c0.1,1.1,0.6,2.2,1.6,2.9c8.4,6.6,18.4,10.8,29,12c0.2,0,0.3,0,0.5,0h0c1.2,0,2.3-0.5,3.1-1.4c0.7-0.8,1-1.8,1-2.9v-45.9c0-0.7-0.9-1.1-1.4-0.6l-13.9,13.9L213.8,273.1z"
        "M197.2,353c-4.1,0-7.4-1.5-10.4-5.4l4-3.5c2,2.6,3.9,3.6,6.4,3.6c4.4,0,7.4-3.3,7.4-8.3v-24.7h5.6v24.7C210.2,347.5,204.8,353,197.2,353z"
        "M232.4,353c-8.1,0-15-6-15-15.8v-22.5h5.6v22.2c0,6.6,3.9,10.8,9.5,10.8c5.6,0,9.5-4.3,9.5-10.8v-22.2h5.6v22.5C247.5,347,240.5,353,232.4,353z"
        "M272,353c-10.8,0-19.5-8.6-19.5-19.3c0-10.8,8.8-19.3,19.5-19.3c4.8,0,9,1.6,12.3,4.4l-3.3,4.1c-3.4-2.4-5.7-3.2-8.9-3.2c-7.7,0-13.8,6.2-13.8,14.1c0,7.9,6.1,14.1,13.8,14.1c3.1,0,5.6-1,8.8-3.2l3.3,4.1C280.1,351.9,276.4,353,272,353z"
        "M290.4,352.5v-37.8h22.7v5H296v11.2h16.5v5H296v11.6h17.2v5H290.4z");
}

//==============================================================================
#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 inline CodeEditorComponent::ColourScheme getDarkCodeEditorColourScheme()
 {
     struct Type
     {
         const char* name;
         juce::uint32 colour;
     };

     const Type types[] =
     {
         { "Error",              0xffe60000 },
         { "Comment",            0xff72d20c },
         { "Keyword",            0xffee6f6f },
         { "Operator",           0xffc4eb19 },
         { "Identifier",         0xffcfcfcf },
         { "Integer",            0xff42c8c4 },
         { "Float",              0xff885500 },
         { "String",             0xffbc45dd },
         { "Bracket",            0xff058202 },
         { "Punctuation",        0xffcfbeff },
         { "Preprocessor Text",  0xfff8f631 }
     };

     CodeEditorComponent::ColourScheme cs;

     for (auto& t : types)
         cs.set (t.name, Colour (t.colour));

     return cs;
 }

 inline CodeEditorComponent::ColourScheme getLightCodeEditorColourScheme()
 {
     struct Type
     {
         const char* name;
         juce::uint32 colour;
     };

     const Type types[] =
     {
         { "Error",              0xffcc0000 },
         { "Comment",            0xff00aa00 },
         { "Keyword",            0xff0000cc },
         { "Operator",           0xff225500 },
         { "Identifier",         0xff000000 },
         { "Integer",            0xff880000 },
         { "Float",              0xff885500 },
         { "String",             0xff990099 },
         { "Bracket",            0xff000055 },
         { "Punctuation",        0xff004400 },
         { "Preprocessor Text",  0xff660000 }
     };

     CodeEditorComponent::ColourScheme cs;

     for (auto& t : types)
         cs.set (t.name, Colour (t.colour));

     return cs;
 }
#endif

//==============================================================================
// This is basically a sawtooth wave generator - maps a value that bounces between
// 0.0 and 1.0 at a random speed
struct BouncingNumber
{
    BouncingNumber()
        : speed (0.0004 + 0.0007 * Random::getSystemRandom().nextDouble()),
          phase (Random::getSystemRandom().nextDouble())
    {
    }

    float getValue() const
    {
        double v = fmod (phase + speed * Time::getMillisecondCounterHiRes(), 2.0);
        return (float) (v >= 1.0 ? (2.0 - v) : v);
    }

protected:
    double speed, phase;
};

struct SlowerBouncingNumber  : public BouncingNumber
{
    SlowerBouncingNumber()
    {
        speed *= 0.3;
    }
};
