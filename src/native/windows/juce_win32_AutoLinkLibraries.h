
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
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "version.lib")

#if JUCE_OPENGL
 #pragma comment(lib, "OpenGL32.Lib")
 #pragma comment(lib, "GlU32.Lib")
#endif

#if JUCE_QUICKTIME
 #pragma comment (lib, "QTMLClient.lib")
#endif

#if JUCE_USE_CAMERA
 #pragma comment (lib, "Strmiids.lib")
#endif
