/* Copyright (C) 2014 NY00123
 *
 * This file is part of Chocolate Keen Dreams.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "SDL.h"
#include "id_heads.h"

/*static*/ SDL_Window *g_sdlWindow;
static SDL_Renderer *g_sdlRenderer;
static SDL_Texture *g_sdlTexture, *g_sdlTargetTexture;
static SDL_Rect g_sdlAspectCorrectionRect;

static bool g_sdlDoRefreshGfxOutput;
static bool g_sdlDoPendingRefreshGfxOutput;

void BE_SDL_MarkGfxForUpdate(void)
{
	if (g_sdlDoPendingRefreshGfxOutput)
	{
		g_sdlDoPendingRefreshGfxOutput = false;
		g_sdlDoRefreshGfxOutput = true;
	}
}

void BE_SDL_MarkGfxForPendingUpdate(void)
{
	g_sdlDoPendingRefreshGfxOutput = true;
}
#if !SDL_VERSION_ATLEAST(2,0,0)
#error "SDL <2.0 support is unimplemented!"
#endif

#define GFX_TEX_WIDTH 320
#define GFX_TEX_HEIGHT 200
#define VGA_TXT_TEX_WIDTH 720
#define VGA_TXT_TEX_HEIGHT 400
#define EGACGA_TXT_TEX_WIDTH 640
#define EGACGA_TXT_TEX_HEIGHT 200

#define TXT_COLS_NUM 80
#define TXT_ROWS_NUM 25

#define VGA_TXT_CHAR_PIX_WIDTH (VGA_TXT_TEX_WIDTH/TXT_COLS_NUM)
#define VGA_TXT_CHAR_PIX_HEIGHT (VGA_TXT_TEX_HEIGHT/TXT_ROWS_NUM)

#define VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE 8
#define VGA_TXT_BLINK_VERT_FRAME_RATE 16


extern uint8_t g_vga_8x16TextFont[256*VGA_TXT_CHAR_PIX_WIDTH*VGA_TXT_CHAR_PIX_HEIGHT];

static uint8_t g_sdlEGAGfxMemory[4][0x10000], g_sdlEGAGfxMemoryCache[4][0x10000];
static uint8_t g_sdlCGAGfxMemory[8192*2], g_sdlCGAGfxMemoryCache[8192*2];
static uint8_t g_sdlTextModeMemory[TXT_COLS_NUM*TXT_ROWS_NUM*2], g_sdlTextModeMemoryCache[TXT_COLS_NUM*TXT_ROWS_NUM*2];
static uint16_t g_sdlScreenStartAddress = 0;
static int g_sdlScreenMode = 3;
static int g_sdlTexWidth, g_sdlTexHeight;
static uint8_t g_sdlBorderColor = 0;
static uint8_t g_sdlPelPanning = 0;
static uint8_t g_sdlLineWidth = 40;
static int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
static int g_sdlTxtColor = 7, g_sdlTxtBackground = 0;

void BE_SDL_SetAspectCorrectionRect(void);

