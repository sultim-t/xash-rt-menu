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
#include "Action.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "YesNoMessageBox.h"
#include "keydefs.h"
#include "MenuStrings.h"
#include "PlayerIntroduceDialog.h"

#include <initializer_list>

#define ART_MINIMIZE_N	"gfx/shell/min_n"
#define ART_MINIMIZE_F	"gfx/shell/min_f"
#define ART_MINIMIZE_D	"gfx/shell/min_d"
#define ART_CLOSEBTN_N	"gfx/shell/cls_n"
#define ART_CLOSEBTN_F	"gfx/shell/cls_f"
#define ART_CLOSEBTN_D	"gfx/shell/cls_d"

class CMenuMain: public CMenuFramework
{
public:
	CMenuMain() : CMenuFramework( "CMenuMain" ) { }

	bool KeyDown( int key ) override;

private:
	void _Init() override;
	void _VidInit( ) override;

	void VidInit(bool connected);

	void QuitDialog( void *pExtra = NULL );
	void DisconnectCb();
	void DisconnectDialogCb();
	
	class CMenuMainBanner : public CMenuBannerBitmap
	{
	public:
		virtual void Draw();
	} banner;

	CMenuPicButton	resumeGame;
	CMenuPicButton	disconnect;
	CMenuPicButton	singleplayer;
    CMenuPicButton	graphics;
    CMenuPicButton	audio;
    CMenuPicButton	controls;
	CMenuPicButton	multiPlayer;
	CMenuPicButton	customGame;
	CMenuPicButton	quit;

	// quit dialog
	CMenuYesNoMessageBox dialog;

	bool bCustomGame;
};

void CMenuMain::CMenuMainBanner::Draw()
{
	if( !CMenuBackgroundBitmap::ShouldDrawLogoMovie() )
		return; // no logos for steam background

	if( EngFuncs::GetLogoLength() <= 0.05f || EngFuncs::GetLogoWidth() <= 32 )
		return;	// don't draw stub logo (GoldSrc rules)

	float	logoWidth, logoHeight, logoPosY;
	float	scaleX, scaleY;

	scaleX = ScreenWidth / 640.0f;
	scaleY = ScreenHeight / 480.0f;

	// a1ba: multiply by height scale to look better on widescreens
	logoWidth = EngFuncs::GetLogoWidth() * scaleX;
	logoHeight = EngFuncs::GetLogoHeight() * scaleY * uiStatic.scaleY;
	logoPosY = 70 * scaleY * uiStatic.scaleY;	// 70 it's empirically determined value (magic number)

	EngFuncs::DrawLogo( "logo.avi", 0, logoPosY, logoWidth, logoHeight );
}

void CMenuMain::QuitDialog(void *pExtra)
{
	if( CL_IsActive() && EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) == 1.0f )
		dialog.SetMessage( L( "StringsList_235" ) );
	else
		dialog.SetMessage( L( "GameUI_QuitConfirmationText" ) );

	dialog.onPositive.SetCommand( FALSE, "quit\n" );
	dialog.Show();
}

void CMenuMain::DisconnectCb()
{
	EngFuncs::ClientCmd( FALSE, "disconnect\n" );
	VidInit( false );
}

void CMenuMain::DisconnectDialogCb()
{
	dialog.onPositive = VoidCb( &CMenuMain::DisconnectCb );
	dialog.SetMessage( L( "Really disconnect?" ) );
	dialog.Show();
}

/*
=================
CMenuMain::Key
=================
*/
bool CMenuMain::KeyDown( int key )
{
	if( UI::Key::IsEscape( key ) )
	{
		if ( CL_IsActive( ))
		{
			if( !dialog.IsVisible() )
				UI_CloseMenu();
		}
		else
		{
			QuitDialog( );
		}
		return true;
	}
	return CMenuFramework::KeyDown( key );
}

