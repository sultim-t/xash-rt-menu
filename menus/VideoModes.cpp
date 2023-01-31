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


static const char *pAmdFsrNames[] =
{
	L("Off"),
	L("Quality"),
	L("Balanced"),
	L("Performance"),
	L("Ultra Performance"),
};
#define SHOW_DLSS 1
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
static const char* pVolumetricNames[] = {
	L("Off"),
	L("Simple"),
	L("Volumetric"),
};
// must match RT_VINTAGE enum from gl_rmain.c
static const char* pVintageNames[] = {
    L( "Off" ),
    L( "CRT" ),
    L( "200p" ),
    L( "480p" ),
    L( "720p" ),
};


class CMenuVidModesModel final : public CMenuBaseArrayModel
{
public:
    void Update() override
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
	int GetRows() const override
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
    int         m_iNumModes{ 0 };
    const char* m_szModes[ 64 ]{};
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
    void _Init() override;
    void _VidInit() override;
	void Draw() override;

public:
	CMenuVidModes() : CMenuFramework( "CMenuVidModes" ) {}

	void SetMode( int mode );
	void SetMode( int w, int h );
    void TrySetChosenVidMode();
	
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
	CMenuCheckBox			muzzleFlash;
	CMenuCheckBox			nearestTextureFiltering;
	CMenuCheckBox			particlesUntextured;
	CMenuCheckBox			classic;
    CMenuCheckBox           twoBounces;
    CMenuCheckBox           volumetric;
    CMenuCheckBox           bloom;

	CMenuVidModesModel		vidListModel;
	CMenuSpinControl		vidList;

	CMenuSpinControl		nvDlss;
	CMenuSpinControl		amdFsr;
    CMenuSpinControl        vintage;
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
void CMenuVidModes::TrySetChosenVidMode()
{
	// NOTE: don't call this function frequently,
	// as there are no checks that width/height are the same

    int modeIndex = Vid_GetCurrentVidModeIndex();
	
	SetMode( modeIndex );
    EngFuncs::CvarSetValue( "vid_mode", modeIndex );
    Vid_SetCurrentVidModeIndex( modeIndex );
}

void CMenuVidModes::Draw()
{
	const int nvDlssAvailable = (int)EngFuncs::GetCvarFloat("_rt_dlss_available");

    nvDlss.SetGrayed( !nvDlssAvailable );
    amdFsr.SetGrayed( false );

	amdFsr.UpdateCvar();
    nvDlss.UpdateCvar();
	vintage.UpdateCvar();

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
	vsync.LinkCvar( "rt_vsync" );
	vsync.bUpdateImmediately = true;

	muzzleFlash.SetNameAndStatus( L( "Muzzle flash" ), L( "enable muzzle flash light source" ) );
	muzzleFlash.LinkCvar( "rt_mzlflash" );
	muzzleFlash.bUpdateImmediately = true;

	nearestTextureFiltering.SetNameAndStatus( L( "Pixelated textures" ), L( "disable texture filtering" ) );
	nearestTextureFiltering.LinkCvar( "rt_texture_nearest" );
	nearestTextureFiltering.bUpdateImmediately = true;

	particlesUntextured.SetNameAndStatus( L( "Vintage particles" ), L( "use squares for particles" ) );
	particlesUntextured.LinkCvar( "rt_particles_notex" );
	particlesUntextured.bUpdateImmediately = true;

    classic.SetNameAndStatus( L( "Classic lighting" ), L( "don't use ray tracing for illumination" ) );
    classic.LinkCvar( "rt_classic" );
    classic.bUpdateImmediately = true;

    bloom.SetNameAndStatus( L( "Bloom" ), L( "enable blooming" ) );
    bloom.LinkCvar( "rt_bloom" );
    bloom.bUpdateImmediately = true;


	static CStringArrayModel nvDlssModel( pNvDlssNames, std::size( pNvDlssNames ) );
	nvDlss.SetNameAndStatus("NVIDIA DLSS 2", L("set Nvidia DLSS"));
	nvDlss.Setup(&nvDlssModel);
	nvDlss.SetCharSize(QM_SMALLFONT);
	nvDlss.LinkCvar("rt_upscale_dlss", CMenuEditable::CVAR_VALUE);
	nvDlss.bUpdateImmediately = true;

	static CStringArrayModel amdFsrModel( pAmdFsrNames, std::size( pAmdFsrNames ) );
	amdFsr.SetNameAndStatus("AMD FSR 2.1", L("set AMD FidelityFX Super Resolution 2.1"));
	amdFsr.Setup(&amdFsrModel);
	amdFsr.SetCharSize(QM_SMALLFONT);
	amdFsr.LinkCvar("rt_upscale_fsr2", CMenuEditable::CVAR_VALUE);
	amdFsr.bUpdateImmediately = true;

    static CStringArrayModel vintageModel( pVintageNames, std::size( pVintageNames ) );
    vintage.SetNameAndStatus( "Vintage", L( "set vintage effects technique" ) );
    vintage.Setup( &vintageModel );
    vintage.SetCharSize( QM_SMALLFONT );
    vintage.LinkCvar( "rt_ef_vintage", CMenuEditable::CVAR_VALUE );
    vintage.bUpdateImmediately = true;
	
    nvDlss.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
        int val = int( dynamic_cast< CMenuEditable* >( pSelf )->CvarValue() );
        EngFuncs::CvarSetValue( "rt_upscale_dlss", float( val ) );
        if( val != 0 )
        {
            EngFuncs::CvarSetValue( "rt_upscale_fsr2", 0 );
            EngFuncs::CvarSetValue( "rt_ef_vintage", 0 );
        }
    };
    amdFsr.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
        int val = int( dynamic_cast< CMenuEditable* >( pSelf )->CvarValue() );
        EngFuncs::CvarSetValue( "rt_upscale_fsr2", float( val ) );
        if( val != 0 )
        {
            EngFuncs::CvarSetValue( "rt_upscale_dlss", 0 );
            EngFuncs::CvarSetValue( "rt_ef_vintage", 0 );
        }
    };
    vintage.onCvarWrite = []( CMenuBaseItem* pSelf, void* pExtra ) {
        int val = int( dynamic_cast< CMenuEditable* >( pSelf )->CvarValue() );
        EngFuncs::CvarSetValue( "rt_ef_vintage", float( val ) );
        if( val != 0 )
        {
            EngFuncs::CvarSetValue( "rt_upscale_dlss", 0 );
            EngFuncs::CvarSetValue( "rt_upscale_fsr2", 0 );
        }
    };

	/*static CStringArrayModel sharpeningModel( pSharpeningNames, std::size( pSharpeningNames ) );
	sharpening.SetNameAndStatus("Sharpening", L("set sharpening to apply on top of image"));
	sharpening.Setup(&sharpeningModel);
	sharpening.SetCharSize(QM_SMALLFONT);
	sharpening.LinkCvar("rt_sharpen", CMenuEditable::CVAR_VALUE);
	sharpening.bUpdateImmediately = true;*/

	/*static CStringArrayModel volumetricModel( pVolumetricNames, std::size( pVolumetricNames ) );
	volumetric.SetNameAndStatus("Scattering", L("set volumetric effects technique"));
    volumetric.Setup( &volumetricModel );
	volumetric.SetCharSize(QM_SMALLFONT);
	volumetric.LinkCvar("rt_volume_type", CMenuEditable::CVAR_VALUE);
    volumetric.bUpdateImmediately = true;*/

    volumetric.SetNameAndStatus( L( "Volumetrics" ),
                              L( "set volumetric effects technique" ) );
    volumetric.LinkCvar( "rt_volume_type" );
    volumetric.bUpdateImmediately = true;


	AddItem( background );
	AddItem( banner );
	AddItem( vidList );
    auto& applyBtn = *AddButton( L( "Apply" ), L( "Apply window size" ), PC_ACTIVATE, VoidCb( &CMenuVidModes::TrySetChosenVidMode ) );
    AddItem( vsync );