void BE_SDL_InitGfx(void)
{
	const char *windowTitle = "Chocolate Keen Dreams";
	if (g_chocolateKeenCfg.isFullscreen)
	{
		if (g_chocolateKeenCfg.fullWidth && g_chocolateKeenCfg.fullHeight)
		{
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), g_chocolateKeenCfg.fullWidth, g_chocolateKeenCfg.fullHeight, SDL_WINDOW_FULLSCREEN);
		}
		else
		{
			g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}
	else
	{
		int actualWinWidth = g_chocolateKeenCfg.winWidth, actualWinHeight = g_chocolateKeenCfg.winHeight;
		if (!actualWinWidth || !actualWinHeight)
		{
			bool doSoftwareRendering;
			if (g_chocolateKeenCfg.sdlRendererDriver >= 0)
			{
				SDL_RendererInfo info;
				SDL_GetRenderDriverInfo(g_chocolateKeenCfg.sdlRendererDriver, &info);
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
				SDL_GetDesktopDisplayMode(g_chocolateKeenCfg.displayNum, &mode);
				if (3*mode.w < 4*mode.h) // Thinner than 4:3
				{
					mode.h = mode.w*3/4;
				}
				else  // As wide as 4:3 at the least
				{
					mode.w = mode.h*4/3;
				}
				// Just for the sake of it, using the golden ratio...
				actualWinWidth = mode.w*500/809;
				actualWinHeight = mode.h*500/809;
			}
		}
		g_sdlWindow = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_chocolateKeenCfg.displayNum), actualWinWidth, actualWinHeight, SDL_WINDOW_RESIZABLE);
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}
#if GRMODE == EGAGR
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_chocolateKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | ((g_chocolateKeenCfg.vSync == VSYNC_OFF) ? 0 : SDL_RENDERER_PRESENTVSYNC));
#elif GRMODE == CGAGR
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_chocolateKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | ((g_chocolateKeenCfg.vSync == VSYNC_ON) ? SDL_RENDERER_PRESENTVSYNC : 0));
#else
#error "Supported GRMODE not defined!"
#endif
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		//Destroy window?
		exit(0);
	}
	SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	BE_SDL_SetScreenMode(3); // Includes SDL_Texture handling
	BE_SDL_SetAspectCorrectionRect();
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
	if ((g_chocolateKeenCfg.scaleFactor > 1) && g_chocolateKeenCfg.isBilinear)
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
		g_sdlTargetTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, g_sdlTexWidth*g_chocolateKeenCfg.scaleFactor, g_sdlTexHeight*g_chocolateKeenCfg.scaleFactor);
	}
	else
	{
		// Use just a single texture
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_chocolateKeenCfg.isBilinear ? "linear" : "nearest");
		g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
	}
}

void BE_SDL_SetAspectCorrectionRect(void)
{
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
	if (g_chocolateKeenCfg.scaleType == SCALE_FILL)
	{
		g_sdlAspectCorrectionRect.w = winWidth;
		g_sdlAspectCorrectionRect.h = winHeight;
		g_sdlAspectCorrectionRect.x = 0;
		g_sdlAspectCorrectionRect.y = 0;
	}
	else
	{
		// Aspect correct
		if (3*winWidth < 4*winHeight) // Thinner than 4:3
		{
			g_sdlAspectCorrectionRect.w = winWidth;
			g_sdlAspectCorrectionRect.h = winWidth*3/4;
			g_sdlAspectCorrectionRect.x = 0;
			g_sdlAspectCorrectionRect.y = (winHeight-g_sdlAspectCorrectionRect.h)/2;
		}
		else // As wide as 4:3 at the least
		{
			g_sdlAspectCorrectionRect.w = winHeight*4/3;
			g_sdlAspectCorrectionRect.h = winHeight;
			g_sdlAspectCorrectionRect.x = (winWidth-g_sdlAspectCorrectionRect.w)/2;
			g_sdlAspectCorrectionRect.y = 0;
		}
	}
}

void BE_SDL_SetScreenStartAddress(id0_unsigned_t crtc)
{
	g_sdlScreenStartAddress = crtc;
	g_sdlDoRefreshGfxOutput = true;
}

id0_byte_t *BE_SDL_GetTextModeMemoryPtr(void)
{
	return g_sdlTextModeMemory;
}

id0_byte_t *BE_SDL_GetCGAMemoryPtr(void)
{
	return g_sdlCGAGfxMemory;
}

