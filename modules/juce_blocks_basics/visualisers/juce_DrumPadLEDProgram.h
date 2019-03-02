/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    @tags{Blocks}
*/
struct DrumPadGridProgram  : public Block::Program
{
    DrumPadGridProgram (Block&);

    //==============================================================================
    /** These let the program dim pads which aren't having gestures performed on them. */

    void startTouch (float startX, float startY);
    void endTouch   (float startX, float startY);

    /** Creates trail effects similar to the onscreen pad trails. */
    void sendTouch (float x, float y, float z, LEDColour);

    //==============================================================================
    /** Call this to match animations to the project tempo.

        @param padIdx           The pad to update. 16 animated pads are supported, so 0 - 15.
        @param loopTimeSecs     The length of time for the pad's animation to loop in seconds. 0 will stop the animation.
        @param currentProgress  The starting progress of the animation. 0.0 - 1.0.
    */
    void setPadAnimationState (uint32 padIdx, double loopTimeSecs, double currentProgress);

    /** If the app needs to close down or suspend, use these to pause & dim animations. */
    void suspendAnimations();
    void resumeAnimations();

    //==============================================================================
    /** Set how each pad in the grid looks. */
    struct GridFill
    {
        enum FillType : uint8
        {
            gradient            = 0,
            filled              = 1,
            hollow              = 2,
            hollowPlus          = 3,

            // Animated pads
            dotPulsing          = 4,
            dotBlinking         = 5,
            pizzaFilled         = 6,
            pizzaHollow         = 7,
        };

        LEDColour colour;
        FillType fillType;
    };

    void setGridFills (int numColumns, int numRows,
                       const Array<GridFill>&);

    /** Set up a new pad layout, with a slide animation from the old to the new. */
    enum SlideDirection : uint8
    {
        up     = 0,
        down   = 1,
        left   = 2,
        right  = 3,

        none   = 255
    };

    void triggerSlideTransition (int newNumColumns, int newNumRows,
                                 const Array<GridFill>& newFills, SlideDirection);

private:
    //==============================================================================
    /** Shared data heap is laid out as below. There is room for two sets of
        pad layouts, colours and fill types to allow animation between two states. */

    static constexpr uint32 numColumns0_byte      = 0;    // 1 byte
    static constexpr uint32 numRows0_byte         = 1;    // 1 byte (ignored for the moment: always square pads to save cycles)
    static constexpr uint32 colours0_byte         = 2;    // 2 byte x 25  (5:6:5 bits for rgb)
    static constexpr uint32 fillTypes0_byte       = 52;   // 1 byte x 25

    static constexpr uint32 numColumns1_byte      = 78;   // 1 byte
    static constexpr uint32 numRows1_byte         = 79;   // 1 byte
    static constexpr uint32 colours1_byte         = 80;   // 2 byte x 25  (5:6:5 bits for rgb)
    static constexpr uint32 fillTypes1_byte       = 130;  // 1 byte x 25

    static constexpr uint32 visiblePads_byte      = 155;  // 1 byte       (i.e. which set of colours/fills to use, 0 or 1)
    static constexpr uint32 slideDirection_byte   = 156;  // 1 byte
    static constexpr uint32 touchedPads_byte      = 158;  // 1 byte x 4   (Zero means empty slot, so stores padIdx + 1)
    static constexpr uint32 animationTimers_byte  = 162;  // 4 byte x 16  (16:16 bits counter:increment)
    static constexpr uint32 totalHeapSize         = 226;

    static constexpr uint32 maxNumPads        = 25;
    static constexpr uint32 colourSizeBytes   = 2;

    int getPadIndex (float posX, float posY) const;
    void setGridFills (int numColumns, int numRows, const Array<GridFill>& fills, uint32 byteOffset);

    String getLittleFootProgram() override;
    String getLittleFootProgramPre25() const;
    String getLittleFootProgramPost25() const;
};

} // namespace juce
