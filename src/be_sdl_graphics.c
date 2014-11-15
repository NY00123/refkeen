#include "SDL.h"
#include "be_cross.h"
#include "be_sdl.h"

/*static*/ SDL_Window *g_sdlWindow;
static SDL_Renderer *g_sdlRenderer;
static SDL_Texture *g_sdlTexture, *g_sdlTargetTexture;
static SDL_Rect g_sdlAspectCorrectionRect, g_sdlAspectCorrectionBorderedRect;

static bool g_sdlDoRefreshGfxOutput;

void BE_SDL_MarkGfxForUpdate(void)
{
	g_sdlDoRefreshGfxOutput = true;
}

#if !SDL_VERSION_ATLEAST(2,0,0)
#error "SDL <2.0 support is unimplemented!"
#endif

#define GFX_TEX_WIDTH 320
#define GFX_TEX_HEIGHT 200
#define VGA_TXT_TEX_WIDTH 720
#define VGA_TXT_TEX_HEIGHT 400
//#define EGACGA_TXT_TEX_WIDTH 640
//#define EGACGA_TXT_TEX_HEIGHT 200

// Overscan border dimensions (for each side of the screen)
#define ENGINE_VGA_GFX_OVERSCAN_LEFT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_GFX_OVERSCAN_RIGHT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING 7    // 200-line doubling
#define ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING 7 // 200-line doubling
#define ENGINE_VGA_TXT_OVERSCAN_LEFT 9
#define ENGINE_VGA_TXT_OVERSCAN_RIGHT 9
#define ENGINE_VGA_TXT_OVERSCAN_TOP 7
#define ENGINE_VGA_TXT_OVERSCAN_BOTTOM 7

#define TXT_COLS_NUM 80
#define TXT_ROWS_NUM 25

#define VGA_TXT_CHAR_PIX_WIDTH (VGA_TXT_TEX_WIDTH/TXT_COLS_NUM)
#define VGA_TXT_CHAR_PIX_HEIGHT (VGA_TXT_TEX_HEIGHT/TXT_ROWS_NUM)

#define VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE 8
#define VGA_TXT_BLINK_VERT_FRAME_RATE 16


extern uint8_t g_vga_8x16TextFont[256*VGA_TXT_CHAR_PIX_WIDTH*VGA_TXT_CHAR_PIX_HEIGHT];
// We can use a union because the memory contents are refreshed on screen mode change
// (well, not on change between modes 0xD and 0xE, both sharing planar A000:0000)
static union {
	uint8_t egaGfx[4][0x10000]; // Contents of A000:0000 (4 planes)
	uint8_t text[TXT_COLS_NUM*TXT_ROWS_NUM*2]; // Textual contents of B800:0000
} g_sdlVidMem;

// Used for simple caching of EGA graphics (due to page flipping and more)
// and similarly CGA graphics (modified only at one place)
static union {
	uint8_t egaGfx[2*320*200]; // Support 640x200 mode for Catacomb Abyss
	uint8_t cgaGfx[320*200];
} g_sdlHostScrMem, g_sdlHostScrMemCache;

static uint16_t g_sdlScreenStartAddress = 0, g_sdlScreenStartAddressCache;
static int g_sdlScreenMode = 3;
static int g_sdlTexWidth, g_sdlTexHeight;
static uint8_t g_sdlPelPanning = 0, g_sdlPelPanningCache;
static uint8_t g_sdlLineWidth = 40, g_sdlLineWidthCache;
static int16_t g_sdlSplitScreenLine = -1, g_sdlSplitScreenLineCache;
static int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
static bool g_sdlTxtCursorEnabled = true;
static int g_sdlTxtColor = 7, g_sdlTxtBackground = 0;

void BE_SDL_SetGfxOutputRects(void);

void BE_SDL_InitGfx(void)
{
#ifdef CHOCO_KEEN_VER_KDREAMS
	const char *windowTitle = "Ref Keen Dreams";
#elif defined CHOCO_KEEN_VER_CATABYSS
	const char *windowTitle = "Ref Catacomb Abyss";
#else
#error "FATAL ERROR: No Ref port game macro is defined!"
#endif
	if (g_refKeenCfg.isFullscreen)
	{
		if (g_refKeenCfg.fullWidth && g_refKeenCfg.fullHeight)
		{
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight, SDL_WINDOW_FULLSCREEN);
		}
		else
		{
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}
	else
	{
		int actualWinWidth = g_refKeenCfg.winWidth, actualWinHeight = g_refKeenCfg.winHeight;
		if (!actualWinWidth || !actualWinHeight)
		{
			bool doSoftwareRendering;
			if (g_refKeenCfg.sdlRendererDriver >= 0)
			{
				SDL_RendererInfo info;
				SDL_GetRenderDriverInfo(g_refKeenCfg.sdlRendererDriver, &info);
				doSoftwareRendering = (info.flags & SDL_RENDERER_SOFTWARE);
			}
			else
			{
				doSoftwareRendering = false;
			}
			if (doSoftwareRendering)
			{
				actualWinWidth = 640;
				actualWinHeight = 480;
			}
			else
			{
				SDL_DisplayMode mode;
				SDL_GetDesktopDisplayMode(g_refKeenCfg.displayNum, &mode);
				// In the 200-lines modes on the VGA, where line doubling is in effect,
				// and after adding the overscan borders, the aspect ratio for the whole output
				// (after aspect correction i.e., multiplying height by 1.2) is 280:207.
				if (207*mode.w < 280*mode.h) // Thinner than 280:207
				{
					mode.h = mode.w*207/280;
				}
				else  // As wide as 280:207 at the least
				{
					mode.w = mode.h*280/207;
				}
				// Just for the sake of it, using the golden ratio...
				actualWinWidth = mode.w*500/809;
				actualWinHeight = mode.h*500/809;
			}
		}
		g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), actualWinWidth, actualWinHeight, SDL_WINDOW_RESIZABLE);
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}
#if GRMODE == EGAGR
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_refKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | ((g_refKeenCfg.vSync == VSYNC_OFF) ? 0 : SDL_RENDERER_PRESENTVSYNC));
#elif GRMODE == CGAGR
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_refKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | ((g_refKeenCfg.vSync == VSYNC_ON) ? SDL_RENDERER_PRESENTVSYNC : 0));
#else
#error "Supported GRMODE not defined!"
#endif
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		//Destroy window?
		exit(0);
	}
	BE_SDL_SetScreenMode(3); // Includes SDL_Texture handling and output rects preparation
}

