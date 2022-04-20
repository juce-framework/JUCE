/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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

#ifndef PIP_DEMO_UTILITIES_INCLUDED
#define PIP_DEMO_UTILITIES_INCLUDED 1

#include <JuceHeader.h>

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
    return File { CharPointer_UTF8 { PIP_JUCE_EXAMPLES_DIRECTORY_STRING } };
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
        "M72.87 84.28A42.36 42.36 0 0130.4 42.14a42.48 42.48 0 0184.95 0 42.36 42.36 0 01-42.48 42.14zm0-78.67A36.74 36.74 0 0036 42.14a36.88 36.88 0 0073.75 0A36.75 36.75 0 0072.87 5.61z"
        "M77.62 49.59a177.77 177.77 0 008.74 18.93A4.38 4.38 0 0092.69 70a34.5 34.5 0 008.84-9 4.3 4.3 0 00-2.38-6.49A176.73 176.73 0 0180 47.32a1.78 1.78 0 00-2.38 2.27zM81.05 44.27a169.68 169.68 0 0020.13 7.41 4.39 4.39 0 005.52-3.41 34.42 34.42 0 00.55-6.13 33.81 33.81 0 00-.67-6.72 4.37 4.37 0 00-6.31-3A192.32 192.32 0 0181.1 41a1.76 1.76 0 00-.05 3.27zM74.47 50.44a1.78 1.78 0 00-3.29 0 165.54 165.54 0 00-7.46 19.89 4.33 4.33 0 003.47 5.48 35.49 35.49 0 005.68.46 34.44 34.44 0 007.13-.79 4.32 4.32 0 003-6.25 187.83 187.83 0 01-8.53-18.79zM71.59 34.12a1.78 1.78 0 003.29.05 163.9 163.9 0 007.52-20.11A4.34 4.34 0 0079 8.59a35.15 35.15 0 00-13.06.17 4.32 4.32 0 00-3 6.26 188.41 188.41 0 018.65 19.1zM46.32 30.3a176.2 176.2 0 0120 7.48 1.78 1.78 0 002.37-2.28 180.72 180.72 0 00-9.13-19.84 4.38 4.38 0 00-6.33-1.47 34.27 34.27 0 00-9.32 9.65 4.31 4.31 0 002.41 6.46zM68.17 49.18a1.77 1.77 0 00-2.29-2.34 181.71 181.71 0 00-19.51 8.82A4.3 4.3 0 0044.91 62a34.36 34.36 0 009.42 8.88 4.36 4.36 0 006.5-2.38 175.11 175.11 0 017.34-19.32zM77.79 35.59a1.78 1.78 0 002.3 2.35 182.51 182.51 0 0019.6-8.88 4.3 4.3 0 001.5-6.25 34.4 34.4 0 00-9.41-9.14A4.36 4.36 0 0085.24 16a174.51 174.51 0 01-7.45 19.59zM64.69 40.6a167.72 167.72 0 00-20.22-7.44A4.36 4.36 0 0039 36.6a33.68 33.68 0 00-.45 5.54 34 34 0 00.81 7.4 4.36 4.36 0 006.28 2.84 189.19 189.19 0 0119-8.52 1.76 1.76 0 00.05-3.26zM20 129.315c0 5-2.72 8.16-7.11 8.16-2.37 0-4.17-1-6.2-3.56l-.69-.78-6 5 .57.76c3.25 4.36 7.16 6.39 12.31 6.39 9 0 15.34-6.57 15.34-16v-28.1H20zM61.69 126.505c0 6.66-3.76 11-9.57 11-5.81 0-9.56-4.31-9.56-11v-25.32h-8.23v25.69c0 10.66 7.4 18.4 17.6 18.4 10 0 17.61-7.72 18-18.4v-25.69h-8.24zM106.83 134.095c-3.58 2.43-6.18 3.38-9.25 3.38a14.53 14.53 0 010-29c3.24 0 5.66.88 9.25 3.38l.76.53 4.78-6-.75-.62a22.18 22.18 0 00-14.22-5.1 22.33 22.33 0 100 44.65 21.53 21.53 0 0014.39-5.08l.81-.64-5-6zM145.75 137.285h-19.06v-10.72h18.3v-7.61h-18.3v-10.16h19.06v-7.61h-27.28v43.53h27.28z"
        "M68.015 83.917c-7.723-.902-15.472-4.123-21.566-8.966-8.475-6.736-14.172-16.823-15.574-27.575C29.303 35.31 33.538 22.7 42.21 13.631 49.154 6.368 58.07 1.902 68.042.695c2.15-.26 7.524-.26 9.675 0 12.488 1.512 23.464 8.25 30.437 18.686 8.332 12.471 9.318 28.123 2.605 41.368-2.28 4.5-4.337 7.359-7.85 10.909A42.273 42.273 0 0177.613 83.92c-2.027.227-7.644.225-9.598-.003zm7.823-5.596c8.435-.415 17.446-4.678 23.683-11.205 5.976-6.254 9.35-13.723 10.181-22.537.632-6.705-1.346-14.948-5.065-21.108C98.88 13.935 89.397 7.602 78.34 5.906c-2.541-.39-8.398-.386-10.96.006C53.54 8.034 42.185 17.542 37.81 30.67c-2.807 8.426-2.421 17.267 1.11 25.444 4.877 11.297 14.959 19.41 26.977 21.709 2.136.408 6.1.755 7.377.645.325-.028 1.48-.094 2.564-.147z"
  );
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

#endif   // PIP_DEMO_UTILITIES_INCLUDED
