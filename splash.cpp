//==============================================================================

#include <hgeresource.h>
#include <hgedistort.h>

#include <splash.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Splash::Splash( Engine * engine )
    :
    Context( engine ),
    m_channel( 0 ),
    m_timer( 0.0f ),
    m_mesh( 0 ),
    m_delta_time( 0.0f )
{
}

//------------------------------------------------------------------------------
Splash::~Splash()
{
    delete m_mesh;
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Splash::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    m_timer = 0.0f;
    m_mesh = new hgeDistortionMesh( 16, 16 );
    m_mesh->SetBlendMode( BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_ZWRITE );
    m_mesh->Clear( 0xFF000000 );
    m_mesh->SetTexture( rm->GetTexture( "game" ) );
    m_mesh->SetTextureRect( 0.0f, 0.0f, 640.0f, 480.0f );
    HMUSIC music = m_engine->getResourceManager()->GetMusic( "theme" );
    m_channel = m_engine->getHGE()->Music_Play( music, true, 100, 0, 0 );
    m_engine->setColour( 0xFFFFFFFF );
}

//------------------------------------------------------------------------------
void
Splash::fini()
{
    m_engine->getHGE()->Channel_Stop( m_channel );
    delete m_mesh;
    m_mesh = 0;
    m_channel = 0;
}

//------------------------------------------------------------------------------
bool
Splash::update( float dt )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );

    m_delta_time = dt;

    bool paused( m_engine->isPaused() );
    if ( paused && hge->Channel_IsPlaying( m_channel ) )
    {
        hge->Channel_Pause( m_channel );
    }
    else if ( ! paused && ! hge->Channel_IsPlaying( m_channel ) )
    {
        hge->Channel_Resume( m_channel );
    }

    if ( m_timer > 30.5f )
    {
        _distortMeshTwo();
    }
    else if ( m_timer > 20.5f )
    {
        _distortMeshOne();
    }

    m_timer = hge->Channel_GetPos( m_channel );

    if ( m_timer >= 15.0f && hge->Input_GetKey() != 0 &&
         ! m_engine->isPaused() )
    {
        m_engine->switchContext( STATE_MENU );
    }

    return false;
}

//------------------------------------------------------------------------------
void
Splash::render()
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );

    int width( hge->System_GetState( HGE_SCREENWIDTH ) );
    int height( hge->System_GetState( HGE_SCREENHEIGHT ) );

    if ( m_timer > 0.0f && m_timer < 5.0f )
    {
        m_engine->setColour( 0xFFFFFFFF );
        _fade( 0.0f, 3.0f, 4.5f, 5.0f, "publisher" );
    }
    else if ( m_timer > 5.0f && m_timer < 10.2f )
    {
        m_engine->setColour( 0xFFFFFFFF );
        _fade( 5.0f, 8.0f, 9.7f, 10.2f, "developer" );
    }
    else if ( m_timer > 10.2f )
    {
        m_engine->setColour( 0x00000000 );
        m_mesh->Render( 0.5f * static_cast<float>( width - 640 ),
                        0.5f * static_cast<float>( height - 480 ) );
    }
    if ( m_timer > 15.0f && static_cast<int>( m_timer ) % 3 != 0 )
    {
        hgeFont * font( rm->GetFont( "menu" ) );
        font->printf( 790.0f, 570.0f, HGETEXT_RIGHT, "press whatever..." );
    }
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
bool
Splash::_onTime( float time )
{
    return time >= m_timer && time <= m_timer + m_delta_time;
}

//------------------------------------------------------------------------------
void
Splash::_distortMeshOne()
{
    for ( int i = 1; i < 15; ++i )
    {
        for ( int j = 1; j < 15; ++j )
        {
            m_mesh->SetDisplacement( j, i, cosf( m_timer * 5 + j / 2 ) * 15, 0,
                                     HGEDISP_NODE );
                                        
        }
    }
}

//------------------------------------------------------------------------------
void
Splash::_distortMeshTwo()
{
    for ( int i = 1; i < 15; ++i )
    {
        for ( int j = 1; j < 15; ++j )
        {
            m_mesh->SetDisplacement( j, i, cosf( m_timer * 10 + (i+j) /2 ) * 5,
                                     sinf( m_timer * 10 + (i+j) / 2) * 5,
                                     HGEDISP_NODE );
        } 
    }
}

//------------------------------------------------------------------------------
void
Splash::_fade( float start_in, float start_out, float end_in, float end_out,
               const char * name )
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeColorRGB color( 0xFFFFFFFF );

    int width( m_engine->getHGE()->System_GetState( HGE_SCREENWIDTH ) );
    int height( m_engine->getHGE()->System_GetState( HGE_SCREENHEIGHT ) );

    if ( m_timer > start_in && m_timer < start_out )
    {
        color.a = ( m_timer - start_in ) / ( start_out - start_in );
    }
    else if ( m_timer > end_in && m_timer < end_out )
    {
        color.a = 1.0f - ( m_timer - end_in ) / ( end_out - end_in );
    }

    hgeSprite * sprite( rm->GetSprite( name ) );
    sprite->SetColor( color.GetHWColor() );
    sprite->RenderEx( 0.5f * static_cast<float>( width ),
                      0.5f * static_cast<float>( height ), 0.0f, 0.5f );
}

//==============================================================================
