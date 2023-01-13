/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Action.h"

#define ART_BANNER		"gfx/shell/head_saveload"

class CMenuSaveLoad : public CMenuFramework
{
public:
	CMenuSaveLoad() : CMenuFramework( "CMenuSaveLoad" ) { }
private:
	void _Init() override;
	void _VidInit() override;

	CMenuPicButton* newgame{ nullptr };
	CMenuPicButton* save{ nullptr };
	CMenuPicButton* load{ nullptr };

	CMenuAction	hintMessage;
	char		hintText[MAX_HINT_TEXT];
};

/*
=================
UI_SaveLoad_Init
=================
*/
void CMenuSaveLoad::_Init( void )
{
    snprintf( hintText,
              sizeof( hintText ),
              L( "During play:\n"
                 "        Press %s - to quickly save the game\n"
                 "        Press %s - to load that saved game" ),
		EngFuncs::KeynumToString( KEY_GetKey( "save quick" ) ),
		EngFuncs::KeynumToString( KEY_GetKey( "load quick" ) ) );

	banner.SetPicture( ART_BANNER );

	hintMessage.iFlags = QMF_INACTIVE;
	hintMessage.colorBase = uiColorHelp;
	hintMessage.SetCharSize( QM_SMALLFONT );
	hintMessage.szName = hintText;
	hintMessage.SetCoord( 440, 280 );

	AddItem( background );
	AddItem( banner );

	newgame = AddButton(
        L( "New game" ), L( "StringsList_189" ), PC_NEW_GAME, UI_NewGame_Menu, QMF_NOTIFY );
	load = AddButton( L( "Load" ), L( "GameUI_LoadGameHelp" ), PC_LOAD_GAME, UI_LoadGame_Menu, QMF_NOTIFY );
    save = AddButton(
        L( "Save" ), L( "GameUI_SaveGameHelp" ), PC_SAVE_GAME, UI_SaveGame_Menu, QMF_NOTIFY );
	AddButton( L( "Back" ), L( "Go back to the Main menu" ), PC_DONE, VoidCb( &CMenuSaveLoad::Hide ), QMF_NOTIFY );
	AddItem( hintMessage );
}

void CMenuSaveLoad::_VidInit()
{
    if( !save || !load || !newgame )
    {
        return;
    }

    bool isSingle = gpGlobals->maxClients < 2;

    bool showNewGame = true;
    bool showLoad = true;
    bool showSave = true;

    if( gMenu.m_gameinfo.gamemode == GAME_MULTIPLAYER_ONLY || gMenu.m_gameinfo.startmap[ 0 ] == 0 )
    {
        showNewGame = false;
    }

    if( gMenu.m_gameinfo.gamemode == GAME_MULTIPLAYER_ONLY )
    {
        showLoad = false;
        showSave = false;
    }

	if( !EngFuncs::CheckGameDll() )
    {
        showNewGame = false;
        showLoad = false;
        showSave = false;
    }

    if( !CL_IsActive() || !isSingle )
    {
        showSave = false;
    }

    newgame->SetGrayed( !showNewGame );
    load->SetGrayed( !showLoad );
    save->SetGrayed( !showSave );
}

ADD_MENU( menu_saveload, CMenuSaveLoad, UI_SaveLoad_Menu );
