/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// C3_MAIN.C

#include "c3_def.h"
#pragma hdrstop

/*
=============================================================================

						   CATACOMB 3-D

					  An Id Software production

						   by John Carmack

=============================================================================
*/

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

memptr		scalesegs[NUMPICS];
id0_char_t		str[80],str2[20];
id0_unsigned_t	tedlevelnum;
id0_boolean_t		tedlevel;
gametype	gamestate;
exittype	playstate;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/



//===========================================================================

// REFKEEN - Commented out

#if 0
// JAB Hack begin
#define	MyInterrupt	0x60
void interrupt (*intaddr)();
void interrupt (*oldintaddr)();
	id0_char_t	*JHParmStrings[] = {"no386",nil};

void
jabhack(void)
{
extern void id0_far jabhack2(void);
extern id0_int_t id0_far	CheckIs386(void);

	id0_int_t	i;

	oldintaddr = getvect(MyInterrupt);

	for (i = 1;i < _argc;i++)
		if (US_CheckParm(_argv[i],JHParmStrings) == 0)
			return;

	if (CheckIs386())
	{
		jabhack2();
		setvect(MyInterrupt,intaddr);
	}
}

void
jabunhack(void)
{
	setvect(MyInterrupt,oldintaddr);
}
//	JAB Hack end
#endif

//===========================================================================


// REFKEEN - New cross-platform methods for reading/writing objects from/to saved games
static id0_boolean_t SaveObject(int file, objtype *o)
{
	id0_int_t dummy = 0;
	// for active enum (anonymous type)
	id0_int_t activeint = (id0_int_t)(o->active);
	// BACKWARD COMPATIBILITY
	id0_int_t statedosoffset = o->state ? o->state->compatdosoffset : 0;
	// Just tells if "o->next" is zero or not
	id0_int_t isnext = o->next ? 1 : 0;
	// Now writing
	size_t BE_Cross_write_classtype_To16LE(int handle, const classtype *ptr);
	size_t BE_Cross_write_dirtype_To16LE(int handle, const dirtype *ptr);
	return ((BE_Cross_writeInt16LE(file, &activeint) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->ticcount) == 2)
	        && (BE_Cross_write_classtype_To16LE(file, &o->obclass) == 2)
	        && (BE_Cross_writeInt16LE(file, &statedosoffset) == 2) // BACKWARD COMPATIBILITY
	        && (BE_Cross_write_boolean_To16LE(file, &o->shootable) == 2)
	        && (BE_Cross_write_boolean_To16LE(file, &o->tileobject) == 2)
	        && (BE_Cross_writeInt32LE(file, &o->distance) == 4)
	        && (BE_Cross_write_dirtype_To16LE(file, &o->dir) == 2)
	        && (BE_Cross_writeInt32LE(file, &o->x) == 4)
	        && (BE_Cross_writeInt32LE(file, &o->y) == 4)
	        && (BE_Cross_writeInt16LE(file, &o->tilex) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->tiley) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->viewx) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->viewheight) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->angle) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->hitpoints) == 2)
	        && (BE_Cross_writeInt32LE(file, &o->speed) == 4)
	        && (BE_Cross_writeInt16LE(file, &o->size) == 2)
	        && (BE_Cross_writeInt32LE(file, &o->xl) == 4)
	        && (BE_Cross_writeInt32LE(file, &o->xh) == 4)
	        && (BE_Cross_writeInt32LE(file, &o->yl) == 4)
	        && (BE_Cross_writeInt32LE(file, &o->yh) == 4)
	        && (BE_Cross_writeInt16LE(file, &o->temp1) == 2)
	        && (BE_Cross_writeInt16LE(file, &o->temp2) == 2)
	        // No need to write prev pointer as-is,
	        // this is ignored on loading. So write dummy value.
	        // Furthermore, all we need to know about next on loading is
	        // if it's zero or not.
	        && (BE_Cross_writeInt16LE(file, &isnext) == 2) // next
	        && (BE_Cross_writeInt8LEBuffer(file, &dummy, 2) == 2) // prev
	);
}

