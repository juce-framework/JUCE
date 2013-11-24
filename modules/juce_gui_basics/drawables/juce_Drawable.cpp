/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

Drawable::Drawable()
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
}

Drawable::Drawable (const Drawable& other)
    : Component (other.getName())
{
    setComponentID (other.getComponentID());
}

Drawable::~Drawable()
{
}

//==============================================================================
void Drawable::draw (Graphics& g, float opacity, const AffineTransform& transform) const
{
    const_cast <Drawable*> (this)->nonConstDraw (g, opacity, transform);
}

void Drawable::nonConstDraw (Graphics& g, float opacity, const AffineTransform& transform)
{
    Graphics::ScopedSaveState ss (g);

    g.addTransform (AffineTransform::translation ((float) -(originRelativeToComponent.x),
                                                  (float) -(originRelativeToComponent.y))
                        .followedBy (getTransform())
                        .followedBy (transform));

    if (! g.isClipEmpty())
    {
        if (opacity < 1.0f)
        {
            g.beginTransparencyLayer (opacity);
            paintEntireComponent (g, true);
            g.endTransparencyLayer();
        }
        else
        {
            paintEntireComponent (g, true);
        }
    }
}

void Drawable::drawAt (Graphics& g, float x, float y, float opacity) const
{
    draw (g, opacity, AffineTransform::translation (x, y));
}

void Drawable::drawWithin (Graphics& g, const Rectangle<float>& destArea,
                           RectanglePlacement placement, float opacity) const
{
    draw (g, opacity, placement.getTransformToFit (getDrawableBounds(), destArea));
}

//==============================================================================
DrawableComposite* Drawable::getParent() const
{
    return dynamic_cast <DrawableComposite*> (getParentComponent());
}

void Drawable::transformContextToCorrectOrigin (Graphics& g)
{
    g.setOrigin (originRelativeToComponent);
}

void Drawable::parentHierarchyChanged()
{
    setBoundsToEnclose (getDrawableBounds());
}

void Drawable::setBoundsToEnclose (const Rectangle<float>& area)
{
    Drawable* const parent = getParent();
    Point<int> parentOrigin;
    if (parent != nullptr)
        parentOrigin = parent->originRelativeToComponent;

    const Rectangle<int> newBounds (area.getSmallestIntegerContainer() + parentOrigin);
    originRelativeToComponent = parentOrigin - newBounds.getPosition();
    setBounds (newBounds);
}

//==============================================================================
void Drawable::setOriginWithOriginalSize (Point<float> originWithinParent)
{
    setTransform (AffineTransform::translation (originWithinParent.x, originWithinParent.y));
}

void Drawable::setTransformToFit (const Rectangle<float>& area, RectanglePlacement placement)
{
    if (! area.isEmpty())
        setTransform (placement.getTransformToFit (getDrawableBounds(), area));
}

//==============================================================================
Drawable* Drawable::createFromImageData (const void* data, const size_t numBytes)
{
    Drawable* result = nullptr;

    Image image (ImageFileFormat::loadFrom (data, numBytes));

    if (image.isValid())
    {
        DrawableImage* const di = new DrawableImage();
        di->setImage (image);
        result = di;
    }
    else
    {
        const String asString (String::createStringFromData (data, (int) numBytes));

        XmlDocument doc (asString);
        ScopedPointer <XmlElement> outer (doc.getDocumentElement (true));

        if (outer != nullptr && outer->hasTagName ("svg"))
        {
            ScopedPointer <XmlElement> svg (doc.getDocumentElement());

            if (svg != nullptr)
                result = Drawable::createFromSVG (*svg);
        }
    }

    return result;
}

Drawable* Drawable::createFromImageDataStream (InputStream& dataSource)
{
    MemoryOutputStream mo;
    mo << dataSource;

    return createFromImageData (mo.getData(), mo.getDataSize());
}

Drawable* Drawable::createFromImageFile (const File& file)
{
    FileInputStream fin (file);

    return fin.openedOk() ? createFromImageDataStream (fin) : nullptr;
}

//==============================================================================
template <class DrawableClass>
class DrawableTypeHandler  : public ComponentBuilder::TypeHandler
{
public:
    DrawableTypeHandler()
        : ComponentBuilder::TypeHandler (DrawableClass::valueTreeType)
    {
    }

    Component* addNewComponentFromState (const ValueTree& state, Component* parent)
    {
        DrawableClass* const d = new DrawableClass();

        if (parent != nullptr)
            parent->addAndMakeVisible (d);

        updateComponentFromState (d, state);
        return d;
    }

    void updateComponentFromState (Component* component, const ValueTree& state)
    {
        DrawableClass* const d = dynamic_cast <DrawableClass*> (component);
        jassert (d != nullptr);
        d->refreshFromValueTree (state, *this->getBuilder());
    }
};

void Drawable::registerDrawableTypeHandlers (ComponentBuilder& builder)
{
    builder.registerTypeHandler (new DrawableTypeHandler <DrawablePath>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableComposite>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableRectangle>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableImage>());
    builder.registerTypeHandler (new DrawableTypeHandler <DrawableText>());
}

Drawable* Drawable::createFromValueTree (const ValueTree& tree, ComponentBuilder::ImageProvider* imageProvider)
{
    ComponentBuilder builder (tree);
    builder.setImageProvider (imageProvider);
    registerDrawableTypeHandlers (builder);

    ScopedPointer<Component> comp (builder.createComponent());
    Drawable* const d = dynamic_cast<Drawable*> (static_cast <Component*> (comp));

    if (d != nullptr)
        comp.release();

    return d;
}

//==============================================================================
Drawable::ValueTreeWrapperBase::ValueTreeWrapperBase (const ValueTree& state_)
    : state (state_)
{
}

String Drawable::ValueTreeWrapperBase::getID() const
{
    return state [ComponentBuilder::idProperty];
}

void Drawable::ValueTreeWrapperBase::setID (const String& newID)
{
    if (newID.isEmpty())
        state.removeProperty (ComponentBuilder::idProperty, nullptr);
    else
        state.setProperty (ComponentBuilder::idProperty, newID, nullptr);
}
