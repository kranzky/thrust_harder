//==============================================================================

#include <hgeresource.h>
#include <hgesprite.h>
#include <hgefont.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>

#include <game.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <viewport.hpp>
#include <score.hpp>

//------------------------------------------------------------------------------
Game::Game( Engine * engine )
    :
    Context( engine ),
    m_ship( 0 ),
    m_urchins(),
    m_coins(),
    m_tiles( 0 ),
    m_num_tiles( 0 ),
    m_backs( 0 ),
    m_num_backs( 0 ),
    m_fores( 0 ),
    m_num_fores( 0 ),
    m_sky( 0 ),
    m_gui( 0 ),
    m_urchin_count( 0 )
{
}

//------------------------------------------------------------------------------
Game::~Game()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Game::init()
{
    HGE * hge( m_engine->getHGE() );
    b2World * b2d( m_engine->getB2D() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    ViewPort * viewport( m_engine->getViewPort() );

    viewport->offset().x = 0.0f;
    viewport->offset().y = 0.0f;
    viewport->bounds().x = 8.0f;
    viewport->bounds().y = 6.0f;

    m_ship = new Ship( m_engine, 0.01f );
    m_ship->init();
    
    m_sky = new hgeSprite( 0, 0, 0, 50, 50 );
    m_sky->SetHotSpot( 25, 25 );

    m_gui = new hgeSprite( 0, 0, 0, 1, 1 );
    
    Database db( "world.db3" );
    Query q( db );
    
    q.get_result( "SELECT texture_id, x, y, angle, scale FROM tiles" );
    
    m_num_tiles = 100;
    hge->System_Log( "Found %d tiles", m_num_tiles );
    m_tiles = new Scenery * [m_num_tiles];
    for ( int i = 0; i < m_num_tiles; ++i )
    {
        hge->System_Log( "Tile %d", i );
        m_tiles[i] = 0;
    }   
    
    int index( 0 );
    while ( q.fetch_row() )
    {
        int texture_id( q.getval() );
        float x( static_cast<float>( q.getnum() ) );
        float y( static_cast<float>( q.getnum() ) );
        float angle( static_cast<float>( q.getnum() ) );
        float scale( static_cast<float>( q.getnum() ) );
        hge->System_Log( "%f %f %f %f", x, y, angle, scale );
        m_tiles[index] = new Scenery( m_engine, scale );
        m_tiles[index]->init( rm->GetTexture( "tiles" ), b2d, x, y, angle,
                              texture_id );
        index += 1;           
    }   
    q.free_result();
    
    m_num_backs = 1;
    m_backs = new Nothing * [m_num_backs];
    for ( int i = 0; i < m_num_backs; ++i )
    {
        m_backs[i] = 0;
    }   
    
    m_backs[0] = new Nothing( 0.5f );
    m_backs[0]->init( rm->GetTexture( "tiles" ), 0.0f, 0.0f, 8 );
    
    m_num_fores = 1;
    m_fores = new Nothing * [m_num_fores];
    for ( int i = 0; i < m_num_fores; ++i )
    {
        m_fores[i] = 0;
    }   
    
    m_fores[0] = new Nothing( 0.3f );
    m_fores[0]->init( rm->GetTexture( "tiles" ), 0.0f, 5.0f, 9 );
}

//------------------------------------------------------------------------------
void
Game::fini()
{
    for ( int i = 0; i < m_num_tiles; ++i )
    {
        delete m_tiles[i];
    }
    delete [] m_tiles; 
    m_tiles = 0;
    for ( int i = 0; i < m_num_backs; ++i )
    {
        delete m_backs[i];
    }
    delete [] m_backs; 
    m_backs = 0;
    for ( int i = 0; i < m_num_fores; ++i )
    {
        delete m_fores[i];
    }
    delete [] m_fores;
    m_fores = 0;
    delete m_ship;
    m_ship = 0;
    std::vector< Urchin * >::iterator i;
    for ( i = m_urchins.begin(); i != m_urchins.end(); ++i )
    {
        delete * i;
    }
    m_urchins.clear();
    m_urchin_count = 0;
    std::vector< Coin * >::iterator j;
    for ( j = m_coins.begin(); j != m_coins.end(); ++j )
    {
        delete * j;
    }
    m_coins.clear();
    delete m_sky;
    m_sky = 0;
    delete m_gui;
    m_gui = 0;
}

//------------------------------------------------------------------------------
bool
Game::update( float dt )
{
    HGE * hge( m_engine->getHGE() );
    b2World * b2d( m_engine->getB2D() );
    ViewPort * viewport( m_engine->getViewPort() );

    if ( m_ship->getScoreData().getLives() <= 0 ||
         m_ship->getScoreData().getUrchins() == 99 )
    {
        int lives( m_ship->getScoreData().getLives() );
        int urchins( m_ship->getScoreData().getUrchins() );
        int coins( m_ship->getScoreData().getCoins() );
        int time( m_ship->getScoreData().getTime() );
        m_engine->switchContext( STATE_SCORE );
        Score * score( static_cast<Score *>( m_engine->getContext() ) );
        score->calculateScore( lives, urchins, coins, time );
        return false;
    }

    if ( ! m_engine->isDebug() )
    {
        m_ship->camera( viewport );
    }

    m_ship->update( dt );

    std::vector< Coin * >::iterator i;
    for ( i = m_coins.begin(); i != m_coins.end(); )
    {
        ( * i )->update( dt );
        if ( ( * i )->isDestroyed() )
        {

            delete * i;
            i = m_coins.erase( i );
        }
        else
        {
            ++i;
        }
    }

    std::vector< Urchin * >::iterator j;
    for ( j = m_urchins.begin(); j != m_urchins.end(); )
    {
        ( * j )->update( dt );
        if ( ( * j )->isDestroyed() )
        {
            for ( int k = 0; k < static_cast<int>((*j)->getScale() * 1000.0f);
                  ++k )
            {
                m_coins.push_back( new Coin( m_engine, 0.0007f ) );
                m_coins.back()->init();
                b2Body * coin( m_coins.back()->getBody() );
                b2Body * urchin( ( * j )->getBody() );
                coin->SetXForm( urchin->GetPosition(), 0.0f );
                float spin( hge->Random_Float( 20.0f, 80.0f ) );
                if ( hge->Random_Int( 0, 1 ) == 0 )
                {
                    spin *= -1.0f;
                }
                coin->SetAngularVelocity( spin );
                b2Vec2 velocity( hge->Random_Float( -0.5f, 0.5f ),
                                 hge->Random_Float( -0.3f, -0.8f ) );
                coin->SetLinearVelocity( velocity );
            }
            m_ship->killed();
            delete * j;
            j = m_urchins.erase( j );
        }
        else
        {
            ++j;
        }
    }

    if ( dt > 0.0f &&
         m_urchin_count < 99 && hge->Random_Float( 0.0f, 1.0f ) < 0.03f )
    {
        m_urchin_count += 1;
        m_urchins.push_back( new Urchin( m_engine,
                                         hge->Random_Float(0.001f, 0.012f) ) );
        m_urchins.back()->init();
        b2Body * body( m_urchins.back()->getBody() );
        float angle( hge->Random_Float( -M_PI, M_PI ) );
        b2Vec2 position( hge->Random_Float( -5.0f, 5.0f ), -10.0f );
        body->SetXForm( position, 0.0f );
    }

    return false;
}

//------------------------------------------------------------------------------
void
Game::render()
{
    HGE * hge( m_engine->getHGE() );
    ViewPort * viewport( m_engine->getViewPort() );

    hge->Gfx_SetTransform( viewport->offset().x,
                           viewport->offset().y,
                           400.0f,
                           300.0f,
                           viewport->angle(),
                           viewport->hscale(),
                           viewport->vscale() );
    m_sky->SetColor( 0xFF330011, 0 );
    m_sky->SetColor( 0xFF000077, 1 );
    m_sky->SetColor( 0xFF8833FF, 2 );
    m_sky->SetColor( 0xFFAA8866, 3 );
    m_sky->Render( 0, 0 );
    hge->Gfx_SetTransform( viewport->offset().x,
                           viewport->offset().y,
                           400.0f,
                           300.0f,
                           viewport->angle(),
                           viewport->hscale() * 0.9f,
                           viewport->vscale() * 0.9f );
    _renderBacks();

    hge->Gfx_SetTransform( viewport->offset().x,
                           viewport->offset().y,
                           400.0f,
                           300.0f,
                           viewport->angle(),
                           viewport->hscale(),
                           viewport->vscale() );
    _renderBodies();        
    m_engine->getParticleManager()->Render();
    hge->Gfx_SetTransform( viewport->offset().x,
                           viewport->offset().y,
                           400.0f,
                           300.0f,
                           viewport->angle(),
                           viewport->hscale() * 1.1f,
                           viewport->vscale() * 1.1f );
    _renderFores();         
    hge->Gfx_SetTransform();
    _renderGui();
}

//------------------------------------------------------------------------------
// private
//------------------------------------------------------------------------------
void
Game::_renderBacks()
{
    return;
    ViewPort * viewport( m_engine->getViewPort() );
    for ( int i = 0; i < m_num_backs; ++i )
    {
        if ( m_backs[i] != 0 )
        {
            m_backs[i]->render( * viewport );
        }
    }
}

//------------------------------------------------------------------------------
void
Game::_renderFores()
{
    return;
    ViewPort * viewport( m_engine->getViewPort() );
    for ( int i = 0; i < m_num_fores; ++i )
    {
        if ( m_fores[i] != 0 )
        {
            m_fores[i]->render( * viewport );
        }
    }
}

//------------------------------------------------------------------------------
void
Game::_renderBodies()
{
    ViewPort * viewport( m_engine->getViewPort() );
    b2World * b2d( m_engine->getB2D() );

    for ( b2Body * body( b2d->GetBodyList() ); body != NULL;
          body = body->GetNext() )
    {
        if ( body->IsDynamic() )
        {
            Entity * entity( static_cast<Entity *>( body->GetUserData() ) );
            if ( entity )
            {
                entity->render();
            }   
        }   
        else
        {
            Scenery * tile( static_cast<Scenery *>( body->GetUserData() ) );
            if ( tile )
            {
                tile->render();
            }   
        }   
    }   
}   

//------------------------------------------------------------------------------
void
Game::_renderGui()
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    float width =
        static_cast< float >( hge->System_GetState( HGE_SCREENWIDTH ) );
    float height =
        static_cast< float >( hge->System_GetState( HGE_SCREENHEIGHT ) );
    m_gui->SetColor( 0x88000000 );
    m_gui->RenderStretch( 0.0f, 0.0f, 90.0f, 40.0f );
    m_gui->RenderStretch( width - 60.0f, 0.0f, width, 40.0f );
    m_gui->RenderStretch( 0.0f, height - 40.0f, 130.0f, height );
    m_gui->RenderStretch( width - 155.0f, height - 40.0f, width, height );
    hgeFont * font( rm->GetFont( "menu" ) );
    font->printf( 80.0f, 10.0f, HGETEXT_RIGHT,
                  "x %d", m_ship->getScoreData().getLives() );
    font->printf( width - 10.0f, 10.0f, HGETEXT_RIGHT,
                  "%03d", m_ship->getScoreData().getTime() );
    font->printf( 120.0f, height - 30.0f, HGETEXT_RIGHT,
                  "x %04d", m_ship->getScoreData().getCoins() );
    font->printf( width - 10.0f, height - 30.0f, HGETEXT_RIGHT,
                  "x %02d/99", m_ship->getScoreData().getUrchins() );
    rm->GetSprite( "ship" )->Render( 20.0f, 20.0f );
    rm->GetSprite( "coin")->SetColor( 0xFFFFFFFF );
    rm->GetSprite( "coin" )->Render( 20.0f, height - 20.0f );
    rm->GetSprite( "urchin_green" )->Render( width - 130.0f, height - 20.0f );
}

//==============================================================================
