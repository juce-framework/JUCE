#ifndef _REAPER_PLUGIN_FX_EMBED_H_
#define _REAPER_PLUGIN_FX_EMBED_H_


/*
 * to support via VST2: canDo("hasCockosEmbeddedUI") should return 0xbeef0000
 * dispatcher will be called with opcode=effVendorSpecific, index=effEditDraw, value=parm2, ptr=(void*)(INT_PTR)parm3, opt=message (REAPER_FXEMBED_WM_*)
 *
 * to support via VST3: IController should support IReaperUIEmbedInterface, see reaper_vst3_interfaces.h
 *
 * to support via LV2: todo
 */

// these alias to win32's WM_*


#define REAPER_FXEMBED_WM_IS_SUPPORTED                 0x0000
/* return 1 if embedding is supported and available
 * return -1 if embedding is supported and unavailable
 * return 0 if embedding is not supported
*/

#define REAPER_FXEMBED_WM_CREATE                       0x0001 // called when embedding begins (return value ignored)
#define REAPER_FXEMBED_WM_DESTROY                      0x0002 // called when embedding ends (return value ignored)



typedef struct REAPER_FXEMBED_DrawInfo // alias of REAPER_inline_positioninfo
{
  int context;        // 0=unknown (v6.23 and earlier), 1=TCP, 2=MCP
  int dpi;            // 0=unknown (v6.23 and earlier), otherwise 24.8 fixed point (256=100%)
  int mousewheel_amt; // for REAPER_FXEMBED_WM_MOUSEWHEEL, 120 = step, typically
  double _res2;

  int width, height;
  int mouse_x, mouse_y;

  int flags; // REAPER_FXEMBED_DRAWINFO_FLAG_PAINT_OPTIONAL etc
  int _res3;

  void *spare[6];
} REAPER_FXEMBED_DrawInfo;

#define REAPER_FXEMBED_DRAWINFO_FLAG_PAINT_OPTIONAL 1
#define REAPER_FXEMBED_DRAWINFO_FLAG_LBUTTON_CAPTURED 0x10000
#define REAPER_FXEMBED_DRAWINFO_FLAG_RBUTTON_CAPTURED 0x20000

#define REAPER_FXEMBED_WM_PAINT                        0x000F
/*
 * draw embedded UI.
 * parm2: REAPER_FXEMBED_IBitmap * to draw into. note
 * parm3: REAPER_FXEMBED_DrawInfo *
 *
 * if flags has REAPER_FXEMBED_DRAWINFO_FLAG_PAINT_OPTIONAL set, update is optional. if no change since last draw, return 0.
 * if flags has REAPER_FXEMBED_DRAWINFO_FLAG_LBUTTON_CAPTURED set, left mouse button is down and captured
 * if flags has REAPER_FXEMBED_DRAWINFO_FLAG_RBUTTON_CAPTURED set, right mouse button is down and captured
 *
 * HiDPI:
 * if REAPER_FXEMBED_IBitmap::Extended(REAPER_FXEMBED_EXT_GET_ADVISORY_SCALING,NULL) returns nonzero, then it is a 24.8 scalefactor for UI drawing
 *
 * return 1 if drawing occurred, 0 otherwise.
 *
 */

#define REAPER_FXEMBED_WM_SETCURSOR                    0x0020 // parm3: REAPER_FXEMBED_DrawInfo*. set mouse cursor and return REAPER_FXEMBED_RETNOTIFY_HANDLED, or return 0.

#define REAPER_FXEMBED_WM_GETMINMAXINFO                0x0024
/*
 * get size hints. parm3 = (REAPER_FXEMBED_SizeHints*). return 1 if supported
 * note that these are just hints, the actual size may vary
 */
typedef struct REAPER_FXEMBED_SizeHints { // alias to MINMAXINFO
  int preferred_aspect; // 16.16 fixed point (65536 = 1:1, 32768 = 1:2, etc)
  int minimum_aspect;   // 16.16 fixed point

  int _res1, _res2, _res3, _res4;

  int min_width, min_height;
  int max_width, max_height;
} REAPER_FXEMBED_SizeHints;

/*
 * mouse messages
 * parm3 = (REAPER_FXEMBED_DrawInfo*)
 * capture is automatically set on mouse down, released on mouse up
 * when not captured, will always receive a mousemove when exiting the window
 */

#define REAPER_FXEMBED_WM_MOUSEMOVE                    0x0200
#define REAPER_FXEMBED_WM_LBUTTONDOWN                  0x0201
#define REAPER_FXEMBED_WM_LBUTTONUP                    0x0202
#define REAPER_FXEMBED_WM_LBUTTONDBLCLK                0x0203
#define REAPER_FXEMBED_WM_RBUTTONDOWN                  0x0204
#define REAPER_FXEMBED_WM_RBUTTONUP                    0x0205
#define REAPER_FXEMBED_WM_RBUTTONDBLCLK                0x0206
#define REAPER_FXEMBED_WM_MOUSEWHEEL                   0x020A


/* REAPER_FXEMBED_WM_SETCURSOR should return REAPER_FXEMBED_RETNOTIFY_HANDLED if a cursor was set
 */
#define REAPER_FXEMBED_RETNOTIFY_HANDLED    0x0000001

/* if the mouse messages return with REAPER_FXEMBED_RETNOTIFY_INVALIDATE set, a non-optional
 * redraw is initiated (generally sooner than the next timer-based redraw)
 */
#define REAPER_FXEMBED_RETNOTIFY_INVALIDATE 0x1000000

/*
 * bitmap interface
 * this is an alias of LICE_IBitmap etc from WDL/lice/lice.h
 *
 */
#define REAPER_FXEMBED_RGBA(r,g,b,a) (((b)&0xff)|(((g)&0xff)<<8)|(((r)&0xff)<<16)|(((a)&0xff)<<24))
#define REAPER_FXEMBED_GETB(v) ((v)&0xff)
#define REAPER_FXEMBED_GETG(v) (((v)>>8)&0xff)
#define REAPER_FXEMBED_GETR(v) (((v)>>16)&0xff)
#define REAPER_FXEMBED_GETA(v) (((v)>>24)&0xff)

#ifdef __cplusplus
class REAPER_FXEMBED_IBitmap // alias of LICE_IBitmap
{
public:
  virtual ~REAPER_FXEMBED_IBitmap() { }

  virtual unsigned int *getBits()=0;
  virtual int getWidth()=0;
  virtual int getHeight()=0;
  virtual int getRowSpan()=0; // includes any off-bitmap data. this is in sizeof(unsigned int) units, not bytes.
  virtual bool isFlipped() { return false;  }
  virtual bool resize(int w, int h)=0;

  virtual void *getDC() { return 0; } // do not use

  virtual INT_PTR Extended(int id, void* data) { return 0; }
};
#endif

#define REAPER_FXEMBED_EXT_GET_ADVISORY_SCALING 0x2003 // data ignored, returns .8 fixed point. returns 0 if unscaled

#endif
