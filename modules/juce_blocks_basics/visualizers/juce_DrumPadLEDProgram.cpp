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

DrumPadGridProgram::DrumPadGridProgram (Block& b)  : Program (b) {}

int DrumPadGridProgram::getPadIndex (float posX, float posY) const
{
    posX = juce::jmin (0.99f, posX / block.getWidth());
    posY = juce::jmin (0.99f, posY / block.getHeight());

    const uint32 offset  = block.getDataByte (visiblePads_byte) ? numColumns1_byte : numColumns0_byte;
    const int numColumns = block.getDataByte (offset + numColumns0_byte);
    const int numRows    = block.getDataByte (offset + numRows0_byte);

    return int (posX * numColumns) + int (posY * numRows) * numColumns;
}

void DrumPadGridProgram::startTouch (float startX, float startY)
{
    const auto padIdx = getPadIndex (startX, startY);

    for (size_t i = 0; i < 4; ++i)
    {
        if (block.getDataByte (touchedPads_byte + i) == 0)
        {
            block.setDataByte (touchedPads_byte + i, static_cast<uint8> (padIdx + 1));
            break;
        }
    }
}

void DrumPadGridProgram::endTouch (float startX, float startY)
{
    const auto padIdx = getPadIndex (startX, startY);

    for (size_t i = 0; i < 4; ++i)
        if (block.getDataByte (touchedPads_byte + i) == (padIdx + 1))
            block.setDataByte (touchedPads_byte + i, 0);
}

void DrumPadGridProgram::sendTouch (float x, float y, float z, LEDColor color)
{
    Block::ProgramEventMessage e;

    e.values[0] = 0x20000000
                    + (juce::jlimit (0, 255, juce::roundToInt (x * (255.0f / block.getWidth())))  << 16)
                    + (juce::jlimit (0, 255, juce::roundToInt (y * (255.0f / block.getHeight()))) << 8)
                    +  juce::jlimit (0, 255, juce::roundToInt (z * 255.0f));

    e.values[1] = (int32) color.getARGB();

    block.sendProgramEvent (e);
}

//==============================================================================
void DrumPadGridProgram::setGridFills (int numColumns, int numRows, const juce::Array<GridFill>& fills)
{
    uint8 visiblePads = block.getDataByte (visiblePads_byte);

    setGridFills (numColumns, numRows, fills, visiblePads * numColumns1_byte);
}

void DrumPadGridProgram::setGridFills (int numColumns, int numRows, const juce::Array<GridFill>& fills, uint32 byteOffset)
{
    jassert (numColumns * numRows == fills.size());

    block.setDataByte (byteOffset + numColumns0_byte, (uint8) numColumns);
    block.setDataByte (byteOffset + numRows0_byte,    (uint8) numRows);

    uint32 i = 0;

    for (auto fill : fills)
    {
        if (i >= maxNumPads)
        {
            jassertfalse;
            break;
        }

        const uint32 colorOffsetBytes = byteOffset + colors0_byte + i * colorSizeBytes;
        const uint32 colorOffsetBits  = colorOffsetBytes * 8;

        block.setDataBits (colorOffsetBits,      5, fill.color.getRed()   >> 3);
        block.setDataBits (colorOffsetBits + 5,  6, fill.color.getGreen() >> 2);
        block.setDataBits (colorOffsetBits + 11, 5, fill.color.getBlue()  >> 3);

        block.setDataByte (byteOffset + fillTypes0_byte + i, static_cast<uint8> (fill.fillType));

        ++i;
    }
}

void DrumPadGridProgram::triggerSlideTransition (int newNumColumns, int newNumRows,
                                                 const juce::Array<GridFill>& newFills, SlideDirection direction)
{
    uint8 newVisible = block.getDataByte (visiblePads_byte) ? 0 : 1;

    setGridFills (newNumColumns, newNumRows, newFills, newVisible * numColumns1_byte);

    block.setDataByte (visiblePads_byte, newVisible);
    block.setDataByte (slideDirection_byte, (uint8) direction);
}

