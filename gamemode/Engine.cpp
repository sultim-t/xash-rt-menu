/*
enginecallback.cpp - actual engine callbacks
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

#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "ConnectionProgress.h"
#include "ConnectionWarning.h"

/* 
===================
Key_GetKey
===================
*/
int KEY_GetKey( const char *binding )
{
	const char *b;

	if ( !binding )
		return -1;

	for ( int i = 0; i < 256; i++ )
	{
		b = EngFuncs::KEY_GetBinding( i );
		if( !b ) continue;

		if( !stricmp( binding, b ))
			return i;
	}
	return -1;
}

void EngFuncs::PIC_Set(HIMAGE hPic, int r, int g, int b, int a)
{
	if( uiStatic.enableAlphaFactor )
		a *= uiStatic.alphaFactor;

	engfuncs.pfnPIC_Set( hPic, r, g, b, a );
}

void EngFuncs::FillRGBA(int x, int y, int width, int height, int r, int g, int b, int a)
{
	if( uiStatic.enableAlphaFactor )
		a *= uiStatic.alphaFactor;

	engfuncs.pfnFillRGBA( x, y, width, height, r, g, b, a );
}

void EngFuncs::DrawLogo( const char *filename, float x, float y, float width, float height )
{
	if( uiStatic.enableAlphaFactor )
		return;

	engfuncs.pfnDrawLogo( filename, x, y, width, height );
}


void EngFuncs::DrawCharacter(int x, int y, int width, int height, int ch, int ulRGBA, HIMAGE hFont)
{
	engfuncs.pfnDrawCharacter( x, y, width, height, ch, ulRGBA, hFont );
}

unsigned int color;

void EngFuncs::DrawSetTextColor(int r, int g, int b, int alpha)
{
	color = PackRGBA( r, g, b, alpha );
}

int EngFuncs::DrawConsoleString(int x, int y, const char *string)
{
	Point pt( x, y );
	Size sz;
	Size charSz;

	sz.w = ScreenWidth - pt.x;
	sz.h = ScreenHeight - pt.y;

	charSz.w = 0;
	charSz.h = g_FontMgr.GetFontTall( uiStatic.hConsoleFont );

	return UI_DrawString( uiStatic.hConsoleFont, pt, sz, string, color, false, charSz, QM_TOPLEFT, false );
}

void EngFuncs::ConsoleStringLen(const char *string, int *length, int *height)
{
	g_FontMgr.GetTextSize( uiStatic.hConsoleFont, string, length, height );
}

int EngFuncs::ConsoleCharacterHeight()
{
	return g_FontMgr.GetFontTall( uiStatic.hConsoleFont );
}

// We have full unicode support now
int EngFuncs::UtfProcessChar(int ch)
{
	return Con_UtfProcessChar( ch );
}

int EngFuncs::UtfMoveLeft(char *str, int pos)
{
	return Con_UtfMoveLeft( str, pos );
}

int EngFuncs::UtfMoveRight(char *str, int pos, int length)
{
	return Con_UtfMoveRight( str, pos, length );
}

void Mode_Init( void )
{
	EngFuncs::Cmd_AddCommand( "menu_connectionprogress", UI_ConnectionProgress_f );
	EngFuncs::Cmd_AddCommand( "menu_connectionwarning", UI_ConnectionWarning_f );

	// can be hijacked, but please, don't do it
	const char *version = EngFuncs::GetCvarString( "host_ver" );

	uiStatic.isForkedEngine = version && version[0];

	// setup game info
	EngFuncs::GetGameInfo( &gMenu.m_gameinfo );

}
