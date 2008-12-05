//==============================================================================

#include <stdarg.h>

#include <hgesprite.h>
#include <hgeanim.h>
#include <hgeresource.h>

#include <engine.hpp>
#include <splash.hpp>
#include <menu.hpp>
#include <game.hpp>
#include <score.hpp>
#include <editor.hpp>
#include <instructions.hpp>
#include <debug.hpp>
#include <entity.hpp>

//------------------------------------------------------------------------------

Engine * Engine::s_instance( 0 );

//------------------------------------------------------------------------------
Engine *
Engine::instance()
{
    if ( s_instance == 0 )
    {
        s_instance = new Engine;
    }
    return s_instance;
}

//------------------------------------------------------------------------------
Engine::Engine()
    :
    m_resource_manager( 0 ),
    m_particle_manager( 0 ),
    m_hge( 0 ),
    m_b2d( 0 ),
    m_viewport(),
    m_colour( 0 ),
    m_debugDraw( 0 ),
    m_overlay( 0 ),
    m_contexts(),
    m_state( STATE_NONE ),
    m_paused( false ),
    m_running( false ),
    m_mouse( false ),
    m_mouse_sprite( 0 )
{
}

//------------------------------------------------------------------------------
Engine::~Engine()
{
    std::vector< Context * >::iterator i;
    for ( i = m_contexts.begin(); i != m_contexts.end(); ++i )
    {
        Context * context( * i );
        context->fini();
        delete context;
    }
    m_contexts.clear();

    delete m_particle_manager;
    m_particle_manager = 0;

    m_resource_manager->Purge();
    delete m_resource_manager;
    m_resource_manager = 0;

    if ( m_hge != 0 )
    {
        m_hge->System_Shutdown();
        m_hge->Release();
        m_hge = 0;
    }

    delete m_b2d;
    delete m_debugDraw;
    delete m_overlay;
}

//------------------------------------------------------------------------------
HGE *
Engine::getHGE()
{
    return m_hge;
}

//------------------------------------------------------------------------------
b2World *
Engine::getB2D()
{
    return m_b2d;
}

//------------------------------------------------------------------------------
ViewPort *
Engine::getViewPort()
{
    return & m_viewport;
}

//------------------------------------------------------------------------------
hgeResourceManager *
Engine::getResourceManager()
{
    return m_resource_manager;
}

//------------------------------------------------------------------------------
hgeParticleManager *
Engine::getParticleManager()
{
    return m_particle_manager;
}

//------------------------------------------------------------------------------
bool
Engine::isPaused()
{
    return m_paused;
}

//------------------------------------------------------------------------------
bool
Engine::isDebug()
{
    return m_debugDraw->GetFlags() != 0;
}