void BE_SDL_ShutdownGfx(void)
{
	SDL_DestroyTexture(g_sdlTexture);
	g_sdlTexture = NULL;
	SDL_DestroyTexture(g_sdlTargetTexture);
	g_sdlTargetTexture = NULL;
	SDL_DestroyRenderer(g_sdlRenderer);
	g_sdlRenderer = NULL;
	SDL_DestroyWindow(g_sdlWindow);
	g_sdlWindow = NULL;
}

static void BEL_SDL_RecreateTexture(void)
{
	if (g_sdlTexture)
	{
		SDL_DestroyTexture(g_sdlTexture);
	}
	if (g_sdlTargetTexture)
	{
		SDL_DestroyTexture(g_sdlTargetTexture);
	}
	// Try using render target
	if ((g_refKeenCfg.scaleFactor > 1) && g_refKeenCfg.isBilinear)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
		// Try, if we fail then simply don't use this
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		g_sdlTargetTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, g_sdlTexWidth*g_refKeenCfg.scaleFactor, g_sdlTexHeight*g_refKeenCfg.scaleFactor);
	}
	else
	{
		// Use just a single texture
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_refKeenCfg.isBilinear ? "linear" : "nearest");
		g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
	}
}

void BE_SDL_SetGfxOutputRects(void)
{
	int srcWidth = g_sdlTexWidth;
	int srcHeight = g_sdlTexHeight;
	if (g_sdlScreenMode != 3)
	{
		// On the VGA, line doubling is in effect for graphical 200 lines modes
		if (g_sdlScreenMode != 0xe) // Is source 320x200 (not 640x200)?
			srcWidth *= 2;
		srcHeight *= 2;
	}
	int srcBorderLeft, srcBorderRight, srcBorderTop, srcBorderBottom;
	if (g_sdlScreenMode == 3)
	{
		srcBorderLeft = ENGINE_VGA_TXT_OVERSCAN_LEFT;
		srcBorderRight = ENGINE_VGA_TXT_OVERSCAN_RIGHT;
		srcBorderTop = ENGINE_VGA_TXT_OVERSCAN_TOP;
		srcBorderBottom = ENGINE_VGA_TXT_OVERSCAN_BOTTOM;
	}
	else
	{
		srcBorderLeft = ENGINE_VGA_GFX_OVERSCAN_LEFT_AFTER_DOUBLING;
		srcBorderRight = ENGINE_VGA_GFX_OVERSCAN_RIGHT_AFTER_DOUBLING;
		srcBorderTop = ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING;
		srcBorderBottom = ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING;
	}
	int srcBorderedWidth = srcBorderLeft+srcWidth+srcBorderRight;
	int srcBorderedHeight = srcBorderTop+srcHeight+srcBorderBottom;
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
	if (g_refKeenCfg.scaleType == SCALE_FILL)
	{
		g_sdlAspectCorrectionBorderedRect.w = winWidth;
		g_sdlAspectCorrectionBorderedRect.h = winHeight;
		g_sdlAspectCorrectionBorderedRect.x = 0;
		g_sdlAspectCorrectionBorderedRect.y = 0;
	}
	else
	{
		// Aspect correct - Includes overscan borders, should multiply
		// the height by 1.2 first (or the width by 5 and height 6),
		// if in 320x200 gfx mode, cause 1.2 == (320*3)/(4*200).
		// More generally, (srcwidth*3)/(4*srcheight).
		// So multiply width by 4*srcheight and height by 3*srcwidth;
		int scaledSrcBorderedWidth = (4*srcHeight)*srcBorderedWidth, scaledSrcBorderedHeight = (3*srcWidth)*srcBorderedHeight;
		if (scaledSrcBorderedHeight*winWidth < scaledSrcBorderedWidth*winHeight) // Thinner than scaledSrcBorderedWidth:scaledSrcBorderedHeight
		{
			g_sdlAspectCorrectionBorderedRect.w = winWidth;
			g_sdlAspectCorrectionBorderedRect.h = winWidth*scaledSrcBorderedHeight/scaledSrcBorderedWidth;
			g_sdlAspectCorrectionBorderedRect.x = 0;
			g_sdlAspectCorrectionBorderedRect.y = (winHeight-g_sdlAspectCorrectionBorderedRect.h)/2;
		}
		else // As wide as scaledSrcBorderedWidth:scaledSrcBorderedHeight at the least
		{
			g_sdlAspectCorrectionBorderedRect.w = winHeight*scaledSrcBorderedWidth/scaledSrcBorderedHeight;
			g_sdlAspectCorrectionBorderedRect.h = winHeight;
			g_sdlAspectCorrectionBorderedRect.x = (winWidth-g_sdlAspectCorrectionBorderedRect.w)/2;
			g_sdlAspectCorrectionBorderedRect.y = 0;
		}
	}
	// Finish with internal (non-bordered) rect
	g_sdlAspectCorrectionRect.x = g_sdlAspectCorrectionBorderedRect.x + g_sdlAspectCorrectionBorderedRect.w*srcBorderLeft/srcBorderedWidth;
	g_sdlAspectCorrectionRect.y = g_sdlAspectCorrectionBorderedRect.y + g_sdlAspectCorrectionBorderedRect.h*srcBorderTop/srcBorderedHeight;
	g_sdlAspectCorrectionRect.w = g_sdlAspectCorrectionBorderedRect.w*srcWidth/srcBorderedWidth;
	g_sdlAspectCorrectionRect.h = g_sdlAspectCorrectionBorderedRect.h*srcHeight/srcBorderedHeight;
}

