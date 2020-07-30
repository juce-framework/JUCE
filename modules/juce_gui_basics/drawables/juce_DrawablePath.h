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

//==============================================================================
/**
    A drawable object which renders a filled or outlined shape.

    For details on how to change the fill and stroke, see the DrawableShape class.

    @see Drawable, DrawableShape

    @tags{GUI}
*/
class JUCE_API  DrawablePath  : public DrawableShape
{
public:
    //==============================================================================
    /** Creates a DrawablePath. */
    DrawablePath();
    DrawablePath (const DrawablePath&);

    /** Destructor. */
    ~DrawablePath() override;

    //==============================================================================
    /** Changes the path that will be drawn.
        @see setFill, setStrokeType
    */
    void setPath (const Path& newPath);

    /** Changes the path that will be drawn.
        @see setFill, setStrokeType
    */
    void setPath (Path&& newPath);

    /** Returns the current path. */
    const Path& getPath() const;

    /** Returns the current path for the outline. */
    const Path& getStrokePath() const;

    //==============================================================================
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;

private:
    //==============================================================================
    DrawablePath& operator= (const DrawablePath&);
    JUCE_LEAK_DETECTOR (DrawablePath)
};

} // namespace juce
