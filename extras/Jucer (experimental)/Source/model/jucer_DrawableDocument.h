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

#ifndef __JUCER_DRAWABLEDOCUMENT_JUCEHEADER__
#define __JUCER_DRAWABLEDOCUMENT_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class DrawableDocument  :  public ValueTree::Listener,
                           public ChangeBroadcaster,
                           public Timer
{
public:
    //==============================================================================
    DrawableDocument (Project* project, const File& drawableFile);
    ~DrawableDocument();

    //==============================================================================
    void setName (const String& name);
    const String getName() const;

    bool reload();
    bool save();
    bool hasChangedSinceLastSave() const;
    void changed();

    ValueTree getRootDrawableNode() const;

    void addRectangle();
    void addCircle();
    void addImage (const File& imageFile);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const var::identifier& name);
    void valueTreeChildrenChanged (ValueTree& tree);
    void valueTreeParentChanged (ValueTree& tree);

    void timerCallback();

    //==============================================================================
    UndoManager* getUndoManager() throw()               { return &undoManager; }

private:
    Project* project;
    File drawableFile;
    ValueTree drawableRoot;
    UndoManager undoManager;
    bool saveAsXml, needsSaving;

    void save (OutputStream& output);
    bool load (InputStream& input);

    void addMissingIds (ValueTree tree) const;
    void addDrawable (Drawable& d);
};


#endif   // __JUCER_DRAWABLEDOCUMENT_JUCEHEADER__