static id0_boolean_t LoadObject(int file, objtype *o)
{
	id0_int_t dummy;
	// for active enum (anonymous type)
	id0_int_t activeint;
	// BACKWARD COMPATIBILITY
	id0_int_t statedosoffset;
	// Just tells if "o->next" is zero or not
	id0_int_t isnext;
	// Now reading
	size_t BE_Cross_read_classtype_From16LE(int handle, classtype *ptr);
	size_t BE_Cross_read_dirtype_From16LE(int handle, dirtype *ptr);
	if ((BE_Cross_readInt16LE(file, &activeint) != 2)
	    || (BE_Cross_readInt16LE(file, &o->ticcount) != 2)
	    || (BE_Cross_read_classtype_From16LE(file, &o->obclass) != 2)
	    || (BE_Cross_readInt16LE(file, &statedosoffset) != 2) // BACKWARD COMPATIBILITY
	    || (BE_Cross_read_boolean_From16LE(file, &o->shootable) != 2)
	    || (BE_Cross_read_boolean_From16LE(file, &o->tileobject) != 2)
	    || (BE_Cross_readInt32LE(file, &o->distance) != 4)
	    || (BE_Cross_read_dirtype_From16LE(file, &o->dir) != 2)
	    || (BE_Cross_readInt32LE(file, &o->x) != 4)
	    || (BE_Cross_readInt32LE(file, &o->y) != 4)
	    || (BE_Cross_readInt16LE(file, &o->tilex) != 2)
	    || (BE_Cross_readInt16LE(file, &o->tiley) != 2)
	    || (BE_Cross_readInt16LE(file, &o->viewx) != 2)
	    || (BE_Cross_readInt16LE(file, &o->viewheight) != 2)
	    || (BE_Cross_readInt16LE(file, &o->angle) != 2)
	    || (BE_Cross_readInt16LE(file, &o->hitpoints) != 2)
	    || (BE_Cross_readInt32LE(file, &o->speed) != 4)
	    || (BE_Cross_readInt16LE(file, &o->size) != 2)
	    || (BE_Cross_readInt32LE(file, &o->xl) != 4)
	    || (BE_Cross_readInt32LE(file, &o->xh) != 4)
	    || (BE_Cross_readInt32LE(file, &o->yl) != 4)
	    || (BE_Cross_readInt32LE(file, &o->yh) != 4)
	    || (BE_Cross_readInt16LE(file, &o->temp1) != 2)
	    || (BE_Cross_readInt16LE(file, &o->temp2) != 2)
	    // No need to read prev pointer as-is,
	    // this is ignored on loading. So read dummy value.
	    // Furthermore, all we need to know about next on loading is
	    // if it's zero or not.
	    || (BE_Cross_readInt16LE(file, &isnext) != 2) // next
	    || (BE_Cross_readInt8LEBuffer(file, &dummy, 2) != 2)
	)
	{
		return false;
	}
	o->active = activeint;
	o->state = (statetype *)BE_Cross_Compat_GetObjStatePtrFromDOSOffset(statedosoffset);
	// HACK: All we need to know is if next was originally NULL or not
	o->next = isnext ? o : NULL;
	return true;
}

// Similar new methods for writing/reading game state
static id0_boolean_t SaveGameState(int file, gametype *state)
{
	return ((BE_Cross_writeInt16LE(file, &state->difficulty) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->mapon) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->bolts) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->nukes) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->potions) == 2)
	        && (BE_Cross_writeInt16LEBuffer(file, state->keys, sizeof(state->keys)) == sizeof(state->keys))
	        && (BE_Cross_writeInt16LEBuffer(file, state->scrolls, sizeof(state->scrolls)) == sizeof(state->scrolls))
	        && (BE_Cross_writeInt32LE(file, &state->score) == 4)
	        && (BE_Cross_writeInt16LE(file, &state->body) == 2)
	        && (BE_Cross_writeInt16LE(file, &state->shotpower) == 2)
	);
}

