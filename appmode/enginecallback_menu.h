/*
enginecallback.h - actual engine callbacks
Copyright (C) 2010 Uncle Mike
Copyright (C) 2017 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once
#ifndef ENGINECALLBACKS_H
#define ENGINECALLBACKS_H

#include "wrect.h"
#include "extdll_menu.h"
#include "Primitive.h"
#include "netadr.h"
#include "con_nprint.h"
#include "cvardef.h"
#include <SDL.h>

// flags for PIC_Load
#define PIC_NEAREST		(1U << 0)		// disable texfilter
#define PIC_KEEP_RGBDATA	(1U << 1)		// some images keep source
#define PIC_NOFLIP_TGA	(1U << 2)		// Steam background completely ignore tga attribute 0x20
#define PIC_KEEP_8BIT	(1U << 3)		// keep original 8-bit image (if present)

typedef struct ui_globalvars_s
{	
	float		time;		// unclamped host.realtime
	float		frametime;

	int		scrWidth;		// actual values
	int		scrHeight;

	int		maxClients;
	int		developer;
	int		demoplayback;
	int		demorecording;
	char		demoname[64];	// name of currently playing demo
	char		maptitle[64];	// title of active map
} ui_globalvars_t;

typedef SDL_Surface *HIMAGE;

class EngFuncs
{
public:
	// image handlers
	static HIMAGE PIC_Load( const char *szPicName, const byte *ucRawImage, long ulRawImageSize, long flags = 0);
	static HIMAGE PIC_Load( const char *szPicName, long flags = 0);
	static void	PIC_Free( const char *szPicName );
	static int	PIC_Width( HIMAGE hPic );
	static int	PIC_Height( HIMAGE hPic );
	static void	PIC_Set( HIMAGE hPic, int r, int g, int b, int a = 255 );
	static void	PIC_Draw( int x, int y, int width, int height, const wrect_t *prc = NULL );
	static void	PIC_DrawHoles( int x, int y, int width, int height, const wrect_t *prc = NULL );
	static void	PIC_DrawTrans( int x, int y, int width, int height, const wrect_t *prc = NULL );
	static void	PIC_DrawAdditive( int x, int y, int width, int height, const wrect_t *prc = NULL );
	static inline void	PIC_Draw( int x, int y, const wrect_t *prc = NULL )
	{ PIC_Draw( x, y, -1, -1, prc ); }
	static inline void	PIC_DrawHoles( int x, int y, const wrect_t *prc = NULL )
	{ PIC_DrawHoles( x, y, -1, -1, prc ); }
	static inline void	PIC_DrawTrans( int x, int y, const wrect_t *prc = NULL )
	{ PIC_DrawTrans( x, y, -1, -1, prc ); }
	static inline void	PIC_DrawAdditive( int x, int y, const wrect_t *prc = NULL )
	{ PIC_DrawAdditive( x, y, -1, -1, prc ); }

	static inline void PIC_Draw( Point p, Size s, const wrect_t *prc = NULL )
	{ PIC_Draw( p.x, p.y, s.w, s.h, prc ); }
	static inline void PIC_DrawHoles( Point p, Size s, const wrect_t *prc = NULL )
	{ PIC_DrawHoles( p.x, p.y, s.w, s.h, prc ); }
	static inline void PIC_DrawTrans( Point p, Size s, const wrect_t *prc = NULL )
	{ PIC_DrawTrans( p.x, p.y, s.w, s.h, prc ); }
	static inline void PIC_DrawAdditive( Point p, Size s, const wrect_t *prc = NULL )
	{ PIC_DrawAdditive( p.x, p.y, s.w, s.h, prc ); }
	static inline void PIC_Draw( Point p, const wrect_t *prc = NULL )
	{ PIC_Draw( p.x, p.y, prc ); }
	static inline void PIC_DrawHoles( Point p, const wrect_t *prc = NULL )
	{ PIC_DrawHoles( p.x, p.y, prc ); }
	static inline void PIC_DrawAdditive( Point p, const wrect_t *prc = NULL )
	{ PIC_DrawAdditive( p.x, p.y, prc ); }
	static inline void PIC_DrawTrans( Point p, const wrect_t *prc = NULL )
	{ PIC_DrawTrans( p.x, p.y, prc ); }

	static void	PIC_EnableScissor( int x, int y, int width, int height );
	static void	PIC_DisableScissor( void );

	// screen handlers
	static void	FillRGBA( int x, int y, int width, int height, int r, int g, int b, int a );

	// sound handlers
	static void	PlayLocalSound( const char *szSound );
	
	// cinematic handlers
	static void	DrawLogo( const char *filename, float x, float y, float width, float height );
	static void	PrecacheLogo( const char *filename );
	static int	GetLogoWidth( void );
	static int	GetLogoHeight( void );
	static float	GetLogoLength( void ); // cinematic duration in seconds

	// text message system
	static void	DrawCharacter( int x, int y, int width, int height, int ch, int ulRGBA, HIMAGE hFont );
	static int DrawConsoleString( int x, int y, const char *string );
	static void	DrawSetTextColor( int r, int g, int b, int alpha = 255 );
	static void	ConsoleStringLen(  const char *string, int *length, int *height );
	static int   ConsoleCharacterHeight();
	static int   DrawConsoleString( Point coord, const char *string );

	static void	SetConsoleDefaultColor( int r, int g, int b ); // color must came from colors.lst
	
	// stubs
	static void ClientCmd( ... ) { }
	static void Cmd_AddCommand( ... ) { }
	static void Cmd_RemoveCommand( ... ) { }
	static void KEY_SetDest( ... ) { }
	static void KEY_ClearStates( ... ) { }
	static void CreateMapsList( ... ) { }
	static bool ClientInGame( void ) { return true; }
	static void KEY_SetOverstrike( ... ) { }
	static bool KEY_GetOverstrike( void ) { return false; }
	
	// key handlers
	static bool KEY_IsDown( int keynum );
	
	
	// cvars & commands
	static cvar_t *CvarRegister( const char *var, const char *value, int flags );
	static void CvarSetString( const char *var, const char *value );
	static void CvarSetValue( const char *var, float value );
	static float GetCvarFloat( const char *var );
	static const char* GetCvarString( const char *var );
	static int CmdArgc( void );
	static const char *CmdArgv( int num );
	
	
	// static inline void	HostError( const char *szFmt, ... );
	static int	FileExists( const char *filename, int gamedironly = 0 );
	static void	GetGameDir( char *szGetGameDir );

	// parse txt files
	static byte*	COM_LoadFile( const char *filename, int *pLength = 0 );
	static char*	COM_ParseFile( char *data, char *token );
	static void	COM_FreeFile( void *buffer );

	// engine memory manager
	static void*	MemAlloc( size_t cb, const char *filename, const int fileline );
	static void	MemFree( void *mem, const char *filename, const int fileline );
	
	// collect info from engine
	static char 	**GetFilesList( const char *pattern, int *numFiles, int gamedironly ); // find in files
	static char	*GetClipboardData( void );

	// engine launcher
	static void	ShellExecute( const char *name, const char *args, int closeEngine );
	static void	PlayBackgroundTrack( const char *introName, const char *loopName );
	static void  StopBackgroundTrack( );
	
	// menu interface is freezed at version 0.75
	// new functions starts here
	static float	RandomFloat( float flLow, float flHigh );
	static long	RandomLong( long lLow, long lHigh );

	static void	SetCursor( void *hCursor );
	static void	ProcessImage( int texnum, float gamma, int topColor = -1, int bottomColor = -1 );
	static int	CompareFileTime( char *filename1, char *filename2, int *iCompare );
	static int COM_SaveFile( const char *filename, const void *buffer, int len );
	static int DeleteFile( const char *filename );

	// text funcs
	static void EnableTextInput( int enable );
	static int UtfProcessChar( int ch );
	static int UtfMoveLeft( char *str, int pos );
	static int UtfMoveRight( char *str, int pos, int length );
};


// built-in memory manager
#define MALLOC( x )		EngFuncs::MemAlloc( x, __FILE__, __LINE__ )
#define CALLOC( x, y )	EngFuncs::MemAlloc((x) * (y), __FILE__, __LINE__ )
#define FREE( x )		EngFuncs::MemFree( x, __FILE__, __LINE__ )

void App_Error( const char *str, ... );
#define Host_Error App_Error
#define CL_IsActive() true
#define Con_DPrintf (printf)
#define Con_NPrintf(...) // IMPLEMENT
#define Con_NXPrintf(...) // IMPLEMENT
#define Con_Printf (printf)

#endif // ENGINECALLBACKS_H