//==============================================================================
void DrumPadGridProgram::setPadAnimationState (uint32 padIdx, double loopTimeSecs, double currentProgress)
{
    // Only 16 animated pads are supported.
    jassert (padIdx < 16);

    // Compensate for bluetooth latency & led resolution, tweaked by eye for POS app.
    currentProgress = std::fmod (currentProgress + 0.1, 1.0);

    uint16 aniValue     = uint16 (juce::roundToInt ((255 << 8) * currentProgress));
    uint16 aniIncrement = loopTimeSecs > 0.0 ? uint16 (juce::roundToInt (((255 << 8) / 25.0) / loopTimeSecs)) : 0;

    uint32 offset = 8 * animationTimers_byte + 32 * padIdx;

    block.setDataBits (offset,      16, aniValue);
    block.setDataBits (offset + 16, 16, aniIncrement);
}

void DrumPadGridProgram::suspendAnimations()
{
    for (uint32 i = 0; i < 16; ++i)
    {
        uint32 offset = 8 * animationTimers_byte + 32 * i;
        block.setDataBits (offset + 16, 16, 0);
    }

    // Hijack touch dimming
    block.setDataByte (touchedPads_byte, 255);
}

void DrumPadGridProgram::resumeAnimations()
{
    // Unhijack touch dimming
    block.setDataByte (touchedPads_byte, 0);
}

//==============================================================================
juce::String DrumPadGridProgram::getLittleFootProgram()
{
    if (block.versionNumber.isEmpty() || block.versionNumber.compare ("0.2.5") < 0)
        return getLittleFootProgramPre25();

    return getLittleFootProgramPost25();
}

