//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>
#include <hgesprite.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>

#include <score.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Score::Score( Engine * engine )
    :
    Context( engine ),
    m_dark( 0 ),
    m_calculate( false ),
    m_lives( 0 ),
    m_urchins( 0 ),
    m_coins( 0 ),
    m_time( 0 ),
    m_timer( 0.0f ),
    m_buffer( 0 ),
    m_high_score()
{
}

//------------------------------------------------------------------------------
Score::~Score()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Score::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    m_dark = new hgeSprite( 0, 0, 0, 1, 1 );
    m_calculate = false;
    HMUSIC music = m_engine->getResourceManager()->GetMusic( "score" );
    m_engine->getHGE()->Music_Play( music, true, 100, 0, 0 );
    _updateScore();
}

//------------------------------------------------------------------------------
void
Score::fini()
{
    m_engine->getHGE()->Channel_StopAll();
    delete m_dark;
    m_dark = 0;
}

//------------------------------------------------------------------------------
bool
Score::update( float dt )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleManager * pm( m_engine->getParticleManager() );
    
    if ( hge->Input_GetKey() != 0 && ! m_engine->isPaused() && ! m_calculate )
    {
        m_engine->switchContext( STATE_MENU );
    }

    if ( m_engine->isPaused() )
    {
        return false;
    }

    if ( m_calculate )
    {
        m_timer += dt;
        if ( static_cast< int >( m_timer * 1000.0f ) % 2 == 0 )
        {
            if ( m_buffer > 0 )
            {
                m_buffer -= 1;
                m_coins += 1;
                float x( hge->Random_Float( 550.0f, 560.0f ) );
                float y( hge->Random_Float( 268.0f, 278.0f ) );
                hgeParticleSystem * particle =
                    pm->SpawnPS(& rm->GetParticleSystem("collect")->info, x, y);
                if ( particle != 0 )
                {
                    particle->SetScale( 1.0f );
                }
                hge->Effect_Play( rm->GetEffect( "bounce" ) );
            }
            else if ( m_lives > 0 )
            {
                m_lives -= 1;
                m_buffer += 100;
                hge->Effect_Play( rm->GetEffect( "collect" ) );
            }
            else if ( m_urchins > 0 )
            {
                m_urchins -= 1;
                m_buffer += 10;
                hge->Effect_Play( rm->GetEffect( "collect" ) );
            }
            else if ( m_time > 0 )
            {
                m_time -= 7;
                if ( m_time < 0 )
                {
                    m_time = 0;
                }
                m_buffer += 1;
                hge->Effect_Play( rm->GetEffect( "collect" ) );
            }
        }
        if ( m_buffer == 0 && m_lives == 0 && m_urchins == 0 && m_time == 0 )
        {
            int character( hge->Input_GetChar() );
            if ( character != 0 )
            {
                if ( ( character == ' ' ||
                       character == '.' ||
                       character == '!' ||
                       character == '?' ||
                       ( character >= '0' && character <= '9' ) ||
                       ( character >= 'a' && character <= 'z' ) ||
                       ( character >= 'A' && character <= 'Z' ) ) && 
                     m_name.size() <= 15 )
                {
                    m_name.push_back( character );
                }
            }
            if ( hge->Input_KeyDown( HGEK_BACKSPACE ) ||
                 hge->Input_KeyDown( HGEK_DELETE ) )
            {
                if ( m_name.size() > 0 )
                {
                    m_name.erase( m_name.end() - 1 );
                }
            }
            if ( hge->Input_KeyDown( HGEK_ENTER ) )
            {
                if ( m_name.size() == 0 )
                {
                    m_name = "Anonymous";
                }
                Database db( "world.db3" );
                Query q( db );
                char query[1024];
                sprintf_s( query, 1024, "INSERT INTO score(name, coins) "
                           "VALUES('%s',%d)", m_name.c_str(), m_coins );
                q.execute( query );
                _updateScore();
                m_calculate = false;
            }
        }
    }
    else if ( dt > 0.0f && hge->Random_Float( 0.0f, 1.0f ) < 0.07f )
    {
        float x( hge->Random_Float( 0.0f, 800.0f ) );
        float y( hge->Random_Float( 0.0f, 600.0f ) );
        hgeParticleSystem * particle =
            pm->SpawnPS( & rm->GetParticleSystem( "explode" )->info, x, y );
        particle->SetScale( hge->Random_Float( 0.5f, 2.0f ) );
    }

    return false;
}