//------------------------------------------------------------------------------
void
Engine::error( const char * format, ... )
{
    char message[1024];

    va_list ap;
    va_start( ap, format );
    vsprintf_s( message, 1024, format, ap );
    va_end( ap );

    m_hge->System_Log( "Error: %s", message );
    MessageBox( NULL, message, "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
}

//------------------------------------------------------------------------------
void
Engine::start()
{
    m_contexts.push_back( new Splash( this ) );
    m_contexts.push_back( new Menu( this ) );
    m_contexts.push_back( new Game( this ) );
    m_contexts.push_back( new Score( this ) );
    m_contexts.push_back( new Editor( this ) );
    m_contexts.push_back( new Instructions( this ) );

    m_particle_manager = new hgeParticleManager();

    _initGraphics();
    _initPhysics();

    if ( m_hge->System_Initiate() )
    {
        _loadData();
        switchContext( STATE_SPLASH );
        m_hge->Random_Seed();
        m_hge->System_Start();
    }
    else
    {
        MessageBox( NULL, m_hge->System_GetErrorMessage(), "Error",
                    MB_OK | MB_ICONERROR | MB_APPLMODAL );
    }
}

//------------------------------------------------------------------------------
void
Engine::switchContext( EngineState state )
{
    m_running = false;
    m_colour = 0;

    if ( m_state != STATE_NONE )
    {
        m_contexts[m_state]->fini();
    }

    m_particle_manager->KillAll();
    hgeInputEvent event;
    while ( m_hge->Input_GetEvent( & event ) );
    setMouse( "cursor" );

    m_state = state;
    m_paused = false;

    int flags( b2DebugDraw::e_shapeBit |
               b2DebugDraw::e_aabbBit |
               b2DebugDraw::e_obbBit );
    m_debugDraw->ClearFlags( flags );

    if ( m_state != STATE_NONE )
    {
        m_contexts[m_state]->init();
    }

    m_running = true;
}

//------------------------------------------------------------------------------
Context *
Engine::getContext()
{
    return m_contexts[m_state];
}

//------------------------------------------------------------------------------
void
Engine::setColour( DWORD colour )
{
    m_colour = colour;
}

//------------------------------------------------------------------------------
void
Engine::showMouse()
{
    m_mouse = true;
}

//------------------------------------------------------------------------------
void
Engine::setMouse( const char * name )
{
    m_mouse_sprite = m_resource_manager->GetSprite( name );
}

//------------------------------------------------------------------------------
void
Engine::hideMouse()
{
    m_mouse = false;
}

//------------------------------------------------------------------------------
// physics:
//------------------------------------------------------------------------------
void
Engine::Violation( b2Body * body )
{
    m_hge->System_Log( "Body left world" );
    Entity * entity( static_cast<Entity *>( body->GetUserData() ) );
    // TODO: delete object here
}

//------------------------------------------------------------------------------
void
Engine::Add( b2ContactPoint * point )
{
    Entity * entity1 =
        static_cast< Entity * >( point->shape1->GetBody()->GetUserData() );
    Entity * entity2 =
        static_cast< Entity * >( point->shape2->GetBody()->GetUserData() );
    entity1->collide( entity2, point );
    entity2->collide( entity1, point );
}

//------------------------------------------------------------------------------
void
Engine::Persist( b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
Engine::Remove( b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
bool
Engine::s_update()
{
    return s_instance->_update();
}

//------------------------------------------------------------------------------
bool
Engine::s_render()
{
    return s_instance->_render();
}

//------------------------------------------------------------------------------
bool
Engine::_update()
{
    float dt( m_hge->Timer_GetDelta() );

    if ( m_hge->Input_KeyDown( HGEK_P ) && m_state != STATE_SCORE )
    {
        m_paused = ! m_paused;
    }

    if ( m_hge->Input_KeyDown( HGEK_O ) && m_state != STATE_SCORE  )
    {
        int flags( b2DebugDraw::e_shapeBit |
                   b2DebugDraw::e_aabbBit |
                   b2DebugDraw::e_obbBit );
        if ( m_debugDraw->GetFlags() != 0 )
        {
            m_debugDraw->ClearFlags( flags );
        }
        else
        {
            m_debugDraw->SetFlags( flags );
        }
    }

    if ( m_debugDraw->GetFlags() != 0 )
    {
        _debugControls( dt );
        m_hge->Gfx_BeginScene();
        m_hge->Gfx_Clear( 0 );
        m_contexts[m_state]->render();
        m_hge->Gfx_SetTransform( m_viewport.offset().x,
                                 m_viewport.offset().y,
                                 400.0f,
                                 300.0f,
                                 m_viewport.angle(),
                                 m_viewport.hscale(),
                                 m_viewport.vscale() );
    }      

    if ( m_paused )
    {
        dt = 0.0f;
    }

    m_b2d->Step( dt, 10 );
    bool retval( m_contexts[m_state]->update( dt ) );
    m_particle_manager->Update( dt );

    if ( m_debugDraw->GetFlags() != 0 )
    {
        m_hge->Gfx_SetTransform();
        _pauseOverlay();
        m_hge->Gfx_EndScene();
    }  

    return retval;
}

//------------------------------------------------------------------------------
void
Engine::_debugControls( float dt )
{
    if ( m_hge->Input_GetKeyState( HGEK_PGUP ) )
    {
        m_viewport.bounds() *= 1.01f;
    }

    if ( m_hge->Input_GetKeyState( HGEK_PGDN ) )
    {
        m_viewport.bounds() *= 0.99f;
    }

    if ( m_hge->Input_GetKeyState( HGEK_UP ) )
    {
        float dy = m_viewport.bounds().y;
        m_viewport.offset().y -= dy * dt;
    }

    if ( m_hge->Input_GetKeyState( HGEK_DOWN ) )
    {
        float dy = m_viewport.bounds().y * dt;
        m_viewport.offset().y += dy;
    }

    if ( m_hge->Input_GetKeyState( HGEK_LEFT ) )
    {
        float dx = m_viewport.bounds().x * dt;
        m_viewport.offset().x -= dx;
    }

    if ( m_hge->Input_GetKeyState( HGEK_RIGHT ) )
    {
        float dx = m_viewport.bounds().x * dt;
        m_viewport.offset().x += dx;
    }
}

//------------------------------------------------------------------------------
bool
Engine::_render()
{
    if ( ! m_running )
    {
        return false;
    }

    if ( m_debugDraw->GetFlags() == 0 )
    {
        m_hge->Gfx_BeginScene();
        m_hge->Gfx_Clear( m_colour );
        m_contexts[m_state]->render();
        m_hge->Gfx_SetTransform();
        if ( m_mouse && m_mouse_sprite != 0 )
        {
            float x( 0.0f ); 
            float y( 0.0f );
            m_hge->Input_GetMousePos( & x, & y );
            m_mouse_sprite->Render( x, y );
        }
        _pauseOverlay();
        m_hge->Gfx_EndScene();
    }

    return false;
}

//------------------------------------------------------------------------------
void
Engine::_pauseOverlay()
{
    if ( ! m_paused && m_debugDraw->GetFlags() == 0 )
    {
        return;
    }

    hgeFont * font( m_resource_manager->GetFont( "menu" ) );
    float width =
        static_cast< float >( m_hge->System_GetState( HGE_SCREENWIDTH ) );
    float height =
        static_cast< float >( m_hge->System_GetState( HGE_SCREENHEIGHT ) );
    if ( m_paused )
    {
        m_overlay->RenderStretch( 0.0f, 0.0f, width, height );
        font->Render( width / 2.0f, 0.0f, HGETEXT_CENTER,
                      "+++ P A U S E D +++" );
    }
    if ( m_debugDraw->GetFlags() != 0 )
    {
        font->Render( width / 2.0f, height - font->GetHeight(), HGETEXT_CENTER,
                      "+++ D E B U G +++" );
    }
}

//------------------------------------------------------------------------------
void
Engine::_initGraphics()
{
    m_hge = hgeCreate( HGE_VERSION );
    m_hge->System_SetState( HGE_LOGFILE, "kranzky.log" );
    m_hge->System_SetState( HGE_FRAMEFUNC, s_update );
    m_hge->System_SetState( HGE_RENDERFUNC, s_render );
    m_hge->System_SetState( HGE_TITLE, "+++ T H R U S T | H A R D E R +++" );
    m_hge->System_SetState( HGE_WINDOWED, true );
    m_hge->System_SetState( HGE_SCREENWIDTH, 800 );
    m_hge->System_SetState( HGE_SCREENHEIGHT, 600 );
    m_hge->System_SetState( HGE_SCREENBPP, 32 );
    m_hge->System_SetState( HGE_USESOUND, true );
    m_hge->System_SetState( HGE_SHOWSPLASH, false );
    m_hge->System_SetState( HGE_TEXTUREFILTER, false );
    m_hge->System_SetState( HGE_FPS, HGEFPS_UNLIMITED );

    m_overlay = new hgeSprite( 0, 0, 0, 1, 1 );
    m_overlay->SetColor( 0xBB000000 );
}

//------------------------------------------------------------------------------
void
Engine::_initPhysics()
{
    b2AABB worldAABB;
    worldAABB.lowerBound.Set( -500.0f, -500.0f );
    worldAABB.upperBound.Set( 500.0f, 500.0f );
    b2Vec2 gravity( 0.0f, 1.00f );
    m_b2d = new b2World( worldAABB, gravity, true );
    m_debugDraw = new DebugDraw( m_hge, & m_viewport );
    m_b2d->SetDebugDraw( m_debugDraw );
    m_b2d->SetListener( static_cast< b2ContactListener *>( this ) );
    m_b2d->SetListener( static_cast< b2BoundaryListener *>( this ) );
    m_viewport.screen().x = 800.0f;
    m_viewport.screen().y = 600.0f;
    m_viewport.offset().x = 0.0f;
    m_viewport.offset().y = 0.0f;
    m_viewport.bounds().x = 8.0f;
    m_viewport.bounds().y = 6.0f;
}

//------------------------------------------------------------------------------
void
Engine::_loadData()
{
    if ( ! m_hge->Resource_AttachPack( "resources.dat" ) )
    {
        error( "Cannot load '%s'", "resources.dat" );
    }

    m_resource_manager = new hgeResourceManager( "data.res" );
    m_resource_manager->Precache();

    m_hge->Resource_RemovePack( "resources.dat" );
}

//==============================================================================