void BE_SDL_SetScreenStartAddress(uint16_t crtc)
{
	g_sdlScreenStartAddress = crtc;
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t *BE_SDL_GetTextModeMemoryPtr(void)
{
	return g_sdlVidMem.text;
}




// Colors in BGRA format/order (on certain platforms)

static const uint32_t g_sdlCGAGfxBGRAScreenColors[] = {
	0xff000000/*black*/,
	0xff00ffff/*light cyan*/,
	0xffff00ff/*light magenta*/,
	0xffffffff/*white*/
};

// Same but for the EGA/VGA (and colored text modes on CGA/EGA/VGA)

static const uint32_t g_sdlEGABGRAScreenColors[] = {
	0xff000000/*black*/, 0xff0000aa/*blue*/, 0xff00aa00/*green*/, 0xff00aaaa/*cyan*/,
	0xffaa0000/*red*/, 0xffaa00aa/*magenta*/, 0xffaa5500/*brown*/, 0xffaaaaaa/*light gray*/,
	0xff555555/*gray*/, 0xff5555ff/*light blue*/, 0xff55ff55/*light green*/, 0xff55ffff/*light cyan*/,
	0xffff5555/*light red*/, 0xffff55ff/*light magenta*/, 0xffffff55/*yellow*/, 0xffffffff/*white*/
};

static uint32_t g_sdlEGACurrBGRAPaletteAndBorder[17], g_sdlEGACurrBGRAPaletteAndBorderCache[17];

/* Gets a value represeting 6 EGA signals determining a color number and
 * returns it in a "Blue Green Red Intensity" 4-bit format.
 * Usually, the 6 signals represented by the given input mean:
 * "Blue Green Red Secondary-Blue Secondary-Green Secondary-Red". However, for
 * the historical reason of compatibility with CGA monitors, on the 200-lines
 * modes used by Keen the Secondary-Green signal is treated as an Intensity
 * one and the two other intensity signals are ignored.
 */
static int BEL_SDL_ConvertEGASignalToEGAEntry(int color)
{
	return (color & 7) | ((color & 16) >> 1);
}


void BE_SDL_SetBorderColor(uint8_t color)
{
	g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[BEL_SDL_ConvertEGASignalToEGAEntry(color)];
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetPaletteAndBorder(const uint8_t *palette)
{
	for (int entry = 0; entry < 16; ++entry)
	{
		g_sdlEGACurrBGRAPaletteAndBorder[entry] =  g_sdlEGABGRAScreenColors[BEL_SDL_ConvertEGASignalToEGAEntry(palette[entry])];
	}
	g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[BEL_SDL_ConvertEGASignalToEGAEntry(palette[16])];
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_SetPelPanning(uint8_t panning)
{
	g_sdlPelPanning = panning;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetLineWidth(uint8_t widthInBytes)
{
	g_sdlLineWidth = widthInBytes;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetSplitScreen(int16_t linenum)
{
	// VGA only for now (200-lines graphics modes)
	if (g_sdlTexHeight == GFX_TEX_HEIGHT)
	{
		// Because 200-lines modes are really double-scanned to 400,
		// a linenum of x was originally replaced with 2x-1 in id_vw.c.
		// In practice it should've probably been 2x+1, and this is how
		// we "correct" it here (one less black line in Catacomb Abyss
		// before gameplay begins in a map, above the status bar).
		g_sdlSplitScreenLine = linenum/2;
	}
	else
		g_sdlSplitScreenLine = linenum;
}

void BE_SDL_EGAUpdateGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t mask)
{
	if (mask & 1)
		g_sdlVidMem.egaGfx[0][destOff] = srcVal;
	if (mask & 2)
		g_sdlVidMem.egaGfx[1][destOff] = srcVal;
	if (mask & 4)
		g_sdlVidMem.egaGfx[2][destOff] = srcVal;
	if (mask & 8)
		g_sdlVidMem.egaGfx[3][destOff] = srcVal;
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_SDL_EGAUpdateGFXByte but picking specific bits out of each byte, and WITHOUT planes mask
void BE_SDL_EGAUpdateGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] = (g_sdlVidMem.egaGfx[0][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] = (g_sdlVidMem.egaGfx[1][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] = (g_sdlVidMem.egaGfx[2][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] = (g_sdlVidMem.egaGfx[3][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_LinearToWrapped_MemCopy
static void BEL_SDL_LinearToEGAPlane_MemCopy(uint8_t *planeDstPtr, uint16_t planeDstOff, const uint8_t *linearSrc, uint16_t num)
{
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		memcpy(planeDstPtr+planeDstOff, linearSrc, num);
	}
	else
	{
		memcpy(planeDstPtr+planeDstOff, linearSrc, bytesToEnd);
		memcpy(planeDstPtr, linearSrc+bytesToEnd, num-bytesToEnd);
	}
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToLinear_MemCopy
static void BEL_SDL_EGAPlaneToLinear_MemCopy(uint8_t *linearDst, const uint8_t *planeSrcPtr, uint16_t planeSrcOff, uint16_t num)
{
	uint16_t bytesToEnd = 0x10000-planeSrcOff;
	if (num <= bytesToEnd)
	{
		memcpy(linearDst, planeSrcPtr+planeSrcOff, num);
	}
	else
	{
		memcpy(linearDst, planeSrcPtr+planeSrcOff, bytesToEnd);
		memcpy(linearDst+bytesToEnd, planeSrcPtr, num-bytesToEnd);
	}
	//No need to since we just read screen data
	//g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToWrapped_MemCopy
static void BEL_SDL_EGAPlaneToEGAPlane_MemCopy(uint8_t *planeCommonPtr, uint16_t planeDstOff, uint16_t planeSrcOff, uint16_t num)
{
	uint16_t srcBytesToEnd = 0x10000-planeSrcOff;
	uint16_t dstBytesToEnd = 0x10000-planeDstOff;
	if (num <= srcBytesToEnd)
	{
		// Source is linear: Same as BE_Cross_LinearToWrapped_MemCopy here
		if (num <= dstBytesToEnd)
		{
			memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, num);
		}
		else
		{
			memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, dstBytesToEnd);
			memcpy(planeCommonPtr, planeCommonPtr+planeSrcOff+dstBytesToEnd, num-dstBytesToEnd);
		}
	}
	// Otherwise, check if at least the destination is linear
	else if (num <= dstBytesToEnd)
	{
		// Destination is linear: Same as BE_Cross_WrappedToLinear_MemCopy, non-linear source
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, srcBytesToEnd);
		memcpy(planeCommonPtr+planeDstOff+srcBytesToEnd, planeCommonPtr, num-srcBytesToEnd);
	}
	// BOTH buffers have wrapping. We don't check separately if
	// srcBytesToEnd==dstBytesToEnd (in such a case planeDstOff==planeSrcOff...)
	else if (srcBytesToEnd <= dstBytesToEnd)
	{
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, srcBytesToEnd);
		memcpy(planeCommonPtr+planeDstOff+srcBytesToEnd, planeCommonPtr, dstBytesToEnd-srcBytesToEnd);
		memcpy(planeCommonPtr, planeCommonPtr+(dstBytesToEnd-srcBytesToEnd), num-dstBytesToEnd);
	}
	else // srcBytesToEnd > dstBytesToEnd
	{
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, dstBytesToEnd);
		memcpy(planeCommonPtr, planeCommonPtr+planeSrcOff+dstBytesToEnd, srcBytesToEnd-dstBytesToEnd);
		memcpy(planeCommonPtr+(srcBytesToEnd-dstBytesToEnd), planeCommonPtr, num-srcBytesToEnd);
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXBuffer(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t mask)
{
	if (mask & 1)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[0], destOff, srcPtr, num);
	if (mask & 2)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[1], destOff, srcPtr, num);
	if (mask & 4)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[2], destOff, srcPtr, num);
	if (mask & 8)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[3], destOff, srcPtr, num);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXByteScrToScr(uint16_t destOff, uint16_t srcOff)
{
	g_sdlVidMem.egaGfx[0][destOff] = g_sdlVidMem.egaGfx[0][srcOff];
	g_sdlVidMem.egaGfx[1][destOff] = g_sdlVidMem.egaGfx[1][srcOff];
	g_sdlVidMem.egaGfx[2][destOff] = g_sdlVidMem.egaGfx[2][srcOff];
	g_sdlVidMem.egaGfx[3][destOff] = g_sdlVidMem.egaGfx[3][srcOff];
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_SDL_EGAUpdateGFXByteScrToScr but picking specific bits out of each byte
void BE_SDL_EGAUpdateGFXBitsScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] = (g_sdlVidMem.egaGfx[0][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[0][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] = (g_sdlVidMem.egaGfx[1][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[1][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] = (g_sdlVidMem.egaGfx[2][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[2][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] = (g_sdlVidMem.egaGfx[3][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[3][srcOff] & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXBufferScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[0], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[1], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[2], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[3], destOff, srcOff, num);
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t BE_SDL_EGAFetchGFXByte(uint16_t destOff, uint16_t planenum)
{
	return g_sdlVidMem.egaGfx[planenum][destOff];
}

void BE_SDL_EGAFetchGFXBuffer(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum)
{
	BEL_SDL_EGAPlaneToLinear_MemCopy(destPtr, g_sdlVidMem.egaGfx[planenum], srcOff, num);
}

void BE_SDL_EGAUpdateGFXPixel4bpp(uint16_t destOff, uint8_t color, uint16_t mask)
{
	for (int currBitNum = 0, currBitMask = 1; currBitNum < 8; ++currBitNum, currBitMask <<= 1)
	{
		if (mask & currBitMask)
		{
			g_sdlVidMem.egaGfx[0][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[0][destOff] |= ((color & 1) << currBitNum);
			g_sdlVidMem.egaGfx[1][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[1][destOff] |= (((color & 2) >> 1) << currBitNum);
			g_sdlVidMem.egaGfx[2][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[2][destOff] |= (((color & 4) >> 2) << currBitNum);
			g_sdlVidMem.egaGfx[3][destOff] &= ~currBitMask;
			g_sdlVidMem.egaGfx[3][destOff] |= (((color & 8) >> 3) << currBitNum);
		}
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXPixel4bppRepeatedly(uint16_t destOff, uint8_t color, uint16_t count, uint16_t mask)
{
	for (uint16_t loopVar = 0; loopVar < count; ++loopVar, ++destOff)
	{
		BE_SDL_EGAUpdateGFXPixel4bpp(destOff, color, mask);
	}
}

void BE_SDL_EGAXorGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t mask)
{
	if (mask & 1)
		g_sdlVidMem.egaGfx[0][destOff] ^= srcVal;
	if (mask & 2)
		g_sdlVidMem.egaGfx[1][destOff] ^= srcVal;
	if (mask & 4)
		g_sdlVidMem.egaGfx[2][destOff] ^= srcVal;
	if (mask & 8)
		g_sdlVidMem.egaGfx[3][destOff] ^= srcVal;
	g_sdlDoRefreshGfxOutput = true;
}

// Like BE_SDL_EGAXorGFXByte, but:
// - OR instead of XOR.
// - Only specific bits are updated in each plane's byte.
void BE_SDL_EGAOrGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] |= (srcVal & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}



void BE_SDL_CGAFullUpdateFromWrappedMem(const uint8_t *segPtr, const uint8_t *offInSegPtr, uint16_t byteLineWidth)
{
	const uint8_t *endSegPtr = segPtr + 0x10000;
	uint8_t *cgaHostPtr = g_sdlHostScrMem.cgaGfx, *cgaHostCachePtr = g_sdlHostScrMemCache.cgaGfx;
	// Remember this is a wrapped copy, offInSegPtr points inside a 65536-bytes long segment beginning at segPtr;
	// But we still want to skip some bytes, assuming byteLineWidth sources bytes per line (picking 80 out of these)

	int lineBytesLeft = byteLineWidth - GFX_TEX_WIDTH/4;
	for (int line = 0, byteInLine; line < GFX_TEX_HEIGHT; ++line)
	{
		for (int byteInLine = 0; byteInLine < GFX_TEX_WIDTH/4; ++byteInLine, ++offInSegPtr, offInSegPtr = (offInSegPtr == endSegPtr) ? segPtr : offInSegPtr)
		{
			*cgaHostPtr = ((*offInSegPtr) & 0xC0) >> 6;
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
			*cgaHostPtr = ((*offInSegPtr) & 0x30) >> 4;
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
			*cgaHostPtr = ((*offInSegPtr) & 0x0C) >> 2;
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
			*cgaHostPtr = ((*offInSegPtr) & 0x03);
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
		}
		offInSegPtr += lineBytesLeft;
		if (offInSegPtr >= endSegPtr)
		{
			offInSegPtr = segPtr + (uint16_t)(offInSegPtr-segPtr);
		}
	}
}


void BE_SDL_SetScreenMode(int mode)
{
	g_sdlDoRefreshGfxOutput = true;
	switch (mode)
	{
	case 3:
		g_sdlTexWidth = VGA_TXT_TEX_WIDTH;
		g_sdlTexHeight = VGA_TXT_TEX_HEIGHT;
		g_sdlTxtColor = 7;
		g_sdlTxtBackground = 0;
		g_sdlTxtCursorPosX = g_sdlTxtCursorPosY = 0;
		BE_SDL_clrscr();
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		break;
	case 4:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memset(g_sdlHostScrMem.cgaGfx, 0, sizeof(g_sdlHostScrMem.cgaGfx));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlHostScrMemCache.cgaGfx[0] = g_sdlHostScrMem.cgaGfx[0]^0xFF; // Force refresh
		break;
	case 0xD:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPaletteAndBorder, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlPelPanning = 0;
		g_sdlLineWidth = 40;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if (g_sdlScreenMode != 0xE)
		{
			memset(g_sdlVidMem.egaGfx, 0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx, 0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	case 0xE:
		g_sdlTexWidth = 2*GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPaletteAndBorder, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlPelPanning = 0;
		g_sdlLineWidth = 80;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if (g_sdlScreenMode != 0xD)
		{
			memset(g_sdlVidMem.egaGfx,  0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx,  0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	}
	g_sdlScreenMode = mode;
	BE_SDL_SetGfxOutputRects();
	BEL_SDL_RecreateTexture();
}

void BE_SDL_textcolor(int color)
{
	g_sdlTxtColor = color;
}

void BE_SDL_textbackground(int color)
{
	g_sdlTxtBackground = color;
}

void BE_SDL_clrscr(void)
{
	uint8_t *currMemByte = g_sdlVidMem.text;
	for (int i = 0; i < 2*TXT_COLS_NUM*TXT_ROWS_NUM; ++i)
	{
		*(currMemByte++) = ' ';
		*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_MoveTextCursorTo(int x, int y)
{
	g_sdlTxtCursorPosX = x;
	g_sdlTxtCursorPosY = y;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_ToggleTextCursor(bool isEnabled)
{
	g_sdlTxtCursorEnabled = isEnabled;
}

static void BEL_SDL_Simplified_putsorprintf(const char *str, bool isprintfcall)
{
	// TODO (CHOCO KEEN): Tabs?
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	for (; *str; ++str)
	{
		if (*str == '\n')
		{
			g_sdlTxtCursorPosX = 0; // Carriage return
			++g_sdlTxtCursorPosY; // Line feed
			currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
		}
		else
		{
			if ((*str == '%') && isprintfcall)
			{
				++str; // This is still a SIMPLIFIED printf...
			}
			*(currMemByte++) = *str;
			*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
			if (g_sdlTxtCursorPosX == TXT_COLS_NUM - 1)
			{
				g_sdlTxtCursorPosX = 0; // Carriage return
				++g_sdlTxtCursorPosY; // Line feed
				currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
			}
			else
			{
				++g_sdlTxtCursorPosX;
			}
		}
		if (g_sdlTxtCursorPosY == TXT_ROWS_NUM)
		{
			--g_sdlTxtCursorPosY;
			// Scroll one line down
			memmove(g_sdlVidMem.text, g_sdlVidMem.text+2*TXT_COLS_NUM, sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM);
			currMemByte = g_sdlVidMem.text+sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM;
			// New empty line
			for (int i = 0; i < TXT_COLS_NUM; ++i)
			{
				*(currMemByte++) = ' ';
				*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
			}
			currMemByte -= 2*TXT_COLS_NUM; // Go back to beginning of line
		}
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_Simplified_printf(const char *str)
{
	BEL_SDL_Simplified_putsorprintf(str, true);
}

void BE_SDL_puts(const char *str)
{
	BEL_SDL_Simplified_putsorprintf(str, false);
	BEL_SDL_Simplified_putsorprintf("\n", false);
}

void BE_SDL_UpdateHostDisplay(void);

void BE_SDL_WaitVBL(int16_t number)
{
	// TODO (CHOCO KEEN) Make a difference based on HW?
	Uint32 startTicks = SDL_GetTicks();
	if (SDL_GetTicks() - startTicks < (int)number*1000000/70086)
	{
		SDL_Delay(1);
		BE_SDL_UpdateHostDisplay();
		BE_SDL_PollEvents();
	}
	while (SDL_GetTicks() - startTicks < (int)number*1000000/70086)
	{
		SDL_Delay(1);
		BE_SDL_PollEvents();
	}
}

// Call during a busy loop of some unknown duration (e.g., waiting for key press/release)
void BE_SDL_ShortSleep(void)
{
	SDL_Delay(1);
	// TODO: Make this more efficient
	BE_SDL_UpdateHostDisplay();
	BE_SDL_PollEvents();
}

// Use this ONLY in Catacombs' CalcTics (from ThreeDRefresh)
void BE_SDL_ThreeDRefreshSleep(void)
{
	SDL_Delay(15);
	// TODO: Make this more efficient?
	BE_SDL_UpdateHostDisplay();
	BE_SDL_PollEvents();
}

static uint64_t g_sdlLastFizzleTimeInMicroSec;

void BE_SDL_FizzleFadeSleepInit(void)
{
	g_sdlLastFizzleTimeInMicroSec = 1000*(uint64_t)SDL_GetTicks();
}

// Use this ONLY in Catacombs' FizzleFade (each internal loop iteration)
void BE_SDL_FizzleFadeSleep(void)
{
	const int64_t fizzleIterationInMicroSSec = 40; // For a few pixels

	if ((int32_t)(1000*(uint64_t)SDL_GetTicks() - g_sdlLastFizzleTimeInMicroSec) < fizzleIterationInMicroSSec)
	{
		BE_SDL_UpdateHostDisplay();
		while ((int64_t)(1000*(uint64_t)SDL_GetTicks() - g_sdlLastFizzleTimeInMicroSec) < fizzleIterationInMicroSSec)
		{
			SDL_Delay(1);
		}
		BE_SDL_PollEvents();
	}
	g_sdlLastFizzleTimeInMicroSec += fizzleIterationInMicroSSec;
}


void BE_SDL_Delay(uint16_t msec) // Replacement for delay from dos.h
{
	uint32_t endTime = SDL_GetTicks() + msec;
	while ((int32_t)(SDL_GetTicks() - endTime) < 0)
	{
		BE_SDL_ShortSleep();
	}
}




void BE_SDL_UpdateHostDisplay(void)
{
	if (g_sdlScreenMode == 3) // Text mode
	{
		// For graphics modes we don't have to refresh if g_sdlDoRefreshGfxOutput is set to false,
		// but in textual mode we have blinking characters and cursor to take care of
		static bool wereBlinkingCharsShown;
		static bool wasBlinkingCursorShown;
		bool areBlinkingCharsShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_BLINK_VERT_FRAME_RATE)) % 2);
		bool isBlinkingCursorShown = g_sdlTxtCursorEnabled && (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE)) % 2);
		if (!g_sdlDoRefreshGfxOutput && (wereBlinkingCharsShown == areBlinkingCharsShown) && (wasBlinkingCursorShown == isBlinkingCursorShown))
		{
			return;
		}
		/****** Do update ******/
		wereBlinkingCharsShown = areBlinkingCharsShown;
		wasBlinkingCursorShown = isBlinkingCursorShown;
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *screenPixelPtr = (uint32_t *)pixels;
		uint8_t currChar;
		const uint8_t *currCharFontPtr;
		uint32_t *currScrPixelPtr, currBackgroundColor, currCharColor;
		int txtByteCounter = 0;
		int currCharPixX, currCharPixY;
		for (int currCharY = 0, currCharX; currCharY < TXT_ROWS_NUM; ++currCharY)
		{
			// Draw striped lines
			for (currCharX = 0; currCharX < TXT_COLS_NUM; ++currCharX)
			{
				currChar = g_sdlVidMem.text[txtByteCounter];
				// Luckily, the width*height product is always divisible by 8...
				// Note that the actual width is always 8,
				// even in VGA mode. We convert to 9 while drawing.
				currCharFontPtr = g_vga_8x16TextFont + currChar*16*8;
				++txtByteCounter;
				currBackgroundColor = g_sdlEGABGRAScreenColors[(g_sdlVidMem.text[txtByteCounter] >> 4) & 7];
				// Should the character blink?
				if (!(g_sdlVidMem.text[txtByteCounter] & 0x80) || areBlinkingCharsShown)
					currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[txtByteCounter] & 15];
				else
					currCharColor = currBackgroundColor;
				++txtByteCounter;
				currScrPixelPtr = screenPixelPtr;
				for (currCharPixY = 0; currCharPixY < VGA_TXT_CHAR_PIX_HEIGHT; ++currCharPixY)
				{
					/* NOTE: The char width is actually 8
					in both of the EGA and VGA fonts. On the
					VGA case, the 9th pixel is determined
					according to the 8th and char number. */
					for (currCharPixX = 0; currCharPixX < 8; ++currCharPixX, ++currCharFontPtr, ++currScrPixelPtr)
					{
						*currScrPixelPtr = (*currCharFontPtr) ? currCharColor : currBackgroundColor;
					}
					// Add an extra 9th column on VGA
					*currScrPixelPtr = ((currChar < 192) || (currChar > 223)) ? currBackgroundColor : *(currScrPixelPtr-1);
					currScrPixelPtr += (g_sdlTexWidth-VGA_TXT_CHAR_PIX_WIDTH+1);
				}
				screenPixelPtr += VGA_TXT_CHAR_PIX_WIDTH;
			}
			// Go to the character right below current one
			screenPixelPtr += g_sdlTexWidth*(VGA_TXT_CHAR_PIX_HEIGHT-1);
		}
		// Finish with outputting the cursor if required
		currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[1+((TXT_COLS_NUM*g_sdlTxtCursorPosY+g_sdlTxtCursorPosX)<<1)] & 15];
		if (isBlinkingCursorShown)
		{
			screenPixelPtr = (uint32_t *)pixels+g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosY*VGA_TXT_CHAR_PIX_HEIGHT*g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosX*VGA_TXT_CHAR_PIX_WIDTH;
			// Out of 3 last scanlines of char, draw to the first 2.
			screenPixelPtr += (VGA_TXT_CHAR_PIX_HEIGHT-3)*g_sdlTexWidth;
			for (currCharPixY = 0; currCharPixY < 2; currCharPixY++)
			{
				for (currCharPixX = 0; currCharPixX < VGA_TXT_CHAR_PIX_WIDTH; currCharPixX++, screenPixelPtr++)
					*screenPixelPtr = currCharColor;
				screenPixelPtr += g_sdlTexWidth - VGA_TXT_CHAR_PIX_WIDTH;
			}
		}
	}
	else if (g_sdlScreenMode == 4) // CGA graphics
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			return;
		}
		// That's easy now since there isn't a lot that can be done...
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr = (uint32_t *)pixels;
		uint8_t *currPalPixPtr = g_sdlHostScrMem.cgaGfx;
		for (int pixnum = 0; pixnum < GFX_TEX_WIDTH*GFX_TEX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
		{
			*currPixPtr = g_sdlCGAGfxBGRAScreenColors[*currPalPixPtr];
		}
	}
	else // EGA graphics mode 0xD or 0xE
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			return;
		}
		uint16_t currLineFirstByte = (g_sdlScreenStartAddress + g_sdlPelPanning/8) % 0x10000;
		uint8_t panningWithinInByte = g_sdlPelPanning%8;
		uint8_t *currPalPixPtr, *currPalPixCachePtr;
		bool doUpdate = false;
		for (int line = 0, col; line < GFX_TEX_HEIGHT; ++line)
		{
			uint8_t currBitNum = 7-panningWithinInByte, currBitMask = 1<<currBitNum;
			uint16_t currByte = currLineFirstByte;
			currPalPixPtr = g_sdlHostScrMem.egaGfx + line*g_sdlTexWidth;
			currPalPixCachePtr = g_sdlHostScrMemCache.egaGfx + line*g_sdlTexWidth;
			for (col = 0; col < g_sdlTexWidth; ++col, ++currPalPixPtr)
			{
				*currPalPixPtr = ((g_sdlVidMem.egaGfx[0][currByte]&currBitMask)>>currBitNum) |
				                 (((g_sdlVidMem.egaGfx[1][currByte]&currBitMask)>>currBitNum)<<1) |
				                 (((g_sdlVidMem.egaGfx[2][currByte]&currBitMask)>>currBitNum)<<2) |
				                 (((g_sdlVidMem.egaGfx[3][currByte]&currBitMask)>>currBitNum)<<3);
				doUpdate |= (*currPalPixPtr != *currPalPixCachePtr);
				*currPalPixCachePtr = *currPalPixPtr;
				if (currBitNum == 0)
				{
					++currByte;
					currByte %= 0x10000;
					currBitNum = 7;
					currBitMask = 0x80;
				}
				else
				{
					--currBitNum;
					currBitMask >>= 1;
				}
				if (col == 8*g_sdlLineWidth)
				{
					++col;
					++currPalPixPtr;
					++currPalPixCachePtr;
					break;
				}
			}
			// Just if this makes sense... (FIXME: Check!)
			for (; col < g_sdlTexWidth; ++col, ++currPalPixPtr, ++currPalPixCachePtr)
			{
				doUpdate |= (*currPalPixCachePtr);
				*currPalPixPtr = 0;
				*currPalPixCachePtr = 0;
			}
			if (g_sdlSplitScreenLine == line)
			{
				currLineFirstByte = 0; // NEXT line begins split screen, NOT g_sdlSplitScreenLine
			}
			else
			{
				currLineFirstByte += g_sdlLineWidth;
				currLineFirstByte %= 0x10000;
			}
		}
		if (!doUpdate)
		{
			int paletteAndBorderEntry;
			for (paletteAndBorderEntry = 0; paletteAndBorderEntry < 17; ++paletteAndBorderEntry)
			{
				if (g_sdlEGACurrBGRAPaletteAndBorder[paletteAndBorderEntry] != g_sdlEGACurrBGRAPaletteAndBorderCache[paletteAndBorderEntry])
				{
					g_sdlEGACurrBGRAPaletteAndBorderCache[paletteAndBorderEntry] = g_sdlEGACurrBGRAPaletteAndBorder[paletteAndBorderEntry];
					doUpdate = true;
				}
			}
			if (!doUpdate)
			{
				g_sdlDoRefreshGfxOutput = false;
				return;
			}
		}
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr = (uint32_t *)pixels;
		currPalPixPtr = g_sdlHostScrMem.egaGfx;
		for (int pixnum = 0; pixnum < g_sdlTexWidth*GFX_TEX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
		{
			*currPixPtr = g_sdlEGACurrBGRAPaletteAndBorder[*currPalPixPtr];
		}
	}
	g_sdlDoRefreshGfxOutput = false;
	SDL_UnlockTexture(g_sdlTexture);
	SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_sdlRenderer);
	SDL_SetRenderDrawColor(g_sdlRenderer, (g_sdlEGACurrBGRAPaletteAndBorder[16]>>16)&0xFF, (g_sdlEGACurrBGRAPaletteAndBorder[16]>>8)&0xFF, g_sdlEGACurrBGRAPaletteAndBorder[16]&0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(g_sdlRenderer, &g_sdlAspectCorrectionBorderedRect);
	if (g_sdlTargetTexture)
	{
		SDL_SetRenderTarget(g_sdlRenderer, g_sdlTargetTexture);
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, NULL);
		SDL_SetRenderTarget(g_sdlRenderer, NULL);
		SDL_RenderCopy(g_sdlRenderer, g_sdlTargetTexture, NULL, &g_sdlAspectCorrectionRect);
	}
	else
	{
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionRect);
	}
        SDL_RenderPresent(g_sdlRenderer);
}