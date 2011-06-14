
// Auto-links to various win32 libs that are needed by library calls..
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "imm32.lib")

#ifdef _NATIVE_WCHAR_T_DEFINED
 #ifdef _DEBUG
  #pragma comment(lib, "comsuppwd.lib")
 #else
  #pragma comment(lib, "comsuppw.lib")
 #endif
#else
 #ifdef _DEBUG
  #pragma comment(lib, "comsuppd.lib")
 #else
  #pragma comment(lib, "comsupp.lib")
 #endif
#endif

#if JUCE_OPENGL
 #pragma comment(lib, "OpenGL32.Lib")
 #pragma comment(lib, "GlU32.Lib")
#endif

#if JUCE_QUICKTIME
 #pragma comment (lib, "QTMLClient.lib")
#endif

#if JUCE_USE_CAMERA
 #pragma comment (lib, "Strmiids.lib")
 #pragma comment (lib, "wmvcore.lib")
#endif

#if JUCE_DIRECT2D
 #pragma comment (lib, "Dwrite.lib")
 #pragma comment (lib, "D2d1.lib")
#endif

#if JUCE_DIRECTSHOW
 #pragma comment (lib, "strmiids.lib")
#endif

#if JUCE_MEDIAFOUNDATION
 #pragma comment (lib, "mfuuid.lib")
#endif
