/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class WEBPLoader
{
public:
    WEBPLoader (InputStream& in)
    {
        WEBPImageFormat imageFmt;
        image = imageFmt.decodeImage(in);
    }

    Image image;

private:
 

    JUCE_DECLARE_NON_COPYABLE (WEBPLoader)
};





//==============================================================================
WEBPImageFormat::WEBPImageFormat() {}
WEBPImageFormat::~WEBPImageFormat() {}

String WEBPImageFormat::getFormatName()                  { return "WEBP"; }
bool WEBPImageFormat::usesFileExtension (const File& f)  { return f.hasFileExtension ("webp"); }

bool WEBPImageFormat::canUnderstand (InputStream& in)
{
    // write inputstream to temp file
    auto tempFile = File::createTempFile(".webp");
    auto out = tempFile.createOutputStream();
    out->writeFromInputStream(in,-1);
    out->flush();

    // now ask dwebp if the file can be decoded
    ChildProcess childProcess;
    StringArray args;
    args.add("dwebp");
    args.add(tempFile.getFullPathName());
    childProcess.start(args);
    auto result = childProcess.readAllProcessOutput();
    DBG(result);

    // delete temp file and return result
    tempFile.deleteFile();
    return result.contains("can be decoded");
}

Image WEBPImageFormat::decodeImage (InputStream& in)
{
    // create temp files
    auto webpFile = File::createTempFile(".webp");
    auto pngFile = File::createTempFile(".png");

    // write inputstream to temp.webp file
    auto out = webpFile.createOutputStream();
    out->writeFromInputStream(in, -1);
    out->flush();

    // use dwebp to convert to png file
    convertToPNG(webpFile, pngFile);

    // load png file
    Image image = PNGImageFormat::loadFrom(pngFile);

    // delete temp files
    webpFile.deleteFile();
    pngFile.deleteFile();

    // return loaded image
    return image;
}

bool WEBPImageFormat::writeImageToStream (const Image& sourceImage, OutputStream& destStream)
{
    // create temp files
    auto webpFile = File::createTempFile(".webp");
    auto pngFile = File::createTempFile(".png");

    // write to temp.png file
    PNGImageFormat pngFormat;
    auto out = pngFile.createOutputStream();
    if (!pngFormat.writeImageToStream(sourceImage, *out))
    {
        jassertfalse;
        return false;
    }

    // use cwebp to convert to webp file
    if(!convertFromPNG(pngFile, webpFile))
    {
        jassertfalse;
        pngFile.deleteFile();
        webpFile.deleteFile();
        return false;
    }

    // write webp file to output stream
    auto in = webpFile.createInputStream();
    auto bytesWritten = destStream.writeFromInputStream(*in, -1);

    // delete temp files
    webpFile.deleteFile();
    pngFile.deleteFile();

    // return successfulness 
    return bytesWritten > 0;
}

Image WEBPImageFormat::loadFrom(File& webpFile)
{
    // create temp files
    auto pngFile = File::createTempFile(".png");

    // use dwebp to convert to png file
    WEBPImageFormat fmt;
    fmt.convertToPNG(webpFile, pngFile);

    // load png file
    Image image = PNGImageFormat::loadFrom(pngFile);

    // delete temp files
    pngFile.deleteFile();

    // return loaded image
    return image;
}

bool juce::WEBPImageFormat::writeTo(const Image& sourceImage, File& webpFile)
{
    // create temp files
    auto pngFile = File::createTempFile(".png");

    // write to temp.png file
    PNGImageFormat pngFmt;
    auto out = pngFile.createOutputStream();
    if (!pngFmt.writeImageToStream(sourceImage, *out))
    {
        jassertfalse;
        pngFile.deleteFile();
        return false;
    }

    // use cwebp to convert to webp file
    WEBPImageFormat webpFmt;
    bool success = webpFmt.convertFromPNG(pngFile, webpFile);

    // delete temp file
    pngFile.deleteFile();

    // return successfulness 
    return success;
}

bool WEBPImageFormat::convertToPNG(File webpFile, File pngFile)
{
    // use dwebp to convert to png file
    ChildProcess childProcess;
    StringArray args;

    args.add("dwebp");
    args.add(webpFile.getFullPathName());
    args.add("-mt"); // multithreaded
    args.add("-o");
    args.add(pngFile.getFullPathName());

    childProcess.start(args);

    auto result = childProcess.readAllProcessOutput();
    DBG(result);

    return result.startsWith("Decoded ");
}

bool WEBPImageFormat::convertFromPNG(File pngFile, File webpFile)
{
    // use cwebp to convert to webp file
    ChildProcess childProcess;
    StringArray args;

    args.add("cwebp");
    args.add("-lossless");
    args.add("-mt"); // multithreaded
    args.add(pngFile.getFullPathName());
    args.add("-o");
    args.add(webpFile.getFullPathName());

    childProcess.start(args);
    auto result = childProcess.readAllProcessOutput();
    DBG(result);

    if (!result.contains("Saving file "))
    {
        jassertfalse;
        return false;
    }
    else 
        return true;
}

} // namespace juce