static id0_boolean_t LoadGameState(int file, gametype *state)
{
	return ((BE_Cross_readInt16LE(file, &state->difficulty) == 2)
	        && (BE_Cross_readInt16LE(file, &state->mapon) == 2)
	        && (BE_Cross_readInt16LE(file, &state->bolts) == 2)
	        && (BE_Cross_readInt16LE(file, &state->nukes) == 2)
	        && (BE_Cross_readInt16LE(file, &state->potions) == 2)
	        && (BE_Cross_readInt16LEBuffer(file, state->keys, sizeof(state->keys)) == sizeof(state->keys))
	        && (BE_Cross_readInt16LEBuffer(file, state->scrolls, sizeof(state->scrolls)) == sizeof(state->scrolls))
	        && (BE_Cross_readInt32LE(file, &state->score) == 4)
	        && (BE_Cross_readInt16LE(file, &state->body) == 2)
	        && (BE_Cross_readInt16LE(file, &state->shotpower) == 2)
	);
}


/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame (void)
{
	memset (&gamestate,0,sizeof(gamestate));
	gamestate.mapon = 0;
	gamestate.body = MAXBODY;
}

//===========================================================================

#define RLETAG	0xABCD

/*
==================
=
= SaveTheGame
=
==================
*/

id0_boolean_t	SaveTheGame(id0_int_t file)
{
	id0_word_t	i,compressed,expanded;
	objtype	*o;
	memptr	bigbuffer;

	// (REFKEEN) Writing fields one-by-one in a cross-platform manner
	if (!SaveGameState(file, &gamestate))
	//if (!CA_FarWrite(file,(void id0_far *)&gamestate,sizeof(gamestate)))
		return(false);

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i+=2)	// Write planes 0 and 2
	{
//
// leave a word at start of compressed data for compressed length
//
		compressed = (id0_unsigned_t)CA_RLEWCompress ((id0_unsigned_t id0_huge *)mapsegs[i]
			,expanded,((id0_unsigned_t id0_huge *)bigbuffer)+1,RLETAG);

		*(id0_unsigned_t id0_huge *)bigbuffer = compressed;

		if (!CA_FarWrite(file,(void id0_far *)bigbuffer,compressed+2) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}
	}

	for (o = player;o;o = o->next)
		// (REFKEEN) Writing fields one-by-one in a cross-platform manner
		if (!SaveObject(file, o))
		//if (!CA_FarWrite(file,(void id0_far *)o,sizeof(objtype)))
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

	MM_FreePtr (&bigbuffer);

	return(true);
}

//===========================================================================

/*
==================
=
= LoadTheGame
=
==================
*/

id0_boolean_t	LoadTheGame(id0_int_t file)
{
	id0_unsigned_t	i,x,y;
	objtype		*obj,*prev,*next,*followed;
	id0_unsigned_t	compressed,expanded;
	id0_unsigned_t	id0_far *map,tile;
	memptr		bigbuffer;

	// (REFKEEN) Reading fields one-by-one in a cross-platform manner
	if (!LoadGameState(file, &gamestate))
	//if (!CA_FarRead(file,(void id0_far *)&gamestate,sizeof(gamestate)))
		return(false);

	SetupGameLevel ();		// load in and cache the base old level

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i+=2)	// Read planes 0 and 2
	{
		if (!CA_FarRead(file,(void id0_far *)&compressed,sizeof(compressed)) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		if (!CA_FarRead(file,(void id0_far *)bigbuffer,compressed) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		CA_RLEWexpand ((id0_unsigned_t id0_huge *)bigbuffer,
			(id0_unsigned_t id0_huge *)mapsegs[i],expanded,RLETAG);
	}

	MM_FreePtr (&bigbuffer);
//
// copy the wall data to a data segment array again, to handle doors and
// bomb walls that are allready opened
//
	memset (tilemap,0,sizeof(tilemap));
	memset (actorat,0,sizeof(actorat));
	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile<NUMFLOORS)
			{
				tilemap[x][y] = tile;
				if (tile>0)
					actorat[x][y] = tile;
					//(id0_unsigned_t)actorat[x][y] = tile;
			}
		}


	// Read the object list back in - assumes at least one object in list

	InitObjList ();
	new = player;
	while (true)
	{
		prev = new->prev;
		next = new->next;
		// (REFKEEN) Reading fields one-by-one in a cross-platform manner
		if (!LoadObject(file, new))
		//if (!CA_FarRead(file,(void id0_far *)new,sizeof(objtype)))
			return(false);
		followed = new->next;
		new->prev = prev;
		new->next = next;
		actorat[new->tilex][new->tiley] = COMPAT_OBJ_CONVERT_OBJ_PTR_TO_DOS_PTR(new);	// drop a new marker
		//actorat[new->tilex][new->tiley] = new;	// drop a new marker

		if (followed)
			GetNewObj (false);
		else
			break;
	}

	return(true);
}

