/* Copyright (C) 2014-2021 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */

#include "be_cross.h"
#include "be_st.h"
#include "be_st_launcher.h"
#include "be_st_sdl_private.h"
#include "be_video.h"
#include "be_video_emu.h"
#include "be_video_textures.h"

// These might be implementation-dependent
BE_ST_Texture *BEL_ST_CreateARGBTexture(int w, int h, bool isTarget, bool isLinear);
void BEL_ST_DestroyTexture(BE_ST_Texture *texture);

void BEL_ST_RecreateMainTextures(void)
{
	if (g_sdlTexture)
		BEL_ST_SDLDestroyTextureWrapper(&g_sdlTexture);
	if (g_sdlTargetTexture)
		BEL_ST_SDLDestroyTextureWrapper(&g_sdlTargetTexture);
	// Try using render target
	if ((g_refKeenCfg.scaleFactor > 1) && g_refKeenCfg.isBilinear)
	{
		BEL_ST_SDLCreateTextureWrapper(&g_sdlTexture, g_sdlTexWidth, g_sdlTexHeight, false, false);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
		// Try, if we fail then simply don't use this
		BEL_ST_SDLCreateTextureWrapper(&g_sdlTargetTexture, g_sdlTexWidth*g_refKeenCfg.scaleFactor, g_sdlTexHeight*g_refKeenCfg.scaleFactor, true, true);
		if (g_sdlTargetTexture)
			BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BEL_ST_RecreateMainTextures: SDL2 target texture created successfully\n");
		else
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateMainTextures:  Failed to (re)create SDL2 target texture, continuing anyway\n%s\n", SDL_GetError());
	}
	else
	{
		// Use just a single texture
		BEL_ST_SDLCreateTextureWrapper(&g_sdlTexture, g_sdlTexWidth, g_sdlTexHeight, false, g_refKeenCfg.isBilinear);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
	}
}

#define MAX_TEXTURES_POOL_SIZE 32

typedef struct {
	BE_ST_Texture **texturePtr;
	int w, h;
	bool isTarget, isLinear;
} BESDLManagedTexture;

static BESDLManagedTexture g_sdlManagedTexturesPool[MAX_TEXTURES_POOL_SIZE];
static int g_sdlNumOfManagedTexturesInPool = 0;

void BEL_ST_SDLCreateTextureWrapper(BE_ST_Texture **pTexture, int w, int h, bool isTarget, bool isLinear)
{
	int i;
	for (i = 0; i < g_sdlNumOfManagedTexturesInPool; ++i)
		if (g_sdlManagedTexturesPool[i].texturePtr == pTexture)
			break;

	if (i >= MAX_TEXTURES_POOL_SIZE)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_SDLCreateTextureWrapper: Managed textures pool overflow error!\n");
		//Destroy window and renderer?
		exit(0);
	}

	BESDLManagedTexture *managedTexture = &g_sdlManagedTexturesPool[i];
	if (i == g_sdlNumOfManagedTexturesInPool)
	{
		g_sdlNumOfManagedTexturesInPool++;
		managedTexture->texturePtr = pTexture;
	}
	managedTexture->w = w;
	managedTexture->h = h;
	managedTexture->isTarget = isTarget;
	managedTexture->isLinear = isLinear;

	*pTexture = BEL_ST_CreateARGBTexture(w, h, isTarget, isLinear);
}

void BEL_ST_SDLDestroyTextureWrapper(BE_ST_Texture **pTexture)
{
	if (*pTexture == NULL)
		return;

	for (int i = 0; i < g_sdlNumOfManagedTexturesInPool; ++i)
		if (g_sdlManagedTexturesPool[i].texturePtr == pTexture)
		{
			// Remove managed texture without moving the rest, except for maybe the last
			g_sdlManagedTexturesPool[i] = g_sdlManagedTexturesPool[--g_sdlNumOfManagedTexturesInPool];
			break;
		}

	BEL_ST_DestroyTexture(*pTexture);
	*pTexture = NULL;
}

void BEL_ST_RecreateAllTextures(void)
{
	BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BEL_ST_RecreateAllTextures: Recreating textures\n");
	BESDLManagedTexture *managedTexture = g_sdlManagedTexturesPool;
	for (int i = 0; i < g_sdlNumOfManagedTexturesInPool; ++i, ++managedTexture)
	{
		BE_ST_Texture **pTexture = managedTexture->texturePtr;
		if (*pTexture == NULL)
			continue;

		BEL_ST_DestroyTexture(*pTexture);
		*pTexture = BEL_ST_CreateARGBTexture
			(managedTexture->w, managedTexture->h,
			 managedTexture->isTarget, managedTexture->isLinear);

		if (*pTexture == NULL)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "BEL_ST_RecreateAllTextures: Failed to recreate SDL2 texture %d out of %d,\n%s\n", i+1, g_sdlNumOfManagedTexturesInPool, SDL_GetError());
			exit(1);
		}
		BE_Cross_LogMessage(BE_LOG_MSG_NORMAL, "BEL_ST_RecreateAllTextures: Recreated texture no %d out of %d\n", i+1, g_sdlNumOfManagedTexturesInPool);
	}

	BEL_ST_ForceHostDisplayUpdate();
	g_sdlDoRefreshGfxOutput = true; // BE_ST_MarkGfxForUpdate();
#ifdef REFKEEN_ENABLE_LAUNCHER
	BE_ST_Launcher_MarkGfxCache();
#endif
	// Also need to force refresh this way
	if (g_sdlScreenMode == 4) // CGA graphics
		g_sdlHostScrMemCache.cgaGfx[0] = g_sdlHostScrMem.cgaGfx[0]^0xFF;
	else
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF;
}
