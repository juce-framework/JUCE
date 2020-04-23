/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class ElementSiblingComponent  : public Component,
                                 public ChangeListener
{
public:
    ElementSiblingComponent (PaintElement* const owner_)
        : owner (owner_)
    {
        setAlwaysOnTop (true);

        owner->getDocument()->addChangeListener (this);
    }

    ~ElementSiblingComponent()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    virtual void updatePosition() = 0;


protected:
    void changeListenerCallback (ChangeBroadcaster*)
    {
        updatePosition();
    }

    PaintElement* const owner;
};