//===========================================================================

/*
==================
=
= ResetGame
=
==================
*/

void ResetGame(void)
{
	NewGame ();

	ca_levelnum--;
	ca_levelbit>>=1;
	CA_ClearMarks();
	ca_levelbit<<=1;
	ca_levelnum++;
}

//===========================================================================


/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId (void)
{
  US_Shutdown ();
#ifndef PROFILE
  SD_Shutdown ();
  IN_Shutdown ();
#endif
  VW_Shutdown ();
  CA_Shutdown ();
  MM_Shutdown ();
}


//===========================================================================

/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

void InitGame (void)
{
	//id0_unsigned_t	segstart,seglength;
	id0_int_t			i,x,y;
	id0_unsigned_t	*blockstart;

#ifdef REFKEEN_VER_CAT3D_100
	US_TextScreen();
#endif
	MM_Startup ();
	VW_Startup ();
#ifndef PROFILE
	IN_Startup ();
	SD_Startup ();
#endif
	US_Startup ();
#ifdef REFKEEN_VER_CAT3D_100
	US_UpdateTextScreen();
#endif
	CA_Startup ();
	US_Setup ();

	US_SetLoadSaveHooks(LoadTheGame,SaveTheGame,ResetGame);

//
// load in and lock down some basic chunks
//

	CA_ClearMarks ();

	CA_MarkGrChunk(STARTFONT);
	CA_MarkGrChunk(STARTTILE8);
	CA_MarkGrChunk(STARTTILE8M);
	CA_MarkGrChunk(HAND1PICM);
	CA_MarkGrChunk(HAND2PICM);
	CA_MarkGrChunk(ENTERPLAQUEPIC);

	CA_CacheMarks (NULL);

	MM_SetLock (&grsegs[STARTFONT],true);
	MM_SetLock (&grsegs[STARTTILE8],true);
	MM_SetLock (&grsegs[STARTTILE8M],true);
	MM_SetLock (&grsegs[HAND1PICM],true);
	MM_SetLock (&grsegs[HAND2PICM],true);
	MM_SetLock (&grsegs[ENTERPLAQUEPIC],true);

	fontcolor = WHITE;


//
// build some tables
//
	for (i=0;i<MAPSIZE;i++)
		nearmapylookup[i] = &tilemap[0][0]+MAPSIZE*i;

	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	BuildTables ();			// 3-d tables

	SetupScaling ();

#ifndef PROFILE
#ifdef REFKEEN_VER_CAT3D_100
	US_FinishTextScreen();
#endif
#endif

//
// reclaim the memory from the linked in text screen
//
	// REFKEEN DIFFERENCE (FIXME: Should we "fix" this at all?)
	// - Don't handle this, a bit more complicated with our setup and the
	// difference is (probably) insignificant with well-defined behaviors
	// anyway...
#if 0
	segstart = FP_SEG(&introscn);
	seglength = 4000/16;
	if (FP_OFF(&introscn))
	{
		segstart++;
		seglength--;
	}

	MML_UseSpace (segstart,seglength);
#endif

	VW_SetScreenMode (GRMODE);
	VW_ColorBorder (3);
	VW_ClearVideo (BLACK);

//
// initialize variables
//
	updateptr = &update[0];
	*(id0_unsigned_t *)(updateptr + UPDATEWIDE*PORTTILESHIGH) = UPDATETERMINATE;
	bufferofs = 0;
	displayofs = 0;
	VW_SetLineWidth(SCREENWIDTH);
}

//===========================================================================

void clrscr (void);		// can't include CONIO.H because of name conflicts...

/*
==========================
=
= Quit
=
==========================
*/