juce::String DrumPadGridProgram::getLittleFootProgramPre25() const
{
    // Uses its own heatmap, not the one provided in newer firmware
    // Also can't use blocks config, introduced in 2.5.

    return R"littlefoot(

    #heapsize: 1351

    int dimFactor;
    int dimDelay;
    int slideAnimationProgress;
    int lastVisiblePads;

    int getGridColor (int index, int colorMapOffset)
    {
        int bit = (2 + colorMapOffset) * 8 + index * 16;

        return makeARGB (255,
                         getHeapBits (bit,      5) << 3,
                         getHeapBits (bit + 5,  6) << 2,
                         getHeapBits (bit + 11, 5) << 3);
    }

    // Returns the current progress and also increments it for next frame
    int getAnimationProgress (int index)
    {
        // Only 16 animated pads supported
        if (index > 15)
            return 0;

        int offsetBits = 162 * 8 + index * 32;

        int currentProgress = getHeapBits (offsetBits, 16);
        int increment = getHeapBits (offsetBits + 16, 16);
        int nextFrame = currentProgress + increment;

        // Set incremented 16 bit number.
        setHeapByte (162 + index * 4, nextFrame & 0xff);
        setHeapByte (163 + index * 4, nextFrame >> 8);

        return currentProgress;
    }

    void outlineRect (int color, int x, int y, int w)
    {
        fillRect (color, x, y, w, 1);
        fillRect (color, x, y + w - 1, w, 1);
        fillRect (color, x, y + 1, 1, w - 1);
        fillRect (color, x + w - 1, y + 1, 1, w - 1);
    }

    void drawPlus (int color, int x, int y, int w)
    {
        fillRect (color, x, y + (w / 2), w, 1);
        fillRect (color, x + (w / 2), y, 1, w);
    }

    void fillGradientRect (int color, int x, int y, int w)
    {
        if (color != 0xff000000)
        {
            int divisor = w + w - 1;

            for (int yy = 0; yy < w; ++yy)
            {
                for (int xx = yy; xx < w; ++xx)
                {
                    int gradColor = blendARGB (color, makeARGB (((xx + yy) * 250) / divisor, 0, 0, 0));

                    setLED (x + xx, y + yy, gradColor);
                    setLED (x + yy, y + xx, gradColor);
                }
            }
        }
    }

    // TODO: Tom M: This is massaged to work with 3x3 pads and for dots to sync
    // with Apple POS loop length. Rework to be more robust & flexible.
    void drawPizzaLED (int color, int x, int y, int w, int progress)
    {
        --w;
        x += 1;

        int numToDo = ((8 * progress) / 255) + 1;
        int totalLen = w * 4;

        for (int i = 1; i <= numToDo; ++i)
        {
            setLED (x, y, color);

            if (i < w)
                ++x;
            else if (i < (w * 2))
                ++y;
            else if (i < (w * 3))
                --x;
            else if (i < totalLen)
                --y;
        }
    }

    void drawPad (int padX, int padY, int padW,
                  int color, int fill, int animateProgress)
    {
        animateProgress >>= 8; // 16 bit to 8 bit
        int halfW = padW / 2;

        if (fill == 0) // Gradient fill
        {
            fillGradientRect (color, padX, padY, padW);
        }

        else if (fill == 1) // Filled
        {
            fillRect (color, padX, padY, padW, padW);
        }

        else if (fill == 2) // Hollow
        {
            outlineRect (color, padX, padY, padW);
        }

        else if (fill == 3) // Hollow with plus
        {
            outlineRect (color, padX, padY, padW);
            drawPlus (0xffffffff, padX, padY, padW);
        }

        else if (fill == 4) // Pulsing dot
        {
            int pulseCol = blendARGB (color, makeARGB (animateProgress, 0, 0, 0));

            setLED (padX + halfW, padY + halfW, pulseCol);
        }

        else if (fill == 5) // Blinking dot
        {
            int blinkCol = animateProgress > 64 ? makeARGB (255, 0, 0, 0) : color;

            setLED (padX + halfW, padY + halfW, blinkCol);
        }

        else if (fill == 6) // Pizza filled
        {
            outlineRect (blendARGB (color, makeARGB (220, 0, 0, 0)), padX, padY, padW); // Dim outline
            setLED (padX + halfW, padY + halfW, color); // Bright center

            drawPizzaLED (color, padX, padY, padW, animateProgress);
        }

        else if (fill == 7) // Pizza hollow
        {
            outlineRect (blendARGB (color, makeARGB (220, 0, 0, 0)), padX, padY, padW); // Dim outline

            drawPizzaLED (color, padX, padY, padW, animateProgress);
            return;
        }
    }

    void fadeHeatMap()
    {
        for (int i = 0; i < 225; ++i)
        {
            int colorOffset = 226 + i * 4;
            int color = getHeapInt (colorOffset);
            int alpha = (color >> 24) & 0xff;

            if (alpha > 0)
            {
                alpha -= getHeapByte (1126 + i);
                setHeapInt (colorOffset, alpha < 0 ? 0 : ((alpha << 24) | (color & 0xffffff)));
            }
        }
    }

    void addToHeatMap (int x, int y, int color)
    {
        if (x >= 0 && y >= 0 && x < 15 && y < 15)
        {
            int offset = 226 + 4 * (x + y * 15);
            color = blendARGB (getHeapInt (offset), color);
            setHeapInt (offset, color);

            int decay = ((color >> 24) & 0xff) / 14; // change divisor to change trail times
            offset = 1126 + (x + y * 15);
            setHeapByte (offset, decay > 0 ? decay : 1);
        }
    }

    int getHeatmapColor (int x, int y)
    {
        return getHeapInt (226 + 4 * (x + y * 15));
    }

    int isPadActive (int index)
    {
        if (getHeapInt (158) == 0) // None active
            return 0;

        ++index;

        return index == getHeapByte (158) ||
               index == getHeapByte (159) ||
               index == getHeapByte (160) ||
               index == getHeapByte (161);
    }

    void updateDimFactor()
    {
        if (getHeapInt (158) == 0)
        {
            if (--dimDelay <= 0)
            {
                dimFactor -= 12;

                if (dimFactor < 0)
                    dimFactor = 0;
            }
        }
        else
        {
            dimFactor = 180;
            dimDelay = 12;
        }
    }

    void drawPads (int offsetX, int offsetY, int colorMapOffset)
    {
        int padsPerSide = getHeapByte (0 + colorMapOffset);

        if (padsPerSide < 2)
            return;

        int blockW = 15 / padsPerSide;
        int blockPlusGapW = blockW + (15 - padsPerSide * blockW) / (padsPerSide - 1);

        for (int padY = 0; padY < padsPerSide; ++padY)
        {
            for (int padX = 0; padX < padsPerSide; ++padX)
            {
                int ledX = offsetX + padX * blockPlusGapW;
                int ledY = offsetY + padY * blockPlusGapW;

                if (ledX < 15 &&
                    ledY < 15 &&
                    (ledX + blockW) >= 0 &&
                    (ledY + blockW) >= 0)
                {
                    int padIdx = padX + padY * padsPerSide;
                    bool padActive = isPadActive (padIdx);

                    int blendCol = padActive ? 255 : 0;
                    int blendAmt = padActive ? dimFactor >> 1 : dimFactor;

                    int color   = blendARGB (getGridColor (padIdx, colorMapOffset),
                                              makeARGB (blendAmt, blendCol, blendCol, blendCol));
                    int fillType = getHeapByte (colorMapOffset + 52 + padIdx);
                    int animate  = getAnimationProgress (padIdx);

                    drawPad (ledX, ledY, blockW, color, fillType, animate);
                }
            }
        }
    }

    void slideAnimatePads()
    {
        int nowVisible = getHeapByte (155);

        if (lastVisiblePads != nowVisible)
        {
            lastVisiblePads = nowVisible;

            if (slideAnimationProgress <= 0)
                slideAnimationProgress = 15;
        }

        // If animation is complete, draw normally.
        if (slideAnimationProgress <= 0)
        {
            drawPads (0, 0, 78 * nowVisible);
            slideAnimationProgress = 0;
        }
        else
        {
            int direction = getHeapByte (156);
            slideAnimationProgress -= 1;

            int inPos  = nowVisible == 0 ? 0  : 78;
            int outPos = nowVisible == 0 ? 78 : 0;

            if (direction == 0) // Up
            {
                drawPads (0, slideAnimationProgress - 16, outPos);
                drawPads (0, slideAnimationProgress,      inPos);
            }
            else if (direction == 1) // Down
            {
                drawPads (0, 16 - slideAnimationProgress, outPos);
                drawPads (0, 0 - slideAnimationProgress,  inPos);
            }
            else if (direction == 2) // Left
            {
                drawPads (16 - slideAnimationProgress, 0, outPos);
                drawPads (slideAnimationProgress,      0, inPos);
            }
            else if (direction == 3) // Right
            {
                drawPads (16 - slideAnimationProgress, 0, outPos);
                drawPads (0 - slideAnimationProgress,  0, inPos);
            }
            else // None
            {
                drawPads (0, 0, 78 * nowVisible);
                slideAnimationProgress = 0;
            }
        }
    }

    void repaint()
    {
        // showErrorOnFail, showRepaintTime, showMovingDot
        //enableDebug (true, true, false);

        // Clear LEDs to black, update dim animation
        fillRect (0xff000000, 0, 0, 15, 15);
        updateDimFactor();

        // Does the main painting of pads
        slideAnimatePads();

        // Overlay heatmap
        for (int y = 0; y < 15; ++y)
            for (int x = 0; x < 15; ++x)
                blendLED (x, y, getHeatmapColor (x, y));

        fadeHeatMap();
    }

    // DrumPadGridProgram::sendTouch results in this callback, giving
    // us more touch updates per frame and therefore smoother trails.
    void handleMessage (int pos, int color, int xx)
    {
        handleMessage (pos, color);
    }

    void handleMessage (int pos, int color)
    {
        if ((pos >> 24) != 0x20)
            return;

        int tx = ((pos >> 16) & 0xff) - 13;
        int ty = ((pos >> 8) & 0xff) - 13;

        int tz = pos & 0xff;
        tz = tz > 30 ? tz : 30;

        int ledCenterX = tx >> 4;
        int ledCenterY = ty >> 4;
        int adjustX = (tx - (ledCenterX << 4)) >> 2;
        int adjustY = (ty - (ledCenterY << 4)) >> 2;

        for (int dy = -2; dy <= 2; ++dy)
        {
            for (int dx = -2; dx <= 2; ++dx)
            {
                int distance = dx * dx + dy * dy;
                int level = distance == 0 ? 255 : (distance == 1 ? 132 : (distance < 5 ? 9 : (distance == 5 ? 2 : 0)));

                level += (dx * adjustX);
                level += (dy * adjustY);

                level = (tz * level) >> 8;

                if (level > 0)
                    addToHeatMap (ledCenterX + dx, ledCenterY + dy,
                                  makeARGB (level, color >> 16, color >> 8, color));
            }
        }
    }

    )littlefoot";
}

