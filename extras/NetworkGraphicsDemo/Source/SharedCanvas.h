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


/**
    This scene description is broadcast to all the clients, and contains a list of all
    the clients involved, as well as the set of shapes to be drawn.

    Each client will draw the part of the path that lies within its own area. It can
    find its area by looking at the list of clients contained in this structure.

    All the path coordinates are roughly in units of inches, and devices will convert
    this to pixels based on their screen size and DPI
*/
struct SharedCanvasDescription
{
    SharedCanvasDescription() {}

    Colour backgroundColour = Colours::black;

    struct ColouredPath
    {
        Path path;
        FillType fill;
    };

    Array<ColouredPath> paths;

    struct ClientArea
    {
        String name;
        Point<float> centre; // in inches
        float scaleFactor; // extra scaling
    };

    Array<ClientArea> clients;

    //==============================================================================
    void reset()
    {
        paths.clearQuick();
        clients.clearQuick();
    }

    void swapWith (SharedCanvasDescription& other)
    {
        std::swap (backgroundColour, other.backgroundColour);
        paths.swapWith (other.paths);
        clients.swapWith (other.clients);
    }

    // This is a fixed size that represents the overall canvas limits that
    // content should lie within
    Rectangle<float> getLimits() const
    {
        float inchesX = 60.0f;
        float inchesY = 30.0f;

        return { inchesX * -0.5f, inchesY * -0.5f, inchesX, inchesY };
    }

    //==============================================================================
    void draw (Graphics& g, Rectangle<float> targetArea, Rectangle<float> clientArea) const
    {
        draw (g, clientArea,
              AffineTransform::fromTargetPoints (clientArea.getX(),     clientArea.getY(),
                                                 targetArea.getX(),     targetArea.getY(),
                                                 clientArea.getRight(), clientArea.getY(),
                                                 targetArea.getRight(), targetArea.getY(),
                                                 clientArea.getRight(), clientArea.getBottom(),
                                                 targetArea.getRight(), targetArea.getBottom()));
    }

    void draw (Graphics& g, Rectangle<float> clientArea, AffineTransform t) const
    {
        g.saveState();
        g.addTransform (t);

        for (const auto& p : paths)
        {
            if (p.path.getBounds().intersects (clientArea))
            {
                g.setFillType (p.fill);
                g.fillPath (p.path);
            }
        }

        g.restoreState();
    }

    const ClientArea* findClient (const String& clientName) const
    {
        for (const auto& c : clients)
            if (c.name == clientName)
                return &c;

        return nullptr;
    }

    //==============================================================================
    // Serialisation...

    void save (OutputStream& out) const
    {
        out.writeInt (magic);
        out.writeInt ((int) backgroundColour.getARGB());

        out.writeInt (clients.size());

        for (const auto& c : clients)
        {
            out.writeString (c.name);
            writePoint (out, c.centre);
            out.writeFloat (c.scaleFactor);
        }

        out.writeInt (paths.size());

        for (const auto& p : paths)
        {
            writeFill (out, p.fill);
            p.path.writePathToStream (out);
        }
    }

    void load (InputStream& in)
    {
        if (in.readInt() != magic)
            return;

        backgroundColour = Colour ((uint32) in.readInt());

        {
            const int numClients = in.readInt();
            clients.clearQuick();

            for (int i = 0; i < numClients; ++i)
            {
                ClientArea c;
                c.name = in.readString();
                c.centre = readPoint (in);
                c.scaleFactor = in.readFloat();
                clients.add (c);
            }
        }

        {
            const int numPaths = in.readInt();
            paths.clearQuick();

            for (int i = 0; i < numPaths; ++i)
            {
                ColouredPath p;
                p.fill = readFill (in);
                p.path.loadPathFromStream (in);

                paths.add (std::move (p));
            }
        }
    }

    MemoryBlock toMemoryBlock() const
    {
        MemoryOutputStream o;
        save (o);
        return o.getMemoryBlock();
    }

private:
    //==============================================================================
    static void writePoint (OutputStream& out, Point<float> p)
    {
        out.writeFloat (p.x);
        out.writeFloat (p.y);
    }

    static void writeRect (OutputStream& out, Rectangle<float> r)
    {
        writePoint (out, r.getPosition());
        out.writeFloat (r.getWidth());
        out.writeFloat (r.getHeight());
    }

    static Point<float> readPoint (InputStream& in)
    {
        Point<float> p;
        p.x = in.readFloat();
        p.y = in.readFloat();
        return p;
    }

    static Rectangle<float> readRect (InputStream& in)
    {
        Rectangle<float> r;
        r.setPosition (readPoint (in));
        r.setWidth (in.readFloat());
        r.setHeight (in.readFloat());
        return r;
    }