void Quit (id0_char_t *error)
{
	void *finscreen;
	//id0_unsigned_t	finscreen;

#ifdef REFKEEN_VER_CAT3D_100
	if (!error)
	{
		CA_SetAllPurge ();
		CA_CacheGrChunk (PIRACY);
		finscreen = grsegs[PIRACY];
		//finscreen = (id0_unsigned_t)grsegs[PIRACY];
	}
#endif

	ShutdownId ();
	if (error && *error)
	{
	  BE_Cross_puts(error);
	  BE_SDL_HandleExit(1);
	}

#ifdef REFKEEN_VER_CAT3D_100
	if (!NoWait)
	{
		memcpy(BE_SDL_GetTextModeMemoryPtr(), finscreen, 4000);
		//movedata (finscreen,0,0xb800,0,4000);
		BE_SDL_BiosScanCode (0);
		BE_SDL_clrscr();
	}
#endif

	BE_SDL_HandleExit(0);
}

//===========================================================================

/*
==================
=
= TEDDeath
=
==================
*/

void	TEDDeath(void)
{
	ShutdownId();
	// REFKEEN - DISABLED
	Quit("Sorry, but TED5.EXE cannot be launched from game in this source port.");
	//execlp("TED5.EXE","TED5.EXE","/LAUNCH",NULL);
}

//===========================================================================

/*
=====================
=
= DemoLoop
=
=====================
*/

static	id0_char_t *ParmStrings[] = {"easy","normal","hard",""};

void	DemoLoop (void)
{
	id0_int_t	i,level;

//
// check for launch from ted
//
	if (tedlevel)
	{
		NewGame();
		gamestate.mapon = tedlevelnum;
		restartgame = gd_Normal;
		for (i = 1;i < id0_argc;i++)
		{
			if ( (level = US_CheckParm(id0_argv[i],ParmStrings)) == -1)
				continue;

			restartgame = gd_Easy+level;
			break;
		}
		GameLoop();
		TEDDeath();
	}


//
// main game cycle
//
	displayofs = bufferofs = 0;
	VW_Bar (0,0,320,200,0);

	while (1)
	{
		CA_CacheGrChunk (TITLEPIC);
		bufferofs = SCREEN2START;
		displayofs = SCREEN1START;
		VWB_DrawPic (0,0,TITLEPIC);
		MM_SetPurge (&grsegs[TITLEPIC],3);
		UNMARKGRCHUNK(TITLEPIC);
		FizzleFade (bufferofs,displayofs,320,200,true);

		if (!IN_UserInput(TickBase*3,false))
		{
			CA_CacheGrChunk (CREDITSPIC);
			VWB_DrawPic (0,0,CREDITSPIC);
			MM_SetPurge (&grsegs[CREDITSPIC],3);
			UNMARKGRCHUNK(CREDITSPIC);
			FizzleFade (bufferofs,displayofs,320,200,true);

		}

		if (!IN_UserInput(TickBase*3,false))
		{
highscores:
			DrawHighScores ();
			FizzleFade (bufferofs,displayofs,320,200,true);
			IN_UserInput(TickBase*3,false);
		}

		if (IN_IsUserInput())
		{
			US_ControlPanel ();

			if (restartgame || loadedgame)
			{
				GameLoop ();
				goto highscores;
			}
		}

	}
}

//===========================================================================

/*
==========================
=
= SetupScalePic
=
==========================
*/

void SetupScalePic (id0_unsigned_t picnum)
{
	id0_unsigned_t	scnum;

	scnum = picnum-FIRSTSCALEPIC;

	if (shapedirectory[scnum])
	{
		MM_SetPurge ((memptr *)&shapedirectory[scnum],0);
		return;					// allready in memory
	}

	CA_CacheGrChunk (picnum);
	DeplanePic (picnum);
	shapesize[scnum] = BuildCompShape (&shapedirectory[scnum]);
	grneeded[picnum]&= ~ca_levelbit;
	MM_FreePtr (&grsegs[picnum]);
}

//===========================================================================

/*
==========================
=
= SetupScaleWall
=
==========================
*/

void SetupScaleWall (id0_unsigned_t picnum)
{
	id0_int_t		x,y;
	id0_unsigned_t	scnum;
	id0_byte_t	id0_far *dest;

	scnum = picnum-FIRSTWALLPIC;

	if (walldirectory[scnum])
	{
		MM_SetPurge (&walldirectory[scnum],0);
		return;					// allready in memory
	}

	CA_CacheGrChunk (picnum);
	DeplanePic (picnum);
	MM_GetPtr(&walldirectory[scnum],64*64);
	dest = (id0_byte_t id0_far *)walldirectory[scnum];
	for (x=0;x<64;x++)
		for (y=0;y<64;y++)
			*dest++ = spotvis[y][x];
	grneeded[picnum]&= ~ca_levelbit;
	MM_FreePtr (&grsegs[picnum]);
}

