/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