void CMenuMain::_Init( void )
{
#if !XASH_RAYTRACING
	if( EngFuncs::GetCvarFloat( "host_allow_changegame" ))
		bCustomGame = true;
	else
#endif
		bCustomGame = false;
	
	resumeGame.SetNameAndStatus( L( "Resume" ), L( "StringsList_188" ) );
	resumeGame.SetPicture( PC_RESUME_GAME );
	resumeGame.iFlags |= QMF_NOTIFY;
	resumeGame.onReleased = UI_CloseMenu;

	disconnect.SetNameAndStatus( L( "GameUI_GameMenu_Disconnect" ), L( "Disconnect from server" ) );
	disconnect.SetPicture( PC_DISCONNECT );
	disconnect.iFlags |= QMF_NOTIFY;
	disconnect.onReleased = VoidCb( &CMenuMain::DisconnectDialogCb );

	singleplayer.SetNameAndStatus( L( "Campaign" ), L( "Start new game, load or create a save" ) );
	singleplayer.SetPicture( PC_SAVE_LOAD_GAME );
	singleplayer.onReleased = UI_SaveLoad_Menu;
	singleplayer.iFlags |= QMF_NOTIFY;

	multiPlayer.SetNameAndStatus( L( "GameUI_Multiplayer" ), L( "StringsList_198" ) );
	multiPlayer.SetPicture( PC_MULTIPLAYER );
	multiPlayer.iFlags |= QMF_NOTIFY;
	multiPlayer.onReleased = UI_MultiPlayer_Menu;

    graphics.SetNameAndStatus( L( "Graphics" ), L( "Rendering API settings, window size" ) );
    graphics.SetPicture( PC_VID_MODES );
    graphics.iFlags |= QMF_NOTIFY;
    graphics.onReleased = UI_VidModes_Menu;

    audio.SetNameAndStatus( L( "GameUI_Audio" ), L( "Sound volume and quality" ) );
    audio.SetPicture( PC_AUDIO );
    audio.iFlags |= QMF_NOTIFY;
    audio.onReleased = UI_Audio_Menu;

    controls.SetNameAndStatus( L( "Controls" ), L( "Keyboard, mouse and gamepad settings" ) );
    controls.SetPicture( PC_CONTROLS );
    controls.iFlags |= QMF_NOTIFY;
    controls.onReleased = UI_Controls_Menu;

	customGame.SetNameAndStatus( L( "GameUI_ChangeGame" ), L( "StringsList_530" ) );
	customGame.SetPicture( PC_CUSTOM_GAME );
	customGame.iFlags |= QMF_NOTIFY;
	customGame.onReleased = UI_CustomGame_Menu;

	quit.SetNameAndStatus( L( "GameUI_GameMenu_Quit" ), L( "GameUI_QuitConfirmationText" ) );
	quit.SetPicture( PC_QUIT );
	quit.iFlags |= QMF_NOTIFY;
	quit.onReleased = MenuCb( &CMenuMain::QuitDialog );
	
	if ( gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY )
		multiPlayer.SetGrayed( true );
	
	// server.dll needs for reading savefiles or startup newgame
	if( !EngFuncs::CheckGameDll( ))
	{
        singleplayer.SetGrayed( true );
	}

	dialog.Link( this );

	AddItem( background );
	AddItem( banner );

	AddItem( resumeGame );

	AddItem( singleplayer );

	AddItem( multiPlayer );

    AddItem( graphics );
    AddItem( controls );
    AddItem( audio );

	AddItem( quit );
    AddItem( disconnect );

    if( bCustomGame )
        AddItem( customGame );
}

void PlaceOnLine( std::initializer_list< CMenuPicButton* > arr, int y )
{
    int i = 0;
    for( auto* b : arr )
    {
        b->SetCoord( BASE_OFFSET_X + ( i * 150 ), y );
        i++;
    }
}

/*
=================
UI_Main_Init
=================
*/
void CMenuMain::VidInit( bool connected )
{
    bool isSingle = gpGlobals->maxClients < 2;

    bool showResume     = connected;
    bool showDisconnect = connected && !isSingle;

    resumeGame.SetVisibility( showResume );
    disconnect.SetVisibility( showDisconnect );

	struct Offset
    {
        int base = 350;
        int Next()
        {
            int c = base;
            base += 50;
            return c;
        }
        void Padding() { base += 0; }
    } offset;

    if( showResume )
    {
        PlaceOnLine( { &resumeGame }, offset.Next() );
        offset.Padding();
    }

    PlaceOnLine( { &singleplayer }, offset.Next() );
    PlaceOnLine( { &multiPlayer }, offset.Next() );
    offset.Padding();

    PlaceOnLine( { &graphics, &controls, &audio }, offset.Next() );
    offset.Padding();

    PlaceOnLine( { &quit, &disconnect, &customGame }, offset.Next() );
}

void CMenuMain::_VidInit()
{
	VidInit( CL_IsActive() );
}

ADD_MENU( menu_main, CMenuMain, UI_Main_Menu );