//===========================================================================

/*
==========================
=
= SetupScaling
=
==========================
*/

void SetupScaling (void)
{
	id0_int_t		i,x,y;
	id0_byte_t	id0_far *dest;

//
// build the compiled scalers
//
	for (i=1;i<=VIEWWIDTH/2;i++)
		BuildCompScale (i*2,&scaledirectory[i]);
}

//===========================================================================

id0_int_t	showscorebox;

void RF_FixOfs (void)
{
}

void HelpScreens (void)
{
}


/*
==================
=
= CheckMemory
=
==================
*/

#define MINMEMORY	335000l

void	CheckMemory(void)
{
	void *finscreen;
	//id0_unsigned_t	finscreen;

	if (mminfo.nearheap+mminfo.farheap+mminfo.EMSmem+mminfo.XMSmem
		>= MINMEMORY)
		return;

	CA_CacheGrChunk (OUTOFMEM);
	finscreen = grsegs[OUTOFMEM];
	//finscreen = (id0_unsigned_t)grsegs[OUTOFMEM];
	ShutdownId ();
	memcpy(BE_SDL_GetTextModeMemoryPtr(), finscreen+7, 4000);
	//movedata (finscreen,7,0xb800,0,4000);
	BE_SDL_MoveTextCursorTo(0, 23); //gotoxy (1,24);
	BE_SDL_HandleExit(1);
}

//===========================================================================


/*
==========================
=
= main
=
==========================
*/

// The original starting point of the game EXE
void cat3d_exe_main (void)
{
	//id0_short_t i;

#ifdef REFKEEN_VER_CAT3D_122
	if (BE_Cross_strcasecmp(id0_argv[1], "/VER") == 0)
	{
		BE_Cross_Simplified_printf("Catacomb 3-D version 1.22  (Rev 1)\n");
		BE_Cross_Simplified_printf("Copyright 1991-93 Softdisk Publishing\n");
		BE_Cross_Simplified_printf("Developed for use with 100%% IBM compatibles\n");
		BE_Cross_Simplified_printf("that have 640K memory and DOS version 3.3 or later\n");
		BE_Cross_Simplified_printf("and EGA graphics or better.\n");
		BE_SDL_HandleExit(0);
	}

	if (BE_Cross_strcasecmp(id0_argv[1], "/?") == 0)
	{
		BE_Cross_Simplified_printf("Catacomb 3-D version 1.22\n");
		BE_Cross_Simplified_printf("Copyright 1991-93 Softdisk Publishing\n\n");
		BE_Cross_Simplified_printf("Syntax:\n");
		BE_Cross_Simplified_printf("CAT3D [/<switch>]\n\n");
		BE_Cross_Simplified_printf("Switch       What it does\n");
		BE_Cross_Simplified_printf("/?           This Information\n");
		BE_Cross_Simplified_printf("/VER         Display Program Version Information\n");
		BE_Cross_Simplified_printf("/COMP        Fix problems with SVGA screens\n");
		BE_Cross_Simplified_printf("/NOAL        No AdLib or SoundBlaster detection\n");
		BE_Cross_Simplified_printf("/NOJOYS      Tell program to ignore joystick\n");
		BE_Cross_Simplified_printf("/NOMOUSE     Tell program to ignore mouse\n");
		BE_Cross_Simplified_printf("/HIDDENCARD  Overrides video detection\n\n");
		BE_Cross_Simplified_printf("Each switch must include a '/' and multiple switches\n");
		BE_Cross_Simplified_printf("must be seperated by at least one space.\n\n");

		BE_SDL_HandleExit(0);
	}
#endif
	// jabhack(); // REFKEEN - Commented out

	InitGame ();

	CheckMemory ();

	LoadLatchMem ();

#ifdef PROFILE
	NewGame ();
	GameLoop ();
#endif

//NewGame ();
//GameLoop ();

	DemoLoop();
	Quit("Demo loop exited???");
}