    static void writeFill (OutputStream& out, const FillType& f)
    {
        if (f.isColour())
        {
            out.writeByte (0);
            out.writeInt ((int) f.colour.getARGB());
        }
        else if (f.isGradient())
        {
            const ColourGradient& cg = *f.gradient;
            jassert (cg.getNumColours() >= 2);

            out.writeByte (cg.isRadial ? 2 : 1);

            writePoint (out, cg.point1);
            writePoint (out, cg.point2);

            out.writeCompressedInt (cg.getNumColours());

            for (int i = 0; i < cg.getNumColours(); ++i)
            {
                out.writeDouble (cg.getColourPosition (i));
                out.writeInt ((int) cg.getColour(i).getARGB());
            }
        }
        else
        {
            jassertfalse;
        }
    }

    static FillType readFill (InputStream& in)
    {
        int type = in.readByte();

        if (type == 0)
            return FillType (Colour ((uint32) in.readInt()));

        if (type > 2)
        {
            jassertfalse;
            return FillType();
        }

        ColourGradient cg;
        cg.point1 = readPoint (in);
        cg.point2 = readPoint (in);

        cg.clearColours();

        int numColours = in.readCompressedInt();

        for (int i = 0; i < numColours; ++i)
        {
            const double pos = in.readDouble();
            cg.addColour (pos, Colour ((uint32) in.readInt()));
        }

        jassert (cg.getNumColours() >= 2);

        return FillType (cg);
    }

    const int magic = 0x2381239a;

    JUCE_DECLARE_NON_COPYABLE (SharedCanvasDescription)
};

//==============================================================================
class CanvasGeneratingContext    : public LowLevelGraphicsContext
{
public:
    CanvasGeneratingContext (SharedCanvasDescription& c)  : canvas (c)
    {
        stateStack.add (new SavedState());
    }

    //==============================================================================
    bool isVectorDevice() const override            { return true; }
    float getPhysicalPixelScaleFactor() override    { return 1.0f; }
    void setOrigin (Point<int> o) override          { addTransform (AffineTransform::translation ((float) o.x, (float) o.y)); }

    void addTransform (const AffineTransform& t) override
    {
        getState().transform = t.followedBy (getState().transform);
    }

    bool clipToRectangle (const Rectangle<int>&) override                   { return true; }
    bool clipToRectangleList (const RectangleList<int>&) override           { return true; }
    void excludeClipRectangle (const Rectangle<int>&) override              {}
    void clipToPath (const Path&, const AffineTransform&) override          {}
    void clipToImageAlpha (const Image&, const AffineTransform&) override   {}

    void saveState() override
    {
        stateStack.add (new SavedState (getState()));
    }

    void restoreState() override
    {
        jassert (stateStack.size() > 0);

        if (stateStack.size() > 0)
            stateStack.removeLast();
    }

    void beginTransparencyLayer (float alpha) override
    {
        saveState();
        getState().transparencyLayer = new SharedCanvasHolder();
        getState().transparencyOpacity = alpha;
    }

    void endTransparencyLayer() override
    {
        const ReferenceCountedObjectPtr<SharedCanvasHolder> finishedTransparencyLayer (getState().transparencyLayer);
        float alpha = getState().transparencyOpacity;
        restoreState();

        if (SharedCanvasHolder* c = finishedTransparencyLayer)
        {
            for (auto& path : c->canvas.paths)
            {
                path.fill.setOpacity (path.fill.getOpacity() * alpha);
                getTargetCanvas().paths.add (path);
            }
        }
    }

    Rectangle<int> getClipBounds() const override
    {
        return canvas.getLimits().getSmallestIntegerContainer()
                .transformedBy (getState().transform.inverted());
    }

    bool clipRegionIntersects (const Rectangle<int>&) override      { return true; }
    bool isClipEmpty() const override                               { return false; }

    //==============================================================================
    void setFill (const FillType& fillType) override                { getState().fillType = fillType; }
    void setOpacity (float op) override                             { getState().fillType.setOpacity (op); }
    void setInterpolationQuality (Graphics::ResamplingQuality) override {}

    //==============================================================================
    void fillRect (const Rectangle<int>& r, bool) override          { fillRect (r.toFloat()); }
    void fillRectList (const RectangleList<float>& list) override   { fillPath (list.toPath(), AffineTransform()); }

    void fillRect (const Rectangle<float>& r) override
    {
        Path p;
        p.addRectangle (r.toFloat());
        fillPath (p, AffineTransform());
    }

