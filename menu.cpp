//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>
#include <hgegui.h>
#include <hgesprite.h>

#include <menu.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Menu::Menu( Engine * engine )
    :
    Context( engine ),
    m_cursor( 0 ),
    m_font( 0 ),
    m_gui( 0 ),
    m_dark( 0 )
{
}

//------------------------------------------------------------------------------
Menu::~Menu()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Menu::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );

    m_font = rm->GetFont( "menu" );
    m_cursor = rm->GetSprite( "cursor" );
    m_gui = new hgeGUI();
    m_gui->AddCtrl( new MenuItem( CTRL_TITLES, 400, 180, "Play Intro", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_INSTRUCTIONS, 400, 240, "Instructions",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_START, 400, 270, "Start Game",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_SCORE, 400, 300, "High Score Table",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_EDITOR, 400, 330, "Level Editor",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_EXIT, 400, 390, "Exit", m_font ) );
    m_gui->SetNavMode( HGEGUI_UPDOWN | HGEGUI_CYCLED );
    m_gui->SetCursor( m_cursor );
    m_gui->SetFocus( 1 );
    m_gui->Enter();
    HMUSIC music = m_engine->getResourceManager()->GetMusic( "menu" );
    int order( 0 );
    int row( 0 );
    m_engine->getHGE()->Music_GetPos( music, & order, & row );
    if ( order == 0 && row == 0 )
    {
        m_engine->getHGE()->Music_Play( music, true, 100, 0, 0 );
    }
    m_dark = new hgeSprite( 0, 0, 0, 1, 1 );
}

//------------------------------------------------------------------------------
void
Menu::fini()
{
    m_gui->DelCtrl( CTRL_TITLES );
    m_gui->DelCtrl( CTRL_INSTRUCTIONS );
    m_gui->DelCtrl( CTRL_START );
    m_gui->DelCtrl( CTRL_SCORE );
    m_gui->DelCtrl( CTRL_EDITOR );
    m_gui->DelCtrl( CTRL_EXIT );
    delete m_gui;
    m_gui = 0;
    m_font = 0;
    m_cursor = 0;
    delete m_dark;
    m_dark = 0;
}

//------------------------------------------------------------------------------
bool
Menu::update( float dt )
{
    HGE * hge( m_engine->getHGE() );

    switch ( static_cast< Control >( m_gui->Update( dt ) ) )
    {
        case CTRL_TITLES:
        {
            _stopMusic();
            m_engine->switchContext( STATE_SPLASH );
            break;
        }
        case CTRL_INSTRUCTIONS:
        {
            m_engine->switchContext( STATE_INSTRUCTIONS );
            break;
        }
        case CTRL_START:
        {
            _stopMusic();
            m_engine->switchContext( STATE_GAME );
            break;
        }
        case CTRL_SCORE:
        {
            _stopMusic();
            m_engine->switchContext( STATE_SCORE );
            break;
        }
        case CTRL_EDITOR:
        {
            m_engine->switchContext( STATE_EDITOR );
            break;
        }
        case CTRL_EXIT:
        {
            _stopMusic();
            return true;
        }
    }

    if ( dt > 0.0f && hge->Random_Float( 0.0f, 1.0f ) < 0.03f )
    {
        hgeResourceManager * rm( m_engine->getResourceManager() );
        hgeParticleManager * pm( m_engine->getParticleManager() );
        float x( hge->Random_Float( 0.0f, 800.0f ) );
        float y( hge->Random_Float( 0.0f, 600.0f ) );
        hgeParticleSystem * particle =
            pm->SpawnPS( & rm->GetParticleSystem( "spawn" )->info, x, y );
        particle->SetScale( hge->Random_Float( 0.5f, 2.0f ) );
    }
    
    return false;
}

//------------------------------------------------------------------------------
void
Menu::render()
{
    m_engine->getParticleManager()->Render();
    m_dark->SetColor( 0xCC000309 );
    m_dark->RenderStretch( 270.0f, 150.0f, 530.0f, 450.0f );
    m_gui->Render();
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Menu::_stopMusic()
{
    m_engine->getHGE()->Channel_StopAll();
    HMUSIC music = m_engine->getResourceManager()->GetMusic( "menu" );
    m_engine->getHGE()->Music_SetPos( music, 0, 0 );
}

//==============================================================================
MenuItem::MenuItem( Control control, float x, float y, const char * title,
                    hgeFont * font )
    :
    hgeGUIObject(),
    m_title( title ),
    m_font( font )

{
    id = static_cast<int>( control );

    bStatic=false;
    bVisible=true;
    bEnabled=true;

    float width( m_font->GetStringWidth( title ) );
    rect.Set( x - width / 2, y, x + width / 2, y + m_font->GetHeight() );
}
 
//------------------------------------------------------------------------------
void
MenuItem::Render()
{
    m_font->Render( rect.x1, rect.y1, HGETEXT_LEFT, m_title );
}

//------------------------------------------------------------------------------
void
MenuItem::Update( float dt )
{
}

//------------------------------------------------------------------------------
void
MenuItem::Enter()
{
}

//------------------------------------------------------------------------------
void
MenuItem::Leave()
{
}

//------------------------------------------------------------------------------
bool
MenuItem::IsDone()
{
    return true;
}

//------------------------------------------------------------------------------
void
MenuItem::Focus( bool focused )
{
}

//------------------------------------------------------------------------------
void
MenuItem::MouseOver( bool over )
{
    if ( over )
    {
        gui->SetFocus( id );
    }
}

//------------------------------------------------------------------------------
bool
MenuItem::MouseLButton( bool down )
{
    if ( down )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
bool
MenuItem::KeyClick( int key, int chr )
{
    if ( key == HGEK_ENTER || key == HGEK_SPACE )
    {
        MouseLButton( true );
        return MouseLButton( false );
    }

    return false;
}

//==============================================================================
