//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>
#include <hgesprite.h>

#include <instructions.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Instructions::Instructions( Engine * engine )
    :
    Context( engine ),
    m_dark( 0 )
{
}

//------------------------------------------------------------------------------
Instructions::~Instructions()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Instructions::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    m_dark = new hgeSprite( 0, 0, 0, 1, 1 );
}

//------------------------------------------------------------------------------
void
Instructions::fini()
{
    delete m_dark;
    m_dark = 0;
}

//------------------------------------------------------------------------------
bool
Instructions::update( float dt )
{
    HGE * hge( m_engine->getHGE() );

    if ( hge->Input_GetKey() != 0 && ! m_engine->isPaused() )
    {
        m_engine->switchContext( STATE_MENU );
    }

    return false;
}

//------------------------------------------------------------------------------
void
Instructions::render()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    m_dark->SetColor( 0xCC000309 );
    m_dark->RenderStretch( 50.0f, 50.0f, 750.0f, 550.0f );
    hgeFont * font( rm->GetFont( "menu" ) );
    font->printf( 400.0f, 80.0f, HGETEXT_CENTER,
                  "I N S T R U C T I O N S" );
    font->printf( 400.0f, 170.0f, HGETEXT_CENTER,
                  "you have 999 seconds and 3 lives to score big" );
    font->printf( 400.0f, 210.0f, HGETEXT_CENTER,
                  "pilot your ship with W to thrust, A & D to rotate" );
    font->printf( 400.0f, 250.0f, HGETEXT_CENTER,
                  "LMB shoots your weapon" );
    font->printf( 400.0f, 290.0f, HGETEXT_CENTER,
                  "land safely to deploy \"Cap'n Onion\", S to return" );
    font->printf( 400.0f, 330.0f, HGETEXT_CENTER,
                  "move him with A & D, with W to jump" );
    font->printf( 400.0f, 370.0f, HGETEXT_CENTER,
                  "LMB shoots his weapon towards the mouse pointer" );
    font->printf( 400.0f, 410.0f, HGETEXT_CENTER,
                  "score by destroying urchin and collecting coin" );
    font->printf( 400.0f, 500.0f, HGETEXT_CENTER,
                  "G O O D   L U C K" );
}