id0_byte_t *BE_SDL_GetEGAMemoryPtr(void)
{
	// TODO: Never actually used...
	return &g_sdlEGAGfxMemory[0];
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

static uint32_t g_sdlEGACurrBGRAPalette[16];


void BE_SDL_SetBorderColor(id0_byte_t color)
{
	g_sdlBorderColor = color;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetPaletteAndBorder(const id0_char_t *palette)
{
	for (int entry = 0; entry < 16; ++entry)
	{
		g_sdlEGACurrBGRAPalette[entry] =  g_sdlEGABGRAScreenColors[(palette[entry] & 7) | ((palette[entry] & 16) >> 1)];
	}
	g_sdlBorderColor = (palette[16] & 7) | ((palette[16] & 16) >> 1);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_SetPelPanning(id0_byte_t panning)
{
	g_sdlPelPanning = panning;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGASetLineWidth(id0_byte_t widthInBytes)
{
	g_sdlLineWidth = widthInBytes;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t mask)
{
	if (mask & 1)
		g_sdlEGAGfxMemory[0][destOff] = srcVal;
	if (mask & 2)
		g_sdlEGAGfxMemory[1][destOff] = srcVal;
	if (mask & 4)
		g_sdlEGAGfxMemory[2][destOff] = srcVal;
	if (mask & 8)
		g_sdlEGAGfxMemory[3][destOff] = srcVal;
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
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlEGAGfxMemory[0], destOff, srcPtr, num);
	if (mask & 2)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlEGAGfxMemory[1], destOff, srcPtr, num);
	if (mask & 4)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlEGAGfxMemory[2], destOff, srcPtr, num);
	if (mask & 8)
		BEL_SDL_LinearToEGAPlane_MemCopy(g_sdlEGAGfxMemory[3], destOff, srcPtr, num);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXByteScrToScr(uint16_t destOff, uint16_t srcOff)
{
	g_sdlEGAGfxMemory[0][destOff] = g_sdlEGAGfxMemory[0][srcOff];
	g_sdlEGAGfxMemory[1][destOff] = g_sdlEGAGfxMemory[1][srcOff];
	g_sdlEGAGfxMemory[2][destOff] = g_sdlEGAGfxMemory[2][srcOff];
	g_sdlEGAGfxMemory[3][destOff] = g_sdlEGAGfxMemory[3][srcOff];
	g_sdlDoRefreshGfxOutput = true;
}

void BE_SDL_EGAUpdateGFXBufferScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlEGAGfxMemory[0], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlEGAGfxMemory[1], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlEGAGfxMemory[2], destOff, srcOff, num);
	BEL_SDL_EGAPlaneToEGAPlane_MemCopy(g_sdlEGAGfxMemory[3], destOff, srcOff, num);
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t BE_SDL_EGAFetchGFXByte(uint16_t destOff, uint16_t planenum)
{
	return g_sdlEGAGfxMemory[planenum][destOff];
}

void BE_SDL_EGAFetchGFXBuffer(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum)
{
	BEL_SDL_EGAPlaneToLinear_MemCopy(destPtr, g_sdlEGAGfxMemory[planenum], srcOff, num);
}

void BE_SDL_EGAUpdateGFXPixel4bpp(uint16_t destOff, uint8_t color, uint16_t mask)
{
	for (int currBitNum = 0, currBitMask = 1; currBitNum < 8; ++currBitNum, currBitMask <<= 1)
	{
		if (mask & currBitMask)
		{
			g_sdlEGAGfxMemory[0][destOff] &= ~currBitMask;
			g_sdlEGAGfxMemory[0][destOff] |= ((color & 1) << currBitNum);
			g_sdlEGAGfxMemory[1][destOff] &= ~currBitMask;
			g_sdlEGAGfxMemory[1][destOff] |= (((color & 2) >> 1) << currBitNum);
			g_sdlEGAGfxMemory[2][destOff] &= ~currBitMask;
			g_sdlEGAGfxMemory[2][destOff] |= (((color & 4) >> 2) << currBitNum);
			g_sdlEGAGfxMemory[3][destOff] &= ~currBitMask;
			g_sdlEGAGfxMemory[3][destOff] |= (((color & 8) >> 3) << currBitNum);
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
		g_sdlEGAGfxMemory[0][destOff] ^= srcVal;
	if (mask & 2)
		g_sdlEGAGfxMemory[1][destOff] ^= srcVal;
	if (mask & 4)
		g_sdlEGAGfxMemory[2][destOff] ^= srcVal;
	if (mask & 8)
		g_sdlEGAGfxMemory[3][destOff] ^= srcVal;
	g_sdlDoRefreshGfxOutput = true;
}


void BE_SDL_SetScreenMode(int mode)
{
	g_sdlScreenMode = mode;
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
		g_sdlTextModeMemoryCache[0] = g_sdlTextModeMemory[0]^0xFF; // Force refresh
		break;
	case 4:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memset(g_sdlCGAGfxMemory,  0, sizeof(g_sdlCGAGfxMemory));
		g_sdlCGAGfxMemoryCache[0] = g_sdlCGAGfxMemory[0]^0xFF; // Force refresh
		break;
	case 0xD:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPalette, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlBorderColor = 0;
		g_sdlPelPanning = 0;
		g_sdlLineWidth = 40;
		memset(g_sdlEGAGfxMemory,  0, sizeof(g_sdlEGAGfxMemory));
		g_sdlEGAGfxMemoryCache[0][0] = g_sdlEGAGfxMemory[0][0]^0xFF; // Force refresh
		break;
	}
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
	uint8_t *currMemByte = g_sdlTextModeMemory;
	for (int i = 0; i < 2*TXT_COLS_NUM*TXT_ROWS_NUM; ++i)
	{
		*(currMemByte++) = ' ';
		*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
	}
}

void BE_SDL_MoveTextCursorTo(int x, int y)
{
	g_sdlTxtCursorPosX = x;
	g_sdlTxtCursorPosY = y;
}

void BE_SDL_Simplified_printf(const char *str)
{
	// TODO (CHOCO KEEN): Tabs?
	uint8_t *currMemByte = g_sdlTextModeMemory + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	for (; *str; ++str)
	{
		if (*str == '\n')
		{
			g_sdlTxtCursorPosX = 0; // Carriage return
			++g_sdlTxtCursorPosY; // Line feed
			currMemByte = g_sdlTextModeMemory + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
		}
		else
		{
			*(currMemByte++) = *str;
			*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
			if (g_sdlTxtCursorPosX == TXT_COLS_NUM - 1)
			{
				g_sdlTxtCursorPosX = 0; // Carriage return
				++g_sdlTxtCursorPosY; // Line feed
				currMemByte = g_sdlTextModeMemory + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
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
			memmove(g_sdlTextModeMemory, g_sdlTextModeMemory+2*TXT_COLS_NUM, sizeof(g_sdlTextModeMemory)-2*TXT_COLS_NUM);
			currMemByte = g_sdlTextModeMemory+sizeof(g_sdlTextModeMemory)-2*TXT_COLS_NUM;
			// New empty line
			for (int i = 0; i < TXT_COLS_NUM; ++i)
			{
				*(currMemByte++) = ' ';
				*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
			}
			currMemByte -= 2*TXT_COLS_NUM; // Go back to beginning of line
		}
	}
}

void BE_SDL_puts(const char *str)
{
	BE_SDL_Simplified_printf(str);
	BE_SDL_Simplified_printf("\n");
}

void BE_SDL_UpdateHostDisplay(void);

void BE_SDL_WaitVBL(id0_int_t number)
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




void BE_SDL_UpdateHostDisplay(void)
{
	if (g_sdlScreenMode == 3) // Text mode
	{
		static bool wereBlinkingCharsShown;
		static bool wasBlinkingCursorShown;
		// Maybe we don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		for (int i = 0; i < sizeof(g_sdlTextModeMemory); ++i)
		{
			if (g_sdlTextModeMemory[i] != g_sdlTextModeMemoryCache[i])
			{
				g_sdlTextModeMemoryCache[i] = g_sdlTextModeMemory[i];
				doUpdate = true;
			}
		}
		// But there are still blinking characters and cursor
		bool areBlinkingCharsShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_BLINK_VERT_FRAME_RATE)) % 2);
		bool isBlinkingCursorShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE)) % 2);
		// We check g_sdlDoRefreshGfxOutput since this is currently set to "true" only after setting video mode (for text mode)
		if (!g_sdlDoRefreshGfxOutput && !doUpdate && (wereBlinkingCharsShown == areBlinkingCharsShown) && (wasBlinkingCursorShown == isBlinkingCursorShown))
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
				currChar = g_sdlTextModeMemory[txtByteCounter];
				// Luckily, the width*height product is always divisible by 8...
				// Note that the actual width is always 8,
				// even in VGA mode. We convert to 9 while drawing.
				currCharFontPtr = g_vga_8x16TextFont + currChar*16*8;
				++txtByteCounter;
				currBackgroundColor = g_sdlEGABGRAScreenColors[(g_sdlTextModeMemory[txtByteCounter] >> 4) & 7];
				// Should the character blink?
				if (!(g_sdlTextModeMemory[txtByteCounter] & 0x80) || areBlinkingCharsShown)
					currCharColor = g_sdlEGABGRAScreenColors[g_sdlTextModeMemory[txtByteCounter] & 15];
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
		currCharColor = g_sdlEGABGRAScreenColors[g_sdlTextModeMemory[1+((TXT_COLS_NUM*g_sdlTxtCursorPosY+g_sdlTxtCursorPosX)<<1)] & 15];
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
		// Maybe we still don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		for (int i = 0; i < sizeof(g_sdlCGAGfxMemory); ++i)
		{
			if (g_sdlCGAGfxMemory[i] != g_sdlCGAGfxMemoryCache[i])
			{
				g_sdlCGAGfxMemoryCache[i] = g_sdlCGAGfxMemory[i];
				doUpdate = true;
			}
		}
		if (!doUpdate)
		{
			return;
		}
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr;
		uint8_t *srcCGAPtr;
		uint8_t currBitLoc; // Location of bits to check inside byte (two bits)
		// First fill the "even" rows, then the "odd" ones
		for (int i = 0; i < 2; ++i)
		{
			currPixPtr = (uint32_t *)pixels + i*GFX_TEX_WIDTH;
			srcCGAPtr = &g_sdlCGAGfxMemory[g_sdlScreenStartAddress+i*8192];
			currBitLoc = 6; // Location of bits to check (two bits)
			for (int line = 0; line < (GFX_TEX_HEIGHT/2); ++line)
			{
				for (int col = 0; col < GFX_TEX_WIDTH; ++col, ++currPixPtr)
				{
					*currPixPtr = g_sdlCGAGfxBGRAScreenColors[((*srcCGAPtr) >> currBitLoc) & 3];
					if (currBitLoc)
					{
						currBitLoc -= 2; // Check following bits in same CGA byte
					}
					else
					{
						currBitLoc = 6;
						// Check next byte
						if ((++srcCGAPtr) == (g_sdlCGAGfxMemory+sizeof(g_sdlCGAGfxMemory)))
						{
							srcCGAPtr = g_sdlCGAGfxMemory; // Wrap around
						}
					}
				}
				currPixPtr += GFX_TEX_WIDTH; // Skip "odd" row
			}
		}
	}
	else // EGA graphics mode 0xD
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			return;
		}
		// TODO: Doesn't work that well with palette cycling...
