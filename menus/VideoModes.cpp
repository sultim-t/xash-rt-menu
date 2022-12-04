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

#include <vector>

#include "Framework.h"
#include "MenuStrings.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Table.h"
#include "CheckBox.h"
#include "Action.h"
#include "YesNoMessageBox.h"
#include "SpinControl.h"
#include "StringArrayModel.h"
#include "utlvector.h"

#define ART_BANNER		"gfx/shell/head_vidmodes"


#define DISABLE_RENDERER_API_SWITCH 1


static const char *pAmdFsrNames[] =
{
	L("Off"),
	L("Quality"),
	L("Balanced"),
	L("Performance"),
	L("Ultra Performance"),
};
static const char *pNvDlssNames[] =
{
	L("Off"),
	L("Quality"),
	L("Balanced"),
	L("Performance"),
	L("Ultra Performance"),
};
static const char *pSharpeningNames[] =
{
	L("Off"),
	L("Naive"),
	L("AMD CAS"),
};


class CMenuVidModesModel : public CMenuBaseArrayModel
{
public:
	void Update()
	{
		int i;

		for (i = 0; i < 64; i++)
		{
			const char *mode = EngFuncs::GetModeString(i);
			if (!mode) break;
			m_szModes[i] = mode;
		}
		m_iNumModes = i;
	}
	int GetRows() const
	{
		return m_iNumModes;
	}
	const char *GetText(int i) override
	{
		return m_szModes[IndexToVidMode(i)];
	}
	int IndexToVidMode(int i)
	{
		ASSERT(m_iNumModes != 0);

		i = Q_max(0, Q_min(i, m_iNumModes - 1));

		// inverse indexing, because higher resolutions
		// should be on the right side
		return (m_iNumModes - 1) - i;
	}
	int VidModeToIndex(int i)
	{
		// same
		return IndexToVidMode(i);
	}

private:
	int m_iNumModes;
	const char *m_szModes[64];
};


class CMenuRenderersModel : public CMenuBaseArrayModel
{
public:
	void Update() override
	{
		refdll temp;

		for( unsigned int i = 0;
			EngFuncs::GetRenderers( i, temp.shortName, sizeof( temp.shortName ), temp.readable, sizeof( temp.readable ));
			i++ )
		{
			m_refs.AddToTail( temp );
		}
	}
	int GetRows() const override { return m_refs.Count(); }
	const char *GetText( int i ) override { return m_refs[i].readable; }

	const char *GetShortName( int i ) { return m_refs[i].shortName; }
	void Add( const char *str )
	{
		refdll temp;

		Q_strncpy( temp.shortName, str, sizeof( temp.shortName ));
		Q_strncpy( temp.readable, str, sizeof( temp.readable ));

		m_refs.AddToTail( temp );
	}
private:
	struct refdll
	{
		char shortName[64];
		char readable[64];
	};

	CUtlVector<refdll> m_refs;
};


class CMenuVidModes : public CMenuFramework
{
private:
	void _Init();
	void _VidInit();
	void Draw(); // put test mode timer here
public:
	CMenuVidModes() : CMenuFramework( "CMenuVidModes" ) { testModeTimer = 0; }

	void SetMode( int mode );
	void SetMode( int w, int h );
	void SetConfig( );
	void SetConfigAndHide( );
	void RevertChanges();
	void ApplyChanges();

	void GetRendererConfig()
	{
		const char *refdll = EngFuncs::GetCvarString( "r_refdll" );

		if( !refdll[0] )
		{
			renderers.SetCurrentValue( 0.0f );
		}

		int i;
		for( i = 0; i < renderersModel.GetRows(); i++ )
		{
			if( !stricmp( renderersModel.GetShortName( i ), refdll ) )
			{
				renderers.SetCurrentValue( i );
				break;
			}
		}

		if( i == renderersModel.GetRows() )
		{
			renderersModel.Add( refdll );
			renderers.SetCurrentValue( i );
		}
	}

