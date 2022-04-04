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

    ~ElementSiblingComponent() override
    {
        owner->getDocument()->removeChangeListener (this);
    }

    virtual void updatePosition() = 0;

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        updatePosition();
    }

protected:
    PaintElement* const owner;
};