//------------------------------------------------------------------------------
void
Score::render()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );

    if ( ! m_calculate )
    {
        m_engine->getParticleManager()->Render();
    }

    m_dark->SetColor( 0xCC000309 );
    m_dark->RenderStretch( 100.0f, 50.0f, 700.0f, 550.0f );
    hgeFont * font( rm->GetFont( "menu" ) );

    if ( m_calculate )
    {
        font->printf( 400.0f, 80.0f, HGETEXT_CENTER, "G A M E   O V E R" );
        font->printf( 200.0f, 200.0f, HGETEXT_LEFT, "x %d", m_lives );
        font->printf( 200.0f, 260.0f, HGETEXT_LEFT, "x %02d/99", m_urchins );
        font->printf( 200.0f, 320.0f, HGETEXT_LEFT, "%03d", m_time );
        font->printf( 580.0f, 260.0f, HGETEXT_LEFT, "x %04d", m_coins );
        m_engine->getParticleManager()->Render();
        rm->GetSprite( "ship" )->Render( 175.0f, 213.0f );
        rm->GetSprite( "coin")->SetColor( 0xFFFFFFFF );
        rm->GetSprite( "coin" )->Render( 555.0f, 273.0f );
        rm->GetSprite( "urchin_green" )->Render( 175.0f, 273.0f );
        if ( m_buffer == 0 && m_lives == 0 && m_urchins == 0 && m_time == 0 )
        {
            font->printf( 400.0f, 400.0f, HGETEXT_CENTER,
                          "%s", m_name.c_str() );
            font->printf( 400.0f, 500.0f, HGETEXT_CENTER,
                          "(well done, you)" );
            if ( static_cast<int>( m_timer * 2.0f ) % 2 != 0 )
            {
                float width = font->GetStringWidth( m_name.c_str() );
                m_dark->SetColor( 0xFFFFFFFF );
                m_dark->RenderStretch( 400.0f + width * 0.5f, 425.0f,
                                       400.0f + width * 0.5f + 16.0f, 427.0f );
            }
        }
    }
    else
    {
        font->printf( 400.0f, 80.0f, HGETEXT_CENTER,
                      "H I G H   S C O R E   T A B L E" );
        int i = 0;
        std::vector< std::pair< std::string, int > >::iterator j;
        for ( j = m_high_score.begin(); j != m_high_score.end(); ++j )
        {
            i += 1;
            font->printf( 200.0f, 120.0f + i * 30.0f, HGETEXT_LEFT,
                          "(%d)", i );
            font->printf( 400.0f, 120.0f + i * 30.0f, HGETEXT_CENTER,
                          "%s", j->first.c_str() );
            font->printf( 600.0f, 120.0f + i * 30.0f, HGETEXT_RIGHT,
                          "%04d", j->second );
        }
        font->printf( 400.0f, 500.0f, HGETEXT_CENTER,
                      "(but you're all winners, really)" );

    }
}

//------------------------------------------------------------------------------
void
Score::calculateScore( int lives, int urchins, int coins, int time )
{
    m_timer = 0.0f;
    m_buffer = 0;
    m_calculate = true;
    m_lives = lives;
    m_urchins = urchins;
    m_coins = coins;
    m_time = time;
    if ( lives == 0 || urchins == 0 )
    {
        m_time = 0;
    }
    m_name.clear();
    if ( m_lives == 0 && m_time == 0 && m_coins == 0 && m_urchins == 0 )
    {
        m_calculate = false;
    }
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Score::_updateScore()
{
    m_high_score.clear();

    Database db( "world.db3" );
    Query q( db );

    q.get_result("SELECT coins, name FROM score ORDER BY coins DESC LIMIT 10");

    while ( q.fetch_row() )
    {
        std::pair< std::string, int > pair( q.getstr(), q.getval() );
        m_high_score.push_back( pair );
    }

    q.free_result();
}
