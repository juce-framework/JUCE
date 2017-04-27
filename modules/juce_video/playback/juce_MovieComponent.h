/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

//==============================================================================
/**
    A component that can play back a video file.
*/
class JUCE_API  MovieComponent   : public NSViewComponent
{
public:
    MovieComponent();
    ~MovieComponent();

    bool loadMovie (const File& file);
    bool loadMovie (const URL& file);

    void closeMovie();
    bool isMovieOpen() const;
    String getCurrentMoviePath() const;

    void play();
    void stop();

    double getDuration() const;
    double getPosition() const;
    void setPosition (double seconds);
    void setVolume (float newVolume);
    float getVolume() const;

    Rectangle<int> getNativeSize() const;
    void setBoundsWithCorrectAspectRatio (Rectangle<int> spaceToFitWithin,
                                          RectanglePlacement placement);

protected:
    void resized() override;

private:
    struct Pimpl;
    ScopedPointer<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MovieComponent)
};
