/************************************************************************************

Filename    :   OvrApp.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
#include <jni.h>

#include "OvrApp.h"
#include "PathUtils.h"
#include "VideoMenu.h"
#include "VRMenu/GuiSys.h"


extern "C" {

jlong Java_oculus_MainActivity_nativeSetAppInterface( JNIEnv * jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	LOG( "nativeSetAppInterface" );
	return (new OvrApp())->SetActivity( jni, clazz, activity, fromPackageName, commandString, uriString );
}

} // extern "C"
float PixelScale( const float x )
{
	return x * VRMenuObject::DEFAULT_TEXEL_SCALE;
}

Vector3f PixelPos( const float x, const float y, const float z )
{
	return Vector3f( PixelScale( x ), PixelScale( y ), PixelScale( z ) );
}

using namespace OVR;

OvrApp::OvrApp()
{
}

OvrApp::~OvrApp()
{
}
void OvrApp::ShowUI()
{
	LOG( "ShowUI" );
	//Cinema.SceneMgr.ForceMono = true;
	app->GetGazeCursor().ShowCursor();

	PlaybackControlsMenu->Open();
	GazeTimer.SetGazeTime();

	PlaybackControlsScale.SetLocalScale( Vector3f( Cinema.SceneMgr.GetScreenSize().y * ( 500.0f / 1080.0f ) ) );
	PlaybackControlsPosition.SetLocalPose( Cinema.SceneMgr.GetScreenPose() );

	//uiActive = true;
}
void OvrApp::HideUI()
{
	LOG( "HideUI" );
	PlaybackControlsMenu->Close();

//	Cinema.app->GetGazeCursor().HideCursor();
//	Cinema.SceneMgr.ForceMono = false;
//	uiActive = false;
//
//	SeekSpeed = 0;
//	PlaybackPos = 0;
//	NextSeekTime = 0;
//
//	BackgroundClicked = false;
//
//	SetSeekIcon( SeekSpeed );
}
void OvrApp::CreateMenu( App * app, OvrVRMenuMgr & menuMgr, BitmapFont const & font )
{
	BackgroundTintTexture.LoadTextureFromApplicationPackage( "assets/backgroundTint.png" );

	RWTexture.LoadTextureFromApplicationPackage( "assets/img_btn_rw.png" );
	RWHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_rw_hover.png" );
	RWPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_rw_pressed.png" );

	FFTexture.LoadTextureFromApplicationPackage( "assets/img_btn_ff.png" );
	FFHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_ff_hover.png" );
	FFPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_ff_pressed.png" );

	PlayTexture.LoadTextureFromApplicationPackage( "assets/img_btn_play.png" );
	PlayHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_play_hover.png" );
	PlayPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_play_pressed.png" );

	PauseTexture.LoadTextureFromApplicationPackage( "assets/img_btn_pause.png" );
	PauseHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_pause_hover.png" );
	PausePressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_pause_pressed.png" );

	CarouselTexture.LoadTextureFromApplicationPackage( "assets/img_btn_carousel.png" );
	CarouselHoverTexture.LoadTextureFromApplicationPackage( "assets/img_btn_carousel_hover.png" );
	CarouselPressedTexture.LoadTextureFromApplicationPackage( "assets/img_btn_carousel_pressed.png" );

	SeekbarBackgroundTexture.LoadTextureFromApplicationPackage( "assets/img_seekbar_background.png" );
	SeekbarProgressTexture.LoadTextureFromApplicationPackage( "assets/img_seekbar_progress_blue.png" );

	SeekPosition.LoadTextureFromApplicationPackage( "assets/img_seek_position.png" );

	SeekFF2x.LoadTextureFromApplicationPackage( "assets/img_seek_ff2x.png" );
	SeekFF4x.LoadTextureFromApplicationPackage( "assets/img_seek_ff4x.png" );
	SeekFF8x.LoadTextureFromApplicationPackage( "assets/img_seek_ff8x.png" );
	SeekFF16x.LoadTextureFromApplicationPackage( "assets/img_seek_ff16x.png" );
	SeekFF32x.LoadTextureFromApplicationPackage( "assets/img_seek_ff32x.png" );

	SeekRW2x.LoadTextureFromApplicationPackage( "assets/img_seek_rw2x.png" );
	SeekRW4x.LoadTextureFromApplicationPackage( "assets/img_seek_rw4x.png" );
	SeekRW8x.LoadTextureFromApplicationPackage( "assets/img_seek_rw8x.png" );
	SeekRW16x.LoadTextureFromApplicationPackage( "assets/img_seek_rw16x.png" );
	SeekRW32x.LoadTextureFromApplicationPackage( "assets/img_seek_rw32x.png" );

    // ==============================================================================
    //
    // reorient message
    //
	MoveScreenMenu = new UIMenu( Cinema );
	MoveScreenMenu->Create( "MoviePlayerMenu" );
	MoveScreenMenu->SetFlags( VRMenuFlags_t( VRMENU_FLAG_TRACK_GAZE ) | VRMenuFlags_t( VRMENU_FLAG_BACK_KEY_DOESNT_EXIT ) );

	MoveScreenLabel.AddToMenu( MoveScreenMenu, NULL );
    MoveScreenLabel.SetLocalPose( Quatf( Vector3f( 0.0f, 1.0f, 0.0f ), 0.0f ), Vector3f( 0.0f, 0.0f, -1.8f ) );
    MoveScreenLabel.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );
    MoveScreenLabel.SetFontScale( 0.5f );
    MoveScreenLabel.SetText( CinemaStrings::MoviePlayer_Reorient );
    MoveScreenLabel.SetTextOffset( Vector3f( 0.0f, -24 * VRMenuObject::DEFAULT_TEXEL_SCALE, 0.0f ) );  // offset to be below gaze cursor
    MoveScreenLabel.SetVisible( false );

    // ==============================================================================
    //
    // Playback controls
    //
    PlaybackControlsMenu = new UIMenu( Cinema );
    PlaybackControlsMenu->Create( "PlaybackControlsMenu" );
    PlaybackControlsMenu->SetFlags( VRMenuFlags_t( VRMENU_FLAG_BACK_KEY_DOESNT_EXIT ) );

    PlaybackControlsPosition.AddToMenu( PlaybackControlsMenu );
    PlaybackControlsScale.AddToMenu( PlaybackControlsMenu, &PlaybackControlsPosition );
    PlaybackControlsScale.SetLocalPosition( Vector3f( 0.0f, 0.0f, 0.05f ) );
    PlaybackControlsScale.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, BackgroundTintTexture, 1080, 1080 );

	// ==============================================================================
    //
    // movie title
    //
    MovieTitleLabel.AddToMenu( PlaybackControlsMenu, &PlaybackControlsScale );
    MovieTitleLabel.SetLocalPosition( PixelPos( 0, 266, 0 ) );
    MovieTitleLabel.SetFontScale( 1.4f );
    MovieTitleLabel.SetText( "" );
    MovieTitleLabel.SetTextOffset( Vector3f( 0.0f, 0.0f, 0.01f ) );
    MovieTitleLabel.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, BackgroundTintTexture, 320, 120 );

	// ==============================================================================
    //
    // seek icon
    //
    SeekIcon.AddToMenu( PlaybackControlsMenu, &PlaybackControlsScale );
    SeekIcon.SetLocalPosition( PixelPos( 0, 0, 0 ) );
    SeekIcon.SetLocalScale( Vector3f( 2.0f ) );
    SetSeekIcon( 0 );

    // ==============================================================================
    //
    // controls
    //
    ControlsBackground.AddToMenuFlags( PlaybackControlsMenu, &PlaybackControlsScale, VRMenuObjectFlags_t( VRMENUOBJECT_RENDER_HIERARCHY_ORDER ) );
    ControlsBackground.SetLocalPosition( PixelPos( 0, -288, 0 ) );
    ControlsBackground.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, BackgroundTintTexture, 1004, 168 );
    ControlsBackground.AddComponent( &GazeTimer );

    RewindButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
    RewindButton.SetLocalPosition( PixelPos( -448, 0, 1 ) );
    RewindButton.SetLocalScale( Vector3f( 2.0f ) );
    RewindButton.SetButtonImages( RWTexture, RWHoverTexture, RWPressedTexture );
    RewindButton.SetOnClick( RewindPressedCallback, this );

	FastForwardButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
	FastForwardButton.SetLocalPosition( PixelPos( -234, 0, 1 ) );
	FastForwardButton.SetLocalScale( Vector3f( 2.0f ) );
	FastForwardButton.SetButtonImages( FFTexture, FFHoverTexture, FFPressedTexture );
	FastForwardButton.SetOnClick( FastForwardPressedCallback, this );
	FastForwardButton.GetMenuObject()->SetLocalBoundsExpand( Vector3f::ZERO, PixelPos( -20, 0, 0 ) );

	// playbutton created after fast forward button to fix z issues
    PlayButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
    PlayButton.SetLocalPosition( PixelPos( -341, 0, 2 ) );
    PlayButton.SetLocalScale( Vector3f( 2.0f ) );
    PlayButton.SetButtonImages( PauseTexture, PauseHoverTexture, PausePressedTexture );
    PlayButton.SetOnClick( PlayPressedCallback, this );

	CarouselButton.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
	CarouselButton.SetLocalPosition( PixelPos( 418, 0, 1 ) );
	CarouselButton.SetLocalScale( Vector3f( 2.0f ) );
	CarouselButton.SetButtonImages( CarouselTexture, CarouselHoverTexture, CarouselPressedTexture );
	CarouselButton.SetOnClick( CarouselPressedCallback, this );
	CarouselButton.GetMenuObject()->SetLocalBoundsExpand( PixelPos( 20, 0, 0 ), Vector3f::ZERO );

	SeekbarBackground.AddToMenu( PlaybackControlsMenu, &ControlsBackground );
	SeekbarBackground.SetLocalPosition( PixelPos( 78, 0, 2 ) );
	SeekbarBackground.SetColor( Vector4f( 0.5333f, 0.5333f, 0.5333f, 1.0f ) );
	SeekbarBackground.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekbarBackgroundTexture, ScrubBarWidth + 6, 46 );
	SeekbarBackground.AddComponent( &ScrubBar );

	SeekbarProgress.AddToMenu( PlaybackControlsMenu, &SeekbarBackground );
	SeekbarProgress.SetLocalPosition( PixelPos( 0, 0, 1 ) );
	SeekbarProgress.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekbarProgressTexture, ScrubBarWidth, 40 );
	SeekbarProgress.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );

	CurrentTime.AddToMenu( PlaybackControlsMenu, &SeekbarBackground );
	CurrentTime.SetLocalPosition( PixelPos( -234, 52, 2 ) );
	CurrentTime.SetLocalScale( Vector3f( 1.0f ) );
	CurrentTime.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekPosition );
	CurrentTime.SetText( "2:33:33" );
	CurrentTime.SetTextOffset( PixelPos( 0, 6, 1 ) );
	CurrentTime.SetFontScale( 0.71f );
	CurrentTime.SetColor( Vector4f( 0 / 255.0f, 93 / 255.0f, 219 / 255.0f, 1.0f ) );
	CurrentTime.SetTextColor( Vector4f( 1.0f, 1.0f, 1.0f, 1.0f ) );
	CurrentTime.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );

	SeekTime.AddToMenu( PlaybackControlsMenu, &SeekbarBackground );
	SeekTime.SetLocalPosition( PixelPos( -34, 52, 4 ) );
	SeekTime.SetLocalScale( Vector3f( 1.0f ) );
	SeekTime.SetImage( 0, SURFACE_TEXTURE_DIFFUSE, SeekPosition );
	SeekTime.SetText( "2:33:33" );
	SeekTime.SetTextOffset( PixelPos( 0, 6, 1 ) );
	SeekTime.SetFontScale( 0.71f );
	SeekTime.SetColor( Vector4f( 47.0f / 255.0f, 70 / 255.0f, 89 / 255.0f, 1.0f ) );
	SeekTime.SetTextColor( Vector4f( 1.0f, 1.0f, 1.0f, 1.0f ) );
	SeekTime.GetMenuObject()->AddFlags( VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_ALL ) );

	ScrubBar.SetWidgets( &SeekbarBackground, &SeekbarProgress, &CurrentTime, &SeekTime, ScrubBarWidth );
	ScrubBar.SetOnClick( ScrubBarCallback, this );
}
void OvrApp::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
	// This is called by the VR thread, not the java UI thread.
	MaterialParms materialParms;
	materialParms.UseSrgbTextureFormats = false;
        
	const char * scenePath = "Oculus/tuscany.ovrscene";
	String	        SceneFile;
	Array<String>   SearchPaths;

	const OvrStoragePaths & paths = app->GetStoragePaths();

	paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
	paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);

	if ( GetFullPath( SearchPaths, scenePath, SceneFile ) )
	{
		Scene.LoadWorldModel( SceneFile , materialParms );
	}
	else
	{
		LOG( "OvrApp::OneTimeInit SearchPaths failed to find %s", scenePath );
	}

	GazeUserId = Cinema.app->GetGazeCursor().GenerateUserId();
	CreateMenu( Cinema.app, Cinema.app->GetVRMenuMgr(), Cinema.app->GetDefaultFont() );
}

void OvrApp::OneTimeShutdown()
{
	// Free GL resources
        
}

void OvrApp::Command( const char * msg )
{
}

Matrix4f OvrApp::DrawEyeView( const int eye, const float fovDegrees )
{
	const Matrix4f view = Scene.DrawEyeView( eye, fovDegrees );

	return view;
}

Matrix4f OvrApp::Frame(const VrFrame vrFrame)
{
	// Player movement
    Scene.Frame( app->GetVrViewParms(), vrFrame, app->GetSwapParms().ExternalVelocity );

    //alwarys display the menu
//    app->GetGuiSys().OpenMenu( app, app->GetGazeCursor(), OvrVideoMenu::MENU_NAME );
//    if ( vrFrame.Input.buttonReleased & BUTTON_TOUCH_DOUBLE )
//    {
//    	VideoMenu->RepositionMenu( app );
//    }



	app->DrawEyeViewsPostDistorted( Scene.CenterViewMatrix() );

	return Scene.CenterViewMatrix();
}
