The VST3 SDK has been modified in two places where warnings from clang-tidy 22
could not be supressed.

public.sdk/source/common/commonstringconvert.cpp:72
public.sdk/source/vst/utility/stringconvert.cpp:100

Both modifications are accompanied by a `// JUCE MODIFICATION` comment.