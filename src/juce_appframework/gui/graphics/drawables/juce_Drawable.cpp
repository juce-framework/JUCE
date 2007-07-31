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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Drawable.h"
#include "juce_DrawableImage.h"
#include "../imaging/juce_ImageFileFormat.h"
#include "../../../../juce_core/text/juce_XmlDocument.h"
#include "../../../../juce_core/io/files/juce_FileInputStream.h"


//==============================================================================
Drawable::Drawable()
{
}

Drawable::~Drawable()
{
}

void Drawable::drawAt (Graphics& g, const float x, const float y) const
{
    draw (g, AffineTransform::translation (x, y));
}

void Drawable::drawWithin (Graphics& g,
                           const int destX,
                           const int destY,
                           const int destW,
                           const int destH,
                           const RectanglePlacement& placement) const
{
    if (destW > 0 && destH > 0)
    {
        float x, y, w, h;
        getBounds (x, y, w, h);

        draw (g, placement.getTransformToFit (x, y, w, h,
                                              (float) destX, (float) destY,
                                              (float) destW, (float) destH));
    }
}

//==============================================================================
Drawable* Drawable::createFromImageData (const void* data, const int numBytes)
{
    Drawable* result = 0;

    Image* const image = ImageFileFormat::loadFrom (data, numBytes);

    if (image != 0)
    {
        DrawableImage* const di = new DrawableImage();
        di->setImage (image, true);
        result = di;
    }
    else
    {
        const String asString (String::createStringFromData (data, numBytes));

        XmlDocument doc (asString);
        XmlElement* const outer = doc.getDocumentElement (true);

        if (outer != 0 && outer->hasTagName (T("svg")))
        {
            XmlElement* const svg = doc.getDocumentElement();

            if (svg != 0)
            {
                result = Drawable::createFromSVG (*svg);
                delete svg;
            }
        }

        delete outer;
    }

    return result;
}

Drawable* Drawable::createFromImageDataStream (InputStream& dataSource)
{
    MemoryBlock mb;
    dataSource.readIntoMemoryBlock (mb);

    return createFromImageData (mb.getData(), mb.getSize());
}

Drawable* Drawable::createFromImageFile (const File& file)
{
    FileInputStream* fin = file.createInputStream();

    if (fin == 0)
        return 0;

    Drawable* d = createFromImageDataStream (*fin);
    delete fin;

    return d;
}


END_JUCE_NAMESPACE
