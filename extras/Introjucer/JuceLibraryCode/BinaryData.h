/* =========================================================================================

   This is an auto-generated file, created by The Introjucer 3.0.0
   Do not edit anything in this file!

*/

namespace BinaryData
{
    extern const char*   AudioPluginXCodeScript_txt;
    const int            AudioPluginXCodeScript_txtSize = 2112;

    extern const char*   brushed_aluminium_png;
    const int            brushed_aluminium_pngSize = 14724;

    extern const char*   cog_icon_svg;
    const int            cog_icon_svgSize = 915;

    extern const char*   juce_icon_png;
    const int            juce_icon_pngSize = 19826;

    extern const char*   jucer_AudioPluginEditorTemplate_cpp;
    const int            jucer_AudioPluginEditorTemplate_cppSize = 1008;

    extern const char*   jucer_AudioPluginEditorTemplate_h;
    const int            jucer_AudioPluginEditorTemplate_hSize = 799;

    extern const char*   jucer_AudioPluginFilterTemplate_cpp;
    const int            jucer_AudioPluginFilterTemplate_cppSize = 4455;

    extern const char*   jucer_AudioPluginFilterTemplate_h;
    const int            jucer_AudioPluginFilterTemplate_hSize = 2400;

    extern const char*   jucer_MainConsoleAppTemplate_cpp;
    const int            jucer_MainConsoleAppTemplate_cppSize = 470;

    extern const char*   jucer_MainTemplate_cpp;
    const int            jucer_MainTemplate_cppSize = 1825;

    extern const char*   jucer_NewCppFileTemplate_cpp;
    const int            jucer_NewCppFileTemplate_cppSize = 232;

    extern const char*   jucer_NewCppFileTemplate_h;
    const int            jucer_NewCppFileTemplate_hSize = 308;

    extern const char*   jucer_WindowTemplate_cpp;
    const int            jucer_WindowTemplate_cppSize = 781;

    extern const char*   jucer_WindowTemplate_h;
    const int            jucer_WindowTemplate_hSize = 1216;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();

    //==============================================================================
    // This class acts as an ImageProvider that will access the BinaryData images
    class ImageProvider  : public juce::ComponentBuilder::ImageProvider
    {
    public:
        ImageProvider() noexcept {}

        juce::Image getImageForIdentifier (const juce::var& imageIdentifier)
        {
            int dataSize = 0;
            const char* const data = getNamedResource (imageIdentifier.toString().toUTF8(), dataSize);

            if (data != nullptr)
                return juce::ImageCache::getFromMemory (data, dataSize);

            return juce::Image();
        }

        juce::var getIdentifierForImage (const juce::Image&)  { return juce::var(); }
    };
}