	void WriteRendererConfig()
	{
		int i = renderers.GetCurrentValue();
		EngFuncs::CvarSetString( "r_refdll", renderersModel.GetShortName( i ));
	}

	int Vid_GetCurrentVidModeIndex()
	{
		return vidListModel.IndexToVidMode(vidList.GetCurrentValue());
	}

	void Vid_SetCurrentVidModeIndex(int vidMode)
	{
		return vidList.SetCurrentValue(vidListModel.VidModeToIndex(vidMode));
	}

public:
	CMenuCheckBox			vsync;
	CMenuCheckBox			disablePvsCulling;
	CMenuCheckBox			muzzleFlash;
	CMenuCheckBox			nearestTextureFiltering;
	CMenuCheckBox			particlesUntextured;

	CMenuVidModesModel		vidListModel;
	CMenuSpinControl		vidList;

	CMenuYesNoMessageBox	testModeMsgBox;

	CMenuRenderersModel		renderersModel;
	CMenuSpinControl		renderers;

	CMenuSpinControl		nvDlss;
	CMenuSpinControl		amdFsr;
	CMenuSpinControl		sharpening;

	int prevMode;
	int prevModeX;
	int prevModeY;
	bool prevFullscreen;
	float testModeTimer;
	char testModeMsg[256];
};


void CMenuVidModes::SetMode( int w, int h )
{
	// only possible on Xash3D FWGS!
	char cmd[64];
	snprintf( cmd, sizeof( cmd ), "vid_setmode %i %i\n", w, h );
	EngFuncs::ClientCmd( TRUE, cmd );
}

void CMenuVidModes::SetMode( int mode )
{
	char cmd[64];
	snprintf( cmd, sizeof( cmd ), "vid_setmode %i\n", mode );

	EngFuncs::ClientCmd( TRUE, cmd );
}

/*
=================
UI_VidModes_SetConfig
=================
*/
void CMenuVidModes::SetConfig( )
{
	// called on applyBtn

	bool testMode = false;
	int  currentModeIndex = Vid_GetCurrentVidModeIndex();
	bool isVidModeChanged = prevMode != currentModeIndex;

	if( isVidModeChanged )
	{
		SetMode( currentModeIndex );
		EngFuncs::CvarSetValue( "vid_mode", currentModeIndex );
		Vid_SetCurrentVidModeIndex( currentModeIndex );
	}

	if( testMode )
	{
		testModeMsgBox.Show();
		testModeTimer = gpGlobals->time + 10.0f; // ten seconds should be enough
	}
}

void CMenuVidModes::SetConfigAndHide()
{
	SetConfig();
	Hide();
}

void CMenuVidModes::ApplyChanges()
{
	prevMode = EngFuncs::GetCvarFloat( "vid_mode" );
	prevFullscreen = EngFuncs::GetCvarFloat( "fullscreen" );
	prevModeX = EngFuncs::GetCvarFloat( "width" );
	prevModeY = EngFuncs::GetCvarFloat( "height" );
}

void CMenuVidModes::RevertChanges()
{
	EngFuncs::CvarSetValue("fullscreen", prevFullscreen);
	
	SetMode( prevModeX, prevModeY );
}

void CMenuVidModes::Draw()
{
	const int nvDlssCvar = (int)EngFuncs::GetCvarFloat("rt_iUpscaleNvDlss");
	const int amdFsrCvar = (int)EngFuncs::GetCvarFloat("rt_iUpscaleAmdFsr2");
	const int nvDlssAvailable = (int)EngFuncs::GetCvarFloat("_rt_bDLSSAvailable");

	if (nvDlssAvailable)
	{
		if (nvDlssCvar == 0 && amdFsrCvar == 0)
		{
			nvDlss.SetGrayed(false);
			amdFsr.SetGrayed(false);
		}
		else if (nvDlssCvar == 0 && amdFsrCvar != 0)
		{
			nvDlss.SetGrayed(true);
			amdFsr.SetGrayed(false);
		}
		else if (nvDlssCvar != 0 && amdFsrCvar == 0)
		{
			nvDlss.SetGrayed(false);
			amdFsr.SetGrayed(true);
		}
		else
		{
			// were unsafely modified by cvars directly, gray both of them
			nvDlss.SetGrayed(true);
			amdFsr.SetGrayed(true);
		}
	}
	else
	{
		nvDlss.SetGrayed(true);
		amdFsr.SetGrayed(false);
	}


	if (testModeMsgBox.IsVisible())
	{
		if (testModeTimer - gpGlobals->time > 0)
		{
			snprintf(testModeMsg, sizeof(testModeMsg) - 1, L("Keep this resolution? %i seconds remaining"), (int)(testModeTimer - gpGlobals->time));
			testModeMsg[sizeof(testModeMsg) - 1] = 0;
		}
		else
		{
			RevertChanges();
			testModeMsgBox.Hide();
		}
	}


	CMenuFramework::Draw();
}

