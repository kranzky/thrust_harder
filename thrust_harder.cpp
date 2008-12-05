//==============================================================================

#include <engine.hpp>

//------------------------------------------------------------------------------
int
WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    Engine * engine( Engine::instance() );
    engine->start();
    return 0;
}

//==============================================================================
