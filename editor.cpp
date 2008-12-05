//==============================================================================

#include <hgeresource.h>
#include <hgefont.h>

#include <editor.hpp>
#include <engine.hpp>

//------------------------------------------------------------------------------
Editor::Editor( Engine * engine )
    :
    Context( engine )
{
}

//------------------------------------------------------------------------------
Editor::~Editor()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Editor::init()
{
}

//------------------------------------------------------------------------------
void
Editor::fini()
{
}

//------------------------------------------------------------------------------
bool
Editor::update( float dt )
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
Editor::render()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeFont * font( rm->GetFont( "menu" ) );
    font->printf( 400.0f, 300.0f, HGETEXT_CENTER,
                  "Coming in 2009" );
}
