/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCER_COLOUREDELEMENT_JUCEHEADER__
#define __JUCER_COLOUREDELEMENT_JUCEHEADER__

#include "../jucer_PaintRoutine.h"
#include "../jucer_JucerDocument.h"
#include "jucer_StrokeType.h"


//==============================================================================
/**
    Base class for paint elements that have a fill colour and stroke.

*/
class ColouredElement   : public PaintElement
{
public:
    //==============================================================================
    ColouredElement (PaintRoutine* owner,
                     const String& name,
                     const bool showOutline_,
                     const bool showJointAndEnd_);

    ~ColouredElement();

    //==============================================================================
    void getEditableProperties (Array <PropertyComponent*>& properties);
    void getColourSpecificProperties (Array <PropertyComponent*>& properties);

    //==============================================================================
    const FillType& getFillType() throw();
    void setFillType (const FillType& newType, const bool undoable);

    bool isStrokeEnabled() const throw();
    void enableStroke (bool enable, const bool undoable);

    const StrokeType& getStrokeType() throw();
    void setStrokeType (const PathStrokeType& newType, const bool undoable);
    void setStrokeFill (const FillType& newType, const bool undoable);

    //==============================================================================
    const Rectangle getCurrentBounds (const Rectangle& parentArea) const;
    void setCurrentBounds (const Rectangle& newBounds, const Rectangle& parentArea, const bool undoable);

    void createSiblingComponents();

    //==============================================================================
    void addColourAttributes (XmlElement* const e) const;
    bool loadColourAttributes (const XmlElement& xml);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    FillType fillType;

    bool isStrokePresent;
    const bool showOutline, showJointAndEnd;
    StrokeType strokeType;

    void convertToNewPathElement (const Path& path);
};


#endif   // __JUCER_COLOUREDELEMENT_JUCEHEADER__
