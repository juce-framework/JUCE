/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../jucer_JucerDocument.h"
#include "../jucer_PaintRoutine.h"
#include "jucer_SnapGridPainter.h"
class JucerDocumentEditor;

//==============================================================================
class PaintRoutineEditor  : public Component,
                            public LassoSource <PaintElement*>,
                            public FileDragAndDropTarget,
                            private ChangeListener
{
public:
    //==============================================================================
    PaintRoutineEditor (PaintRoutine& graphics,
                        JucerDocument& document,
                        JucerDocumentEditor* const docHolder);
    ~PaintRoutineEditor() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void paintOverChildren (Graphics& g) override;
    void resized() override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void visibilityChanged() override;

    void findLassoItemsInArea (Array <PaintElement*>& results, const Rectangle<int>& area) override;

    SelectedItemSet <PaintElement*>& getLassoSelection() override;

    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped (const StringArray& filenames, int x, int y) override;

    Rectangle<int> getComponentArea() const;

    //==============================================================================
    void refreshAllElements();

private:
    PaintRoutine& graphics;
    JucerDocument& document;
    JucerDocumentEditor* const documentHolder;
    LassoComponent <PaintElement*> lassoComp;
    SnapGridPainter grid;
    Image componentOverlay;
    float componentOverlayOpacity;

    Colour currentBackgroundColour;

    void removeAllElementComps();
    void updateComponentOverlay();
    void updateChildBounds();
};
