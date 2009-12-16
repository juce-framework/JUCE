/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
#define __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__

#include "juce_Drawable.h"


//==============================================================================
/**
    A drawable object which acts as a container for a set of other Drawables.

    @see Drawable
*/
class JUCE_API  DrawableComposite  : public Drawable
{
public:
    //==============================================================================
    /** Creates a composite Drawable.
    */
    DrawableComposite();

    /** Destructor. */
    virtual ~DrawableComposite();

    //==============================================================================
    /** Adds a new sub-drawable to this one.

        This passes in a Drawable pointer for this object to look after. To add a copy
        of a drawable, use the form of this method that takes a Drawable reference instead.

        @param drawable         the object to add - this will be deleted automatically
                                when no longer needed, so the caller mustn't keep any
                                pointers to it.
        @param transform        the transform to apply to this drawable when it's being
                                drawn
        @param index            where to insert it in the list of drawables. 0 is the back,
                                -1 is the front, or any value from 0 and getNumDrawables()
                                can be used
        @see removeDrawable
    */
    void insertDrawable (Drawable* drawable,
                         const AffineTransform& transform = AffineTransform::identity,
                         const int index = -1);

    /** Adds a new sub-drawable to this one.

        This takes a copy of a Drawable and adds it to this object. To pass in a Drawable
        for this object to look after, use the form of this method that takes a Drawable
        pointer instead.

        @param drawable         the object to add - an internal copy will be made of this object
        @param transform        the transform to apply to this drawable when it's being
                                drawn
        @param index            where to insert it in the list of drawables. 0 is the back,
                                -1 is the front, or any value from 0 and getNumDrawables()
                                can be used
        @see removeDrawable
    */
    void insertDrawable (const Drawable& drawable,
                         const AffineTransform& transform = AffineTransform::identity,
                         const int index = -1);

    /** Deletes one of the Drawable objects.

        @param index    the index of the drawable to delete, between 0
                        and (getNumDrawables() - 1).
        @param deleteDrawable   if this is true, the drawable that is removed will also
                        be deleted. If false, it'll just be removed.
        @see insertDrawable, getNumDrawables
    */
    void removeDrawable (const int index, const bool deleteDrawable = true);

    /** Returns the number of drawables contained inside this one.

        @see getDrawable
    */
    int getNumDrawables() const throw()                         { return drawables.size(); }

    /** Returns one of the drawables that are contained in this one.

        Each drawable also has a transform associated with it - you can use getDrawableTransform()
        to find it.

        The pointer returned is managed by this object and will be deleted when no longer
        needed, so be careful what you do with it.

        @see getNumDrawables
    */
    Drawable* getDrawable (const int index) const throw()                           { return drawables [index]; }

    /** Returns the transform that applies to one of the drawables that are contained in this one.

        The pointer returned is managed by this object and will be deleted when no longer
        needed, so be careful what you do with it.

        @see getNumDrawables
    */
    const AffineTransform* getDrawableTransform (const int index) const throw()     { return transforms [index]; }

    /** Brings one of the Drawables to the front.

        @param index    the index of the drawable to move, between 0
                        and (getNumDrawables() - 1).
        @see insertDrawable, getNumDrawables
    */
    void bringToFront (const int index);


    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    void getBounds (float& x, float& y, float& width, float& height) const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    ValueTree createValueTree() const throw();
    /** @internal */
    static DrawableComposite* createFromValueTree (const ValueTree& tree) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <Drawable> drawables;
    OwnedArray <AffineTransform> transforms;

    DrawableComposite (const DrawableComposite&);
    const DrawableComposite& operator= (const DrawableComposite&);
};


#endif   // __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