juce::String DrumPadGridProgram::getLittleFootProgramPost25() const
{
    // Uses heatmap provided in firmware (so the program's smaller)
    // Initializes config items introduced in firmware 2.5

    return R"littlefoot(

    #heapsize: 256

    int dimFactor;
    int dimDelay;
    int slideAnimationProgress;
    int lastVisiblePads;
    bool gammaCorrected;

    void initialize()
    {
        for (int i = 0; i < 32; ++i)
            setLocalConfigActiveState (i, true, true);

        // Enable gamma correction if supported on hardware
        setLocalConfig (33, 1);
        gammaCorrected = getLocalConfig (33) > 0;
    }

    int getGridColor (int index, int colorMapOffset)
    {
        int bit = (2 + colorMapOffset) * 8 + index * 16;

        return makeARGB (255,
                         getHeapBits (bit,      5) << 3,
                         getHeapBits (bit + 5,  6) << 2,
                         getHeapBits (bit + 11, 5) << 3);
    }

    // Returns the current progress and also increments it for next frame
    int getAnimationProgress (int index)
    {
        // Only 16 animated pads supported
        if (index > 15)
            return 0;

        int offsetBits = 162 * 8 + index * 32;

        int currentProgress = getHeapBits (offsetBits, 16);
        int increment = getHeapBits (offsetBits + 16, 16);
        int nextFrame = currentProgress + increment;

        // Set incremented 16 bit number.
        setHeapByte (162 + index * 4, nextFrame & 0xff);
        setHeapByte (163 + index * 4, nextFrame >> 8);

        return currentProgress;
    }

    void outlineRect (int color, int x, int y, int w)
    {
        fillRect (color, x, y, w, 1);
        fillRect (color, x, y + w - 1, w, 1);
        fillRect (color, x, y + 1, 1, w - 1);
        fillRect (color, x + w - 1, y + 1, 1, w - 1);
    }

    void drawPlus (int color, int x, int y, int w)
    {
        fillRect (color, x, y + (w / 2), w, 1);
        fillRect (color, x + (w / 2), y, 1, w);
    }

    void fillGradientRect (int color, int x, int y, int w)
    {
        if (color != 0xff000000)
        {
            int divisor = w + w - 1;

            for (int yy = 0; yy < w; ++yy)
            {
                for (int xx = yy; xx < w; ++xx)
                {
                    int gradColor = blendARGB (color, makeARGB (((xx + yy) * 250) / divisor, 0, 0, 0));

                    fillPixel (gradColor, x + xx, y + yy);
                    fillPixel (gradColor, x + yy, y + xx);
                }
            }
        }
    }

    // TODO: Tom M: This is massaged to work with 3x3 pads and for dots to sync
    // with Apple POS loop length. Rework to be more robust & flexible.
    void drawPizzaLED (int color, int x, int y, int w, int progress)
    {
        --w;
        x += 1;

        int numToDo = ((8 * progress) / 255) + 1;
        int totalLen = w * 4;

        for (int i = 1; i <= numToDo; ++i)
        {
            fillPixel (color, x, y);

            if (i < w)
                ++x;
            else if (i < (w * 2))
                ++y;
            else if (i < (w * 3))
                --x;
            else if (i < totalLen)
                --y;
        }
    }

    void drawPad (int padX, int padY, int padW,
                  int color, int fill, int animateProgress)
    {
        animateProgress >>= 8; // 16 bit to 8 bit
        int halfW = padW / 2;

        if (fill == 0) // Gradient fill
        {
            fillGradientRect (color, padX, padY, padW);
        }
        else if (fill == 1) // Filled
        {
            fillRect (color, padX, padY, padW, padW);
        }
        else if (fill == 2) // Hollow
        {
            outlineRect (color, padX, padY, padW);
        }
        else if (fill == 3) // Hollow with plus
        {
            outlineRect (color, padX, padY, padW);
            drawPlus (0xffffffff, padX, padY, padW);
        }
        else if (fill == 4) // Pulsing dot
        {
            int pulseCol = blendARGB (color, makeARGB (animateProgress, 0, 0, 0));

            fillPixel (pulseCol, padX + halfW, padY + halfW);
        }
        else if (fill == 5) // Blinking dot
        {
            int blinkCol = animateProgress > 64 ? 0xff000000 : color;

            fillPixel (blinkCol, padX + halfW, padY + halfW);
        }
        else if (fill == 6) // Pizza filled
        {
            outlineRect (blendARGB (color, 0xdc000000), padX, padY, padW); // Dim outline
            fillPixel (color, padX + halfW, padY + halfW); // Bright center

            drawPizzaLED (color, padX, padY, padW, animateProgress);
        }
        else  // Pizza hollow
        {
            outlineRect (blendARGB (color, 0xdc000000), padX, padY, padW); // Dim outline

            drawPizzaLED (color, padX, padY, padW, animateProgress);
        }
    }

    int isPadActive (int index)
    {
        if (getHeapInt (158) == 0) // None active
            return 0;

        ++index;

        return index == getHeapByte (158) ||
               index == getHeapByte (159) ||
               index == getHeapByte (160) ||
               index == getHeapByte (161);
    }

    void updateDimFactor()
    {
        if (getHeapInt (158) == 0)
        {
            if (--dimDelay <= 0)
            {
                dimFactor -= 12;

                if (dimFactor < 0)
                    dimFactor = 0;
            }
        }
        else
        {
            dimFactor = gammaCorrected ? 100 : 180;
            dimDelay = 12;
        }
    }

    void drawPads (int offsetX, int offsetY, int colorMapOffset)
    {
        int padsPerSide = getHeapByte (0 + colorMapOffset);

        if (padsPerSide < 2)
            return;

        int blockW = 15 / padsPerSide;
        int blockPlusGapW = blockW + (15 - padsPerSide * blockW) / (padsPerSide - 1);

        for (int padY = 0; padY < padsPerSide; ++padY)
        {
            for (int padX = 0; padX < padsPerSide; ++padX)
            {
                int ledX = offsetX + padX * blockPlusGapW;
                int ledY = offsetY + padY * blockPlusGapW;

                if (ledX < 15 &&
                    ledY < 15 &&
                    (ledX + blockW) >= 0 &&
                    (ledY + blockW) >= 0)
                {
                    int padIdx = padX + padY * padsPerSide;
                    bool padActive = isPadActive (padIdx);

                    int blendCol = padActive ? 255 : 0;
                    int blendAmt = padActive ? dimFactor >> 1 : dimFactor;

                    int color   = blendARGB (getGridColor (padIdx, colorMapOffset),
                                              makeARGB (blendAmt, blendCol, blendCol, blendCol));
                    int fillType = getHeapByte (colorMapOffset + 52 + padIdx);
                    int animate  = getAnimationProgress (padIdx);

                    drawPad (ledX, ledY, blockW, color, fillType, animate);
                }
            }
        }
    }

    void slideAnimatePads()
    {
        int nowVisible = getHeapByte (155);

        if (lastVisiblePads != nowVisible)
        {
            lastVisiblePads = nowVisible;

            if (slideAnimationProgress <= 0)
                slideAnimationProgress = 15;
        }

        // If animation is complete, draw normally.
        if (slideAnimationProgress <= 0)
        {
            drawPads (0, 0, 78 * nowVisible);
            slideAnimationProgress = 0;
        }
        else
        {
            int direction = getHeapByte (156);
            slideAnimationProgress -= 1;

            int inPos  = nowVisible == 0 ? 0  : 78;
            int outPos = nowVisible == 0 ? 78 : 0;

            if (direction == 0) // Up
            {
                drawPads (0, slideAnimationProgress - 16, outPos);
                drawPads (0, slideAnimationProgress,      inPos);
            }
            else if (direction == 1) // Down
            {
                drawPads (0, 16 - slideAnimationProgress, outPos);
                drawPads (0, 0 - slideAnimationProgress,  inPos);
            }
            else if (direction == 2) // Left
            {
                drawPads (16 - slideAnimationProgress, 0, outPos);
                drawPads (slideAnimationProgress,      0, inPos);
            }
            else if (direction == 3) // Right
            {
                drawPads (16 - slideAnimationProgress, 0, outPos);
                drawPads (0 - slideAnimationProgress,  0, inPos);
            }
            else // None
            {
                drawPads (0, 0, 78 * nowVisible);
                slideAnimationProgress = 0;
            }
        }
    }

    void repaint()
    {
        // showErrorOnFail, showRepaintTime, showMovingDot
        //enableDebug (true, true, false);

        // Clear LEDs to black, update dim animation
        fillRect (0xff000000, 0, 0, 15, 15);
        updateDimFactor();

        // Does the main painting of pads
        slideAnimatePads();

        // Overlay heatmap
        drawPressureMap();
        fadePressureMap();
    }

    // DrumPadGridProgram::sendTouch results in this callback, giving
    // us more touch updates per frame and therefore smoother trails.
    void handleMessage (int pos, int color, int dummy)
    {
        if ((pos >> 24) != 0x20)
            return;

        int tx = (pos >> 16) & 0xff;
        int ty = (pos >> 8) & 0xff;
        int tz = pos & 0xff;

        addPressurePoint (color,
                          tx * (2.0 / (256 + 20)),
                          ty * (2.0 / (256 + 20)),
                          tz * (1.0 / 3.0));
    }

    )littlefoot";
}

} // namespace juce
