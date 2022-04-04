/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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