#if SHOW_DLSS
	AddItem( nvDlss );
#endif
	AddItem( amdFsr );
    AddItem( classic );
    AddItem( vintage );
    std::vector< CMenuBaseItem* > checkboxes[] = {
        { &volumetric, &bloom },
        { &muzzleFlash, &nearestTextureFiltering },
    };
    for( auto& line : checkboxes )
    {
        for( CMenuBaseItem* pc : line )
        {
            AddItem( pc );
        }
    }
    auto& doneBtn  = *AddButton( L( "Done" ), L( "Return back to previous menu" ), PC_DONE, VoidCb( &CMenuVidModes::Hide ) );


	{
	    // clang-format off
		#define SPIN_W 300
		#define SPIN_H 24

        auto GetX = []( int j ) {
            return BASE_OFFSET_X + j * ( SPIN_W + 72 );
        };
        auto GetXSmall = []( int j ) {
            return BASE_OFFSET_X + j * ( SPIN_W / 2 + 72 );
        };
        auto GetY = []( float i ) {
            return ( int )( 230 + i * ( SPIN_H * 2 ) );
        };

        float i = 0;


        // spin-controls
        vidList.SetRect( GetX( 0 ), GetY( i ), SPIN_W, SPIN_H ); vsync.SetCoord( GetX( 1 ), GetY( i - 0.1f ) );
        i += 1.0f;

		applyBtn.SetCoord( GetX( 0 ), GetY( i - 0.05f ) );
        i += 2.0f;

#if SHOW_DLSS
        nvDlss.SetRect( GetX( 0 ), GetY( i ), SPIN_W, SPIN_H ); amdFsr.SetRect( GetX( 1 ), GetY( i ), SPIN_W, SPIN_H );
#else
        amdFsr.SetRect( GetX( 0 ), GetY( i ), SPIN_W, SPIN_H );
#endif
        i += 1.5f;
	    classic.SetCoord( GetX( 0 ), GetY( i ) ); vintage.SetRect( GetX( 1 ), GetY( i + 0.1f ), SPIN_W, SPIN_H );
        i += 1.25f;
        // clang-format on


        // switch-buttons
        for( auto& line : checkboxes )
        {
            for( int x = 0; x < int( line.size() ); x++ )
            {
                line[ x ]->SetCoord( GetXSmall( x ), GetY( i ) );
            }
            i += 1.25f;
        }


        // back button
        i += 0.5f;
        doneBtn.SetCoord( GetX( 0 ), GetY( i ) );
	}
}

void CMenuVidModes::_VidInit()
{
	Vid_SetCurrentVidModeIndex( static_cast< int >( EngFuncs::GetCvarFloat( "vid_mode" ) ) );
}

ADD_MENU( menu_vidmodes, CMenuVidModes, UI_VidModes_Menu );