#if 0
		// Maybe we still don't have to update screen (e.g., Keen Dreams control panel)
		bool doUpdate = false;
		for (int planeNum = 0; planeNum < 4; ++planeNum)
		{
			for (int i = 0; i < sizeof(g_sdlEGAGfxMemory[planeNum]); ++i)
			{
				if (g_sdlEGAGfxMemory[planeNum][i] != g_sdlEGAGfxMemoryCache[planeNum][i])
				{
					g_sdlEGAGfxMemoryCache[planeNum][i] = g_sdlEGAGfxMemory[planeNum][i];
					doUpdate = true;
				}
			}
		}
		if (!doUpdate)
		{
			return;
		}
#endif
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		// TODO DEBUG
		//g_sdlPelPanning = 0;
		//g_sdlLineWidth = 64;
		//
		uint16_t currLineFirstByte = (g_sdlScreenStartAddress + g_sdlPelPanning/8) % 0x10000;
		uint8_t panningWithinInByte = g_sdlPelPanning%8;
		for (int line = 0, col; line < GFX_TEX_HEIGHT; ++line)
		{
			uint8_t currBitNum = 7-panningWithinInByte, currBitMask = 1<<currBitNum;
			uint16_t currByte = currLineFirstByte;
			uint32_t *currPixPtr = (uint32_t *)pixels + line*GFX_TEX_WIDTH;
			for (col = 0; col < GFX_TEX_WIDTH; ++col, ++currPixPtr)
			{
				*currPixPtr = g_sdlEGACurrBGRAPalette[
					((g_sdlEGAGfxMemory[0][currByte]&currBitMask)>>currBitNum) |
					(((g_sdlEGAGfxMemory[1][currByte]&currBitMask)>>currBitNum)<<1) |
					(((g_sdlEGAGfxMemory[2][currByte]&currBitMask)>>currBitNum)<<2) |
					(((g_sdlEGAGfxMemory[3][currByte]&currBitMask)>>currBitNum)<<3)
				];
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
					++currPixPtr;
					break;
				}
			}
			// Just if this makes sense... (FIXME: Check!)
			for (; col < GFX_TEX_WIDTH; ++col, ++currPixPtr)
			{
				*currPixPtr = g_sdlEGACurrBGRAPalette[0];
			}
			currLineFirstByte += g_sdlLineWidth;
			currLineFirstByte %= 0x10000;
		}
	}
	g_sdlDoRefreshGfxOutput = false;
	SDL_UnlockTexture(g_sdlTexture);
	SDL_RenderClear(g_sdlRenderer);
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