/*
=================
UI_VidModes_Init
=================
*/
void CMenuVidModes::_Init( void )
{
	banner.SetPicture(ART_BANNER);


	vidListModel.Update();
	vidList.szName = L( "Window size" );
	vidList.Setup( &vidListModel );
	vidList.SetCharSize( QM_SMALLFONT );
	vidList.bUpdateImmediately = true;


	vsync.SetNameAndStatus( L( "VSync" ), L( "enable vertical synchronization for RT renderer" ) );
	vsync.LinkCvar( "rt_bVsync" );
	vsync.bUpdateImmediately = true;

	disablePvsCulling.SetNameAndStatus( L( "Render all" ), L( "if true, in-engine PVS culling is disabled, e.g. shadows of the objects behind walls will be visible" ) );
	disablePvsCulling.LinkCvar( "rt_bDisablePvsCulling" );
	disablePvsCulling.bUpdateImmediately = true;

	muzzleFlash.SetNameAndStatus( L( "Muzzle flash" ), L( "enable muzzle flash light source" ) );
	muzzleFlash.LinkCvar( "rt_bMuzzleFlashEnable" );
	muzzleFlash.bUpdateImmediately = true;

	nearestTextureFiltering.SetNameAndStatus( L( "Vintage textures" ), L( "disable texture filtering" ) );
	nearestTextureFiltering.LinkCvar( "rt_bTextureFilterNearest" );
	nearestTextureFiltering.bUpdateImmediately = true;

	particlesUntextured.SetNameAndStatus( L( "Vintage particles" ), L( "use squares for particles" ) );
	particlesUntextured.LinkCvar( "rt_bParticlesUntextured" );
	particlesUntextured.bUpdateImmediately = true;

	CMenuBaseItem *pCheckboxes[] =
	{
		&vsync,
		&muzzleFlash,
		&nearestTextureFiltering,
		&particlesUntextured,
		// &disablePvsCulling,
	};


	testModeMsgBox.SetMessage( testModeMsg );
	testModeMsgBox.onPositive = VoidCb( &CMenuVidModes::ApplyChanges );
	testModeMsgBox.onNegative = VoidCb( &CMenuVidModes::RevertChanges );
	testModeMsgBox.Link( this );


	renderersModel.Update();
	renderers.szName = L( "GameUI_Renderer" );
	renderers.Setup( &renderersModel );
	renderers.SetCharSize( QM_SMALLFONT );
	renderers.onCvarGet = VoidCb( &CMenuVidModes::GetRendererConfig );
	renderers.onCvarWrite = VoidCb( &CMenuVidModes::WriteRendererConfig );
	renderers.bUpdateImmediately = true;


	static CStringArrayModel nvDlssModel(pNvDlssNames, V_ARRAYSIZE(pNvDlssNames));
	nvDlss.SetNameAndStatus("NVIDIA DLSS 2", L("set Nvidia DLSS"));
	nvDlss.Setup(&nvDlssModel);
	nvDlss.SetCharSize(QM_SMALLFONT);
	nvDlss.LinkCvar("rt_iUpscaleNvDlss", CMenuEditable::CVAR_VALUE);
	nvDlss.bUpdateImmediately = true;

	static CStringArrayModel amdFsrModel(pAmdFsrNames, V_ARRAYSIZE(pAmdFsrNames));
	amdFsr.SetNameAndStatus("AMD FSR 2.1", L("set AMD FidelityFX Super Resolution 2.1"));
	amdFsr.Setup(&amdFsrModel);
	amdFsr.SetCharSize(QM_SMALLFONT);
	amdFsr.LinkCvar("rt_iUpscaleAmdFsr2", CMenuEditable::CVAR_VALUE);
	amdFsr.bUpdateImmediately = true;

	static CStringArrayModel sharpeningModel(pSharpeningNames, V_ARRAYSIZE(pSharpeningNames));
	sharpening.SetNameAndStatus("Sharpening", L("set sharpening to apply on top of image"));
	sharpening.Setup(&sharpeningModel);
	sharpening.SetCharSize(QM_SMALLFONT);
	sharpening.LinkCvar("rt_iSharpeningMode", CMenuEditable::CVAR_VALUE);
	sharpening.bUpdateImmediately = true;


	AddItem( background );
	AddItem( banner );
	AddItem( renderers );
	AddItem( vidList );
	AddItem( nvDlss );
	AddItem( amdFsr );
	AddItem( sharpening );

	for (auto *pc : pCheckboxes)
	{
		AddItem(pc);
	}

#if DISABLE_RENDERER_API_SWITCH
	// since we don't have Renderer API change, Apply and Done have the same meaning
	auto &doneBtn = *AddButton(L("Apply"), L("Apply renderer settings"), PC_DONE, VoidCb(&CMenuVidModes::SetConfigAndHide));
#else
	// auto &applyBtn = *AddButton( L( "Apply" ), L( "Apply renderer and window size" ), PC_ACTIVATE, VoidCb( &CMenuVidModes::SetConfig ) );
	// auto &doneBtn  = *AddButton( L( "Done" ), L( "Return back to previous menu" ), PC_DONE, VoidCb( &CMenuVidModes::Hide ) );
#endif


	{
		#define SPIN_W 300
		#define SPIN_H 24
		#define NUM_COLUMNS 2
		auto GetX = [] (int j)   { return BASE_OFFSET_X + j * (SPIN_W + 72); };
		auto GetY = [] (float i) { return (int)(230 + i * (SPIN_H * 2)); };

		float i = 0;


		// spin-controls
#if DISABLE_RENDERER_API_SWITCH
		vidList.SetRect(GetX(0), GetY(i), SPIN_W, SPIN_H);
		i += 2.5f;
#else
		renderers .SetRect(GetX(0), GetY(i), SPIN_W, SPIN_H); vidList.SetRect(GetX(1), GetY(i), SPIN_W, SPIN_H);
		i += 1.0f;
		applyBtn  .SetCoord(GetX(0), GetY(i - 0.1f));
		i += 2.5f;
#endif

		nvDlss    .SetRect(GetX(0), GetY(i), SPIN_W, SPIN_H); amdFsr .SetRect(GetX(1), GetY(i), SPIN_W, SPIN_H);
		i += 1.5f;

		sharpening.SetRect(GetX(0), GetY(i), SPIN_W, SPIN_H);
		i += 1.5f;


		// switch-buttons
		for (int k = 0; k < (int)std::size(pCheckboxes); k++)
		{
			if (k % NUM_COLUMNS == 0 && k != 0)
			{
				i++;
			}

			pCheckboxes[k]->SetCoord(GetX(k % NUM_COLUMNS), GetY(i));
		}


		// back button
		i += 2.0f;
		doneBtn.SetCoord(GetX(0), GetY(i));
	}

	renderers.LinkCvar( "r_refdll", CMenuEditable::CVAR_STRING );
}

void CMenuVidModes::_VidInit()
{
	// don't overwrite prev values
	if( !testModeMsgBox.IsVisible() )
	{
		ApplyChanges( );
		Vid_SetCurrentVidModeIndex( prevMode );
	}
}

ADD_MENU( menu_vidmodes, CMenuVidModes, UI_VidModes_Menu );
