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

DrawablePath::DrawablePath() {}
DrawablePath::~DrawablePath() {}

DrawablePath::DrawablePath (const DrawablePath& other)  : DrawableShape (other)
{
    setPath (other.path);
}

std::unique_ptr<Drawable> DrawablePath::createCopy() const
{
    return std::make_unique<DrawablePath> (*this);
}

void DrawablePath::setPath (const Path& newPath)
{
    path = newPath;
    pathChanged();
}

void DrawablePath::setPath (Path&& newPath)
{
    path = std::move (newPath);
    pathChanged();
}

const Path& DrawablePath::getPath() const           { return path; }
const Path& DrawablePath::getStrokePath() const     { return strokePath; }

} // namespace juce