    void fillPath (const Path& p, const AffineTransform& t) override
    {
        Path p2 (p);
        p2.applyTransform (t.followedBy (getState().transform));

        getTargetCanvas().paths.add ({ std::move (p2), getState().fillType });
    }

    void drawImage (const Image&, const AffineTransform&) override {}

    void drawLine (const Line<float>& line) override
    {
        Path p;
        p.addLineSegment (line, 1.0f);
        fillPath (p, AffineTransform());
    }

    //==============================================================================
    const Font& getFont() override                  { return getState().font; }
    void setFont (const Font& newFont) override     { getState().font = newFont; }

    void drawGlyph (int glyphNumber, const AffineTransform& transform) override
    {
        Path p;
        Font& font = getState().font;
        font.getTypefacePtr()->getOutlineForGlyph (glyphNumber, p);
        fillPath (p, AffineTransform::scale (font.getHeight() * font.getHorizontalScale(), font.getHeight()).followedBy (transform));
    }

private:
    //==============================================================================
    struct SharedCanvasHolder  : public ReferenceCountedObject
    {
        SharedCanvasDescription canvas;
    };

    struct SavedState
    {
        FillType fillType;
        AffineTransform transform;
        Font font;
        ReferenceCountedObjectPtr<SharedCanvasHolder> transparencyLayer;
        float transparencyOpacity = 1.0f;
    };

    SharedCanvasDescription& getTargetCanvas() const
    {
        if (SharedCanvasHolder* c = getState().transparencyLayer)
            return c->canvas;

        return canvas;
    }

    SavedState& getState() const noexcept
    {
        jassert (stateStack.size() > 0);
        return *stateStack.getLast();
    }

    SharedCanvasDescription& canvas;
    OwnedArray<SavedState> stateStack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CanvasGeneratingContext)
};

//==============================================================================
/** Helper for breaking and reassembling a memory block into smaller checksummed
    blocks that will fit inside UDP packets
*/
struct BlockPacketiser
{
    void createBlocksFromData (const MemoryBlock& data, size_t maxBlockSize)
    {
        jassert (blocks.size() == 0);

        int offset = 0;
        size_t remaining = data.getSize();

        while (remaining > 0)
        {
            auto num = (size_t) jmin (maxBlockSize, remaining);
            blocks.add (MemoryBlock (addBytesToPointer (data.getData(), offset), num));
            offset += (int) num;
            remaining -= num;
        }

        MemoryOutputStream checksumBlock;
        checksumBlock << getLastPacketPrefix() << MD5 (data).toHexString() << (char) 0 << (char) 0;
        blocks.add (checksumBlock.getMemoryBlock());

        for (int i = 0; i < blocks.size(); ++i)
        {
            auto index = (uint32) ByteOrder::swapIfBigEndian (i);
            blocks.getReference(i).append (&index, sizeof (index));
        }
    }

    // returns true if this is an end-of-sequence block
    bool appendIncomingBlock (MemoryBlock data)
    {
        if (data.getSize() > 4)
            blocks.addSorted (*this, data);

        return String (CharPointer_ASCII ((const char*) data.getData())).startsWith (getLastPacketPrefix());
    }

    bool reassemble (MemoryBlock& result)
    {
        result.reset();

        if (blocks.size() > 1)
        {
            for (int i = 0; i < blocks.size() - 1; ++i)
                result.append (blocks.getReference(i).getData(), blocks.getReference(i).getSize() - 4);

            String storedMD5 (String (CharPointer_ASCII ((const char*) blocks.getLast().getData()))
                                .fromFirstOccurrenceOf (getLastPacketPrefix(), false, false));

            blocks.clearQuick();

            if (MD5 (result).toHexString().trim().equalsIgnoreCase (storedMD5.trim()))
                return true;
        }

        result.reset();
        return false;
    }

    static int compareElements (const MemoryBlock& b1, const MemoryBlock& b2)
    {
        auto i1 = ByteOrder::littleEndianInt (addBytesToPointer (b1.getData(), b1.getSize() - 4));
        auto i2 = ByteOrder::littleEndianInt (addBytesToPointer (b2.getData(), b2.getSize() - 4));
        return (int) (i1 - i2);
    }

    static const char* getLastPacketPrefix()   { return "**END_OF_PACKET_LIST** "; }

    Array<MemoryBlock> blocks;
};


//==============================================================================
struct AnimatedContent
{
    virtual ~AnimatedContent() {}

    virtual String getName() const = 0;
    virtual void reset() = 0;
    virtual void generateCanvas (Graphics&, SharedCanvasDescription& canvas, Rectangle<float> activeArea) = 0;
    virtual void handleTouch (Point<float> position) = 0;
};
